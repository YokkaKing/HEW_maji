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
static	ID3D11ShaderResourceView* g_TextureTipBg = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_TextureTip = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_TextureLoading = NULL;	//テクスチャ１枚を表すオブジェクト

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
static bool g_FadeOutOnly = false;
void Fade_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	//テクスチャ読み込み
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\fade.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture);
	assert(g_Texture);

	LoadFromWICFile(L"asset\\texture\\selectBg_2.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTipBg);
	assert(g_TextureTipBg);

	LoadFromWICFile(L"asset\\texture\\tips.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTip);
	assert(g_TextureTip);

	LoadFromWICFile(L"asset\\texture\\nowLoading.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureLoading);
	assert(g_TextureLoading);


	g_Fade.fadecolor.x = 1.0f;
	g_Fade.fadecolor.y = 1.0f;
	g_Fade.fadecolor.z = 1.0f;
	g_Fade.fadecolor.w = 1.0f;
	g_Fade.frame[0] = 0.0f;
	g_Fade.frame[1] = 0.0f;
	g_Fade.frame[2] = 0.0f;
	g_Fade.col = { 1.0f,1.0f,1.0f,0.0f };
	g_Fade.state = FADE_STATE::FADE_NONE;
	g_FadeOutOnly = false;
}

void Fade_Finalize()
{
	if (g_Texture != NULL)
	{
		g_Texture->Release();
		g_Texture = NULL;
	}
	if (g_TextureTipBg != NULL)
	{
		g_TextureTipBg->Release();
		g_TextureTipBg = NULL;
	}
	if (g_TextureTip != NULL)
	{
		g_TextureTip->Release();
		g_TextureTip = NULL;
	}
	if (g_TextureLoading != NULL)
	{
		g_TextureLoading->Release();
		g_TextureLoading = NULL;
	}

}

void Fade_Update()
{
	if (g_Fade.state == FADE_STATE::FADE_NONE)
		return;


	// フェード速度（いま Draw でやってた 1/4 をここへ）
	const float STEP = (1.0f / 2.0f);

	switch (g_Fade.state)
	{
	case FADE_STATE::FADE_IN:
		
		g_Fade.frame[0] -= STEP;  // 透明へ向かう
		g_Fade.col = { 1.0f,1.0f,1.0f,0.0f };

		if (g_Fade.frame[0] <= 0.0f)
		{
			g_Fade.frame[0] = 0.0f;
			g_Fade.state = FADE_STATE::FADE_NONE;
		}
		break;

	case FADE_STATE::FADE_OUT:
		g_Fade.frame[0] += STEP;
		
		
		if (GetScene() == SCENE_SELECT_WT)
		{
			g_Fade.col = { 1.0f,1.0f,1.0f,1.0f };
			if (g_Fade.frame[1] < 59)
			{
				g_Fade.frame[1] += 2.0f;
			}
			else
			{
				g_Fade.frame[1] = 59.0f;
			}
			if (g_Fade.frame[2] < 29)
			{
				g_Fade.frame[2] += 2.0f;
			}
			else
			{
				g_Fade.frame[2] = 0.0f;
			}
		}
		

		if (g_Fade.frame[0] >= 29.0f)
		{
			// フェードアウト完了
			g_Fade.frame[0] = 29.0f;

			// 先にシーン切り替え
			SetScene(g_Fade.scene);

			if (g_FadeOutOnly)
			{
				// 今回は FADE_IN に行かない（Result -> TeamLogo 用）
				g_Fade.state = FADE_STATE::FADE_NONE;
				g_FadeOutOnly = false; // 次回のために戻す
			}
			else
			{
				// 通常挙動（今まで通り）
				g_Fade.state = FADE_STATE::FADE_IN;
				g_Fade.frame[0] = 28.0f;
			}
		}
		break;
	default:
		break;
	}
}


void Fade_Draw()
{
	if (g_Fade.state == FADE_STATE::FADE_NONE)
		return;

	Shader_Begin();

	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f, SCREEN_WIDTH,
		SCREEN_HEIGHT, 0.0f,
		0.0f, 1.0f));

	g_pContext->PSSetShaderResources(0, 1, &g_Texture);
	SetBlendState(BLENDSTATE_ALFA);

	XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
	XMFLOAT2 size = { SCREEN_WIDTH * 1.8f, SCREEN_HEIGHT * 1.8f };

	DrawSpriteEx(pos, size, g_Fade.fadecolor, g_Fade.frame[0], fADE_WIDTH, fADE_HEIGHT, -25.0f);


	if (GetScene() == SCENE_SELECT_WT)
	{
		g_pContext->PSSetShaderResources(0, 1, &g_TextureTipBg);
		SetBlendState(BLENDSTATE_ALFA);
		pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
		size = { SCREEN_WIDTH , SCREEN_HEIGHT };
		DrawSprite(pos, size, g_Fade.col);

		g_pContext->PSSetShaderResources(0, 1, &g_TextureTip);
		SetBlendState(BLENDSTATE_ALFA);
		DrawSpriteEx(pos, size, g_Fade.col, g_Fade.frame[1], 8, 8);

		g_pContext->PSSetShaderResources(0, 1, &g_TextureLoading);
		SetBlendState(BLENDSTATE_ALFA);
		DrawSpriteEx(pos, size, g_Fade.col, g_Fade.frame[1], 6, 5);
	}


}


void SetFade(int fadeframe, XMFLOAT4 color, FADE_STATE state, SCENE scene)
{
	g_Fade.state = state;

	g_Fade.scene = scene;
	if (g_Fade.state == FADE_IN)
	{
		g_Fade.frame[0] = 28.0f; // 黒→透明
	}
	else if (g_Fade.state == FADE_OUT)
	{
		g_Fade.frame[0] = 0.0f;  // 透明→黒

		if (GetRoundCount() == 1) StopAudio(g_round1);
		else                      StopAudio(g_round2);

		PlayAudio(g_fade);
	}
}


FADE_STATE	GetFadeState()
{
	return	g_Fade.state;	//現在の状態
}
void SetFadeOutOnly(int fadeframe, XMFLOAT4 color, SCENE scene)
{
	g_FadeOutOnly = true;
	SetFade(fadeframe, color, FADE_OUT, scene);
}

