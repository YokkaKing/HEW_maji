/*
* ファイル名	Number.cpp
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"

#include"fade.h"
#include"shader.h"
#include "number.h"
//================================================================
//	マクロ定義
//================================================================
#define NUMBER_MAX (2)
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_Texture = NULL;	//テクスチャ１枚を表すオブジェクト
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static float time;

void Number_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\number.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);//読み込み失敗時にダイアログを表示

	//フェードインのセット
	XMFLOAT4	color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    time = 60.0f;

}
void Number_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);

}
void Number_Update()
{ 
    float frame = 1 / 60.0f;
    time -= frame;
}
void Number_Draw()
{
    int PatNo[2] = { 0,0 };
    int temp = (int)time;

    PatNo[0] = temp / 10;
    PatNo[1] = temp % 10;
   
    // シェーダーを描画パイプラインに設定
    Shader_Begin();

    // 画面サイズ取得
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    // 頂点シェーダーに正射影行列を設定
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0.0f,
        0.0f,
        1.0f));

    // World 行列は単位行列を設定
    Shader_SetWorldMatrix(XMMatrixIdentity());

    // テクスチャをセット
    g_pContext->PSSetShaderResources(0, 1, &g_Texture);
    // 色と位置・サイズを設定
    XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMFLOAT2 pos = { SCREEN_WIDTH / 2-40, 90 };
    XMFLOAT2 size = { 150, 80 };
    // BlendState 設定
    SetBlendState(BLENDSTATE_ALFA);
    for (int i = 0; i < NUMBER_MAX; i++)
    {

        // 描画
        DrawSpriteEx(pos, size, col, PatNo[i], 5, 2);
		pos.x += size.x-42; // 次の数字の位置調整
    }

}


