/*
* ファイル名	Manager.cpp
* タイトル	マネージャー
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"direct3d.h"
#include"Manager.h"
#include"keyboard.h"
#include "Controller.h"
#include"Game.h"
#include"Title.h"
#include "Entry.h"
#include"Result.h"
#include "Score.h"
#include "Player.h"
#include "Player2.h"
#include"fade.h"
#include"selectWeaponTerrain.h"
#include "Result_Ui.h"
#include "ResultSystem.h"
#include "selectWeaponUi3D.h"
//================================================================
//	グローバル変数
//================================================================
static	SCENE	g_Scene = SCENE_NONE;	//現在のシーン番号
static  inGameWTselect g_currentWTselect; //シーン間で保持する選択した武器・地形データ

static int  g_RoundCount = 0;     // 現在のラウンド数
static int  g_P1Wins = 0;         // 1P勝利数
static int  g_P2Wins = 0;         // 2P勝利数
static bool g_roundResultLocked = false; // 決着を1回だけ処理するため
static int g_lastRoundResult = 0;
static bool g_deathUseSlow = true; // true=STOP→SLOW→SCORE, false=STOP→SCORE
static void StartDeathSequence(SCENE nextScene);
static void StartDeathSequence(SCENE nextScene, bool useSlow);
//================================================================
//  死亡演出シーケンス
//  誰か倒れたら：1秒ストップ → 1秒スロウ → スコア表示 → フェード
//================================================================
enum DEATH_SEQUENCE
{
	DS_NONE = 0,
	DS_STOP,        // 1秒ストップ
	DS_SLOW,        // 1秒スロウ
	DS_SHOW_SCORE,  // スコア表示
	DS_FADE         // フェード開始（次シーンへ）
};

static DEATH_SEQUENCE g_deathSeq = DS_NONE;
static float         g_deathSeqTimer = 0.0f;
static SCENE         g_deathNextScene = SCENE_NONE; // フェード後に行くシーン

// スコア表示時間（好みで変えてOK）
static const float SCORE_SHOW_TIME = 4.0f;
static void StartDeathSequence(SCENE nextScene);

void Manager_Initialize()
{ 
	Fade_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

	////本来はtitleの初期化でフェードインをセットする
	//XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//SetFade(60.0f, color, FADE_STATE::FADE_IN, SCENE_GAME);
	//SetScene(SCENE_GAME);	//最初に動かすシーンに切り替える


	SetScene(SCENE_TITLE);	//最初に動かすシーンに切り替える

}

void Manager_Finalize()
{ 
	Fade_Finalize();
	SetScene(SCENE_NONE);
}

void Manager_Update()
{
	extern Controller g_Controller[2];
	for (int i = 0; i < 2; i++) {
		g_Controller[i].Update();
	}
	switch (g_Scene)	//現在シーンのアップデート関数を呼び出す
	{
		
		case SCENE_NONE:
			break;
		case SCENE_TITLE:
			Title_Update();	
			break;
		case SCENE_ENTRY:
			Entry_Update();
			break;
		case SCENE_SELECT_WT:
			selectWT_Update();
			Selectweaponui3d_Update();

			break;
		case SCENE_GAME:
		{
			const float dt = 1.0f / 60.0f;
			//========================================================
			// 死亡演出シーケンス中：ゲームを進めずに演出だけ進める
			//========================================================
			if (g_deathSeq != DS_NONE)
			{
				g_deathSeqTimer -= dt;

				if (g_deathSeq == DS_STOP)
				{
					// 完全停止（Game_Updateは呼ばない）
					if (g_deathSeqTimer <= 0.0f)
					{
						if (g_deathUseSlow)
						{
							// 死亡決着：1秒スロウへ
							g_deathSeq = DS_SLOW;
							g_deathSeqTimer = 1.0f;
							Game_SetTimeScale(0.5f);
						}
						else
						{
							// 時間切れ決着：スロウなしでそのままスコア表示へ
							g_deathSeq = DS_SHOW_SCORE;
							g_deathSeqTimer = SCORE_SHOW_TIME;
							Game_SetTimeScale(1.0f);

							Score_BeginShow(g_lastRoundResult);
							Game_SetShowScore(true);
						}
						break; // 演出中は通常処理しない
					}
				}
				else if (g_deathSeq == DS_SLOW)
				{
					if (g_deathSeqTimer <= 0.0f)
					{
						g_deathSeq = DS_SHOW_SCORE;
						g_deathSeqTimer = SCORE_SHOW_TIME;
						Game_SetTimeScale(1.0f);

						// ★スコア演出をここで初期化（0 0にして、1秒後に+1アニメ）
						Score_BeginShow(g_lastRoundResult);

						// ★描画ON
						Game_SetShowScore(true);
					}
				}
				else if (g_deathSeq == DS_SHOW_SCORE)
				{
					// ★Game_Updateは止めてるけどスコア演出だけは進める
					Score_Update();

					if (g_deathSeqTimer <= 0.0f)
					{
						Game_SetShowScore(false);
						XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
						SetFade(40.0f, color, FADE_OUT, g_deathNextScene);
						g_deathSeq = DS_FADE;
					}
				}
				else if (g_deathSeq == DS_FADE)
				{
					// フェードは Fade_Update が進めるので、ここでは待つだけ
					// シーン遷移したら SetScene 内で必要なリセットをする
					break; // 演出中は通常処理しない
				}

	
			}

			//========================================================
			// 通常のゲーム更新
			//========================================================
			Game_Update();

			// 勝敗判定を取得
			int result = Game_GetRoundResult();

			// 決着を同じラウンドで複数回カウントしない
			if (g_roundResultLocked)
			{
				break;
			}

			// 決着がついた場合（0以外が返ってきた場合）
			if (result != 0)
			{
				// まずロック（これが超重要）
				g_roundResultLocked = true;
				g_lastRoundResult = result;
				// 勝利カウント加算
				if (result == 1)
				{
					g_P1Wins++;
					Player2_PlusLoseCount();
					Player_PlusScore(200);
				}
				else if (result == 2)
				{
					g_P2Wins++;
					Player_PlusLoseCount();
					Player2_PlusScore(200);
				}
				// 引き分けの場合は result==0 で入ってこない想定

				g_RoundCount++; // ラウンドを進める

				// --- 試合終了判定（2勝で終わり） ---
				bool isMatchOver = false;
				if (g_P1Wins >= 2 || g_P2Wins >= 2) isMatchOver = true;

				// --- 死亡決着かどうか（誰か倒れたら演出） ---
				bool p1Dead = false;
				bool p2Dead = false;
				if (PLAYER* p1 = GetPlayer())  p1Dead = p1->isDead();
				if (PLAYER2* p2 = GetPlayer2()) p2Dead = p2->isDead();
				const bool decidedByDeath = (p1Dead || p2Dead);

				//====================================================
				// 今回の仕様：
				// だれか倒れたら
				// 1秒ストップ → 1秒スロウ → スコア表示 → フェード
				//====================================================
				if (decidedByDeath)
				{
					// 試合終了なら Result へ、まだ続くなら Game をリスタート（再Initialize）へ
					if (isMatchOver)
					{
						// ===== Resultへ渡すデータを保存 =====
						int winnerId = 0;
						if (g_P1Wins > g_P2Wins) winnerId = 1;
						else if (g_P2Wins > g_P1Wins) winnerId = 2;
						else winnerId = 1;

						SetupResultUi(true, winnerId);

						RESULT_MATCH_INFO info;
						info.winnerId = winnerId;
						info.decidedRound = g_RoundCount;

						if (PLAYER* p1 = GetPlayer())
						{
							info.p1.baseWT = p1->m_baseWT;
							info.p1.t0 = p1->GetReservedWT(0);
							info.p1.t1 = p1->GetReservedWT(1);
						}
						if (PLAYER2* p2 = GetPlayer2())
						{
							info.p2.baseWT = p2->m_baseWT;
							info.p2.t0 = p2->GetReservedWT(0);
							info.p2.t1 = p2->GetReservedWT(1);
						}

						ResultSystem_SetMatchInfo(info);

						// 演出後は Result へ
						StartDeathSequence(SCENE_RESULT, true);
					}
					else
					{
						// 演出後は Game を再Initialize（次ラウンド扱い）
						StartDeathSequence(SCENE_GAME,true);
					}

					break;
				}

				//====================================================
				// もし「時間切れ」等の死亡じゃない決着があるなら
				// ここは今まで通りでもOK（必要なら後で同じ演出に統一できる）
				//====================================================
				if (isMatchOver)
				{
					// Resultへ渡すデータを保存
					int winnerId = 0;
					if (g_P1Wins > g_P2Wins) winnerId = 1;
					else if (g_P2Wins > g_P1Wins) winnerId = 2;
					else winnerId = 1;

					SetupResultUi(true, winnerId);

					RESULT_MATCH_INFO info;
					info.winnerId = winnerId;
					info.decidedRound = g_RoundCount;

					if (PLAYER* p1 = GetPlayer())
					{
						info.p1.baseWT = p1->m_baseWT;
						info.p1.t0 = p1->GetReservedWT(0);
						info.p1.t1 = p1->GetReservedWT(1);
					}
					if (PLAYER2* p2 = GetPlayer2())
					{
						info.p2.baseWT = p2->m_baseWT;
						info.p2.t0 = p2->GetReservedWT(0);
						info.p2.t1 = p2->GetReservedWT(1);
					}

					ResultSystem_SetMatchInfo(info);

					// ここは従来通り即フェードでもOK
					XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
					SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
				}
				else
				{
					// まだ続く場合：ここも従来通りでもOK
					// 次ラウンドへ移る方法は「Game_ResetRound」でも「SetScene(SCENE_GAME)」でも好きな方に
					// 今回は簡単に暗転せず即リセット（必要ならここも演出統一できる）
					Game_ResetRound();
					g_roundResultLocked = false;
				}
			}

		}
		break;
		case SCENE_RESULT:
			Result_Update();
			break;
		default:
			break;
	}


	Fade_Update();

}

void Manager_Draw_Player1()
{ 
	switch (g_Scene)	//現在シーンの描画関数を呼び出す
	{
		case SCENE_NONE:
			break;
		case SCENE_TITLE:
			Title_Draw();	
			break;
		case SCENE_ENTRY:
			Entry_Draw();
			break;
		case SCENE_SELECT_WT:
			
			selectWT_Draw(0);
			Selectweaponui3d_Draw();
			break;
		case SCENE_GAME:
			Game_Draw_Player1();
			break;
		case SCENE_RESULT:
			Result_Draw_Player1();
			break;
		default:
			break;
	}

	Fade_Draw();

}
void Manager_Draw_Player2()
{
	switch (g_Scene)	//現在シーンの描画関数を呼び出す
	{
	case SCENE_NONE:
		break;
	case SCENE_TITLE:
		Title_Draw();
		break;
	case SCENE_ENTRY:
		Entry_Draw();
		break;
	case SCENE_SELECT_WT:

		selectWT_Draw(1);
		Selectweaponui3d_Draw();
		break;
	case SCENE_GAME:
		Game_Draw_Player2();
		break;
	case SCENE_RESULT:
		Result_Draw_Player2();
		break;
	default:
		break;
	}

	Fade_Draw();

}

inGameWTselect Manager_GetWTselect()
{
	return g_currentWTselect;
}

void Manager_SetWTselect(const inGameWTselect& select)
{
	g_currentWTselect = select;
}

void SetScene(SCENE scene) //シーンを切り替える
{

	//実行中のシーンを終了させる
	switch (g_Scene)	//現在シーンの終了関数を呼び出す
	{
		case SCENE_NONE:
			break;
		case SCENE_TITLE:
			Title_Finalize();	
			break;
		case SCENE_ENTRY:
			Entry_Finalize();
			break;
		case SCENE_SELECT_WT:
			selectWT_Finalize();
			Selectweaponui3d_Finalize();
			break;
		case SCENE_GAME:
			Game_Finalize();
			break;
		case SCENE_RESULT:
			Result_Finalize();
			break;
		default:
			break;
	}

	g_Scene = scene;	//指定のシーンへ切り替える

	//次のシーンを初期化する
	switch (g_Scene)	//現在シーンの初期化関数を呼び出す
	{
		case SCENE_NONE:
			break;
		case SCENE_TITLE:
			Title_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			break;
		case SCENE_ENTRY:
			Entry_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			break;
		case SCENE_GAME:
			StopAudio(g_title);

			// ★ゲームに入るたびに演出関連を安全に初期化
			g_deathSeq = DS_NONE;
			g_deathSeqTimer = 0.0f;
			g_deathNextScene = SCENE_NONE;

			g_roundResultLocked = false;
			Game_SetTimeScale(1.0f);
			Game_SetShowScore(false);

			Game_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext(), g_currentWTselect);
			break;
			break;
		case SCENE_SELECT_WT:
			selectWT_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			Selectweaponui3d_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());

			break;
		case SCENE_RESULT:
			Result_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
			break;
		default:
			break;
	}

}
int GetRoundCount()
{
	return g_RoundCount;
}
SCENE GetScene()
{
	return g_Scene;
}
void ResetWinCount()
{
	g_P1Wins = 0;
	g_P2Wins = 0;
}
static void StartDeathSequence(SCENE nextScene, bool useSlow)
{
	// 二重開始防止
	if (g_deathSeq != DS_NONE) return;

	g_deathNextScene = nextScene;
	g_deathUseSlow = useSlow;

	// まずは1秒停止
	g_deathSeq = DS_STOP;
	g_deathSeqTimer = 1.0f;

	Game_SetShowScore(false);
	Game_SetTimeScale(0.0f);
}
int GetP1WinCount()
{
	return g_P1Wins;
}
int GetP2WinCount()
{
	return g_P2Wins;

}
