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
#include "Stage.h"
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
static bool isSelected = false; // マップが選択されたかどうか
extern Controller g_Controller[2];
static int g_selectedMapIndex = 0; // 0: 草原, 1: 溶岩
static float frame = 0; // フレームカウンター
void SelectMap_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;
  

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\stadium_grass.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureSelectMap[0]);
	assert(&g_TextureSelectMap[0]);

    LoadFromWICFile(L"asset\\texture\\stadium_lava.png", WIC_FLAGS_NONE, &metadata, image);
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
    g_selectedMapIndex = 0;
    frame = 0;
    isSelected = false;
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
    int controllerIdx = 0;
    Controller& ctrl = g_Controller[controllerIdx];
    int controllerIdx2 = 1;
    Controller& ctrl2 = g_Controller[controllerIdx2];
    bool isMove = false;
    if (g_selectedMapIndex == 0&&!isMove)
    {
        if (Keyboard_IsKeyDownTrigger(KK_A) || Keyboard_IsKeyDownTrigger(KK_D)||
            ctrl.IsButtonPushed(ControllerButton::L_SHOULDER)|| ctrl.IsButtonPushed(ControllerButton::R_SHOULDER)||
            ctrl2.IsButtonPushed(ControllerButton::L_SHOULDER) || ctrl2.IsButtonPushed(ControllerButton::R_SHOULDER))
        {
            g_selectedMapIndex = 1;
            PlayAudio(g_cursorMove, false);

            isMove = true;
        }
    }
    if (g_selectedMapIndex == 1 && !isMove)
    {
        if (Keyboard_IsKeyDownTrigger(KK_A) || Keyboard_IsKeyDownTrigger(KK_D) ||
            ctrl.IsButtonPushed(ControllerButton::L_SHOULDER) || ctrl.IsButtonPushed(ControllerButton::R_SHOULDER) ||
            ctrl2.IsButtonPushed(ControllerButton::L_SHOULDER) || ctrl2.IsButtonPushed(ControllerButton::R_SHOULDER))
        {
            g_selectedMapIndex = 0;
            PlayAudio(g_cursorMove, false);

            isMove = true;

        }
    }
        
    if (!isSelected&&(Keyboard_IsKeyDown(KK_ENTER)|| g_Controller[controllerIdx].IsButtonPushed(ControllerButton::A_BUTTON)|| g_Controller[controllerIdx2].IsButtonPushed(ControllerButton::A_BUTTON)))
    {
        XMFLOAT4 fadeColor(0.0f, 0.0f, 0.0f, 1.0f);
        SetFade(40.0f, fadeColor, FADE_STATE::FADE_OUT, SCENE_SELECT_WT);
        PlayAudio(g_button, false);

        isSelected = true;
    }
    if (frame <= 29)
    {
        frame++;
    }
    else
    {
        frame = 0;
    }

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

   
        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap[g_selectedMapIndex]);
        SetBlendState(BLENDSTATE_ALFA);
        // 色と位置・サイズを設定
        XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT};
        DrawSprite(pos, size, col);

        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = { SCREEN_WIDTH / 2 + 50, SCREEN_HEIGHT / 2 };
        size = { SCREEN_WIDTH + 100, SCREEN_HEIGHT + 100 };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_Bg);
        SetBlendState(BLENDSTATE_ALFA);

        DrawSprite(pos, size, col);

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_SelectButton);
        SetBlendState(BLENDSTATE_ALFA);
        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = { SCREEN_WIDTH- 250 , SCREEN_HEIGHT  -80};
        size = {263*0.7, 100*0.7 };
        DrawSprite(pos, size, col);

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_MapName[g_selectedMapIndex]);
        SetBlendState(BLENDSTATE_ALFA);
        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = {  300 , SCREEN_HEIGHT/2};
        size = { 827*0.8, 411*0.8 };
        DrawSprite(pos, size, col);

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_Tips[g_selectedMapIndex]);
        SetBlendState(BLENDSTATE_ALFA);
        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = { SCREEN_WIDTH - 300 , 300 };
        size = { 623*0.8, 410*0.8 };
        DrawSprite(pos, size, col);

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_Button[0]);
        SetBlendState(BLENDSTATE_ALFA);
        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = { SCREEN_WIDTH/2 - 800 , SCREEN_HEIGHT-200 };
        size = { 300*0.8, 300*0.8 };
        DrawSpriteEx(pos, size, col, frame, 6, 5);

        g_pContext->PSSetShaderResources(0, 1, &g_TextureSelectMap_Button[1]);
        SetBlendState(BLENDSTATE_ALFA);
        col = { 1.0f, 1.0f, 1.0f, 1.0f };
        pos = { SCREEN_WIDTH/2 + 800 , SCREEN_HEIGHT - 200 };
        size = { 300*0.8, 300*0.8 };
        DrawSpriteEx(pos, size, col,(int)frame,6,5);
}
int GetSelectedMapIndex()
{
	return g_selectedMapIndex;
}
