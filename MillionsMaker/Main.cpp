//#include "pch.h"
#include <iostream>
#include <string>
#include <math.h>

#include "gecko.h"

#include <direct.h> // _getcwd
#include "BasicLoggers.h"
#include <set>
#include "ImguiSetup.h"
#include "CryptoConnection.h"


std::string get_working_path()
{
  char temp[250];
  return (_getcwd(temp, sizeof(temp)) ? std::string(temp) : std::string(""));
}

//// Custom allocators in case of memory issues
//// Overriding the global 'new' operator
//void* operator new(size_t size)
//{
//  void* ptr = std::malloc(size);
//  if (!ptr)
//  {
//    throw std::bad_alloc();  // If malloc fails, throw a bad_alloc exception
//  }
//
//#ifdef _DEBUG
//  char buffer[256];
//  snprintf(buffer, sizeof(buffer), "Allocating %zu bytes at %p\n", size, ptr);
//  OutputDebugStringA(buffer);
//#endif
//
//  return ptr;
//}
//
//// Overriding the global 'delete' operator
//void operator delete(void* ptr) noexcept
//{
//  if (ptr)
//  {
//#ifdef _DEBUG
//    char buffer[256];
//    snprintf(buffer, sizeof(buffer), "Deallocating memory at %p\n", ptr);
//    OutputDebugStringA(buffer);
//#endif
//    std::free(ptr);
//  }
//}
//
//// Overriding the global 'new[]' operator (for array allocations)
//void* operator new[](size_t size)
//{
//  void* ptr = std::malloc(size);
//  if (!ptr)
//  {
//    throw std::bad_alloc();  // If malloc fails, throw a bad_alloc exception
//  }
//
//#ifdef _DEBUG
//  char buffer[256];
//  snprintf(buffer, sizeof(buffer), "Allocating array of %zu bytes at %p\n", size, ptr);
//  OutputDebugStringA(buffer);
//#endif
//
//  return ptr;
//}
//
//// Overriding the global 'delete[]' operator (for array deallocations)
//void operator delete[](void* ptr) noexcept
//{
//  if (ptr)
//  {
//#ifdef _DEBUG
//    char buffer[256];
//    snprintf(buffer, sizeof(buffer), "Deallocating array memory at %p\n", ptr);
//    OutputDebugStringA(buffer);
//#endif
//    std::free(ptr);
//  }
//}


#include <windows.h>
#include <processthreadsapi.h>
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  BasicLogging::RegisterBasicLoggers();
  g_Loggers.emplace_back(std::bind(&MessageLog::AddLog, &g_log, std::placeholders::_1));

  std::thread renderingThread([]() { SetupAndRenderImgui(); });

  const std::string configName = "config.ini";
  if(!gecko::api::Initialize(configName))
  { 
      ReportError("CRITICAL: %s is missing. Couldn't find API info. Will quit.", configName.c_str());
      std::this_thread::sleep_for(std::chrono::milliseconds(4000));
      renderingThread.detach();
      return -1;
  }
  
  gecko::api coinGecko;
  while (!coinGecko.ping())
  {
    ReportError("CoinGecko is offline! Waiting...");
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  }

  renderingThread.join();
  gecko::api::Shutdown();

  return 0;
}