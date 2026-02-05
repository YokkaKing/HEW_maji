/*
* ファイル名	SELECT_TRANSFORM_UI.cpp
* タイトル	タイトル
* 作成者		カンジェウォン
* 作成日		02月05日
* 更新日		02月05日
*/

//================================================================
//	インクルード
//================================================================
#include "Select_Transform_Ui.h"
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"fade.h"
#include"shader.h"
#include "player.h"
#include "player2.h"
#include <random>
#include <cmath>
#include <algorithm>
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureBg[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;
static	ID3D11ShaderResourceView* g_TextureTimer[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureButton = NULL;
//================================================================
//	初期化
//================================================================
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
SELECT_TRANSFORM_UI g_Ui;
SELECT_TRANSFORM_TIMER g_Timer;
static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);
void Select_Weapon_Ui_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    std::random_device rd;
    g_Rng.seed(rd());

    TexMetadata		metadata;
    ScratchImage	image;
    LoadFromWICFile(L"asset\\texture\\Transform_Timer.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTimer[0]);
    assert(&g_TextureTimer[0]);

    LoadFromWICFile(L"asset\\texture\\Transform_Timer2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTimer[1]);
    assert(&g_TextureTimer[1]);

    LoadFromWICFile(L"asset\\texture\\Transform_Select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBg[0]);
    assert(&g_TextureBg[0]);

    LoadFromWICFile(L"asset\\texture\\Transform_Select2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBg[1]);
    assert(&g_TextureBg[1]);

    LoadFromWICFile(L"asset\\texture\\number.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureNumber);
    assert(&g_TextureNumber);

    LoadFromWICFile(L"asset\\texture\\choose_button.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureButton);
    assert(&g_TextureButton);



	g_Ui.col = { 1.0f, 1.0f, 1.0f, 1.0f };
	g_Ui.pos = { 0, 0 };
	g_Ui.size = { 1920,1080 };
	g_Ui.isUsed = false;   
    g_Ui.SelectNum = 0;

    
	g_Timer.col = { 1.0f, 1.0f, 1.0f, 1.0f };
	g_Timer.pos = { 0, 0 };
	g_Timer.size = { 300, 120 };
	g_Timer.time = 20.0f;
	g_Timer.frame = 0.0f;


}
void Select_Weapon_Ui_Finalize()
{
    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_TextureBg[i]);
		SAFE_RELEASE(g_TextureTimer[i]);
    }
    SAFE_RELEASE(g_TextureNumber);
    SAFE_RELEASE(g_TextureButton);
}
void Select_Weapon_Ui_Update()
{
    if (g_Ui.isUsed)
    {


    }

}
void Select_Weapon_Ui_Draw()
{
    if (g_Ui.isUsed)
    {
        Shader_Begin();
        const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
        const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
        g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2, 100);
        Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
            0.0f,
            SCREEN_WIDTH,
            SCREEN_HEIGHT,
            0.0f,
            0.0f,
            1.0f));
        Shader_SetWorldMatrix(XMMatrixIdentity());
        //タイマー描画
        g_pContext->PSSetShaderResources(0, 1, &g_TextureTimer[g_Ui.SelectNum]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(g_Timer.pos, g_Timer.size, g_Timer.col);

        g_Ui.pos = XMFLOAT2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2-25);
        g_Ui.size = XMFLOAT2(SCREEN_WIDTH, SCREEN_HEIGHT+50);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureBg[g_Ui.SelectNum]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(g_Ui.pos, g_Ui.size, g_Ui.col);

    
        g_pContext->PSSetShaderResources(0, 1, &g_TextureButton);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(XMFLOAT2(SCREEN_WIDTH/2-100,SCREEN_HEIGHT -100), XMFLOAT2(263,100), g_Ui.col);

        //数字描画
        int time[2];
        time[0] = (int)(g_Timer.time) / 10 - 1;
        time[1] = (int)(g_Timer.time) % 10 - 1;
		float timerScale = 156* 0.8f;
        for (int i = 0; i < 2; i++)
        {
            g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2 - timerScale/4 + i * timerScale/2, 100);
            g_pContext->PSSetShaderResources(0, 1, &g_TextureNumber);
            SetBlendState(BLENDSTATE_ALFA);
            DrawSpriteEx(g_Timer.pos, XMFLOAT2(timerScale, timerScale), g_Timer.col, time[i], 10, 1);
        }
    }
   
}

void SetTransformUi_IsUsed(bool flg)
{
	g_Ui.isUsed = flg;
}
void SetTransformUi_time(float time)
{
	g_Timer.time = time;
}
void SetTransformUi_SelectNum(int num)
{
	g_Ui.SelectNum = num;
}