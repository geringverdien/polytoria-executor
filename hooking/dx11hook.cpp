/**
 * @file dx11hook.cpp
 * @author ElCapor (infosec@yu-tech.cloud)
 * @brief Direct X 11 Hooking Library Implementation
 * @version 0.1
 * @date 2026-02-15
 * 
 * CC-BY-SA 4.0 Copyright (c) 2026 @ElCapor
 * 
 */
#include <hooking/dx11hook.h>
#include <comdef.h>
#include <detours.h>
#include <hooking/hookmanager.h>
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <spdlog/spdlog.h>

// define your own macro for encryption, in case you want one
#ifndef TXT
#define TXT(text) text
#endif

// Static render target view for resize handling
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declaration of ResizeBuffers hook
static HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
static dx11::ResizeBuffers_t oResizeBuffers = nullptr;

// The swap chain for dx11
IDXGISwapChain* dx11::CreateSwapChain()
{
    WNDCLASSEX wc { 0 };
	wc.cbSize = sizeof(wc);
	wc.lpfnWndProc = DefWindowProc;
	wc.lpszClassName = TXT(TEXT("dummy class"));
	RegisterClassExA(&wc);
	HWND hwnd = CreateWindow(wc.lpszClassName, TXT(TEXT("")), WS_DISABLED, 0, 0, 0, 0, NULL, NULL, NULL, nullptr);

	DXGI_SWAP_CHAIN_DESC desc{ 0 };
	desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	desc.SampleDesc.Count = 1;
	desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	desc.BufferCount = 1;
	desc.OutputWindow = hwnd;
	desc.Windowed = TRUE;
	desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	D3D_FEATURE_LEVEL featureLevels[] =
	{
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_11_1
	};

	ID3D11Device* dummyDevice = nullptr;
	IDXGISwapChain* dummySwapChain = nullptr;
	HRESULT result = D3D11CreateDeviceAndSwapChain(
		NULL, 
		D3D_DRIVER_TYPE_HARDWARE,
		NULL, 
		0, 
		featureLevels, 
		1,
		D3D11_SDK_VERSION,
		&desc, 
		&dummySwapChain,
		&dummyDevice, 
		NULL, 
		NULL);

	DestroyWindow(desc.OutputWindow);
	UnregisterClass(wc.lpszClassName, GetModuleHandle(nullptr));
	if (FAILED(result))
	{
		_com_error err(result);
		std::cout << TXT("ERROR : ") << err.ErrorMessage() << std::endl;
		return nullptr;
	}

	return dummySwapChain;
}

void dx11::HookSwapChain(IDXGISwapChain* dummySwapChain, Present_t hookedFunction)
{
	void** vtable = *(void***)dummySwapChain;
	
	// Hook Present at index 8
	static Present_t oPresent = (Present_t)(vtable[8]);
	HookManager::Install(oPresent, hookedFunction);
	
	// Hook ResizeBuffers at index 13
	oResizeBuffers = (dx11::ResizeBuffers_t)(vtable[13]);
	HookManager::Install(oResizeBuffers, hkResizeBuffers);

	dummySwapChain->Release();
	
	spdlog::info("[DX11Hook] Hooked Present (index 8) and ResizeBuffers (index 13)");
}

bool init = false;

IDXGISwapChain* dx11::GetSwapChain(IDXGISwapChain* swp = nullptr)
{
    static IDXGISwapChain* swap = nullptr;
    if (swp != nullptr)
    {
        swap = swp;
        return swap;
    }
    return swap;
}

ID3D11Device* dx11::GetDevice()
{
    static ID3D11Device* device = nullptr;
    if (device == nullptr)
    {
        IDXGISwapChain* swp = GetSwapChain();
        if (!swp)
            std::cout << TXT("Failed to get swap chain") << std::endl;

        if (SUCCEEDED(swp->GetDevice(__uuidof(ID3D11Device), (void**)&device)))
        {
            return device;
        }
        std::cout << TXT("Failed to get device ") << std::endl;
    }
    return device;
}

ID3D11DeviceContext* dx11::GetContext()
{
    static ID3D11DeviceContext* context = nullptr;
    if (context == nullptr)
    {
        ID3D11Device* device = GetDevice();
        if (device != nullptr)
        {
            device->GetImmediateContext(&context);
        }
    }
    if (context == nullptr)
    {
        std::cout << TXT("Failed to get context") << std::endl;
    }
    return context;
}

ID3D11RenderTargetView* dx11::GetRenderTarget()
{
    if (g_mainRenderTargetView == nullptr)
    {
        ID3D11Device* dev = GetDevice();
        if (dev != nullptr)
        {
            ID3D11Texture2D* backBuffer = nullptr;
            IDXGISwapChain* swp = GetSwapChain();
            swp->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&backBuffer);
            dev->CreateRenderTargetView(backBuffer, 0, &g_mainRenderTargetView);
            backBuffer->Release();
            return g_mainRenderTargetView;
        }
        std::cout << TXT("Failed to get render target") << std::endl;

    }

    return g_mainRenderTargetView;
}

void dx11::SetRenderTarget(ID3D11RenderTargetView* rtv)
{
    g_mainRenderTargetView = rtv;
}

void dx11::CleanupRenderTarget()
{
    if (g_mainRenderTargetView)
    {
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }
}

// ResizeBuffers hook implementation
static HRESULT __stdcall hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags)
{
    spdlog::info("[DX11Hook] ResizeBuffers called: {}x{}", Width, Height);
    
    // Cleanup ImGui DX11 resources before resize
    ImGui_ImplDX11_InvalidateDeviceObjects();
    
    // Release the old render target view
    if (g_mainRenderTargetView)
    {
        ID3D11DeviceContext* ctx = dx11::GetContext();
        if (ctx)
        {
            ctx->OMSetRenderTargets(0, 0, 0);
        }
        g_mainRenderTargetView->Release();
        g_mainRenderTargetView = nullptr;
    }

    // Call the original ResizeBuffers
    HRESULT hr = HookManager::Call(hkResizeBuffers, pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
    
    if (FAILED(hr))
    {
        spdlog::error("[DX11Hook] ResizeBuffers failed: 0x{:08X}", hr);
        return hr;
    }

    // Recreate the render target view
    ID3D11Device* pDevice = dx11::GetDevice();
    ID3D11DeviceContext* pContext = dx11::GetContext();
    
    if (!pDevice || !pContext)
    {
        spdlog::error("[DX11Hook] Failed to get device or context after resize");
        return hr;
    }

    ID3D11Texture2D* pBuffer = nullptr;
    HRESULT bufferResult = pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBuffer);
    if (bufferResult != S_OK)
    {
        spdlog::error("[DX11Hook] Failed to get swap chain buffer: 0x{:08X}", bufferResult);
        return hr;
    }

    HRESULT rtvResult = pDevice->CreateRenderTargetView(pBuffer, NULL, &g_mainRenderTargetView);
    if (rtvResult != S_OK)
    {
        spdlog::error("[DX11Hook] Failed to create render target view: 0x{:08X}", rtvResult);
        pBuffer->Release();
        return hr;
    }

    // Release the buffer
    pBuffer->Release();

    // Set the render target
    pContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);

    // Set up the viewport
    D3D11_VIEWPORT vp;
    vp.Width = static_cast<float>(Width);
    vp.Height = static_cast<float>(Height);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    vp.TopLeftX = 0;
    vp.TopLeftY = 0;
    pContext->RSSetViewports(1, &vp);
    
    // Recreate ImGui DX11 resources after resize
    ImGui_ImplDX11_CreateDeviceObjects();
    
    spdlog::info("[DX11Hook] ResizeBuffers completed successfully");

    return hr;
}