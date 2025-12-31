/*==============================================================================

   Direct3Dの初期化関連 [direct3d.cpp]
														 Author : Youhei Sato
														 Date   : 2025/05/12
--------------------------------------------------------------------------------

==============================================================================*/
#ifndef DIRECT3D_H
#define DIRECT3D_H


#include <Windows.h>
#include <d3d11.h>
#include <mmsystem.h>

#include "DirectXTex.h"/////////////0602
#if _DEBUG
 #pragma comment(lib, "DirectXTex_Debug.lib");
#else
 #pragma comment(lib, "DirectXTex_Release.lib");
#endif




// セーフリリースマクロ
#define SAFE_RELEASE(o) if (o) { (o)->Release(); o = NULL; }
enum DX_WINDOW_ID
{
	DX_WINDOW_ID_1 =0,		// メインウィンドウ
	DX_WINDOW_ID_2,			// サブウィンドウ
	DX_WINDOW_ID_MAX
};;

//bool Direct3D_Initialize(HWND hWnd); // Direct3Dの初期化
bool Direct3D_Initialize(HWND hWnd, HWND hWnd2); // Direct3Dの初期化

void Direct3D_Finalize(); // Direct3Dの終了処理

//void Direct3D_Clear(); // バックバッファのクリア
//void Direct3D_Present(); // バックバッファの表示

void Direct3D_Clear(DX_WINDOW_ID id); // バックバッファのクリア
void Direct3D_Present(DX_WINDOW_ID id); // バックバッファの表示
void Direct3D_SetRenderTarget(DX_WINDOW_ID id); //ウィンドウのRenderTargetをセット
////////////////////////////////////////////////追加
ID3D11Device* Direct3D_GetDevice(); // デバイスの取得
ID3D11DeviceContext* Direct3D_GetDeviceContext(); // デバイスコンテキストの取得

unsigned int Direct3D_GetBackBufferWidth(); // バックバッファの幅を取得
unsigned int Direct3D_GetBackBufferHeight(); // バックバッファの高さを取得

HWND Direct3D_GetWindowHandle();

void	SetDepthTest(bool flg);	//深度テスト切り替え
void Direct3D_Reset();

enum	BLENDSTATE
{
	BLENDSTATE_NONE = 0,	//ブレンドしない
	BLENDSTATE_ALFA,		//普通のαブレンド
	BLENDSTATE_ADD,			//加算合成 
	BLENDSTATE_SUB,			//減算合成

	BLENDSTATE_MAX
};
void SetBlendState(BLENDSTATE blend);


//ブロック縦横配列サイズ
#define BLOCK_COLS		(6)	//ブロックスタック横の数
#define BLOCK_ROWS		(13)//ブロックスタック縦の数

//ブロックサイズ
#define		BLOCK_WIDTH		(50.0f)
#define		BLOCK_HEIGHT	(50.0f)

//スクロール値
#define		POSITION_OFFSET_X	(490.0f)
#define		POSITION_OFFSET_Y	(34.0f)


////////////////////////////////////////////////////


using namespace DirectX;
struct Vertex3D
{
	XMFLOAT3 position; // 頂点座標  //XMFLOAT3へ変更
	XMFLOAT3 normal;
	XMFLOAT4 color;		//頂点カラー（R,G,B,A）
	XMFLOAT2 texCoord;	//テクスチャ座標
	UINT boneIndex[4];
	float boneWeight[4];
};

class LIGHT
{
	public:
		BOOL	Enable;			//ライティングスイッチ
		BOOL	dummy[3];
		XMFLOAT4	Direction;//光の方向
		XMFLOAT4	Diffuse;	//光の色
		XMFLOAT4	Ambient;	//環境光の色
};
class LIGHTOBJECT
{
	public:
		LIGHT	Light;	//ライト構造体
		void	SetEnable(bool b) { Light.Enable = b; }
		void	SetDirection(XMFLOAT4 d) { Light.Direction = d; }
		void	SetDiffuse(XMFLOAT4 d) { Light.Diffuse = d; }
		void	SetAmbient(XMFLOAT4 a) { Light.Ambient = a; }
};





#endif // DIRECT3D_H
