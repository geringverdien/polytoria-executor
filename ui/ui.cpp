#include <ui/ui.h>
#include <hooking/dx11hook.h>
#include <hooking/hookmanager.h>
#include <unity/unity.h>


// +--------------------------------------------------------+
// |                       Variables                        |
// +--------------------------------------------------------+
UI::UiState UI::state = UI::NotReady;

int UI::keybind = VK_DELETE;

// +--------------------------------------------------------+
// |                       Functions                        |
// +--------------------------------------------------------+

// Custom present, as the one in dx11hook , is not permissible enough
HRESULT HookPresent(IDXGISwapChain* swap, UINT swapInterval, UINT flags);


// +--------------------------------------------------------+
// |                     User Interface                     |
// +--------------------------------------------------------+

void UI::Setup()
{
    // hook dx11 swap chain
    IDXGISwapChain *dummySwapChain = dx11::CreateSwapChain();
    dx11::HookSwapChain(dummySwapChain, HookPresent);
}

void UI::Draw()
{
    switch (state)
    {
    case NotReady:
        DrawWaitingScreen();
        break;
    case Ready:
        DrawMainUI();
        break;
    case Closed:
        DrawClosedHint();
        break;
    }
}

// +--------------------------------------------------------+
// |                        WndProc                         |
// +--------------------------------------------------------+
WNDPROC oWndProc = nullptr;
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT __stdcall WndProcHook(const HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_KEYDOWN && wParam == UI::keybind) {
		if (UI::state == UI::Ready) {
            UI::state = UI::Closed;
        } else if (UI::state == UI::Closed) 
        {
            UI::state = UI::Ready;
        }
	}

	if (ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam))
		return true;

	if (UI::state == UI::Ready) {

		// Block mouse input
		if (uMsg >= WM_MOUSEFIRST && uMsg <= WM_MOUSELAST)
			return true;

        // Need to fix this, it doesn't seem to work well, have to look into unity to block really everything
		if (uMsg == WM_MOUSEWHEEL || uMsg == WM_MOUSEHWHEEL)
			return true;

		// Block keyboard input
		if (uMsg >= WM_KEYFIRST && uMsg <= WM_KEYLAST)
			return true;
	}

	return CallWindowProc(oWndProc, hWnd, uMsg, wParam, lParam);
}
// +--------------------------------------------------------+
// |                      Direct X 11                       |
// +--------------------------------------------------------+
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>

HRESULT HookPresent(IDXGISwapChain* swap, UINT swapInterval, UINT flags)
{
    static bool init = false;
    if (!init)
    {
        // Attach to unity
        // so that the ui can make unity calls
        Unity::ThreadAttach();

        dx11::GetSwapChain(swap);
        ID3D11Device *dev = dx11::GetDevice();
        ID3D11DeviceContext *ctx = dx11::GetContext();
        DXGI_SWAP_CHAIN_DESC sd;
        swap->GetDesc(&sd);
        HWND wnd = sd.OutputWindow;
        dx11::GetRenderTarget();

        oWndProc = (WNDPROC)SetWindowLongPtr(wnd, GWLP_WNDPROC, (LONG_PTR)WndProcHook);

        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        (void)io;
        ImGuiStyle &style = ImGui::GetStyle();
        io.IniFilename = nullptr;
        io.LogFilename = nullptr;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        style.FrameRounding = 3.0f;
        style.GrabRounding = 3.0f;

        io.Fonts->AddFontDefault();

        ImGui_ImplWin32_Init(wnd);
        ImGui_ImplDX11_Init(dev, ctx);
        init = true;
    }

    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();

    UI::Draw();
    
    ImGui::Render();
    ID3D11RenderTargetView *target = dx11::GetRenderTarget();
    dx11::GetContext()->OMSetRenderTargets(1, &target, 0);
    ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
    return HookManager::Scall(HookPresent, swap, swapInterval, flags);
}


// +--------------------------------------------------------+
// |                User Interface (Private)                |
// +--------------------------------------------------------+
#include <ui/explorer.h>
#include <ui/saveinstance.h>

void UI::DrawWaitingScreen()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 300) / 2, (ImGui::GetIO().DisplaySize.y - 100) / 2), ImGuiCond_Always);
    ImGui::Begin("Waiting for Unity", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("Waiting for Unity...");
    ImGui::End();
}

void UI::DrawMainUI()
{
    ImGui::Begin("PolyHack - @ElCapor", nullptr, ImGuiWindowFlags_MenuBar);
    ImGui::Text("#1 Open Source Cheat for Polytoria - V3 Rewrite");
    if (ImGui::BeginTabBar("##tabs", ImGuiTabBarFlags_Reorderable))
    {
        if (ImGui::BeginTabItem("Explorer"))
        {
            ExplorerUI::DrawTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Save Instance"))
        {
            SaveInstanceUI::DrawTab();
            ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
    }
    ImGui::End();
}

void UI::DrawClosedHint()
{
    ImGui::SetNextWindowSize(ImVec2(300, 100), ImGuiCond_Always);
    ImGui::SetNextWindowPos(ImVec2((ImGui::GetIO().DisplaySize.x - 300) / 2, (ImGui::GetIO().DisplaySize.y - 100) / 2), ImGuiCond_Always);
    ImGui::Begin("UI Closed", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    ImGui::Text("UI is closed. Press Keybind (DELTE) to open it again.");
    ImGui::End();
}