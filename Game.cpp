/*
* ファイル名	Game.cpp
* タイトル	ゲーム
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Manager.h"
#include"sprite.h"
#include"Game.h"
#include"keyboard.h"
#include"field.h"
#include"Effect.h"
#include"Audio.h"
#include"Camera.h"
#include"fade.h"
#include"Player.h"
#include"managerCollider.h"
#include"terrain.h"
#include"Player2.h"
#include"Evolution.h"
#include"Viewport.h"
#include"direct3d.h"
#include "HpBar.h"
#include "HpBar2.h"
#include "timer.h"
#include "number.h"
#include "Hp.h"
#include "Hp2.h"
#include "generateWT.h"
#include "transformManager.h"
#include"Item.h"
//================================================================
//	グローバル変数
//================================================================
LIGHTOBJECT		Light;//<<<<<<ライト管理オブジェクト
// 全オブジェクト
std::vector<GameObject*> g_gameObjects;
static	int		g_BgmID = NULL;	//サウンド管理ID
static int frame;
static TransformManager g_transformMngr;
static int g_selectionPhase = 0;
ITEM_SPONER g_sponer;

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, const inGameWTselect& select)
{
	
	frame = 10;
	//Controller_Initialize();
	Field_Initialize(pDevice, pContext); // フィールドの初期化
	
	g_sponer.Initialize();

	EvolutionInitialize(select.player1, select.player2);

	PlayerInitialize(pDevice, pContext, select.player1); //
	Player2Initialize(pDevice, pContext, select.player2);

	TerrainInitialize(pDevice, pContext, select.player1, select.player2);//地形にP1,P2のそれぞれ選択した武器・地形情報を渡す

	PLAYER* pP1 = GetPlayer();
	PLAYER2* pP2 = GetPlayer2();
	//generateWT_Apply(Manager_GetWTselect(), pP1, pP2, pDevice, pContext);

	Camera_Initialize();	//カメラ初期化
	Camera2_Initialize();	//カメラ初期化

	g_transformMngr.Initialize(pDevice, pContext); //変身先選択の初期化
	g_selectionPhase = 0;
	g_transformMngr.StartSelection(WeaponTerrain::NONE, WeaponTerrain::NONE);

	//===========UI===========
	Hpbar_Initialize(pDevice, pContext);
	HpBar2_Initialize(pDevice, pContext);
	Timer_Initialize(pDevice, pContext);
	Number_Initialize(pDevice, pContext);
	Hp_Initialize(pDevice, pContext);
	Hp2_Initialize(pDevice, pContext);

	//========================
	//ビューポートの初期化
	//Viewport_Initialize(Direct3D_GetWindowHandle());

	//g_BgmID = LoadAudio("asset\\Audio\\bgm.wav");	//サウンドロード
	//PlayAudio(g_BgmID, true);	//再生開始（ループあり）
	//PlayAudio(g_BgmID);			//再生開始（ループなし）
	//PlayAudio(g_BgmID, false);	//再生開始（ループなし）

	//ライト初期化
	XMFLOAT4	para;

	para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);//環境光の色
	Light.SetAmbient(para);

	para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);//光の色
	Light.SetDiffuse(para);

	para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);//光方向
	float	len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
	para.x /= len;
	para.y /= len;
	para.z /= len;
	Light.SetDirection(para);//光の方向（正規化済）
}

void Game_Finalize()
{
	Field_Finalize();	// フィールドの終了処理
	TerrainFinalize();

	g_sponer.Finalize();

	PlayerFinalize();	// ボールの終了処理
	Player2Finalize();
	Camera_Finalize();	//カメラ終了処理
	Camera2_Finalize();	//カメラ終了処理



	//=======UI===========
	Hpbar_Finalize();
	HpBar2_Finalize();
	Timer_Finalize();
	Number_Finalize();
	Hp_Finalize();
	Hp2_Finalize();
	g_transformMngr.Finalize();
	//=====================
	ManagerCollider::ClearCollider();
	//UnloadAudio(g_BgmID);//サウンドの解放
}

void Game_Update()
{
	//少しの秒がアップデート時間を上げる
	if (frame > 0) 
	{
		frame -= 1;
	}
	if (g_transformMngr.IsActive()&&frame <= 0)
	{
		g_transformMngr.Update(1.0f / 60.0f);

		if (!g_transformMngr.IsActive())
		{
			inGameWTselect selectionData = g_transformMngr.GetPlayerSelectionWT();

			if (g_selectionPhase == 0)
			{//１回目の変身先選択完了時
				//P1,P2のスロット0に保存
				g_Player.SetReservedWT(0, selectionData.player1);
				g_Player2.SetReservedWT(0, selectionData.player2);

				//変身先選択(2回目)に移る
				g_selectionPhase = 1;

				g_transformMngr.StartSelection(selectionData.player1, selectionData.player2);
			}
			else if (g_selectionPhase == 1)
			{//２回目の変身先選択完了時
				//P1,P2のスロット1に保存
				g_Player.SetReservedWT(1, selectionData.player1);
				g_Player2.SetReservedWT(1, selectionData.player2);

				//変身先選択を終了してゲームへ移行
				g_selectionPhase = 2;
			}
		}
		
		return;
	}
	else
	{
		//更新処理
		for (auto obj : g_gameObjects)
		{
			obj->Update();
		}
		PlayerUpdate();
		Player2Update();
		Field_Update();
		TerrainUpdate();

		g_sponer.Update();

		//=======UI===========
		Hpbar_Update();
		HpBar2_Update();
		Timer_Update();
		Number_Update();
		Hp_Update();
		Hp2_Update();
		//=====================

		//======当たり判定======
		ManagerCollider::UpdateAllCollisions();

		auto it = std::remove_if(
			g_gameObjects.begin(), g_gameObjects.end(),
			[](GameObject* obj) {
				if (obj->m_isDead)
				{
					// 削除される前に、持っているコライダーをすべてマネージャーから外す
					// ※Colliderをshared_ptrで持っているなら、ここでの解除が重要です
					for (auto& collider : obj->GetColliders())
					{
						ManagerCollider::RemoveCollider(collider);
					}

					delete obj; // メモリを解放 (newで作っている場合)
					return true;
				}
				return false;
			});

		// リストから除去
		g_gameObjects.erase(it, g_gameObjects.end());
		//=====================

		//キー入力チェック
		//スタートボタンが押されたらシーンを切り替え
		//フェード処理中はキーを受け付けない
		if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE))
		{
			//フェードアウトさせてシーンを切り替える
			XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
			SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
		}
	}
	Camera_Update();	//カメラ更新処理
	Camera2_Update();   //カメラ2更新処理
}

void Game_Draw_Player1()
{
	//=================================================
	//	1つのフィールドで2人のプレイヤーを描画する場合、
	//	シェーダーの行列関数を両画面の処理で呼ぶことで
	//	別々のカメラを描画することができる
	//=================================================
	Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);

	//ID3D11DeviceContext* g_pContext = Direct3D_GetDeviceContext();

//================================================================
//	画面分割用関数(左画面)
//================================================================
	//g_pContext->RSSetViewports(1, &g_LeftViewPort);

	Camera_Draw();		//Drawの最初で呼ぶ！
	Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());
	Field_Draw();
	TerrainDraw();
	PlayerDraw();
	Player2Draw();

	for (auto obj : g_gameObjects)
	{
		obj->Draw();
	}


	//==========lightがtrueだとUIが暗く見えるので、一回解除=========
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);
	//===UI描画========
	if (g_transformMngr.IsActive())
	{
		g_transformMngr.Draw(0);
	}

	//Hpbar_Draw(); //<--HpBar描画
	//Timer_Draw();
	//Number_Draw();
	//Hp_Draw();

	//================
	Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);
	//============lightをまたtrueにして、camera2に影響がないように================
}
void Game_Draw_Player2()
{
	//g_pContext->RSSetViewports(1, &g_RightViewPort);

	Camera2_Draw();
	Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());
	Field_Draw();
	TerrainDraw();
	PlayerDraw();
	Player2Draw();
	
	for (auto obj : g_gameObjects)
	{
		obj->Draw();
	}

	//2D描画
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);

	if (g_transformMngr.IsActive())
	{
		g_transformMngr.Draw(1);
	}

	//HpBar2_Draw();
	//Timer_Draw();
	//Number_Draw();
	//Hp2_Draw();
}

int Game_GetRoundResult()
{
	// プレイヤーの死亡判定関数をここで使用
	bool p1Dead = g_Player.isDead();
	bool p2Dead = g_Player2.isDead();

	if (p1Dead && p2Dead) return 3; // 引き分け（同時死亡）
	if (p2Dead) return 1;           // P1の勝ち
	if (p1Dead) return 2;           // P2の勝ち

	return 0; // 戦闘継続中
}

void Game_ResetRound()
{
	// プレイヤーを初期位置に戻して蘇生
	g_Player.RoundReset(XMFLOAT3(0.0f, 0.5f, 1.0f));
	g_Player2.RoundReset(XMFLOAT3(2.0f, 0.5f, 2.0f));

	g_selectionPhase = 0;
	// 変身選択マネージャだけ再開（Initializeはしない）
	g_transformMngr.StartSelection(WeaponTerrain::NONE, WeaponTerrain::NONE);
}
