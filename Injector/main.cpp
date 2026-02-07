#include <Windows.h>
#include <iostream>
#include <fstream>
#include <TlHelp32.h>

struct MMAP_DATA {
    LPVOID LoadLibraryA;
    LPVOID GetProcAddress;
    BYTE* BaseAddress;
};

DWORD WINAPI __stdcall Shellcode(MMAP_DATA* InjectionData) {
    if (!InjectionData) {
        // this should never happen!!!
        return 0;
    }

    BYTE* PointerToBase = InjectionData->BaseAddress;
    auto* pOpt = &reinterpret_cast<IMAGE_NT_HEADERS*>(PointerToBase + reinterpret_cast<IMAGE_DOS_HEADER*>(PointerToBase)->e_lfanew)->OptionalHeader;

    auto _LoadLibraryA = reinterpret_cast<HMODULE(WINAPI*)(LPCSTR)>(InjectionData->LoadLibraryA);
    auto _GetProcAddress = reinterpret_cast<FARPROC(WINAPI*)(HMODULE, LPCSTR)>(InjectionData->GetProcAddress);

    //Fix Relocations
    BYTE* LocationDelta = PointerToBase - pOpt->ImageBase;
    if (LocationDelta) {
        if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].Size) {
            auto* pRelocDir = reinterpret_cast<IMAGE_BASE_RELOCATION*>(PointerToBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_BASERELOC].VirtualAddress);
            while (pRelocDir->VirtualAddress) {
                UINT AmountOfEntries = (pRelocDir->SizeOfBlock - sizeof(IMAGE_BASE_RELOCATION)) / sizeof(WORD);
                WORD* pRelativeInfo = reinterpret_cast<WORD*>(pRelocDir + 1);
                for (UINT i = 0; i != AmountOfEntries; ++i, ++pRelativeInfo) {
                    if ((*pRelativeInfo >> 12) == IMAGE_REL_BASED_DIR64) {
                        UINT_PTR* pPatch = reinterpret_cast<UINT_PTR*>(PointerToBase + pRelocDir->VirtualAddress + ((*pRelativeInfo) & 0xFFF));
                        *pPatch += reinterpret_cast<UINT_PTR>(LocationDelta);
                    }
                }
                pRelocDir = reinterpret_cast<IMAGE_BASE_RELOCATION*>(reinterpret_cast<BYTE*>(pRelocDir) + pRelocDir->SizeOfBlock);
            }
        }
    }

    //Resolve Imports
    if (pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].Size) {
        auto* pImportDescr = reinterpret_cast<IMAGE_IMPORT_DESCRIPTOR*>(PointerToBase + pOpt->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
        while (pImportDescr->Name) {
            // we could also mmap the Imports but i don't think thats needed for now!
            HMODULE hMod = _LoadLibraryA(reinterpret_cast<char*>(PointerToBase + pImportDescr->Name));
            auto* pThunk = reinterpret_cast<IMAGE_THUNK_DATA*>(PointerToBase + pImportDescr->FirstThunk);
            auto* pIAT = reinterpret_cast<IMAGE_THUNK_DATA*>(PointerToBase + pImportDescr->OriginalFirstThunk);
            if (!pIAT) pIAT = pThunk;

            for (; pIAT->u1.AddressOfData; ++pIAT, ++pThunk) {
                if (IMAGE_SNAP_BY_ORDINAL(pIAT->u1.Ordinal)) {
                    pThunk->u1.Function = reinterpret_cast<UINT_PTR>(_GetProcAddress(hMod, reinterpret_cast<char*>(pIAT->u1.Ordinal & 0xFFFF)));
                }
                else {
                    auto* pImportByName = reinterpret_cast<IMAGE_IMPORT_BY_NAME*>(PointerToBase + pIAT->u1.AddressOfData);
                    pThunk->u1.Function = reinterpret_cast<UINT_PTR>(_GetProcAddress(hMod, pImportByName->Name));
                }
            }
            pImportDescr++;
        }
    }
    auto DllMain = reinterpret_cast<BOOL(WINAPI*)(HINSTANCE, DWORD, LPVOID)>(PointerToBase + pOpt->AddressOfEntryPoint);
    DllMain(reinterpret_cast<HINSTANCE>(PointerToBase), DLL_PROCESS_ATTACH, 0);
    return 1;
}

bool ManualMap(HANDLE hProc, const char* dllPath) {
    HANDLE hFile = CreateFileA(dllPath, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return false;
    }

    DWORD FileSize = GetFileSize(hFile, NULL);
    if (FileSize == INVALID_FILE_SIZE) {
        CloseHandle(hFile);
        return false;
    }

    BYTE* PointerToSourceData = new BYTE[FileSize];
    DWORD bytesRead;
    if (!ReadFile(hFile, PointerToSourceData, FileSize, &bytesRead, NULL) || bytesRead != FileSize) {
        delete[] PointerToSourceData;
        CloseHandle(hFile);
        return false;
    }
    CloseHandle(hFile);

    auto* PointerToDosHeader = reinterpret_cast<IMAGE_DOS_HEADER*>(PointerToSourceData);
    auto* PointerToNtHeader = reinterpret_cast<IMAGE_NT_HEADERS*>(PointerToSourceData + PointerToDosHeader->e_lfanew);

    BYTE* TargetBase = (BYTE*)VirtualAllocEx(hProc, 0, PointerToNtHeader->OptionalHeader.SizeOfImage, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);

    WriteProcessMemory(hProc, TargetBase, PointerToSourceData, PointerToNtHeader->OptionalHeader.SizeOfHeaders, 0);
    auto* pSectionHeader = IMAGE_FIRST_SECTION(PointerToNtHeader);
    for (UINT i = 0; i != PointerToNtHeader->FileHeader.NumberOfSections; ++i, ++pSectionHeader) {
        WriteProcessMemory(hProc, TargetBase + pSectionHeader->VirtualAddress, PointerToSourceData + pSectionHeader->PointerToRawData, pSectionHeader->SizeOfRawData, 0);
    }

    MMAP_DATA MappingData = { 0 };
    MappingData.LoadLibraryA = LoadLibraryA;
    MappingData.GetProcAddress = GetProcAddress;
    MappingData.BaseAddress = TargetBase;

    LPVOID RemoteData = VirtualAllocEx(hProc, 0, sizeof(MappingData), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
    WriteProcessMemory(hProc, RemoteData, &MappingData, sizeof(MappingData), 0);

    // todo: hijack memory?
    LPVOID PointerToRemoteShellcode = VirtualAllocEx(hProc, 0, 0x1000, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
    WriteProcessMemory(hProc, PointerToRemoteShellcode, Shellcode, 0x1000, 0);

    // todo hijack a thread?
    HANDLE hThread = CreateRemoteThread(hProc, 0, 0, (LPTHREAD_START_ROUTINE)PointerToRemoteShellcode, RemoteData, 0, 0);
    WaitForSingleObject(hThread, INFINITE);

    delete[] PointerToSourceData;
    return true;
}


DWORD GetProcessIdByName(const wchar_t* ProcessName) {
    DWORD PID = 0;
    HANDLE Snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    PROCESSENTRY32W entry = { sizeof(entry) };
    if (Process32FirstW(Snapshot, &entry)) {
        do {
            if (!_wcsicmp(entry.szExeFile, ProcessName)) {
                PID = entry.th32ProcessID;
                break;
            }
        } while (Process32NextW(Snapshot, &entry));
    }
    CloseHandle(Snapshot);
    return PID;
}

int main() {
    DWORD PID = GetProcessIdByName(L"Polytoria Client.exe");

    if (PID) {
        HANDLE Handle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, PID);
        if (ManualMap(Handle, "poly.dll")) {
            std::cout << "Successfully injected into Polytoria!" << std::endl;
            CloseHandle(Handle);
            return 0;
        }
        else {
            std::cout << "Mapping failed gg" << std::endl;
            CloseHandle(Handle);
        }
    }
    else {
        std::cout << "unable to find 'Polytoria Client.exe'!" << std::endl;
        return 1;
    }
    return 1;
}