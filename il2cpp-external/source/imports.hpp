#pragma once
#include <Windows.h>
#include <cstdint>
#include <TlHelp32.h>
#include <string>
#include <vector>
#include <filesystem>
#include <winternl.h>
#include <atomic>

#ifdef max
#undef max
#endif
#ifdef min
#undef min
#endif

#include <source/memory/memory.hpp>

#include <source/emulator/extcall.hpp>

#include <source/il2cpp/handler.hpp>