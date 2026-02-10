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
#include"Result.h"
#include "Winner.h"
#include "Player.h"
#include "Player2.h"
#include"fade.h"
#include"selectWeaponTerrain.h"
//================================================================
//	グローバル変数
//================================================================
static	SCENE	g_Scene = SCENE_NONE;	//現在のシーン番号
static  inGameWTselect g_currentWTselect; //シーン間で保持する選択した武器・地形データ

static int g_RoundCount = 0; // 現在のラウンド数
static int g_P1Wins = 0;     // 1P勝利数
static int g_P2Wins = 0;     // 2P勝利数

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
		case SCENE_SELECT_WT:
			selectWT_Update();
			break;
		case SCENE_GAME:
		{
			Game_Update();

			// 勝敗判定を取得
			int result = Game_GetRoundResult();

			// 決着がついた場合（0以外が返ってきた場合）
			if (result != 0)
			{
				// 勝利カウント加算
				if (result == 1) g_P1Wins++;
				if (result == 2) g_P2Wins++;
				// 引き分けの場合は両者加算しない、あるいは再試合などの調整可能

				g_RoundCount++; // ラウンドを進める

				// --- 試合終了判定 ---
				bool isMatchOver = false;

				// ここはどちらでも調整可能
				//if (g_P1Wins >= 2 || g_P2Wins >= 2) isMatchOver = true; // 2勝したら終わり
				if (g_RoundCount >= 3) isMatchOver = true;         // 3ラウンド終わったら終わり

				if (isMatchOver)
				{
					// 全試合終了 -> リザルトへ
					// ここで初めてGameシーンを破棄する
					//Game_Finalize();
					SetScene(SCENE_RESULT);
					//複数回のゲームプレイを想定してゲームループ用変数を初期化
					g_RoundCount = 0;
					g_P1Wins = 0;
					g_P2Wins = 0;
					isMatchOver = false;
				}
				else
				{
					// --- まだ続く場合 ---
					// シーン遷移(SetScene)は使わず、リセット関数を呼ぶ
					Game_ResetRound();
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
		case SCENE_SELECT_WT:
			selectWT_Draw(0);
			break;
		case SCENE_GAME:
			Game_Draw_Player1();
			break;
		case SCENE_RESULT:
			Result_Draw();
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
	case SCENE_SELECT_WT:
		selectWT_Draw(1);
		break;
	case SCENE_GAME:
		Game_Draw_Player2();
		break;
	case SCENE_RESULT:
		Result_Draw();
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
		case SCENE_SELECT_WT:
			selectWT_Finalize();
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
		case SCENE_GAME:
			StopAudio(g_title);
			Game_Initialize( Direct3D_GetDevice(), Direct3D_GetDeviceContext(), g_currentWTselect);
			break;
		case SCENE_SELECT_WT:
			selectWT_Initialize(Direct3D_GetDevice(), Direct3D_GetDeviceContext());
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