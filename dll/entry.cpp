/**
 * @file entry.cpp
 * @author ElCapor (infosec@yu-tech.cloud)
 * @brief 
 * @version 0.1
 * @date 2026-02-13
 * 
 * CC-BY-SA 4.0 Copyright (c) 2026 @ElCapor
 * 
 */

#include <Windows.h>
#include <thread>
#include <cheat/cheat.h>


 BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved) {
     switch (ul_reason_for_call) {
     case DLL_PROCESS_ATTACH:
        std::thread(main_thread).detach();
        break;
     case DLL_THREAD_ATTACH:
     case DLL_THREAD_DETACH:
     case DLL_PROCESS_DETACH:
         break;
     }
     return TRUE;
 }