//=========================================
// âÊñ ï™äÑ(2ï™äÑ)óp
// Viewport.cpp
// 2025/11/20
// óÈñÿÅ@çã
//=========================================
#include "Viewport.h"

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
