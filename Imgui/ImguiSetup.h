#pragma once

#include <imgui.h>
#include "backends/imgui_impl_dx11.h"
#include "backends/imgui_impl_win32.h"
#include <tchar.h>
#include <d3d11.h>
#include "Logging.h"
#include <mutex>

#pragma optimize("", off)

static ID3D11Device*            g_pd3dDevice = NULL;
static ID3D11DeviceContext*     g_pd3dDeviceContext = NULL;
static IDXGISwapChain*          g_pSwapChain = NULL;
static ID3D11RenderTargetView*  g_mainRenderTargetView = NULL;
static bool g_main_window = true;
static bool g_log_window = true;

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
    //MessagesData.push_back(MessageData());
  }

  //void    AddLog(MessageType type, const char* fmt, ...) IM_FMTARGS(2)
  //{
  //  MessageData message;
  //  message.m_type = type;
  //  message.m_lineOffset = Buf.size();
  //  m_lock.lock();
  //  MessagesData.push_back(message);
  //  m_lock.unlock();

  //  int old_size = Buf.size();
  //  va_list args;
  //  va_start(args, fmt);
  //  Buf.appendfv(fmt, args);
  //  va_end(args);

  //  // looking for additional \n
  //  int additionalNewLineCount = 0;
  //  int newLinePos = old_size;
  //  for (int new_size = Buf.size(); newLinePos < new_size; newLinePos++)
  //  {
  //    if (Buf[newLinePos] == '\n')
  //    {
  //      if (additionalNewLineCount == 0)
  //      {
  //        additionalNewLineCount++;
  //      }
  //      else // more than 0
  //      {
  //        message.m_lineOffset = MessagesData.empty() ? 0 : newLinePos + 1;
  //        m_lock.lock();
  //        MessagesData.push_back(message);
  //        m_lock.unlock();
  //        additionalNewLineCount++;
  //      }
  //    }
  //  }
  //}

  void    AddLog(const Message& msg)
  {
    MessageData message;
    message.m_type = msg.m_Type;
    message.m_lineOffset = Buf.size();
    m_lock.lock();
    MessagesData.push_back(message);
    m_lock.unlock();

    int old_size = Buf.size();
    Buf.appendf(msg.m_Text);

    // looking for additional \n
    int additionalNewLineCount = 0;
    int newLinePos = old_size;
    for (int new_size = Buf.size(); newLinePos < new_size; newLinePos++)
    {
      if (Buf[newLinePos] == '\n')
      {
        if (additionalNewLineCount == 0)
        {
          additionalNewLineCount++;
        }
        else // more than 0
        {
          message.m_lineOffset = MessagesData.empty() ? 0 : newLinePos + 1;
          m_lock.lock();
          MessagesData.push_back(message);
          m_lock.unlock();
          additionalNewLineCount++;
        }
      }
    }
  }

  void Draw(bool* p_open = NULL);
};

extern MessageLog g_log;

void DrawLogWindow();

void RenderImgui();

void SetupAndRenderImgui();