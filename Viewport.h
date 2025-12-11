/*
* ファイル名	Viewport.h
* タイトル	画面分割
* 作成者		鈴木豪
* 作成日		11月20日
* 更新日		12月09日
*/

#ifndef VIEWPPORT_H
#define VIEWPPORT_H

//================================================================
//	インクルード
//================================================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;

//================================================================
//	グローバル変数
//================================================================
extern D3D11_VIEWPORT g_LeftViewPort;
extern D3D11_VIEWPORT g_RightViewPort;

bool Viewport_Initialize(HWND hWnd);
void Viewport_Reset();

#endif // VIEWPPORT_H