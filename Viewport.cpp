/*
* ファイル名	Viewport.cpp
* タイトル	画面分割
* 作成者		鈴木豪
* 作成日		11月20日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"Viewport.h"

//================================================================
//	グローバル変数
//================================================================
D3D11_VIEWPORT g_LeftViewPort;
D3D11_VIEWPORT g_RightViewPort;

bool Viewport_Initialize(HWND hWnd)
{
	float windowWidth = (float)Direct3D_GetBackBufferWidth();
	float windowHeight = (float)Direct3D_GetBackBufferHeight();


	g_LeftViewPort.TopLeftX = 0;
	g_LeftViewPort.TopLeftY = 0;
	g_LeftViewPort.Width = windowWidth / 2.0f;
	g_LeftViewPort.Height = windowHeight;
	g_LeftViewPort.MinDepth = 0.0f;
	g_LeftViewPort.MaxDepth = 1.0f;

	g_RightViewPort.TopLeftX = windowWidth / 2.0f;
	g_RightViewPort.TopLeftY = 0;
	g_RightViewPort.Width = windowWidth / 2.0f;
	g_RightViewPort.Height = windowHeight;
	g_RightViewPort.MinDepth = 0.0f;
	g_RightViewPort.MaxDepth = 1.0f;

	return true;
}

void Viewport_Reset()
{
	ID3D11DeviceContext* g_pContext = Direct3D_GetDeviceContext();

	//フルスクリーンの値に戻す
	D3D11_VIEWPORT DefaultScreen = {
		0.0f, //TopLeftX
		0.0f, //TopLeftY
		(float)Direct3D_GetBackBufferWidth(),//width
		(float)Direct3D_GetBackBufferHeight(),//height
		0.0f, //MinDepth
		1.0f  //MaxDepth
	};

	if (g_pContext)
	{
		g_pContext->RSSetViewports(1, &DefaultScreen);
	}
}