/*
* ファイル名	SelectMap.cpp
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"Controller.h"
#include"SelectMap.h"
#include"fade.h"
#include"shader.h"
#include "Entry.h"
#include "CameraIntroSequence.h"
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureSelectMap[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureSelectMap_Bg = NULL;
static	ID3D11ShaderResourceView* g_TextureSelectMap_MapName[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureSelectMap_Tips[2] = {NULL};
static	ID3D11ShaderResourceView* g_TextureSelectMap_Button[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureSelectMap_SelectButton = NULL;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
extern Controller g_Controller[2];

void SelectMap_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;
  

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\bg01.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap[0]);
	assert(&g_TextureSelectMap[0]);

    LoadFromWICFile(L"asset\\texture\\bg01.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap[1]);
    assert(&g_TextureSelectMap[1]);

    LoadFromWICFile(L"asset\\texture\\SelectMap_Bg.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_Bg);
    assert(&g_TextureSelectMap_Bg);


    LoadFromWICFile(L"asset\\texture\\choose_button.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_SelectButton);
    assert(&g_TextureSelectMap_SelectButton);

    LoadFromWICFile(L"asset\\texture\\map_grass.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_MapName[0]);
    assert(&g_TextureSelectMap_MapName[0]);

    LoadFromWICFile(L"asset\\texture\\map_lava.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_MapName[1]);
    assert(&g_TextureSelectMap_MapName[1]);

    LoadFromWICFile(L"asset\\texture\\Tip_1.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_Tips[0]);
    assert(&g_TextureSelectMap_Tips[0]);

    LoadFromWICFile(L"asset\\texture\\Tip_2.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_Tips[1]);
    assert(&g_TextureSelectMap_Tips[1]);

    LoadFromWICFile(L"asset\\texture\\L_button2.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_Button[0]);
    assert(&g_TextureSelectMap_Button[0]);

    LoadFromWICFile(L"asset\\texture\\R_button2.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap_Button[1]);
    assert(&g_TextureSelectMap_Button[1]);
}
void SelectMap_Finalize()
{

	SAFE_RELEASE(g_TextureSelectMap_Bg);
    SAFE_RELEASE(g_TextureSelectMap_SelectButton);
    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_TextureSelectMap[i]);
        SAFE_RELEASE(g_TextureSelectMap_Button[i]);
        SAFE_RELEASE(g_TextureSelectMap_Tips[i]);
        SAFE_RELEASE(g_TextureSelectMap_MapName[i]);
    }

}
void SelectMap_Update()
{

}
void SelectMap_Draw()
{
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

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_Bg);
        SetBlendState(BLENDSTATE_ALFA);
        // 色と位置・サイズを設定
        XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT + 200 };
        // 描画
        DrawSprite(pos, size, col);


}

