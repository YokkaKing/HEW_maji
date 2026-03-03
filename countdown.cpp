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
#include "Manager.h"
#include <cmath>


//================================================================
//	グローバル変数
//================================================================
static ID3D11ShaderResourceView* g_TextureCountdown = NULL;
static ID3D11ShaderResourceView* g_TextureStart = NULL;
static ID3D11ShaderResourceView* g_TextureFinish = NULL;

// start countdown
static bool  g_startActive = false;
static float g_startRemain = 0.0f; // sec remaining
static float g_startTotal = 0.0f;
static bool  g_startMsgActive = false;
static float g_startMsgRemain = 0.0f;
static const float g_startMsgDur = 0.85f;

// anim per label
static int   g_prevKey = -99999;
static float g_animT = 1.0f;      // 0..1
static float g_animDur = 0.45f;   // 速いなら増やす(0.35~0.60くらい)
static bool g_isPlayed = false;
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

	LoadFromWICFile(L"asset\\texture\\START.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureStart);
	assert(g_TextureStart);

	LoadFromWICFile(L"asset\\texture\\FINISH.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureFinish);
	assert(g_TextureFinish);
	g_isPlayed = false;

}

void CountdownUI_Finalize()
{
	SAFE_RELEASE(g_TextureCountdown);
	SAFE_RELEASE(g_TextureStart);
	SAFE_RELEASE(g_TextureFinish);
}

void CountdownUI_Start(float seconds)
{
	g_startActive = true;
	g_startRemain = seconds;
	g_startTotal = seconds;
	g_startMsgActive = false;
	g_startMsgRemain = 0.0f;

	g_prevKey = -99999;
	g_animT = 1.0f;
	g_animDur = 0.45f;
}

bool CountdownUI_IsBlockingGameplay()
{
	return (g_startActive || g_startMsgActive);
}

void CountdownUI_Update(float dt)
{
	// anim progress
	if (g_animT < 1.0f)
	{
		g_animT += (g_animDur <= 0.0f) ? 1.0f : (dt / g_animDur);
		if (g_animT > 1.0f) g_animT = 1.0f;
	}

	if (g_startActive)
	{
		g_startRemain -= dt;

		float elapsed = g_startTotal - g_startRemain;

		// 3秒経過したら 3,2,1 は終了 → STARTへ
		if (elapsed >= 3.0f)
		{
			g_startActive = false;

			if (!g_isPlayed)
			{
				if (GetRoundCount() == 0) PlayAudio(g_round1, false);
				else                      PlayAudio(g_round2, false);
				g_isPlayed = true; 
			}

			g_startMsgActive = true;
			g_startMsgRemain = g_startMsgDur;

			// STARTアニメを確実に最初から
			g_prevKey = -99999;
		}
	}
	if (g_startMsgActive)
	{
		g_startMsgRemain -= dt;
		if (g_startMsgRemain <= 0.0f)
		{
			g_startMsgRemain = 0.0f;
			g_startMsgActive = false;
		}
	}
}

// 4秒スタートの表示ラベル：3,2,1,GO(0)
// seconds remain -> label
int CountdownUI_GetStartLabel()
{
	// 経過時間で 3→2→1 を各1秒に固定
	float elapsed = g_startTotal - g_startRemain;     // 0,1,2,3...
	int step = (int)floorf(elapsed);                  // 0..3

	int label = 3 - step;                             // 3,2,1,0
	if (label > 3) label = 3;
	if (label < 1) label = 1;                         // 描画は1以上だけ
	return label;
}

int CountdownUI_GetEndLabel(float remain)
{
	int sec = (int)remain;
	if (sec < 1) sec = 1;
	if (sec > 4) sec = 5;
	return sec; // 5..1
}

static void DrawTextureZoom(ID3D11ShaderResourceView* tex, int bno, int wc, int hc,
	bool rotate, float baseW, float baseH, float fromScale)
{
	if (!tex) return;
	const float sw = (float)Direct3D_GetBackBufferWidth();
	const float sh = (float)Direct3D_GetBackBufferHeight();

	float eScale = EaseCos01(g_animT);
	float scale = fromScale + (0.7f - fromScale) * eScale;

	float rad = 0.0f;
	if (rotate)
	{
		float rotT = Clamp01(g_animT * 2.0f);
		float eRot = EaseCos01(rotT);
		rad = (XM_2PI * 2.0f) * (1.0f - eRot); // 2 turns -> 0
	}

	XMFLOAT2 center(sw * 0.5f, sh * 0.5f);
	XMFLOAT2 size(baseW * scale, baseH * scale);
	float a = EaseCos01(g_animT);      // 0→1
	XMFLOAT4 col(1, 1, 1, a);

	Shader_Begin();
	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, sw, sh, 0.0f, 0.0f, 1.0f));
	Shader_SetWorldMatrix(XMMatrixIdentity());
	ID3D11DeviceContext* ctx = Direct3D_GetDeviceContext();
	ctx->PSSetShaderResources(0, 1, &tex);
	SetBlendState(BLENDSTATE_ALFA);

	DrawSpriteExRotation(center, size, col, bno, wc, hc, rad);
}

void CountdownUI_DrawLabel(int label)
{
	if (!g_TextureCountdown) return;
	if (label < 1) return;

	// key = 1..5 (digit)
	const int key = label;
	if (key != g_prevKey)
	{
		g_prevKey = key;
		g_animT = 0.0f;
		g_animDur = 0.45f;

		// SE (start countdown only)
		if (g_drawMode == CountdownDrawMode::Start)
		{
			if (label == 3 || label == 2 || label == 1)
			{
				PlayAudio(g_countdown, false);
			}
		}
	}

	// digits are drawn from countdown.png (5 columns x 1 row)
	const float sw = (float)Direct3D_GetBackBufferWidth();
	const float sh = (float)Direct3D_GetBackBufferHeight();
	const float m = (sw < sh) ? sw : sh;

	DrawTextureZoom(
		g_TextureCountdown,
		label - 1, // sheet index
		5, 1,
		true,
		m * 1.2f, m * 1.2f,
		0.15f);
}

static void CountdownUI_DrawStartMessage()
{
	if (!g_TextureStart) return;

	const int key = 100;
	if (key != g_prevKey)
	{
		g_prevKey = key;
		g_animT = 0.0f;
		g_animDur = 0.30f;

		// Optional: play GO/START sound here if you want
		// PlayAudio(g_go, false);
	}
	
	const float sw = (float)Direct3D_GetBackBufferWidth();
	const float sh = (float)Direct3D_GetBackBufferHeight();
	DrawTextureZoom(
		g_TextureStart,
		0, 1, 1,
		false,
		sw, sh,
		0.05f);
}

static void CountdownUI_DrawFinishMessage()
{
	if (!g_TextureFinish) return;

	const int key = 200;
	if (key != g_prevKey)
	{
		g_prevKey = key;
		g_animT = 0.0f;
		g_animDur = 0.35f;
	}

	const float sw = (float)Direct3D_GetBackBufferWidth();
	const float sh = (float)Direct3D_GetBackBufferHeight();
	DrawTextureZoom(g_TextureStart, 0, 1, 1, false, sw, sh, 2.5f);
	DrawTextureZoom(g_TextureFinish, 0, 1, 1, false, sw, sh, 2.5f);
}

void CountdownUI_DrawStart()
{
	g_drawMode = CountdownDrawMode::Start;

	if (g_startMsgActive)
	{
		CountdownUI_DrawStartMessage();
		return;
	}

	if (!g_startActive) return;

	CountdownUI_DrawLabel(CountdownUI_GetStartLabel());
}

void CountdownUI_DrawEnd(float remain)
{
	g_drawMode = CountdownDrawMode::End;

	if (remain <= 0.0f)
	{
		CountdownUI_DrawFinishMessage();
		return;
	}

	CountdownUI_DrawLabel(CountdownUI_GetEndLabel(remain));
}