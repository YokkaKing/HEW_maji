/*
* ファイル名	Score.cpp
* タイトル	スコア
* 作成者		かんじぇうぉん
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include "Manager.h"
#include "sprite.h"
#include "keyboard.h"
#include "Score.h"
#include "fade.h"
#include "shader.h"
#include "player.h"
#include "player2.h"
#include "Transform.h"
#include "Audio.h"
#include "Game.h"

#include <random>
#include <cmath>
#include <algorithm>
#include <utility>

//================================================================
//	グローバル変数
//================================================================
static ID3D11ShaderResourceView* g_TextureScore = NULL;
static ID3D11ShaderResourceView* g_TextureRound[3] = { NULL };
static ID3D11ShaderResourceView* g_TextureScoreNumber = NULL;
static ID3D11ShaderResourceView* g_TextureMatch = NULL;

static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

SCORE g_Score;

// 表示用スコア（遅延演出用）
static int   g_ShowWin[2] = { 0, 0 };

// 勝者（Game_GetRoundResult() の戻り値）
// 1=P1勝ち, 2=P2勝ち, 3=引き分け, 0=継続中
static int   g_Winner = 0;

// 「背景が出てから1秒待つ」用
static float g_ScoreWait = 0.0f;
static bool  g_ScoreChangeTriggered = false;

//==================== match演出用 ====================
struct MatchAnim
{
	bool  active = false;

	bool  showP1 = false;   // ★追加
	bool  showP2 = false;   // ★追加

	int   phase = 0;        // 1:左→右出現 2:全体アニメ 3:終了
	float timer = 0.0f;
	float revealT = 0.0f;
	float fullT = 0.0f;
	float alpha = 1.0f;
	float scale = 1.0f;
};
static MatchAnim g_MatchAnim;

// 調整値（60fps基準）
static const float MATCH_REVEAL_SPEED = 0.12f;  // 左→右の速さ


static const float MATCH_FULL_SPEED = 0.08f;  // 全体アニメ進行
// 乱数（必要ならそのまま）
static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);

//==================== 数字アニメ用 ====================
struct ScoreDigitAnim
{
	int current = 0;       // 描画中の数字
	int target = 0;        // 目標の数字
	int prevTarget = 0;    // 前回のtarget（変化検知用）

	float scale = 1.0f;

	// 0:待機 1:縮む(旧数字) 2:ポップ(新数字)
	int state = 0;

	bool popReached = false; // MAX到達したか
};

static ScoreDigitAnim g_Digit[2];

// 調整値（フレームベース）
static const float DIGIT_SHRINK_SPEED = 0.10f;
static const float DIGIT_POP_SPEED = 0.18f;
static const float DIGIT_SETTLE_SPEED = 0.08f;
static const float DIGIT_POP_MAX = 1.35f;

// 60fps想定（deltaTimeが無い環境用）
static const float SCORE_DT = 1.0f / 60.0f;
//======================================================


//================================================================
//	ローカル関数
//================================================================
static void UpdateScoreDigit(ScoreDigitAnim& d, int newTarget)
{
	// 勝利数が「変化した瞬間だけ」アニメ開始
	if (newTarget != d.prevTarget)
	{
		d.target = newTarget;

		// idleのときだけ縮み開始（アニメ中ならtargetだけ更新）
		if (d.state == 0 && d.current != d.target)
		{
			d.state = 1;
		}
	}
	d.prevTarget = newTarget;

	switch (d.state)
	{
	case 0: // idle
		d.scale = 1.0f; // 完全停止
		break;

	case 1: // shrink
		d.scale -= DIGIT_SHRINK_SPEED;
		if (d.scale <= 0.0f)
		{
			d.scale = 0.0f;
			d.current = d.target;   // 0になった瞬間に数字変更
			d.popReached = false;
			d.state = 2;
		}
		break;

	case 2: // pop → settle
		if (!d.popReached)
		{
			// まず大きくする
			d.scale += DIGIT_POP_SPEED;
			if (d.scale >= DIGIT_POP_MAX)
			{
				d.scale = DIGIT_POP_MAX;
				d.popReached = true;
			}
		}
		else
		{
			// その後 1.0 に戻す
			d.scale -= DIGIT_SETTLE_SPEED;
			if (d.scale <= 1.0f)
			{
				d.scale = 1.0f;
				d.state = 0;
				d.popReached = false;
			}
		}
		break;
	}
}
static void StartMatchAnim(int winner)
{
	if (winner != 1 && winner != 2) return;

	g_MatchAnim.active = true;
	g_MatchAnim.phase = 1;
	g_MatchAnim.timer = 0.0f;
	g_MatchAnim.revealT = 0.0f;
	g_MatchAnim.fullT = 0.0f;
	g_MatchAnim.alpha = 1.0f;
	g_MatchAnim.scale = 1.0f;

	// 一旦リセット
	g_MatchAnim.showP1 = false;
	g_MatchAnim.showP2 = false;

	// ==============================
	// 表示先を決める
	// - 1-1 の時は両方表示
	// - それ以外は勝った側だけ表示
	// ==============================
	if (g_ShowWin[0] == 1 && g_ShowWin[1] == 1)
	{
		g_MatchAnim.showP1 = true;
		g_MatchAnim.showP2 = true;
	}
	else
	{
		if (winner == 1) g_MatchAnim.showP1 = true;
		if (winner == 2) g_MatchAnim.showP2 = true;
	}
}

static void UpdateMatchAnim()
{
	if (!g_MatchAnim.active) return;

	switch (g_MatchAnim.phase)
	{
	case 1: // 中央帯を左→右に出現
		g_MatchAnim.revealT += MATCH_REVEAL_SPEED;
		if (g_MatchAnim.revealT >= 1.0f)
		{
			g_MatchAnim.revealT = 1.0f;
			g_MatchAnim.phase = 2;
			g_MatchAnim.timer = 0.0f;
			g_MatchAnim.fullT = 0.0f;
		}
		break;

	case 2: // 全体アニメ（例：少しポップして戻る）
		g_MatchAnim.fullT += MATCH_FULL_SPEED;
		g_MatchAnim.timer += SCORE_DT;

		// ちょいポップ演出（好みで調整）
		// 0→少し大きく→1.0へ戻る
		if (g_MatchAnim.fullT < 0.4f)
		{
			g_MatchAnim.scale = 1.0f + (g_MatchAnim.fullT / 0.4f) * 0.20f; // 最大1.2
		}
		else
		{
			float t = (g_MatchAnim.fullT - 0.4f) / 0.6f;
			if (t > 1.0f) t = 1.0f;
			g_MatchAnim.scale = 1.2f - t * 0.2f; // 1.2 -> 1.0
		}

		if (g_MatchAnim.fullT >= 1.0f)
		{
			g_MatchAnim.fullT = 1.0f;
			g_MatchAnim.scale = 1.0f;
			g_MatchAnim.phase = 3;   // 終了（表示維持）
		}
		break;

	case 3:
		// 表示維持したいなら何もしない
		// しばらくしたら消したいならここで timer加算して active=false にする
		break;
	}
}
static void DrawMatchOne(XMFLOAT2 matchPos)
{
	const float baseMatchW = 230.0f;
	const float baseMatchH = 55.0f;
	XMFLOAT2 matchSize(baseMatchW * g_MatchAnim.scale, baseMatchH * g_MatchAnim.scale);

	g_pContext->PSSetShaderResources(0, 1, &g_TextureMatch);
	SetBlendState(BLENDSTATE_ALFA);

	XMFLOAT4 col = { 1,1,1,g_MatchAnim.alpha };

	if (g_MatchAnim.phase == 1)
	{
		float t = g_MatchAnim.revealT;
		if (t < 0.0f) t = 0.0f;
		if (t > 1.0f) t = 1.0f;

		XMFLOAT2 revealSize(baseMatchW * t, baseMatchH * g_MatchAnim.scale);

		float leftX = matchPos.x - (baseMatchW * 0.5f);
		XMFLOAT2 revealPos(leftX + revealSize.x * 0.5f, matchPos.y);

		DrawSpriteUV(
			revealPos,
			revealSize,
			col,
			0.0f, 0.4f,
			t, 0.6f,-2.0f
		);
	}
	else
	{
		DrawSpriteUV(
			matchPos,
			matchSize,
			col,
			0.0f, 0.0f,
			1.0f, 1.0f,-2.0f
		);
	}
}
//================================================================
//	初期化
//================================================================
void Score_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	std::random_device rd;
	g_Rng.seed(rd());

	TexMetadata metadata;
	ScratchImage image;

	// スコア背景
	LoadFromWICFile(L"asset\\texture\\score.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureScore);
	assert(g_TextureScore);

	// round 1~3
	LoadFromWICFile(L"asset\\texture\\round1.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureRound[0]);
	assert(g_TextureRound[0]);

	LoadFromWICFile(L"asset\\texture\\round2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureRound[1]);
	assert(g_TextureRound[1]);

	LoadFromWICFile(L"asset\\texture\\round3.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureRound[2]);
	assert(g_TextureRound[2]);

	// 数字
	LoadFromWICFile(L"asset\\texture\\scoreNumber.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureScoreNumber);
	assert(g_TextureScoreNumber);

	// match
	LoadFromWICFile(L"asset\\texture\\match.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureMatch);
	assert(g_TextureMatch);

	// 初期化
	for (int i = 0; i < 5; i++)
	{
		g_Score.col[i] = { 1.0f, 1.0f, 1.0f, 0.0f }; // 最初は透明
		g_Score.pos[i] = { 0, 0 };
		g_Score.size[i] = { 0, 0 };
		g_Score.frame[i] = 0.0f;
	}

	// Round画像と数字は見えるように（※これが無いと col が 0 のままで見えない）
	g_Score.col[1] = { 1,1,1,1 };
	g_Score.col[2] = { 1,1,1,1 };
	g_Score.col[3] = { 1,1,1,1 };

	// 勝者を固定（Updateで毎回呼ぶと副作用があるのでInitializeで1回だけ）
	g_Winner = Game_GetRoundResult(); // 1:P1, 2:P2, 3:draw, 0:continue

	// “いまの勝利数”はゲーム側で既に加算済みの可能性があるので、
	// 表示は「増える前」にしたい → 勝った側だけ -1 して 0 0 を作る
	g_ShowWin[0] = GetP1WinCount();
	g_ShowWin[1] = GetP2WinCount();

	if (g_Winner == 1) // P1勝ち
	{
		g_ShowWin[0] -= 1;
		if (g_ShowWin[0] < 0) g_ShowWin[0] = 0;
	}
	else if (g_Winner == 2) // P2勝ち
	{
		g_ShowWin[1] -= 1;
		if (g_ShowWin[1] < 0) g_ShowWin[1] = 0;
	}
	// draw(3) / continue(0) は増やさない（表示はそのまま）

	// 1秒待ちリセット
	g_ScoreWait = 0.0f;
	g_ScoreChangeTriggered = false;

	// 数字アニメ初期化（必ず g_ShowWin の後）
	g_Digit[0].current = g_Digit[0].target = g_Digit[0].prevTarget = g_ShowWin[0];
	g_Digit[1].current = g_Digit[1].target = g_Digit[1].prevTarget = g_ShowWin[1];
	g_Digit[0].scale = g_Digit[1].scale = 1.0f;
	g_Digit[0].state = g_Digit[1].state = 0;
	g_Digit[0].popReached = g_Digit[1].popReached = false;
}


//================================================================
//	終了
//================================================================
void Score_Finalize()
{
	SAFE_RELEASE(g_TextureScore);
	for (int i = 0; i < 3; i++)
	{
		SAFE_RELEASE(g_TextureRound[i]);
	}
	SAFE_RELEASE(g_TextureScoreNumber);
	SAFE_RELEASE(g_TextureMatch);
}


//================================================================
//	更新
//================================================================
void Score_Update()
{
	// 背景アニメ（0→28まで進める）
	if (g_Score.frame[0] <= 28.0f)
	{
		g_Score.frame[0] += 1.0f;
		g_Score.col[0] = { 1.0f, 1.0f, 1.0f, 1.0f }; // 背景は表示
	}
	else
	{
		g_Score.frame[0] = 28.0f;
	}

	// 背景が出た（完全に出た）後に 1秒待ってスコア加算 → アニメ開始
	if (g_Score.frame[0] >= 28.0f)
	{
		if (!g_ScoreChangeTriggered)
		{
			g_ScoreWait += SCORE_DT;

			if (g_ScoreWait >= 1.0f)
			{
				// 1秒後に表示スコアを +1（ここで target が変化 → アニメ発火）
				if (g_Winner == 1)      g_ShowWin[0] += 1; // P1
				else if (g_Winner == 2) g_ShowWin[1] += 1; // P2
				// draw(3) / continue(0) は増やさない
				StartMatchAnim(g_Winner);
				g_ScoreChangeTriggered = true;
			}
		}
	}

	// 数字アニメ更新（※ target は必ず g_ShowWin を基準にする）
	UpdateScoreDigit(g_Digit[0], g_ShowWin[0]);
	UpdateScoreDigit(g_Digit[1], g_ShowWin[1]);
	UpdateMatchAnim();
}


//================================================================
//	描画
//================================================================
void Score_Draw()
{
	Shader_Begin();

	const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
	const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

	Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
		0.0f,
		SCREEN_WIDTH,
		SCREEN_HEIGHT,
		0.0f,
		0.0f,
		1.0f));

	Shader_SetWorldMatrix(XMMatrixIdentity());

	// 背景（score.png）
	g_Score.pos[0] = XMFLOAT2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
	g_Score.size[0] = XMFLOAT2(SCREEN_WIDTH + 50, SCREEN_HEIGHT + 50);

	g_pContext->PSSetShaderResources(0, 1, &g_TextureScore);
	SetBlendState(BLENDSTATE_ALFA);
	DrawSpriteEx(g_Score.pos[0], g_Score.size[0], g_Score.col[0], (int)g_Score.frame[0], 6, 5, -2.0f);

	// Round画像
	g_Score.pos[1] = XMFLOAT2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 120);
	g_Score.size[1] = XMFLOAT2(1062 * 0.5f, 856 * 0.5f);

	// GetRoundCount() が 0~2 の前提（範囲外ならここが原因で落ちるので注意）
	g_pContext->PSSetShaderResources(0, 1, &g_TextureRound[GetRoundCount()-1]);
	SetBlendState(BLENDSTATE_ALFA);
	DrawSpriteEx(g_Score.pos[1], g_Score.size[1], g_Score.col[1], 1, 1, 1, -2.0f);

	// P1数字
	g_Score.pos[2] = XMFLOAT2(SCREEN_WIDTH / 2 - 200, SCREEN_HEIGHT / 2 + 50);

	const float baseSize = 200.0f;
	XMFLOAT2 numSize(baseSize * g_Digit[0].scale, baseSize * g_Digit[0].scale);

	g_pContext->PSSetShaderResources(0, 1, &g_TextureScoreNumber);
	SetBlendState(BLENDSTATE_ALFA);
	DrawSpriteEx(g_Score.pos[2], numSize, g_Score.col[2], g_Digit[0].current, 4, 1, -2.0f);

	// P2数字
	g_Score.pos[3] = XMFLOAT2(SCREEN_WIDTH / 2 + 200, SCREEN_HEIGHT / 2 + 35);

	const float baseSize2 = 200.0f;
	XMFLOAT2 numSize2(baseSize2 * g_Digit[1].scale, baseSize2 * g_Digit[1].scale);

	g_pContext->PSSetShaderResources(0, 1, &g_TextureScoreNumber);
	SetBlendState(BLENDSTATE_ALFA);
	DrawSpriteEx(g_Score.pos[3], numSize2, g_Score.col[3], g_Digit[1].current, 4, 1, -2.0f);

	if (g_MatchAnim.active)
	{
		// 位置（ここで調整する）
		XMFLOAT2 p1MatchPos(g_Score.pos[2].x - 380, g_Score.pos[2].y + 90.0f);
		XMFLOAT2 p2MatchPos(g_Score.pos[3].x + 380, g_Score.pos[3].y + 60.0f);

		// 微調整したいならここで
		// p1MatchPos.x += 10.0f;
		// p2MatchPos.x -= 10.0f;

		if (g_MatchAnim.showP1)
		{
			DrawMatchOne(p1MatchPos);
		}

		if (g_MatchAnim.showP2)
		{
			DrawMatchOne(p2MatchPos);
		}
	}
}
void Score_BeginShow(int roundResult)
{
	// 勝者を保存（Game_GetRoundResult(): 1=P1,2=P2,3=Draw）
	g_Winner = roundResult;
	PlayAudio(g_change, false);
	// 背景アニメを最初から
	g_Score.frame[0] = 0.0f;
	g_Score.col[0] = { 1,1,1,1 };

	// Round/数字は常に見えるように
	g_Score.col[1] = { 1,1,1,1 };
	g_Score.col[2] = { 1,1,1,1 };
	g_Score.col[3] = { 1,1,1,1 };

	// 1秒待ちリセット
	g_ScoreWait = 0.0f;
	g_ScoreChangeTriggered = false;

	// いまの勝利数（※Manager側で既に +1 済み）
	g_ShowWin[0] = GetP1WinCount();
	g_ShowWin[1] = GetP2WinCount();

	// 「増える前」を見せるので勝った側だけ -1
	if (g_Winner == 1)
	{
		g_ShowWin[0] -= 1;
		if (g_ShowWin[0] < 0) g_ShowWin[0] = 0;
	}
	else if (g_Winner == 2)
	{
		g_ShowWin[1] -= 1;
		if (g_ShowWin[1] < 0) g_ShowWin[1] = 0;
	}

	// Digitを表示スコアに合わせて初期化（これがないとアニメが発火しない）
	g_Digit[0].current = g_Digit[0].target = g_Digit[0].prevTarget = g_ShowWin[0];
	g_Digit[1].current = g_Digit[1].target = g_Digit[1].prevTarget = g_ShowWin[1];
	g_Digit[0].scale = g_Digit[1].scale = 1.0f;
	g_Digit[0].state = g_Digit[1].state = 0;
	g_Digit[0].popReached = g_Digit[1].popReached = false;
	// match演出リセット
	g_MatchAnim.active = false;
	g_MatchAnim.showP1 = false;
	g_MatchAnim.showP2 = false;
	g_MatchAnim.phase = 0;
	g_MatchAnim.timer = 0.0f;
	g_MatchAnim.revealT = 0.0f;
	g_MatchAnim.fullT = 0.0f;
	g_MatchAnim.alpha = 1.0f;
	g_MatchAnim.scale = 1.0f;
}

