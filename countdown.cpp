/*
* ファイル名	countdown.cpp
* タイトル	カウント
* 作成者		カンジェウォン
* 作成日		02/07
* 更新日		02/07
*/


//================================================================
//	インクルード
//================================================================
#include "countdown.h"
#include "sprite.h"
#include "shader.h"
#include "Audio.h"
#include <cmath>


//================================================================
//	グローバル変数
//================================================================
static ID3D11ShaderResourceView* g_TextureCountdown = NULL;
// start countdown
static bool  g_startActive = false;
static float g_startRemain = 0.0f; // sec remaining
// anim per label
static int   g_prevLabel = -999;
static float g_animT = 1.0f;      // 0..1
static float g_animDur = 0.45f;   // 速いなら増やす(0.35~0.60くらい)
// utils
static inline float Clamp01(float v) { return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); }
static inline float EaseCos01(float t) { t = Clamp01(t); return 0.5f - 0.5f * cosf(t * XM_PI); }

enum class CountdownDrawMode
{
	Start,
	End,
};

static CountdownDrawMode g_drawMode = CountdownDrawMode::Start;

void CountdownUI_Initialize(ID3D11Device* pDevice)
{
	TexMetadata metadata;
	ScratchImage image;

	// ★ 1..5 のスプライトシートをここに置く
	LoadFromWICFile(L"asset\\texture\\countdown.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureCountdown);
	assert(g_TextureCountdown);
}

void CountdownUI_Finalize()
{
	SAFE_RELEASE(g_TextureCountdown);
}

void CountdownUI_Start(float seconds)
{
	g_startActive = true;
	g_startRemain = seconds;

	g_prevLabel = -999;
	g_animT = 1.0f;
}

bool CountdownUI_IsBlockingGameplay()
{
	return g_startActive;
}

void CountdownUI_Update(float dt)
{
	// anim progress
	if (g_animT < 1.0f)
	{
		g_animT += (g_animDur <= 0.0f) ? 1.0f : (dt / g_animDur);
		if (g_animT > 1.0f) g_animT = 1.0f;
	}

	// start countdown tick
	if (g_startActive)
	{
		g_startRemain -= dt;
		if (g_startRemain <= 0.0f)
		{
			g_startRemain = 0.0f;
			g_startActive = false;
		}
	}
}

// 4秒スタートの表示ラベル：3,2,1,GO(0)
// seconds remain -> label
int CountdownUI_GetStartLabel()
{
	// 表示を 3,2,1,GO にしたいので
	// remain 4.0~3.0 -> 3
	// remain 3.0~2.0 -> 2
	// remain 2.0~1.0 -> 1
	// remain 1.0~0.0 -> GO(0)
	int sec = (int)ceilf(g_startRemain);
	if (sec >= 4) return 3;
	if (sec == 3) return 2;
	if (sec == 2) return 1;
	return 0; // GO
}

int CountdownUI_GetEndLabel(float remain)
{
	int sec = (int)remain;
	if (sec < 1) sec = 1;
	if (sec > 4) sec = 5;
	return sec; // 5..1
}

void CountdownUI_DrawLabel(int label)
{
	if (!g_TextureCountdown) return;

	// label changed => restart anim
	if (label != g_prevLabel)
	{
		g_prevLabel = label;
		g_animT = 0.0f;

		//========================
		// SE（開始カウントのみ）
		//========================
		if (g_drawMode == CountdownDrawMode::Start)
		{
			if (label == 3 || label == 2 || label == 1)
			{
			
				PlayAudio(g_countdown, false);
			}

			
			 if (label == 0)PlayAudio(g_go, false);
		}
	
	}

	// GO is not yet available
	if (label == 0)
	{
		// TODO: GO用の画像を追加したら、ここで描画する
		// 例:
		// ctx->PSSetShaderResources(0, 1, &g_TextureGo);
		// DrawSprite(...);
		return;
	}

	float e = EaseCos01(g_animT);

	// 画面いっぱい級：最終サイズは min(幅,高さ)*1.2
	const float sw = (float)Direct3D_GetBackBufferWidth();
	const float sh = (float)Direct3D_GetBackBufferHeight();
	const float m = (sw < sh) ? sw : sh;
	float eScale = EaseCos01(g_animT);

	float rotT = Clamp01(g_animT * 2.0f);
	float eRot = EaseCos01(rotT);

	// 2回転：-4π -> 0
	float rad = (XM_2PI * 2.0f) * (1.0f - eRot);
	float scale = 0.15f + (1.0f - 0.15f) * eScale;
	// 中央にドン
	XMFLOAT2 center(sw * 0.5f, sh * 0.5f);

	// ベースサイズ（ほぼ画面埋める）
	XMFLOAT2 baseSize(m * 1.2f, m * 1.2f);
	XMFLOAT2 size(baseSize.x * scale, baseSize.y * scale);

	XMFLOAT4 col(1, 1, 1, 1);

	Shader_Begin();
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, sw, sh, 0.0f, 0.0f, 1.0f));
	Shader_SetWorldMatrix(XMMatrixIdentity()); // DrawSpriteExRotation は頂点側で回転するのでOK

	ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();
	ctx->PSSetShaderResources(0, 1, &g_TextureCountdown);
	SetBlendState(BLENDSTATE_ALFA);

	// sprite sheet: 1 row, 5 columns (1..5) => index 0..4
	DrawSpriteExRotation(center, size, col, label - 1, 5, 1, rad);
}
void CountdownUI_DrawStart()
{
	g_drawMode = CountdownDrawMode::Start;
	CountdownUI_DrawLabel(CountdownUI_GetStartLabel());
}
void CountdownUI_DrawEnd(float remain)
{
	g_drawMode = CountdownDrawMode::End;
	CountdownUI_DrawLabel(CountdownUI_GetEndLabel(remain));
}