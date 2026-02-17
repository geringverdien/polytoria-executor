/**
 * @file spoofer.cpp
 * @author ElCapor (infosec@yu-tech.cloud)
 * @brief
 * @version 0.1
 * @date 2026-02-11
 *
 * CC-BY-SA 4.0 Copyright (c) 2026 @ElCapor
 *
 */
#include <hwidspoofer/hwidspoofer.h>
#include <cstdint>
#include <hooking/hookmanager.h>
#include <iostream>
#include <unity/unity.h>
#include <random>
#include <algorithm>
#include <core/core.h>
#include <filesystem>
#include <hwidspoofer/multiclient.h>

std::string scramble(const std::string &input)
{
    std::string scrambled = input;
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(scrambled.begin(), scrambled.end(), g);
    return scrambled;
}

// UnityPlayer.dll + 0x804710
// constexpr std::uintptr_t UNITY_PLR_INITIALIZE_GAMEASSEMBLY = 0x804710;
// using UnityPlayer_InitializeGameAssembly_t = bool(*)(void*, void*);

// +--------------------------------------------------------+
// |                       Functions                        |
// +--------------------------------------------------------+
HMODULE LoadLibraryW_Hook(LPCWSTR lpLibFileName);

void spoofer::main_thread()
{
    OpenConsole();
    
    // I am gonna test a new approach to hook gameassembly...
    HMODULE kernel32 = GetModuleHandleA("kernel32.dll");
    if (!kernel32)
    {
        std::cerr << "Failed to get kernel32.dll handle" << std::endl;
        return;
    }

    // Get the address of LoadLibraryW
    auto LoadLibraryW_addr = GetProcAddress(kernel32, "LoadLibraryW");
    if (!LoadLibraryW_addr)
    {
        std::cerr << "Failed to get LoadLibraryW address" << std::endl;
        return;
    }

    // Hook LoadLibraryW to detect when GameAssembly.dll is loaded
    HookManager::Install(reinterpret_cast<HMODULE(*)(LPCWSTR)>(LoadLibraryW_addr), LoadLibraryW_Hook);
    std::cout << "[SkipSpoofer] LoadLibraryW hooked" << std::endl;

    multiclient::CloseAllMutexesInCurrentProcess();
}

// +--------------------------------------------------------+
// |                       ADDRESSES                        |
// +--------------------------------------------------------+
constexpr std::uintptr_t GetDeviceUniqueID_Addr = 0x1A56880;
// it takes a MethodInfo* ptr, but because of stdcall, it's in a register, so we can ignore it
using GetDeviceUniqueID_t = UnityString*(*)();

// +--------------------------------------------------------+
// |                       HWID Spoof                       |
// +--------------------------------------------------------+
UnityString*GetDeviceUniqueID_Hook()
{
    static bool init = false;
    if (!init)
    {
        Unity::Init();
        Unity::ThreadAttach();
        // good time to load dll if it exists
        if (std::filesystem::exists("wowiezz.dll"))
        {
            std::cout << "[SkipSpoofer] Loading wowiezz.dll..." << std::endl;
            LoadLibraryA("wowiezz.dll");
        }
        init = true;
    }
    UnityString*originalID = HookManager::Call(GetDeviceUniqueID_Hook);
    if (!originalID) {
        // we might be in a vm
        return UnityString::New(scramble("9dafafc41f9790ff32f5e65ef03c503b97d8fdb3"));
    } else if (originalID->ToString() == "")
    {
        return UnityString::New(scramble("9dafafc41f9790ff32f5e65ef03c503b97d8fdb3"));
    }

    std::string originalIDStr = originalID->ToString();
    std::string scrambledID = scramble(originalIDStr);
    UnityString* fakeID = UnityString::New(scrambledID);
    std::cout << "[SkipSpoofer] Original HWID: " << originalIDStr << std::endl;
    std::cout << "[SkipSpoofer] Scrambled HWID: " << scrambledID << std::endl;
    return fakeID;
}

// +--------------------------------------------------------+
// |                         HOOKS                          |
// +--------------------------------------------------------+
HMODULE LoadLibraryW_Hook(LPCWSTR lpLibFileName)
{
    // WE are going to wait for gameassembly to be loaded, then we will hook it
    HMODULE module = HookManager::Call(LoadLibraryW_Hook, lpLibFileName);

    std::wstring libName(lpLibFileName);
    if (libName.find(L"GameAssembly.dll") != std::wstring::npos)
    {
        std::cout << "[SkipSpoofer] GameAssembly.dll loaded, hooking..." << std::endl;
        std::cout << "[SkipSpoofer] Base address: " << std::hex <<module << std::endl;

        // Hook GetDeviceUniqueID
        GetDeviceUniqueID_t originalGetDeviceUniqueID = reinterpret_cast<GetDeviceUniqueID_t>((std::uintptr_t)module + GetDeviceUniqueID_Addr);
        HookManager::Install(originalGetDeviceUniqueID, GetDeviceUniqueID_Hook);
    }

    return module;
}

//
// Unity forces a single instance through a mutex: \Sessions\1\BaseNamedObjects\t-1-4-155-Polytoria-Client-exe-SingleInstanceMutex-Default
//
void ForceCloseMutex()
{
    const char*mutexName = "t-1-4-155-Polytoria-Client-exe-SingleInstanceMutex-Default";
}