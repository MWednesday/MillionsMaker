#include "gecko.h"
#include "BasicLoggers.h"
#include "ImguiSetup.h"
#include "ThreadName.h"

int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  SetCurrentThreadName("Main");
  BasicLogging::RegisterBasicLoggers(true, false);
  g_Loggers.emplace_back(std::bind(&MessageLog::AddLog, &g_log, std::placeholders::_1));

  std::thread renderingThread([]() { SetupAndRenderImgui(); });
  SetThreadName(renderingThread, "Rendering");

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