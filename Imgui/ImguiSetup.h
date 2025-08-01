#pragma once

#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include "Logging.h"
#include <tchar.h>
#include <d3d11.h>
#include <mutex>

static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
static bool g_main_window = true;

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void CleanupRenderTarget();

void CreateRenderTarget();

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd);

void CleanupDeviceD3D();

struct MessageData
{
public:
  int m_lineOffset;
  MessageType m_type;
};

struct MessageLog
{
  ImGuiTextBuffer       Buf;
  ImGuiTextFilter       Filter;
  ImVector<MessageData> MessagesData; // Index to lines offset. We maintain this with AddLog() calls.
  bool                  AutoScroll;  // Keep scrolling if already at the bottom.
  std::mutex m_lock;

  MessageLog()
  {
    AutoScroll = true;
    Clear();
  }

  void    Clear()
  {
    std::lock_guard<std::mutex> guard(m_lock);
    Buf.clear();
    MessagesData.clear();
  }

  void    AddLog(const Message& msg)
  {
    MessageData message;
    message.m_type = msg.m_Type;

    std::lock_guard<std::mutex> guard(m_lock);
    message.m_lineOffset = Buf.size();
    MessagesData.push_back(message);
    
    int oldSize = Buf.size(); // new line's position
    Buf.append(msg.m_Text);
    const int newSize = Buf.size();

    // looking for additional \n
    for (int currentChar = oldSize; currentChar < newSize; currentChar++)
    {
      // Don't add a separate new line for last \n because we already do it for each new line at the start of the function
      if (Buf[currentChar] == '\n' && currentChar != newSize - 1) 
      {
          message.m_lineOffset = MessagesData.empty() ? 0 : currentChar + 1;
          MessagesData.push_back(message);
      }
    }
  }

  void Draw();
};

extern MessageLog g_log;

void DrawLogWindow();

void RenderImgui();

void SetupAndRenderImgui();