#include "ImguiSetup.h"
#include "CryptoConnection.h"
#include "ScopeTimer.h"
#include "ThreadName.h"
#include "imgui_internal.h"
#include <algorithm>
#include <set>

MessageLog g_log;

void CleanupRenderTarget()
{
  if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = NULL; }
}

void CreateRenderTarget()
{
  ID3D11Texture2D* pBackBuffer;
  g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
  g_pd3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &g_mainRenderTargetView);
  pBackBuffer->Release();
}

LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
    return true;

  switch (msg)
  {
  case WM_SIZE:
    if (g_pd3dDevice != NULL && wParam != SIZE_MINIMIZED)
    {
      CleanupRenderTarget();
      g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
      CreateRenderTarget();
    }
    return 0;
  case WM_SYSCOMMAND:
    if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
      return 0;
    break;
  case WM_DESTROY:
    ::PostQuitMessage(0);
    return 0;
  }
  return ::DefWindowProc(hWnd, msg, wParam, lParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool CreateDeviceD3D(HWND hWnd)
{
  // Setup swap chain
  DXGI_SWAP_CHAIN_DESC sd;
  ZeroMemory(&sd, sizeof(sd));
  sd.BufferCount = 2;
  sd.BufferDesc.Width = 0;
  sd.BufferDesc.Height = 0;
  sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
  sd.BufferDesc.RefreshRate.Numerator = 60;
  sd.BufferDesc.RefreshRate.Denominator = 1;
  sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
  sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
  sd.OutputWindow = hWnd;
  sd.SampleDesc.Count = 1;
  sd.SampleDesc.Quality = 0;
  sd.Windowed = TRUE;
  sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

  UINT createDeviceFlags = 0;
  D3D_FEATURE_LEVEL featureLevel;
  const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
  if (D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK)
    return false;

  CreateRenderTarget();
  return true;
}

void CleanupDeviceD3D()
{
  CleanupRenderTarget();
  if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = NULL; }
  if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = NULL; }
  if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = NULL; }
}

void PrintMessageWithCorrectColor(MessageType type, const char* line_start, const char* line_end)
{
  if (type == MessageType::Debug)
  {
    ImGui::TextUnformatted(line_start, line_end);
  }
  else // pick color
  {
    size_t text_line_size = line_end + 2 - line_start;
    char* text_line = new char[text_line_size];
    strncpy_s(text_line, text_line_size, line_start, text_line_size - 1);

    if (type == MessageType::Info)
    {
      ImGui::TextColored(ImVec4(0.3f, 0.7f, 1.0f, 1.0f), text_line);
    }
    if (type == MessageType::Warning)
    {
      ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), text_line);
    }
    else if (type == MessageType::Error)
    {
      ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), text_line);
    }
    delete[] text_line;
  }
}

void MessageLog::Draw(bool* p_open)
{
  // Options menu
  if (ImGui::BeginPopup("Options"))
  {
    ImGui::Checkbox("Auto-scroll", &AutoScroll);
    ImGui::EndPopup();
  }

  // Main window
  if (ImGui::Button("Options"))
  {
      ImGui::OpenPopup("Options");
  }
  ImGui::SameLine();
  bool clear = ImGui::Button("Clear");
  ImGui::SameLine();
  bool copy = ImGui::Button("Copy");
  ImGui::SameLine();
  Filter.Draw("Filter", -100.0f);

  ImGui::Separator();
  ImGui::BeginChild("scrolling", ImVec2(0, 0), false,  ImGuiWindowFlags_HorizontalScrollbar);

  if (clear)
    Clear();
  if (copy)
    ImGui::LogToClipboard();

  ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

  if (Filter.IsActive())
  {
    // In this example we don't use the clipper when Filter is enabled.
    // This is because we don't have a random access on the result on our filter.
    // A real application processing logs with ten of thousands of entries may want to store the result of
    // search/filter.. especially if the filtering function is not trivial (e.g. reg-exp).
    for (int line_no = 0; line_no < MessagesData.Size; line_no++)
    {
      m_lock.lock();
      MessageData message = MessagesData[line_no];
      const char* line_start = Buf.begin() + message.m_lineOffset;
      const char* line_end = (line_no + 1 < MessagesData.Size) ? (Buf.begin() + MessagesData[line_no + 1].m_lineOffset - 1) : Buf.end();
      const std::string line(line_start, line_end + 1);
      m_lock.unlock();

      if (Filter.PassFilter(line.data(), line.data() + line.size()))
      {
        PrintMessageWithCorrectColor(message.m_type, line.data(), line.data() + line.size());
      }
    }
  }
  else
  {
    // The simplest and easy way to display the entire buffer:
    //   ImGui::TextUnformatted(buf_begin, buf_end);
    // And it'll just work. TextUnformatted() has specialization for large blob of text and will fast-forward
    // to skip non-visible lines. Here we instead demonstrate using the clipper to only process lines that are
    // within the visible area.
    // If you have tens of thousands of items and their processing cost is non-negligible, coarse clipping them
    // on your side is recommended. Using ImGuiListClipper requires
    // - A) random access into your data
    // - B) items all being the  same height,
    // both of which we can handle since we an array pointing to the beginning of each line of text.
    // When using the filter (in the block of code above) we don't have random access into the data to display
    // anymore, which is why we don't use the clipper. Storing or skimming through the search result would make
    // it possible (and would be recommended if you want to search through tens of thousands of entries).
    ImGuiListClipper clipper;
    clipper.Begin(MessagesData.Size);
    while (clipper.Step())
    {
      for (int line_no = clipper.DisplayStart; line_no < clipper.DisplayEnd; line_no++)
      {
        m_lock.lock();
        MessageData message = MessagesData[line_no];
        const char* line_start = Buf.begin() + message.m_lineOffset;
        const char* line_end = (line_no + 1 < MessagesData.Size) ? (Buf.begin() + MessagesData[line_no + 1].m_lineOffset - 1) : Buf.end();
        const std::string line(line_start, line_end + 1); // Saving value to free the lock
        m_lock.unlock();

        PrintMessageWithCorrectColor(message.m_type, line.data(), line.data() + line.size());
      }
    }
    clipper.End();
  }
  ImGui::PopStyleVar();

  if (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
  {
    ImGui::SetScrollHereY(0.5f);
  }

  ImGui::EndChild();
}

void DrawLogWindow()
{
  const ImGuiViewport* viewport = ImGui::GetMainViewport();
  float yLength = viewport->WorkSize.y / 4;
  ImVec2 LogWindowPos = ImVec2(viewport->WorkPos.x, viewport->WorkSize.y - yLength);
  ImVec2 LogWindowSize = ImVec2(viewport->WorkSize.x, yLength);

  ImGui::SetNextWindowPos(LogWindowPos);
  ImGui::SetNextWindowSize(LogWindowSize);
  ImGui::Begin("Log", &g_log_window, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize);

  g_log.Draw(&g_log_window);

  ImGui::End();
}

bool BufferingBar(const char* label, const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col)
{
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);

  ImVec2 pos = window->DC.CursorPos;
  ImVec2 size = size_arg;
  size.x -= style.FramePadding.x * 2;

  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ImGui::ItemSize(bb, style.FramePadding.y);
  if (!ImGui::ItemAdd(bb, id))
    return false;

  // Render
  const float circleStart = size.x;
  const float circleEnd = circleStart * 1.3f;
  const float circleWidth = circleEnd - circleStart;

  //window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart * 1.1, bb.Max.y), bg_col);
  window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + size.x, bb.Max.y), fg_col);

  const double t = g.Time;
  const float r = size.y / 2;
  const float speed = 1.5f; // speed of circles. The smaller the faster

  const float a = speed * 0;
  const float b = speed * 0.333f;
  const float c = speed * 0.666f;

  const float o1 = (circleWidth + r) * (t + a - speed * (int)((t + a) / speed)) / speed;
  const float o2 = (circleWidth + r) * (t + b - speed * (int)((t + b) / speed)) / speed;
  const float o3 = (circleWidth + r) * (t + c - speed * (int)((t + c) / speed)) / speed;

  window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
  window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
  window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
}

bool Spinner(const char* label, float radius, float thickness, const ImU32& color) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  if (window->SkipItems)
    return false;

  ImGuiContext& g = *GImGui;
  const ImGuiStyle& style = g.Style;
  const ImGuiID id = window->GetID(label);

  ImVec2 pos = window->DC.CursorPos;
  ImVec2 size((radius) * 2, (radius + style.FramePadding.y) * 2);

  const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
  ImGui::ItemSize(bb, style.FramePadding.y);
  if (!ImGui::ItemAdd(bb, id))
    return false;

  // Render
  window->DrawList->PathClear();

  int num_segments = 30;
  float start = abs(ImSin((float)g.Time*1.8f)*(num_segments - 5));

  const float a_min = IM_PI * 2.0f * (start) / (float)num_segments;
  const float a_max = IM_PI * 2.0f * ((float)num_segments - 3) / (float)num_segments;

  const ImVec2 centre = ImVec2(pos.x + radius, pos.y + radius + style.FramePadding.y);

  for (int i = 0; i < num_segments; i++) {
    const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
    window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a + g.Time * 8) * radius,
      centre.y + ImSin(a + g.Time * 8) * radius));
  }

  window->DrawList->PathStroke(color, false, thickness);
}

void AnalyzeCryptoData(CryptoConnection& cryptoConnection)
{
  cryptoConnection.SyncCryptoDataFromCoinGecko();
  cryptoConnection.ScanAndReportSuccessfulCoins();
}

void SetupImguiWindow()
{
  // Create application window
  WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("Millions Maker"), NULL };
  ::RegisterClassEx(&wc);
  HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("Millions Maker"), WS_OVERLAPPEDWINDOW, 100, 100, 1280, 800, NULL, NULL, wc.hInstance, NULL);

  // Initialize Direct3D
  if (!CreateDeviceD3D(hwnd))
  {
    CleanupDeviceD3D();
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);
    return; // failed
  }

  // Show the window
  ::ShowWindow(hwnd, SW_SHOWDEFAULT);
  ::UpdateWindow(hwnd);

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO& io = ImGui::GetIO(); (void)io;

  // Setup Platform/Renderer backends
  ImGui_ImplWin32_Init(hwnd);
  ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

  ImGui::StyleColorsDark();
}

static const ImGuiTableSortSpecs* s_current_sort_specs = NULL;

static bool IMGUI_CDECL CompareWithSortSpecs(Coin& a, Coin& b)
{
  for (int n = 0; n < s_current_sort_specs->SpecsCount; n++)
  {
    // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
    // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
    const ImGuiTableColumnSortSpecs* sort_spec = &s_current_sort_specs->Specs[n];
    float delta = 0;
    switch (sort_spec->ColumnUserID)
    {
    case 0:           delta = (strcmp(a.m_name.c_str(), b.m_name.c_str()));                break;
    case 1:           delta = (a.m_current_price - b.m_current_price);     break;
    //case 2:           delta = (a.m_market_cap - b.m_market_cap);     break;
    case 2:           delta = (a.m_atl_change_percentage - b.m_atl_change_percentage);     break;
    case 3:           delta = (a.m_price_change_percentage_1h_in_currency - b.m_price_change_percentage_1h_in_currency);     break;
    case 4:           delta = (a.m_price_change_percentage_24h_in_currency - b.m_price_change_percentage_24h_in_currency);     break;
    case 5:           delta = (a.m_price_change_percentage_7d_in_currency - b.m_price_change_percentage_7d_in_currency);     break;
    case 6:           delta = (a.m_price_change_percentage_14d_in_currency - b.m_price_change_percentage_14d_in_currency);     break;
    case 7:           delta = (a.m_price_change_percentage_30d_in_currency - b.m_price_change_percentage_30d_in_currency);     break;
    default: IM_ASSERT(0); break;
    }
    if (delta > 0)
      return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? false : true;
    if (delta < 0)
      return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? true : false;
  }

  return strcmp(a.m_name.c_str(), b.m_name.c_str()) < 0;
}

// Will display the number in red if negative, in green if positive
void PrintNumberInTableWithColor(float number)
{
  if (number == 0)
  {
    ImGui::Text("%.2f%%", number);
  }
  else if (number > 0)
  {
    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%.2f%%", number);
  }
  else if (number < 0)
  {
    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "%.2f%%", number);
  }
}

void RenderImgui()
{
  CryptoConnection cryptoConnection;
  std::vector<Coin> coinList;

  ImGuiTextFilter filter;
  std::vector<int> filteredCoinList; // contains indexes into coinList. Avoids making copies of coins!
  int oldNumOfFilterLetters = 0;
  int numOfFilterLetters = 0;

  bool startedCryptoSetUp = false;
  bool tableNeedsToBeRefiltered = false;

  std::set<std::string> coinsAlreadyCheckedOnCoingecko; // search is faster in set than in vector. TODO: consider using unordered_set. Btw check if we can use some int instead of string. Maybe we can add a unique int ID to each Coin
  std::set<std::string> coinsAlreadyCheckedOnCMC;
  std::set<std::string> coinsWithBuyLinkVisited;

  while (g_main_window)
  {
    // Poll and handle messages (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
    // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
    MSG msg;
    while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
    {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
      if (msg.message == WM_QUIT)
      {
        g_main_window = false;
      }
    }

    // Start the Dear ImGui frame
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    // Make sure it's full screen
    bool use_work_area = true;
    ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings | /*ImGuiWindowFlags_MenuBar |*/ ImGuiWindowFlags_NoBringToFrontOnFocus;
    // We demonstrate using the full viewport area or the work area (without menu-bars, task-bars etc.)
    // Based on your use case you may want one of the other.
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(use_work_area ? viewport->WorkPos : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? viewport->WorkSize : viewport->Size);

    // Create Main Window
    ImGui::Begin("Main Window", &g_main_window, flags);

    const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);
    
    if (!startedCryptoSetUp)
    {
      std::thread cryptoThread([](auto var) { AnalyzeCryptoData(var); }, std::ref(cryptoConnection)); // passing as ref as std::thread by default does a copy
      SetThreadName(cryptoThread, "Crypto Sync");
      cryptoThread.detach();
      startedCryptoSetUp = true;
    }
    
    if (cryptoConnection.GetCoinsToParse() > 0 && cryptoConnection.GetCoinList().GetCoinList().size() < cryptoConnection.GetCoinsToParse() - 5) // ignoring 5 in case server decides to return a few less cryptos
    {
      ImGui::Text("Getting cryptocurrencies info from the server:  %d/%d", cryptoConnection.GetCoinList().GetCoinList().size(), cryptoConnection.GetCoinsToParse());
      BufferingBar("##buffer_bar", ImVec2(viewport->WorkSize.x / cryptoConnection.GetCoinsToParse() * cryptoConnection.GetCoinList().GetCoinList().size(), viewport->WorkSize.y / 70), bg, col);
    }
    
    if (cryptoConnection.IsSetupFinished() && coinList.empty())
    {
      coinList = cryptoConnection.GetCoinList().GetCoinList(); // This list should never change after this. Then we can use indexes of vector as free permanent IDs for coins which is great for optimizations.
    }
    
    if (cryptoConnection.IsSetupFinished())
    {
        // TODO implement? const std::vector<Coin>& coinList = cryptoConnection.GetCoinList().GetCoinList(); // This list must never change after this. This way we can use indexes of vector as free permanent IDs for coins which is great for optimizations.

      filter.Draw("Filter", 200.0f);
      ImGui::SameLine();
      static bool bscChecked = false;
      if (ImGui::Checkbox("BSC coins", &bscChecked))
      {
        tableNeedsToBeRefiltered = true;
      }
      ImGui::SameLine();
      static bool ethChecked = false;
      if (ImGui::Checkbox("ETH coins", &ethChecked))
      {
        tableNeedsToBeRefiltered = true;
      }
    
      // Options
      static ImGuiTableFlags flags =
        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable | ImGuiTableFlags_Sortable | ImGuiTableFlags_SortMulti
        | ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter | ImGuiTableFlags_BordersV | ImGuiTableFlags_NoBordersInBody
        | ImGuiTableFlags_ScrollY;
    
      if (ImGui::BeginTable("Cryptocurrencies", 10, flags, ImVec2(0.0f, viewport->WorkSize.y / 4 * 2.8), 0.0f))
      {
        // Declare columns
        // We use the "user_id" parameter of TableSetupColumn() to specify a user id that will be stored in the sort specifications.
        // This is so our sort function can identify a column given our own identifier. We could also identify them based on their index!
    
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_DefaultSort | ImGuiTableColumnFlags_WidthStretch, 0.0f, 0);
        ImGui::TableSetupColumn("Price (USD)", ImGuiTableColumnFlags_WidthFixed, 0.0f, 1);
        //ImGui::TableSetupColumn("Market cap", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);
        ImGui::TableSetupColumn("ATL change", ImGuiTableColumnFlags_WidthFixed, 0.0f, 2);
        ImGui::TableSetupColumn("Price change 1h", ImGuiTableColumnFlags_WidthFixed, 0.0f, 3);
        ImGui::TableSetupColumn("Price change 24h", ImGuiTableColumnFlags_WidthFixed, 0.0f, 4);
        ImGui::TableSetupColumn("Price change 7d", ImGuiTableColumnFlags_WidthFixed, 0.0f, 5);
        ImGui::TableSetupColumn("Price change 14d", ImGuiTableColumnFlags_WidthFixed, 0.0f, 6);
        ImGui::TableSetupColumn("Price change 30d", ImGuiTableColumnFlags_WidthFixed, 0.0f, 7);
        ImGui::TableSetupColumn("More info", ImGuiTableColumnFlags_NoSort| ImGuiTableColumnFlags_WidthFixed, 0.0f, 8);
        ImGui::TableSetupColumn("Buy link", ImGuiTableColumnFlags_NoSort | ImGuiTableColumnFlags_WidthFixed, 0.0f, 9);
        ImGui::TableSetupScrollFreeze(0, 1); // Make row always visible
        ImGui::TableHeadersRow();
    
        // Sort our data if sort specs have been changed!
        if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
          if (sorts_specs->SpecsDirty)
          {
            // If the user presses on the same column and only the sorting direction changes, then we actually could get away
            // with chaning the direction of drawing, instead of resorting again. But the code below sorts.
            tableNeedsToBeRefiltered = true;
            s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
            if (coinList.size() > 1)
            {
              MeasureScopeTime(SortImguiTable);
              std::sort(coinList.begin(), coinList.end(), CompareWithSortSpecs);
            }
            s_current_sort_specs = NULL;
            sorts_specs->SpecsDirty = false;
          }
    
        bool someFilterIsOn = filter.IsActive() || bscChecked || ethChecked || tableNeedsToBeRefiltered;
    
        // Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        
        // Filtering
        if (someFilterIsOn)
        {
          numOfFilterLetters = strlen(filter.InputBuf);
          if (oldNumOfFilterLetters != numOfFilterLetters || tableNeedsToBeRefiltered) // don't use filter.IsActive() here or it will sort every frame
          {
            filteredCoinList.clear();
            oldNumOfFilterLetters = numOfFilterLetters;
    
            int requiredPlatform = 0;
            if (bscChecked)
            {
              requiredPlatform |= static_cast<int>(Coin::Platform::BINANCE_SMART_CHAIN);
            }
            if (ethChecked)
            {
              requiredPlatform |= static_cast<int>(Coin::Platform::ETHEREUM);
            }
            
            std::string filterText = filter.InputBuf;

            for (size_t index = 0; index < coinList.size(); index++)
            {
                // filter by name
                auto compareLowercaseChars = [](char a, char b) {
                    return std::tolower((unsigned char)(a)) == std::tolower((unsigned char)(b));
                    }; 

                bool foundNameMatch = std::search(
                    coinList[index].m_name.begin(), coinList[index].m_name.end(),
                    filterText.begin(), filterText.end(),
                    compareLowercaseChars)   != coinList[index].m_name.end();

                if (foundNameMatch)
                {
                    // Letter pattern was found. Now let's filter by required platforms
                    if (requiredPlatform != 0)
                    {
                        for (auto coinPlatform : coinList[index].m_platforms)
                        {
                            if ((static_cast<int>(coinPlatform.first) & requiredPlatform) != 0)
                            {
                                filteredCoinList.push_back(index);
                            }
                        }
                    }
                    filteredCoinList.push_back(index);
                }
            }

            tableNeedsToBeRefiltered = false;
          }
          clipper.Begin(filteredCoinList.size());
        }
        else
        {
          clipper.Begin(coinList.size());
        }
    
        ImVec4 buttonColor(0.72f, 0.33f, 0.82f, 1.0f);
        // This processes only the amount of elements that visibly fit into the table, for example 45
        while (clipper.Step())
          for (int row_n = clipper.DisplayStart; row_n < clipper.DisplayEnd; row_n++)
          {
            // Display a data item
            Coin* coin;

            (filter.IsActive() || bscChecked || ethChecked) ? coin = &coinList[filteredCoinList[row_n]] : coin = &coinList[row_n];
    
            ImGui::PushID(coin->m_id.c_str());
            ImGui::TableNextRow();
    
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(coin->m_name.c_str());
    
            ImGui::TableNextColumn();
            ImGui::Text("%.10g", coin->m_current_price);
    
            ImGui::TableNextColumn();
            //ImGui::Text("%f", coin->m_market_cap);
            ImGui::Text("%f", coin->m_atl_change_percentage);
    
            ImGui::TableNextColumn();
            PrintNumberInTableWithColor(coin->m_price_change_percentage_1h_in_currency);
    
            ImGui::TableNextColumn();
            PrintNumberInTableWithColor(coin->m_price_change_percentage_24h_in_currency);
    
            ImGui::TableNextColumn();
            PrintNumberInTableWithColor(coin->m_price_change_percentage_7d_in_currency);
    
            ImGui::TableNextColumn();
            PrintNumberInTableWithColor(coin->m_price_change_percentage_14d_in_currency);
    
            ImGui::TableNextColumn();
            PrintNumberInTableWithColor(coin->m_price_change_percentage_30d_in_currency);
    
            ImGui::TableNextColumn();
            {
              // CoinGecko button
              // Potentially slow. We are checking every frame whether all buttons were already visited and searching a coin id in a set. The more buttons visited the slower.
              // Every coin could have additional flags... More RAM but saving on performance...
              bool alreadyVisitedOnCoingecko = coinsAlreadyCheckedOnCoingecko.find(coin->m_id) != coinsAlreadyCheckedOnCoingecko.end(); 
              if (alreadyVisitedOnCoingecko)
              {
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor); //RGBA
              }
              if (ImGui::SmallButton("CG"))
              {
                std::wstring webPage = L"https://www.coingecko.com/en/coins/" + std::wstring(coin->m_id.begin(), coin->m_id.end());
                ShellExecute(0, 0, webPage.c_str(), 0, 0, SW_SHOW);
                coinsAlreadyCheckedOnCoingecko.insert(coin->m_id);
              }
              if (alreadyVisitedOnCoingecko)
              {
                ImGui::PopStyleColor();
              }
              if (ImGui::IsItemHovered())
              {
                ImGui::BeginTooltip();
                ImGui::Text("CoinGecko");
                ImGui::EndTooltip();
              }
    
              // CoinMarketCap button
              ImGui::SameLine();
              bool alreadyVisitedOnCMC = coinsAlreadyCheckedOnCMC.find(coin->m_id) != coinsAlreadyCheckedOnCMC.end();
              if (alreadyVisitedOnCMC)
              {
                ImGui::PushStyleColor(ImGuiCol_Button, buttonColor); //RGBA
              }
              if (ImGui::SmallButton("CMC"))
              {
                std::wstring webPage = L"https://coinmarketcap.com/currencies/" + std::wstring(coin->m_id.begin(), coin->m_id.end());
                ShellExecute(0, 0, webPage.c_str(), 0, 0, SW_SHOW);
                coinsAlreadyCheckedOnCMC.insert(coin->m_id);
              }
              if (alreadyVisitedOnCMC)
              {
                ImGui::PopStyleColor();
              }
    
              if (ImGui::IsItemHovered())
              {
                ImGui::BeginTooltip();
                ImGui::Text("CoinMarketCap");
                ImGui::EndTooltip();
              }
            }
    
            // Buy Link button
            ImGui::TableNextColumn();
            bool alreadyVisitedBuyLink = coinsWithBuyLinkVisited.find(coin->m_id) != coinsWithBuyLinkVisited.end();
            if (alreadyVisitedBuyLink)
            {
              ImGui::PushStyleColor(ImGuiCol_Button, buttonColor); //RGBA
            }
            if (ImGui::SmallButton("Visit##Buy"))
            {
              const std::wstring buyLink = cryptoConnection.GetBuyLink(coin->m_id);
              if (!buyLink.empty())
              {
                ShellExecute(0, 0, buyLink.c_str(), 0, 0, SW_SHOW);
              }
              coinsWithBuyLinkVisited.insert(coin->m_id);
            }
            if (alreadyVisitedBuyLink)
            {
              ImGui::PopStyleColor();
            }
    
            ImGui::PopID();
          }
        ImGui::EndTable();
      }
    }

    DrawLogWindow();

    ImGui::End();

    ImGui::Render();
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
    const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
    g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
    g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

    g_pSwapChain->Present(1, 0); // Present with vsync
  }
}

void SetupAndRenderImgui()
{
  // Data, such as HWND can be accessed only by one thread, that's why we have a function which
  // does everything - SetupAndRenderImgui()
  SetupImguiWindow();

  RenderImgui();
}

