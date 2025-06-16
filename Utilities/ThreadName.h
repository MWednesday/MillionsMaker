#pragma once

#include <thread>
#include <stringapiset.h>
#include <string>
#include <processthreadsapi.h>

inline void SetCurrentThreadName(std::string_view name)
{
    // Convert UTF-8 name to UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), nullptr, 0);
    std::wstring wname(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wname.data(), wlen);

    SetThreadDescription(GetCurrentThread(), wname.c_str());
}

inline void SetThreadName(std::thread& thr, std::string_view name)
{
    // Convert UTF-8 name to UTF-16
    int wlen = MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), nullptr, 0);
    std::wstring wname(wlen, L'\0');
    MultiByteToWideChar(CP_UTF8, 0, name.data(), (int)name.size(), wname.data(), wlen);

    // thr.native_handle() is a HANDLE on Windows
    SetThreadDescription(thr.native_handle(), wname.c_str());
}