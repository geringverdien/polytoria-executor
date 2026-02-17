#include <hwidspoofer/multiclient.h>

#include <windows.h>
#include <winternl.h>

// -------------------------------------------------------------------
// Type definitions for internal system structures
// -------------------------------------------------------------------
typedef struct _SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX
{
    PVOID Object;
    ULONG_PTR UniqueProcessId;
    ULONG_PTR HandleValue;
    ULONG GrantedAccess;
    USHORT CreatorBackTraceIndex;
    USHORT ObjectTypeIndex;
    ULONG HandleAttributes;
    ULONG Reserved;
} SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX, *PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX;

typedef struct _SYSTEM_HANDLE_INFORMATION_EX
{
    ULONG_PTR NumberOfHandles;
    ULONG_PTR Reserved;
    SYSTEM_HANDLE_TABLE_ENTRY_INFO_EX Handles[1];
} SYSTEM_HANDLE_INFORMATION_EX, *PSYSTEM_HANDLE_INFORMATION_EX;

// Function pointers for NT APIs
typedef NTSTATUS (NTAPI *pNtQuerySystemInformation)(
    SYSTEM_INFORMATION_CLASS SystemInformationClass,
    PVOID SystemInformation,
    ULONG SystemInformationLength,
    PULONG ReturnLength);

typedef NTSTATUS (NTAPI *pNtQueryObject)(
    HANDLE Handle,
    OBJECT_INFORMATION_CLASS ObjectInformationClass,
    PVOID ObjectInformation,
    ULONG ObjectInformationLength,
    PULONG ReturnLength);

typedef struct _OBJECT_TYPE_INFORMATION
{
    UNICODE_STRING TypeName;
    ULONG TotalNumberOfObjects;
    ULONG TotalNumberOfHandles;
    ULONG TotalPagedPoolUsage;
    ULONG TotalNonPagedPoolUsage;
    ULONG TotalNamePoolUsage;
    ULONG TotalHandleTableUsage;
    ULONG HighWaterNumberOfObjects;
    ULONG HighWaterNumberOfHandles;
    ULONG HighWaterPagedPoolUsage;
    ULONG HighWaterNonPagedPoolUsage;
    ULONG HighWaterNamePoolUsage;
    ULONG HighWaterHandleTableUsage;
    ULONG InvalidAttributes;
    GENERIC_MAPPING GenericMapping;
    ULONG ValidAccessMask;
    BOOLEAN SecurityRequired;
    BOOLEAN MaintainHandleCount;
    UCHAR TypeIndex; // since WINBLUE
    CHAR ReservedByte;
    ULONG PoolType;
    ULONG DefaultPagedPoolCharge;
    ULONG DefaultNonPagedPoolCharge;
} OBJECT_TYPE_INFORMATION, *POBJECT_TYPE_INFORMATION;

// -------------------------------------------------------------------
// Dynamically retrieve the object type index for "Mutant" on the
// current Windows version.
// -------------------------------------------------------------------
USHORT GetMutantTypeIndex()
{
    // NtQueryObject with ObjectTypeInformation returns a structure
    // that contains the type name and its index. We create a dummy
    // mutant, query its type, and extract the index.
    HANDLE hDummy = CreateMutexW(NULL, FALSE, L"DummyMutexForTypeIndex");
    if (!hDummy)
        return 0;

    // We need NtQueryObject from ntdll.dll
    pNtQueryObject NtQueryObject = (pNtQueryObject)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "NtQueryObject");
    if (!NtQueryObject)
    {
        CloseHandle(hDummy);
        return 0;
    }

    // First call to get required buffer size
    ULONG size = 0;
    NTSTATUS status = NtQueryObject(hDummy, ObjectTypeInformation, NULL, 0, &size);
    if (status != 0xC0000004 && status != 0) // STATUS_INFO_LENGTH_MISMATCH is expected
    {
        CloseHandle(hDummy);
        return 0;
    }

    // Allocate buffer
    PBYTE buffer = (PBYTE)VirtualAlloc(NULL, size, MEM_COMMIT, PAGE_READWRITE);
    if (!buffer)
    {
        CloseHandle(hDummy);
        return 0;
    }

    status = NtQueryObject(hDummy, ObjectTypeInformation, buffer, size, NULL);
    if (status < 0)
    {
        VirtualFree(buffer, 0, MEM_RELEASE);
        CloseHandle(hDummy);
        return 0;
    }

    // The structure returned is OBJECT_TYPE_INFORMATION, which contains
    // a UNICODE_STRING for the type name and a ULONG TypeIndex.
    // We just need the TypeIndex.
    POBJECT_TYPE_INFORMATION pTypeInfo = (POBJECT_TYPE_INFORMATION)buffer;
    USHORT typeIndex = (USHORT)pTypeInfo->TypeIndex;   // Note: TypeIndex is ULONG, but fits in USHORT

    VirtualFree(buffer, 0, MEM_RELEASE);
    CloseHandle(hDummy);
    return typeIndex;
}

// -------------------------------------------------------------------
// Main function: close all mutexes owned by the current process.
// Must be called early from DllMain (or a dedicated thread) to avoid
// interfering with the application after it has started.
// -------------------------------------------------------------------
void multiclient::CloseAllMutexesInCurrentProcess()
{
    // 1. Dynamically resolve the Mutant type index
    static USHORT mutantTypeIndex = 0;
    if (mutantTypeIndex == 0)
    {
        mutantTypeIndex = GetMutantTypeIndex();
        if (mutantTypeIndex == 0)
        {
            // Fallback: common values on modern Windows (10/11)
            mutantTypeIndex = 19;   // Often 19, but not guaranteed
        }
    }

    // 2. Get NtQuerySystemInformation
    pNtQuerySystemInformation NtQuerySystemInformation = (pNtQuerySystemInformation)GetProcAddress(
        GetModuleHandleW(L"ntdll.dll"), "NtQuerySystemInformation");
    if (!NtQuerySystemInformation)
        return;

    DWORD currentPid = GetCurrentProcessId();

    // 3. Loop with a reasonable buffer size – grow if needed
    ULONG bufferSize = 0x10000; // 64KB initial
    PBYTE buffer = NULL;
    NTSTATUS status;

    do
    {
        if (buffer)
            VirtualFree(buffer, 0, MEM_RELEASE);

        buffer = (PBYTE)VirtualAlloc(NULL, bufferSize, MEM_COMMIT, PAGE_READWRITE);
        if (!buffer)
            return;

        status = NtQuerySystemInformation((SYSTEM_INFORMATION_CLASS)64, // SystemExtendedHandleInformation = 64
                                          buffer, bufferSize, NULL);
        if (status == 0xC0000004) // STATUS_INFO_LENGTH_MISMATCH
        {
            bufferSize *= 2;
            continue;
        }
        else if (status < 0)
        {
            VirtualFree(buffer, 0, MEM_RELEASE);
            return;
        }
        break;
    } while (TRUE);

    // 4. Parse the handle list
    PSYSTEM_HANDLE_INFORMATION_EX handleInfo = (PSYSTEM_HANDLE_INFORMATION_EX)buffer;
    for (ULONG_PTR i = 0; i < handleInfo->NumberOfHandles; i++)
    {
        PSYSTEM_HANDLE_TABLE_ENTRY_INFO_EX entry = &handleInfo->Handles[i];

        // Filter: only our process and only Mutant objects
        if (entry->UniqueProcessId != currentPid)
            continue;
        if (entry->ObjectTypeIndex != mutantTypeIndex)
            continue;

        // 5. Attempt to close this handle using DUPLICATE_CLOSE_SOURCE
        HANDLE hTargetProcess = OpenProcess(PROCESS_DUP_HANDLE, FALSE, currentPid);
        if (!hTargetProcess)
            continue;

        HANDLE hDup = NULL;
        BOOL ret = DuplicateHandle(
            hTargetProcess,                 // Process that owns the handle
            (HANDLE)entry->HandleValue,     // Handle value to close
            GetCurrentProcess(),             // Receiving process (our own)
            &hDup,                           // We'll receive a copy (must close it)
            0,                               // Ignored because of DUPLICATE_SAME_ACCESS
            FALSE,
            DUPLICATE_CLOSE_SOURCE);         // This flag closes the source handle

        if (ret && hDup)
        {
            // Success: the original handle in the target process is now closed.
            // We must close our local copy.
            CloseHandle(hDup);
        }

        CloseHandle(hTargetProcess);
    }

    VirtualFree(buffer, 0, MEM_RELEASE);
}