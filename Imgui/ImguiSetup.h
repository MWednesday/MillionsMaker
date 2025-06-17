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
    Buf.appendf(msg.m_Text);
    const int newSize = Buf.size();

    // looking for additional \n
    int additionalNewLineCount = 0;
    for (int currentChar = oldSize; currentChar < newSize; currentChar++)
    {
      if (Buf[currentChar] == '\n')
      {
        if (additionalNewLineCount == 0)
        {
          additionalNewLineCount++;
        }
        else // more than 0
        {
          message.m_lineOffset = MessagesData.empty() ? 0 : currentChar + 1;
          MessagesData.push_back(message);
          additionalNewLineCount++;
        }
      }
    }
  }

  void Draw();
};

extern MessageLog g_log;

void DrawLogWindow();

void RenderImgui();

void SetupAndRenderImgui();