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
#include <spoofer.hpp>
#include <cstdint>
#include <HookManager.hpp>
#include <iostream>
#include <unity/u.hpp>
#include <random>
#include <algorithm>
#include <simple_filesystem/mod.hpp>

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
void OpenConsole();
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
}

// +--------------------------------------------------------+
// |                       ADDRESSES                        |
// +--------------------------------------------------------+
constexpr std::uintptr_t GetDeviceUniqueID_Addr = 0x1A56880;
// it takes a MethodInfo* ptr, but because of stdcall, it's in a register, so we can ignore it
using GetDeviceUniqueID_t = US *(*)();

// +--------------------------------------------------------+
// |                        CONSOLE                         |
// +--------------------------------------------------------+
void OpenConsole()
{
    AllocConsole();
    FILE *fp;
    SetConsoleTitleA("SkipSped Console");
    freopen_s(&fp, "CONOUT$", "w", stdout);
    freopen_s(&fp, "CONERR$", "w", stderr);
    freopen_s(&fp, "CONIN$", "r", stdin);
    std::cout << "[SkipSpoofer] Started" << std::endl;
}

bool init = false;
// +--------------------------------------------------------+
// |                       HWID Spoof                       |
// +--------------------------------------------------------+
US *GetDeviceUniqueID_Hook()
{
    if (!init)
    {
        UnityResolve::Init(GetModuleHandleW(L"GameAssembly.dll"), U::Mode::Il2Cpp);
        UnityResolve::ThreadAttach();
        init = true;
    }
    US *originalID = HookManager::Call(GetDeviceUniqueID_Hook);
    std::string originalIDStr = originalID->ToString();
    std::string scrambledID = scramble(originalIDStr);
    US* fakeID = US::New(scrambledID);
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