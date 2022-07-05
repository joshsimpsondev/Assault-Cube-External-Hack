#pragma once
#include <iostream>
#include <vector>
#include <Windows.h>
#include <TlHelp32.h>

DWORD GetProcId(const char* procName);
uintptr_t GetModuleBaseAdress(DWORD procId, const char* modName);


