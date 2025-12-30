/*==============================================================================

   Direct3Dの初期化関連 [direct3d.cpp]
--------------------------------------------------------------------------------

==============================================================================*/
#include <d3d11.h>
#include "direct3d.h"
#include "debug_ostream.h"

#pragma comment(lib, "d3d11.lib")//DirectXのプログラムを追加する
// #pragma comment(lib, "dxgi.lib")

/* 各種インターフェース */
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pDeviceContext = nullptr;
//static IDXGISwapChain* g_pSwapChain = nullptr;
static IDXGISwapChain* g_pSwapChain[DX_WINDOW_ID_MAX] = { nullptr };

/* バックバッファ関連 */
//static ID3D11RenderTargetView* g_pRenderTargetView = nullptr;
//static ID3D11Texture2D* g_pDepthStencilBuffer = nullptr;
//static ID3D11DepthStencilView* g_pDepthStencilView = nullptr;
//static D3D11_TEXTURE2D_DESC g_BackBufferDesc{};
static ID3D11RenderTargetView* g_pRenderTargetView[DX_WINDOW_ID_MAX] = { nullptr };
static ID3D11Texture2D* g_pDepthStencilBuffer[DX_WINDOW_ID_MAX] = { nullptr };
static ID3D11DepthStencilView* g_pDepthStencilView[DX_WINDOW_ID_MAX] = { nullptr };
static D3D11_TEXTURE2D_DESC g_BackBufferDesc = {};
static D3D11_VIEWPORT g_Viewport[DX_WINDOW_ID_MAX] = {};
static HWND g_hWnd = nullptr;

//static bool configureBackBuffer(); // バックバッファの設定・生成
static void releaseBackBuffer(); // バックバッファの解放


static float	bFactor[4] = { 0.0f,0.0f,0.0f,0.0f };
static ID3D11BlendState* bState[BLENDSTATE_MAX] = { nullptr };
static ID3D11BlendState* g_currentBlendState = nullptr;
static ID3D11DepthStencilState* g_DepthStateEnable;
static ID3D11DepthStencilState* g_DepthStateDisable;


bool Direct3D_Initialize(HWND hWnd, HWND hWnd2)
{
	/* デバイス、スワップチェーン、コンテキスト生成 */
	HWND windows[DX_WINDOW_ID_MAX] = { hWnd, hWnd2 };
	DXGI_SWAP_CHAIN_DESC swap_chain_desc{};
	swap_chain_desc.Windowed = TRUE;
	swap_chain_desc.BufferCount = 2;
	// swap_chain_desc.BufferDesc.Width = 0;
	// swap_chain_desc.BufferDesc.Height = 0;
	// ⇒ ウィンドウサイズに合わせて自動的に設定される
	swap_chain_desc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swap_chain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swap_chain_desc.SampleDesc.Count = 1;
	swap_chain_desc.SampleDesc.Quality = 0;
	swap_chain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;//0にしてみる
	//swap_chain_desc.OutputWindow = hWnd;
	swap_chain_desc.OutputWindow = windows[DX_WINDOW_ID_1];
	g_hWnd = hWnd;

	/*
	IDXGIFactory1* pFactory;
	CreateDXGIFactory1(IID_PPV_ARGS(&pFactory));
	IDXGIAdapter1* pAdapter;
	pFactory->EnumAdapters1(1, &pAdapter); // セカンダリアダプタを取得
	pFactory->Release();
	DXGI_ADAPTER_DESC1 desc;
	pAdapter->GetDesc1(&desc); // アダプタの情報を取得して確認したい場合
	pAdapter->Release(); // D3D11CreateDeviceAndSwapChain()の第１引数に渡して利用し終わったら解放する
	*/

	UINT device_flags = 0;

#if defined(DEBUG) || defined(_DEBUG)
	//device_flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL levels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0
	};

	D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_0;

	HRESULT hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		device_flags,
		levels,
		ARRAYSIZE(levels),
		D3D11_SDK_VERSION,
		&swap_chain_desc,
		/* &g_pSwapChain,*/
		&g_pSwapChain[DX_WINDOW_ID_1],
		&g_pDevice,
		&feature_level,
		&g_pDeviceContext);

	if (FAILED(hr)) {
		MessageBox(hWnd, "Direct3Dの初期化に失敗しました", "エラー", MB_OK);
		return false;
	}

	/*if (!configureBackBuffer()) {
		MessageBox(hWnd, "バックバッファの設定に失敗しました", "エラー", MB_OK);
		return false;
	}*/
	//二枚目のウィンドウ作成
	IDXGIDevice* pDXGIDevice = nullptr;
	g_pDevice->QueryInterface(__uuidof(pDXGIDevice), (void**)&pDXGIDevice);
	IDXGIAdapter* pAdapter = nullptr;
	pDXGIDevice->GetAdapter(&pAdapter);
	IDXGIFactory* pFactory = nullptr;
	pAdapter->GetParent(__uuidof(pFactory), (void**)&pFactory);

	swap_chain_desc.OutputWindow = windows[DX_WINDOW_ID_2];
	pFactory->CreateSwapChain(g_pDevice, &swap_chain_desc, &g_pSwapChain[DX_WINDOW_ID_2]);

	//各ウィンドウのRenderTargetViewとDepthStencilViewループを作成
	for (int i = 0; i < DX_WINDOW_ID_MAX; i++)
	{
		ID3D11Texture2D* pBackBuffer = nullptr;
		g_pSwapChain[i]->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&pBackBuffer);
		g_pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_pRenderTargetView[i]);

		//バックバッファの情報を取得（각 윈도우마다 사이즈를 가져와 뷰포트 및 depth버퍼 생성에 사용）
		D3D11_TEXTURE2D_DESC backDesc;
		pBackBuffer->GetDesc(&backDesc);

		// 첫 번째 윈도우의 정보는 전역 g_BackBufferDesc에도 저장 (기존 호환성 유지)
		if (i == 0)
		{
			g_BackBufferDesc = backDesc;
		}

		// デプスステンシルバッファの生成（각 윈도우 별로 생성）
		D3D11_TEXTURE2D_DESC depth_stencil_desc{};
		depth_stencil_desc.Width = backDesc.Width;
		depth_stencil_desc.Height = backDesc.Height;
		depth_stencil_desc.MipLevels = 1;
		depth_stencil_desc.ArraySize = 1;
		depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depth_stencil_desc.SampleDesc.Count = backDesc.SampleDesc.Count;
		depth_stencil_desc.SampleDesc.Quality = backDesc.SampleDesc.Quality;
		depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
		depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depth_stencil_desc.CPUAccessFlags = 0;
		depth_stencil_desc.MiscFlags = 0;

		hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pDepthStencilBuffer[i]);
		if (FAILED(hr))
		{
			// 深刻なエラーなのでメッセージを出して失敗とする
			MessageBox(hWnd, "DepthStencilBufferの生成に失敗しました", "エラー", MB_OK);
			pBackBuffer->Release();
			pFactory->Release();
			pAdapter->Release();
			pDXGIDevice->Release();
			return false;
		}

		// デプスステンシルビューの生成
		D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
		depth_stencil_view_desc.Format = depth_stencil_desc.Format;
		depth_stencil_view_desc.ViewDimension = (depth_stencil_desc.SampleDesc.Count > 1) ? D3D11_DSV_DIMENSION_TEXTURE2DMS : D3D11_DSV_DIMENSION_TEXTURE2D;
		depth_stencil_view_desc.Texture2D.MipSlice = 0;
		depth_stencil_view_desc.Flags = 0;
		hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer[i], &depth_stencil_view_desc, &g_pDepthStencilView[i]);
		if (FAILED(hr))
		{
			MessageBox(hWnd, "DepthStencilViewの生成に失敗しました", "エラー", MB_OK);
			SAFE_RELEASE(g_pDepthStencilBuffer[i]);
			pBackBuffer->Release();
			pFactory->Release();
			pAdapter->Release();
			pDXGIDevice->Release();
			return false;
		}

		// 深度ステンシルステート設定
		D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
		depthStencilDesc.DepthEnable = TRUE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilDesc.StencilEnable = FALSE;
		// 深度ステートは 전역으로 하나만 생성해도 좋지만 기존 코드는 루프 내부에서 생성하므로 호환을 유지
		g_pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateEnable);//深度有効ステート
		depthStencilDesc.DepthEnable = FALSE;
		depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
		g_pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateDisable);//深度無効ステート

		// 초기에는 깊이 비활성(원래 코드 호환)
		g_pDeviceContext->OMSetDepthStencilState(g_DepthStateDisable, NULL); //デフォルト　深度無効

		// ビューポート設定（各ウィンドウ 백버퍼 크기에 맞춰 설정）
		g_Viewport[i].TopLeftX = 0.0f;
		g_Viewport[i].TopLeftY = 0.0f;
		g_Viewport[i].Width = static_cast<FLOAT>(backDesc.Width);
		g_Viewport[i].Height = static_cast<FLOAT>(backDesc.Height);
		g_Viewport[i].MinDepth = 0.0f;
		g_Viewport[i].MaxDepth = 1.0f;

		// RenderTarget と DepthStencilView を 바인드
		g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView[i], g_pDepthStencilView[i]);
		g_pDeviceContext->RSSetViewports(1, &g_Viewport[i]); // ビューポートの設定

		pBackBuffer->Release();
	}

	pFactory->Release();
	pAdapter->Release();
	pDXGIDevice->Release();
	// サンプラーステート設定
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(samplerDesc));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;//ちょっといいフィルターにする
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;//横の座標範囲外は画像繰り返し
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;//縦の座標範囲外は画像繰り返し
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;//未使用
	samplerDesc.MipLODBias = 0;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	ID3D11SamplerState* samplerState = NULL;
	g_pDevice->CreateSamplerState(&samplerDesc, &samplerState);
	//サンプラーをシェーダーへセット
	g_pDeviceContext->PSSetSamplers(0, 1, &samplerState);



	// ブレンドステート設定
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//ブレンド無効
	blendDesc.RenderTarget[0].BlendEnable = FALSE;	//ブレンド無効
	g_pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_NONE]);

	//αブレンド
	blendDesc.RenderTarget[0].BlendEnable = TRUE;	//ブレンド有効
	g_pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_ALFA]);//<<ALPHA！

	//加算合成
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	g_pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_ADD]);

	//減算合成
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_SUBTRACT;//<<<<表示色 = 背景 - ポリゴン
	//	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;//<<<<表示色 = 背景 - ポリゴン
	g_pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_SUB]);

	//透過用ブレンド
	blendDesc.RenderTarget[0].BlendEnable = TRUE;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_BLEND_FACTOR;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_BLEND_FACTOR;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	hr = g_pDevice->CreateBlendState(&blendDesc, &bState[BLENDSTATE_TRANSPARENCY]);
	if (FAILED(hr)) return false;
	bFactor[3] = 0.4f;

	SetBlendState(BLENDSTATE_ALFA);//デフォルト設定


	// 深度ステンシルステート設定
	//D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	//ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	//depthStencilDesc.DepthEnable = TRUE;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	//depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	//depthStencilDesc.StencilEnable = FALSE;
	//g_pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateEnable);//深度有効ステート
	//depthStencilDesc.DepthEnable = FALSE;
	//depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;
	//g_pDevice->CreateDepthStencilState(&depthStencilDesc, &g_DepthStateDisable);//深度無効ステート
	//g_pDeviceContext->OMSetDepthStencilState(g_DepthStateDisable, NULL); //デフォルト　深度無効


	return true;
}
void Direct3D_SetRenderTarget(DX_WINDOW_ID id)
{
	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView[id], g_pDepthStencilView[id]);
	g_pDeviceContext->RSSetViewports(1, &g_Viewport[id]);
}

void	SetDepthTest(bool flg)
{
	if (flg == true)
	{
		g_pDeviceContext->OMSetDepthStencilState(g_DepthStateEnable, NULL); //デフォルト　深度無効
	}
	else
	{
		g_pDeviceContext->OMSetDepthStencilState(g_DepthStateDisable, NULL); //デフォルト　深度無効
	}


}

void Direct3D_Finalize()
{
	releaseBackBuffer();

	//if (g_pSwapChain) {
	//	g_pSwapChain->Release();
	//	g_pSwapChain = nullptr;
	//}

	for (int i = 0; i < DX_WINDOW_ID_MAX; i++)
	{
		SAFE_RELEASE(g_pSwapChain[i]);
	}

	/*if (g_pDeviceContext) {
		g_pDeviceContext->Release();
		g_pDeviceContext = nullptr;
	}

	if (g_pDevice) {
		g_pDevice->Release();
		g_pDevice = nullptr;
	}*/

	SAFE_RELEASE(g_pDeviceContext);
	SAFE_RELEASE(g_pDevice);
}

//void Direct3D_Clear()
//{
//	float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
//	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView, clear_color);
//	g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
//
//	// レンダーターゲットビューとデプスステンシルビューの設定/////////////追加
//	g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);
//
//
//}
void Direct3D_Clear(DX_WINDOW_ID id)
{
	float clear_color[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	g_pDeviceContext->ClearRenderTargetView(g_pRenderTargetView[id], clear_color);
	if (g_pDepthStencilView[id])
	{
		g_pDeviceContext->ClearDepthStencilView(g_pDepthStencilView[id], D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	// レンダーターゲットビューとデプスステンシルビューの設定/////////////追加
	//g_pDeviceContext->OMSetRenderTargets(1, &g_pRenderTargetView, g_pDepthStencilView);


}
//void Direct3D_Present()
//{
//	// スワップチェーンの表示
//	g_pSwapChain->Present(1, 0);
//}
void Direct3D_Present(DX_WINDOW_ID id)
{
	// スワップチェーンの表示
	g_pSwapChain[id]->Present(1, 0);
}

//////////////////////////////////////////////追加

ID3D11Device* Direct3D_GetDevice()
{
	return g_pDevice;
}

ID3D11DeviceContext* Direct3D_GetDeviceContext()
{
	return g_pDeviceContext;
}

unsigned int Direct3D_GetBackBufferWidth()
{
	return g_BackBufferDesc.Width;
}

unsigned int Direct3D_GetBackBufferHeight()
{
	return g_BackBufferDesc.Height;
}

HWND Direct3D_GetWindowHandle()
{
	return g_hWnd;
}

////////////////////////////////////////////////////////




//bool configureBackBuffer()
//{
//    HRESULT hr;
//
//    ID3D11Texture2D* back_buffer_pointer = nullptr;
//
//	// バックバッファの取得
//	hr = g_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&back_buffer_pointer);
//
//    if (FAILED(hr)) {
//		hal::dout << "バックバッファの取得に失敗しました" << std::endl;
//        return false;
//    }
//
//	// バックバッファのレンダーターゲットビューの生成
//	hr = g_pDevice->CreateRenderTargetView(back_buffer_pointer, nullptr, &g_pRenderTargetView);
//
//    if (FAILED(hr)) {
//        back_buffer_pointer->Release();
//        hal::dout << "バックバッファのレンダーターゲットビューの生成に失敗しました" << std::endl;
//        return false;
//    }
//
//	// バックバッファの状態（情報）を取得
//    back_buffer_pointer->GetDesc(&g_BackBufferDesc);
//
//	back_buffer_pointer->Release(); // バックバッファのポインタは不要なので解放
//
//	// デプスステンシルバッファの生成
//	D3D11_TEXTURE2D_DESC depth_stencil_desc{};
//	depth_stencil_desc.Width = g_BackBufferDesc.Width;
//	depth_stencil_desc.Height = g_BackBufferDesc.Height;
//	depth_stencil_desc.MipLevels = 1;
//	depth_stencil_desc.ArraySize = 1;
//	depth_stencil_desc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
//	depth_stencil_desc.SampleDesc.Count = 1;
//	depth_stencil_desc.SampleDesc.Quality = 0;
//	depth_stencil_desc.Usage = D3D11_USAGE_DEFAULT;
//	depth_stencil_desc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
//	depth_stencil_desc.CPUAccessFlags = 0;
//	depth_stencil_desc.MiscFlags = 0;
//	hr = g_pDevice->CreateTexture2D(&depth_stencil_desc, nullptr, &g_pDepthStencilBuffer);
//
//	if (FAILED(hr)) {
//		hal::dout << "デプスステンシルバッファの生成に失敗しました" << std::endl;
//		return false;
//	}
//
//	// デプスステンシルビューの生成
//	D3D11_DEPTH_STENCIL_VIEW_DESC depth_stencil_view_desc{};
//	depth_stencil_view_desc.Format = depth_stencil_desc.Format;
//	depth_stencil_view_desc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
//	depth_stencil_view_desc.Texture2D.MipSlice = 0;
//	depth_stencil_view_desc.Flags = 0;
//	hr = g_pDevice->CreateDepthStencilView(g_pDepthStencilBuffer, &depth_stencil_view_desc, &g_pDepthStencilView);
//
//	if (FAILED(hr)) {
//		hal::dout << "デプスステンシルビューの生成に失敗しました" << std::endl;
//		return false;
//	}
//
//
//	// ビューポートの設定/////////////////////追加
//	g_Viewport.TopLeftX = 0.0f;
//	g_Viewport.TopLeftY = 0.0f;
//	g_Viewport.Width = static_cast<FLOAT>(g_BackBufferDesc.Width);
//	g_Viewport.Height = static_cast<FLOAT>(g_BackBufferDesc.Height);
//	g_Viewport.MinDepth = 0.0f;
//	g_Viewport.MaxDepth = 1.0f;
//	g_pDeviceContext->RSSetViewports(1, &g_Viewport); // ビューポートの設定
//	////////////////////////////////////////////追加
//
//
//    return true;
//}

void releaseBackBuffer()
{

	for (int i = 0; i < DX_WINDOW_ID_MAX; i++)
	{
		SAFE_RELEASE(g_pRenderTargetView[i]);
		SAFE_RELEASE(g_pDepthStencilBuffer[i]);
		SAFE_RELEASE(g_pDepthStencilView[i]);
	}
	/*if (g_pRenderTargetView) {
		g_pRenderTargetView->Release();
		g_pRenderTargetView = nullptr;
	}

	if (g_pDepthStencilBuffer) {
		g_pDepthStencilBuffer->Release();
		g_pDepthStencilBuffer = nullptr;
	}

	if (g_pDepthStencilView) {
		g_pDepthStencilView->Release();
		g_pDepthStencilView = nullptr;
	}*/
}


//以下の関数を一番下へ追加
void SetBlendState(BLENDSTATE blend)
{

	//範囲チェック
	if (blend < BLENDSTATE_NONE || blend >= BLENDSTATE_MAX) return;

	//既に同じブレンドステートが設定されている場合はスキップ
	if (bState[blend] == g_currentBlendState) return;

	g_currentBlendState = bState[blend];

	// BLENDSTATE_TRANSPARENCY の場合はブレンドファクタを渡す
	if (blend == BLENDSTATE_TRANSPARENCY)
	{
		// R, G, B は 1.0f (フル) のままで、アルファのみ bFactor[3] (0.4f) を使用
		float factor[4] = { 1.0f, 1.0f, 1.0f, bFactor[3] };
		g_pDeviceContext->OMSetBlendState(bState[blend], factor, 0xffffffff);
	}
	else
	{
		// 他のブレンドステートは NULL (または {0,0,0,0}) を渡す
		g_pDeviceContext->OMSetBlendState(bState[blend], bFactor, 0xffffffff);
	}
}