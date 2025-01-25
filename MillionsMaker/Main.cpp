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

//#include <d3d11.h>
//
//void CreateRenderTarget()
//{
//  ID3D11Texture2D* pBackBuffer;
//  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
//  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
//  pBackBuffer->Release();
//}
//
//bool CreateDeviceD3D(HWND hWnd)
//{
//  // Setup swap chain
//  DXGI_SWAP_CHAIN_DESC sd;
//  ZeroMemory(&sd, sizeof(sd));
//  sd.BufferCount = 2;
//  sd.BufferDesc.Width = 0;
//  sd.BufferDesc.Height = 0;
//  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
//  sd.BufferDesc.RefreshRate.Numerator = 60;
//  sd.BufferDesc.RefreshRate.Denominator = 1;
//  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
//  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
//  sd.OutputWindow = hWnd;
//  sd.SampleDesc.Count = 1;
//  sd.SampleDesc.Quality = 0;
//  sd.Windowed = TRUE;
//  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
//
//  UINT createDeviceFlags = 0;
//  //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
//  D3D_FEATURE_LEVEL featureLevel;
//  const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
//  if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
//    return false;
//
//  CreateRenderTarget();
//  return true;
//}
#include <windows.h>
#include <processthreadsapi.h>
int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR lpCmdLine, int nCmdShow)
{
  BasicLogging::RegisterBasicLoggers();
  g_Loggers.emplace_back(std::bind(&MessageLog::AddLog, &g_log, std::placeholders::_1));

  std::thread renderingThread([]() { SetupAndRenderImgui(); }); 

  gecko::api coinGecko;
  while (!coinGecko.ping())
  {
    ReportError("CoinGecko is offline! Waiting...");
    std::this_thread::sleep_for(std::chrono::milliseconds(4000));
  }
  renderingThread.join();
  return 0;
}