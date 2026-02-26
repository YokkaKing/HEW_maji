/*
* ファイル名	TeamLogo.cpp
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
#include"TeamLogo.h"
#include"fade.h"
#include"shader.h"
#include "Entry.h"
#include "CameraIntroSequence.h"

#define TEAMLOGO_YOKO (7)
#define TEAMLOGO_TATE (7)

//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_Texture_TeamLogo = NULL;	//テクスチャ１枚を表すオブジェクト
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
extern Controller g_Controller[2];
static float frame = 0.0f;  
static float timer = 0.0f; // タイマー変数

void TeamLogo_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;
 

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\team_Logo_sprite.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_TeamLogo);
	assert(&g_Texture_TeamLogo);//読み込み失敗時にダイアログを表示
    frame = 0.0f;

}
void TeamLogo_Finalize()
{
	SAFE_RELEASE(g_Texture_TeamLogo);
}
void TeamLogo_Update()
{ 
	 // フレームを進める（速度は調整可能）
    if (frame == 30)
    {
        PlayAudio(g_teamLogo, false);
    }
    if (frame >= (TEAMLOGO_TATE * TEAMLOGO_YOKO) - 1)
    {
        timer++;
        if (timer >= 100)
        {
			SetScene(SCENE_TITLE);
			timer = 0.0f; // タイマーをリセット
			frame = 0.0f; // フレームをリセット
        }
    }
    else
    {
        frame += 0.5f;
    }
}
void TeamLogo_Draw()
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

    g_pContext->PSSetShaderResources(0, 1, &g_Texture_TeamLogo);
    SetBlendState(BLENDSTATE_ALFA);
    
    XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
    // 描画
    DrawSpriteEx(pos, size, col,(int)frame,TEAMLOGO_YOKO,TEAMLOGO_TATE);


 
}

