/*
* ファイル名	Title.cpp
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
#include"Title.h"
#include"fade.h"
#include"shader.h"

//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_Texture = NULL;	//テクスチャ１枚を表すオブジェクト
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
extern Controller g_Controller[2];

void Title_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\Title.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);//読み込み失敗時にダイアログを表示

	//フェードインのセット
	XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(60.0f, color, FADE_IN, SCENE_SELECT_WT);
    PlayAudio(g_title, true);
}
void Title_Finalize()
{
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture);

}
void Title_Update()
{ 
	//キー入力チェック
	//スタートボタンが押されたらシーンを切り替え
	//フェード処理中はキーを受け付けない
    if ((Keyboard_IsKeyDownTrigger(KK_ENTER) || g_Controller[0].IsButtonPushed(ControllerButton::A_BUTTON)) && (GetFadeState() == FADE_NONE))
	{
        PlayAudio(g_fade, false);
		//フェードアウトさせてシーンを切り替える
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_SELECT_WT);
	}

}
void Title_Draw()
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

    // テクスチャをセット
    g_pContext->PSSetShaderResources(0, 1, &g_Texture);

    // BlendState 設定
    SetBlendState(BLENDSTATE_NONE);

    // 色と位置・サイズを設定
    XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
    XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
    XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };

    // 描画
    DrawSprite(pos, size, col);

}

