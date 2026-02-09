/*
* ファイル名	fade.cpp
* タイトル	フェード
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"fade.h"
#include"shader.h"
#include "Audio.h"
#define fADE_HEIGHT (6)
#define fADE_WIDTH (5)
//================================================================
//	グローバル変数
//================================================================
FadeObject	g_Fade;		//フェード処理構造体

static	ID3D11ShaderResourceView* g_Texture = NULL;	//テクスチャ１枚を表すオブジェクト
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

void Fade_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	//テクスチャ読み込み
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\fade.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);//読み込み失敗時にダイアログを表示

	g_Fade.fadecolor.x = 1.0f;
	g_Fade.fadecolor.y = 1.0f;
	g_Fade.fadecolor.z = 1.0f;
	g_Fade.fadecolor.w = 1.0f;
	g_Fade.frame = 0.0f;	//60フレームでフェード完了
	g_Fade.state = FADE_STATE::FADE_NONE;

}

void Fade_Finalize()
{
	if (g_Texture != NULL)
	{
		g_Texture->Release();
		g_Texture = NULL;
	}
}

void Fade_Update()
{
}

void Fade_Draw()
{ 
	//現在の状況
	switch (g_Fade.state)
	{
		case FADE_STATE::FADE_NONE:
			return;
		case FADE_STATE::FADE_IN:
			if (g_Fade.frame < 0.0f)
			{//フェードイン終了
				g_Fade.frame = 0.0f;
				g_Fade.state = FADE_STATE::FADE_NONE;
			}
			break;
		case FADE_STATE::FADE_OUT:
			if (g_Fade.frame >= 29.0f)
			{//フェードアウト終了
				g_Fade.fadecolor.w = 29.0f;
				//フェードイン初期化
				SetFade(g_Fade.frame, g_Fade.fadecolor, FADE_STATE::FADE_IN, g_Fade.scene);
				//シーン切り替え
				SetScene(g_Fade.scene);
			}
			break;
	}

	//スプライト表示

//---------------------------------------------------
	// シェーダーを描画パイプラインに設定
	Shader_Begin();

	// 画面サイズ取得
	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	// 頂点シェーダーに変換行列を設定
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));
	//---------------------------------------------------

	//テクスチャをセット
	g_pContext->PSSetShaderResources(0, 1, &g_Texture);

	//スプライト描画
	SetBlendState(BLENDSTATE_ALFA);//αブレンド
	XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	XMFLOAT2 size = { SCREEN_WIDTH*1.8f, SCREEN_HEIGHT*1.8f };
	DrawSpriteEx(pos, size, g_Fade.fadecolor,g_Fade.frame,fADE_WIDTH,fADE_HEIGHT,-25.0f);

	//フェード処理
	switch (g_Fade.state)
	{
		case FADE_STATE::FADE_IN:
			g_Fade.frame -= (1.0f/4.0f);//透明にしていく
			break;
		case FADE_STATE::FADE_OUT:
			g_Fade.frame += (1.0f / 4.0f);//不透明にしていく
			break;
	}

}

void	SetFade(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene)
{ 
	g_Fade.state = state;
	g_Fade.scene = scene;

	if (g_Fade.state == FADE_IN)
	{
		g_Fade.frame = 28.0f;	//不透明にする

	}
	else
	{
		g_Fade.frame = 0.0f;	//透明にする
		if (GetRoundCount() == 0)
		{
			StopAudio(g_round1);
		}
		else
		{
			StopAudio(g_round2);
		}
		PlayAudio(g_fade);
	}


}

FADE_STATE	GetFadeState()
{
	return	g_Fade.state;	//現在の状態
}


