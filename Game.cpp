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
#include"Viewport.h"
#include"direct3d.h"
#include "HpBar.h"
#include "HpBar2.h"
#include "timer.h"
#include "number.h"
//================================================================
//	グローバル変数
//================================================================
LIGHTOBJECT		Light;//<<<<<<ライト管理オブジェクト
// 全オブジェクト
std::vector<GameObject*> g_gameObjects;
static	int		g_BgmID = NULL;	//サウンド管理ID

void Game_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	//Controller_Initialize();

	Field_Initialize(pDevice, pContext); // フィールドの初期化
	TerrainInitialize(pDevice, pContext);
	PlayerInitialize(pDevice, pContext); // ボールの初期化
	Player2Initialize(pDevice, pContext);
	Camera_Initialize();	//カメラ初期化
	//===========UI===========
	Hpbar_Initialize(pDevice, pContext);
	HpBar2_Initialize(pDevice, pContext);
	Timer_Initialize(pDevice, pContext);
	Number_Initialize(pDevice, pContext);
	//========================
	//ビューポートの初期化
	Viewport_Initialize(Direct3D_GetWindowHandle());

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
	PlayerFinalize();	// ボールの終了処理
	Player2Finalize();
	Camera_Finalize();	//カメラ終了処理


	//=======UI===========
	Hpbar_Finalize();
	HpBar2_Finalize();
	Timer_Finalize();
	Number_Finalize();
	//=====================
	//UnloadAudio(g_BgmID);//サウンドの解放
}

void Game_Update()
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
	//=======UI===========
	Hpbar_Update();
	HpBar2_Update();
	Timer_Update();
	Number_Update();
	//=====================
	ManagerCollider::UpdateAllCollisions();
	//キー入力チェック
	//スタートボタンが押されたらシーンを切り替え
	//フェード処理中はキーを受け付けない
	if (Keyboard_IsKeyDownTrigger(KK_ENTER) && (GetFadeState() == FADE_NONE))
	{
		//フェードアウトさせてシーンを切り替える
		XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
		SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
	}
	Camera_Update();	//カメラ更新処理
	Camera2_Update();   //カメラ2更新処理
}

void Game_Draw()
{ 
	//=================================================
	//	1つのフィールドで2人のプレイヤーを描画する場合、
	//	シェーダーの行列関数を両画面の処理で呼ぶことで
	//	別々のカメラを描画することができる
	//=================================================
	Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);

	ID3D11DeviceContext* g_pContext = Direct3D_GetDeviceContext();
	
//================================================================
//	画面分割用関数(左画面)
//================================================================
	g_pContext->RSSetViewports(1, &g_LeftViewPort);

	Camera_Draw();		//Drawの最初で呼ぶ！
	Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());
	Field_Draw();
	TerrainDraw();
	PlayerDraw();
	Player2Draw();


	//==========lightがtrueだとUIが暗く見えるので、一回解除=========
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);
	//===UI描画========
	Hpbar_Draw(); //<--HpBar描画
	Timer_Draw();
	Number_Draw();
	//================
	Light.SetEnable(TRUE);			//ライティングON
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(TRUE);
	//============lightをまたtrueにして、camera2に影響がないように================

//================================================================
//	画面分割用関数(右画面)
//================================================================
	g_pContext->RSSetViewports(1, &g_RightViewPort);

	Camera2_Draw();
	Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());
	Field_Draw();
	TerrainDraw();
	PlayerDraw();
	Player2Draw();
	

	//2D描画
	Light.SetEnable(FALSE);			//ライティングOFF
	Shader_SetLight(Light.Light);	//ライト構造体をシェーダーへセット
	SetDepthTest(FALSE);
	HpBar2_Draw();
	Timer_Draw();
	Number_Draw();
}