
/* ファイル名	Player.cpp
* タイトル	プレイヤー
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	マクロ定義
//================================================================
//#define JUMP_FORCE (0.15f)
#define CLIMB_SPEED (m_jumpForce / 2.0f)

//================================================================
//	インクルード
//================================================================
#include"keyboard.h"
#include"Controller.h"
#include"Player.h"
#include"Camera.h"
#include"shader.h"
#include"Transform.h"
#include"colliderFactory.h"
#include"debug_ostream.h"
#include"fade.h"
#include"sword.h"
#include"spear.h"
#include"hammer.h"
#include"arrow.h"
#include"syuriken.h"
#include"terrain.h"
#include<memory>
#include"generateWT.h"
#include"selectWeaponTerrain.h"
#include"Item.h"
//================================================================
//	グローバル変数
//================================================================
PLAYER	g_Player;
ID3D11Device* g_pDevice;
ID3D11DeviceContext* g_pContext;
extern Controller g_Controller[2]; //ID 0のコントローラーを使用
extern const char* INITIAL_MODEL_PATH_P1;
MODEL* g_modelP1;
WeaponTerrain g_setWTP1; // プレイヤーの武器と地形情報
unsigned int g_changeP1;
static bool g_Player1AttackPlaying = false; // 攻撃ワンショット再生中フラグ
static bool g_Player1JumpPlaying = false; // ジャンプワンショット再生中フラグ
static int g_Player1CurrentAnim = 0; // 0: idle, 1: move, 2: attack 3:jump
bool g_isChangeP1;
ITEM_SPONER gp_itemSponer;
XMFLOAT3 gp1_slopeSpeed;
bool gp1_move; // プレイヤーが動いているかのフラグ
bool gp1_koyoteFlag; // コヨーテタイムを回復するかどうか

void PlayerDie()
{
	hal::dout << "Player died!" << std::endl;
	//死亡処理

	//プレイヤーを非表示にする
	if (g_Player.m_gameObject != nullptr)
	{
		g_Player.m_gameObject->m_isEnable = false;
	}

	// 例: 入力を受け付けないようにする（状態をIDLEにするなど）
	g_Player.State = PLAYER_STATE::PLAYER_STATE_IDLE;
	g_Player.m_isDeadFlag = true;
	XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(40.0f, color, FADE_OUT, SCENE_GAME);
}

void PlayerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp1)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	if (INITIAL_MODEL_PATH_P1 == nullptr) {
		g_Player.m_model = ModelLoad("asset\\model\\block.fbx"); // 確実に存在するファイル
	}
	else {
		g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH_P1);
	}	//g_modelP1 = ModelLoad("asset\\model\\block.fbx");

	g_Player.m_position = XMFLOAT3(-10.0f, 0.5f, 1.0f);
	g_Player.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Player.m_tag = "Player";
	g_Player.m_layer = 0;
	g_Player1AttackPlaying = false;
	g_Player.State = PLAYER_STATE::PLAYER_STATE_MOVE;

	g_Player.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player.FrictionRate = 0.98f;
	g_Player.TransformType = TRANSFORM_TYPE::TRANSFORM_TYPE_NONE;
	g_Player.m_currentHp = g_Player.m_maxHp;
	g_Player.m_isDead = false;
	g_Player.m_baseWT = setWTp1;
	g_Player.m_isAttacked = false;
	g_Player.m_isTransformed = false;
	g_Player.m_isDeadFlag = false;
	g_Player.m_moveMul = 1.0f;
	auto collider = g_Player.AddComponent<BoxCollider>(&g_Player, g_Player.m_scale);
	ManagerCollider::AddCollider(collider);

	// のちのちセレクト画面から分岐できるようにする
	// 自分をownerとして武器を生成

	g_changeP1 = 3;
	g_setWTP1 = setWTp1;
	if (g_setWTP1 == WeaponTerrain::SWORD_WALL)
	{
		g_Player.m_model = ModelLoad("asset\\model\\default_sword.fbx");
	}
	else if (g_setWTP1 == WeaponTerrain::SPEAR_HILL)
	{
		g_Player.m_model = ModelLoad("asset\\model\\default_spear.fbx");
	}
	else if (g_setWTP1 == WeaponTerrain::BOW_HILL)
	{
		g_Player.m_model = ModelLoad("asset\\model\\default_bow.fbx");
	}
	else if (g_setWTP1 == WeaponTerrain::HAMMER_)
	{
		g_Player.m_model = ModelLoad("asset\\model\\default_hammer.fbx");
	}
	else if (g_setWTP1 == WeaponTerrain::SHURIKEN_)
	{
		g_Player.m_model = ModelLoad("asset\\model\\default_shuriken.fbx");
	}

	g_Player.EquipBaseWeapon();
	
	gp1_slopeSpeed = { 0.0f, 0.0f, 0.0f };
	gp1_move = false; // 最初は動いてない
	gp1_koyoteFlag = false; // 最初はフラグをオフ

	g_isChangeP1 = false;
}
void PlayerFinalize()
{
	ModelRelease(g_Player.m_model);
}
void	PlayerUpdate()
{
	TransformPlayer();
	// こいつの中でscaleが1.0fに固定されている
	ApplyTransformEffect();   // 進化タイプに応じたパラメータを適用
	if (g_Player.m_isDead)return;	//死亡している場合は更新処理をスキップ

	//ヒットアクション
	g_Player.m_hitAction.Update(g_Player.m_position);
	//ヒットストップ中ならこの関数自体を抜けるため今後の処理がすべてスキップされる
	if (g_Player.m_hitAction.IsStopping())
	{
		return;
	}
	
//================================================================
//	武器変更処理(一旦)
//================================================================
	/*
	int slotToUse = -1; 

	if (Keyboard_IsKeyDownTrigger(KK_D1) && !GetIsUsedA_P1())
	{
		slotToUse = 0;
		g_isChangeP1 = true;
	}
	if (Keyboard_IsKeyDownTrigger(KK_D0) && !GetIsUsedB_P1())
	{
		slotToUse = 1;
		g_isChangeP1 = true;
	}


	if (slotToUse != -1)
	{
		WeaponTerrain reserved = g_Player.GetReservedWT(slotToUse);

		if (reserved != WeaponTerrain::NONE)
		{
			inGameWTselect data;
			data.player1 = reserved;       
			data.player2 = g_Player2.GetCurrentWT();

			// generateWT_Apply
			//generateWT_Apply(data, &g_Player, &g_Player2, g_pDevice, g_pContext);
			TerrainSet(reserved, FALSE);

			switch (reserved) {
			case WeaponTerrain::SWORD_WALL: 
				g_changeP1 = 1;
				g_Player.m_model = ModelLoad("asset\\model\\sword.fbx"); break;
				g_Player.EquipWeapon(std::make_unique<Sword>(&g_Player, FALSE));
			case WeaponTerrain::SPEAR_HILL:
				g_changeP1 = 2;
				g_Player.m_model = ModelLoad("asset\\model\\spear.fbx"); break;
				g_Player.EquipWeapon(std::make_unique<Spear>(&g_Player, FALSE));
			case WeaponTerrain::BOW_HILL:   
				g_changeP1 = 3;
				g_Player.m_model = ModelLoad("asset\\model\\bow.fbx"); break;
				g_Player.EquipWeapon(std::make_unique<Arrow>(&g_Player, FALSE));
			case WeaponTerrain::HAMMER_:   
				g_changeP1 = 4;
				g_Player.m_model = ModelLoad("asset\\model\\hammer.fbx");
				g_Player.EquipWeapon(std::make_unique<Hammer>(&g_Player, FALSE)); 
				break;
			case WeaponTerrain::SHURIKEN_: 
				g_changeP1 = 5; 
				g_Player.m_model = ModelLoad("asset\\model\\char_shuriken_motion.fbx");
				g_Player.EquipWeapon(std::make_unique<Shuriken>(&g_Player, FALSE));
				break;
			}
			g_setWTP1 = reserved;
			//g_Player.SetCurrentWT(reserved);
		}
	}
	*/

//================================================================
//	攻撃処理(変身前)
//================================================================

	if (Keyboard_IsKeyDownTrigger(KK_C) || g_Controller[0].IsButtonPushed(ControllerButton::X_BUTTON))
	{
		// 武器が存在し攻撃中でなければ攻撃開始
		if (g_Player.m_currentWeapon && !g_Player1AttackPlaying)
		{
			g_Player.m_currentWeapon->Attack();
			if (g_Player.m_isTransformed)
			{		
				switch (g_Player.m_currentWT)
				{
				case WeaponTerrain::SWORD_WALL: // Sword
					ModelPlayClip(g_Player.m_model, 167, 227, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SPEAR_HILL: // spear
					ModelPlayClip(g_Player.m_model, 500, 600, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::BOW_HILL: // arrow
					ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::HAMMER_: // hammer
					ModelPlayClip(g_Player.m_model, 360, 539, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SHURIKEN_: //shuriken
					ModelPlayClip(g_Player.m_model, 151, 210, 60.0f, false, 4.0f);
					break;
				}
				
			}
			else
			{
				switch (g_setWTP1)
				{
				case WeaponTerrain::SWORD_WALL: // Sword
					ModelPlayClip(g_Player.m_model, 167, 227, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SPEAR_HILL: // spear
					ModelPlayClip(g_Player.m_model, 500, 600, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::BOW_HILL: // arrow
					ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::HAMMER_: // hammer
					ModelPlayClip(g_Player.m_model, 360, 539, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SHURIKEN_: //shuriken
					ModelPlayClip(g_Player.m_model, 151, 210, 60.0f, false, 4.0f);
					break;
				}
			}
			g_Player1AttackPlaying = true;
			g_Player1CurrentAnim = 2; // attack 状態
		}

		hal::dout << "Player 攻撃入力検出\n";
	}

//================================================================
//	武器の更新
//================================================================
	if (g_Player.m_currentWeapon)
	{
		g_Player.m_currentWeapon->Update();
	}

	Player_ManualMove();
	//死亡判定
	if (g_Player.m_currentHp <= 0.0f && !g_Player.m_isDead)
	{
		g_Player.m_isDead = true;
		PlayerDie();
	}
	//================================================================
	// アニメーション処理
	// ================================================================
	// 移動速度判定
	float moveSpeed = sqrtf(g_Player.m_velocity.x * g_Player.m_velocity.x +
		g_Player.m_velocity.z * g_Player.m_velocity.z);
	bool isMoving = (moveSpeed > 0.001f);

	// アニメーション状態管理：
	//  - 攻撃ワンショット再生中はその完了を監視し、完了したら移動/待機ループへ復帰
	//  - 攻撃中でなければ移動/待機のループアニメを確実に再生しておく
	if (g_Player1AttackPlaying||g_Player1JumpPlaying)
	{
		// ワンショットクリップが終了したか確認
		if (ModelConsumeClipFinished(g_Player.m_model))
		{
			// 攻撃アニメ完了: フラグ解除して適切なループへ戻す
			g_Player1AttackPlaying = false;
			g_Player1JumpPlaying = false;
			if (isMoving)
			{
				// 移動ループ
				if (g_Player1CurrentAnim != 1)
				{
					if (g_Player.m_isTransformed)
					{
						switch (g_Player.m_currentWT)
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player.m_model, 121, 150, 60.0f, true, 1.0f);
							break;
						}
						
					}
					else
					{
						switch (g_setWTP1) //移動
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player.m_model, 121, 150, 60.0f, true, 1.0f);
							break;
						}
					}
					
					g_Player1CurrentAnim = 1;
				}
			}
			else
			{
				// 待機ループ（0~60）
				if (g_Player1CurrentAnim != 0)
				{
					if (g_Player.m_isTransformed)
					{
						switch (g_Player.m_currentWT)
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
							break;
						}
					}
					else
					{
						switch (g_setWTP1) //移動
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
							break;
						}
						
					}
					g_Player1CurrentAnim = 0;
				}
			}
		}
		// 攻撃中は移動による切替を行わない（攻撃優先）
	}
	else
	{
		// 攻撃中でなければ移動/待機を維持
		if (isMoving)
		{
			if (g_Player1CurrentAnim != 1)
			{
				if (g_Player.m_isTransformed)
				{
					switch (g_Player.m_currentWT)
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::SPEAR_HILL: // spear
						ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::BOW_HILL: // hammer
						ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::HAMMER_: // arrow
						ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player.m_model, 121, 150, 60.0f, true, 1.0f);
						break;
					}
				}
				else
				{
					switch (g_setWTP1) //移動
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::SPEAR_HILL: // spear
						ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::BOW_HILL: // hammer
						ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::HAMMER_: // arrow
						ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player.m_model, 121, 150, 60.0f, true, 1.0f);
						break;
					}
				}
				
				g_Player1CurrentAnim = 1;
			}
		}
		else
		{
			if (g_Player1CurrentAnim != 0)
			{
				if (g_Player.m_isTransformed)
				{
					switch (g_Player.m_currentWT)
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
						break;
					case WeaponTerrain::SPEAR_HILL: // spear
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::BOW_HILL: // hammer
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::HAMMER_: // arrow
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
						break;
					}
				}
				else
				{
					switch (g_setWTP1) //待機
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
						break;
					case WeaponTerrain::SPEAR_HILL: // spear
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::BOW_HILL: // hammer
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::HAMMER_: // arrow
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
						break;
					}
				}
				g_Player1CurrentAnim = 0;
			}
		}
	}

	// アニメーション時間の進行は Update 側で一度だけ行う（フレーム固定レート環境を想定して 1/60 を使用）
	// deltaTime が利用可能ならそちらを使ってください（例: ModelUpdateAnimation(g_Player.m_model, deltaTime);）
	ModelUpdateAnimation(g_Player.m_model, 1.0f / 60.0f);

	// 死亡判定
	if (g_Player.m_currentHp <= 0.0f && !g_Player.m_isDead)
	{
		g_Player.m_isDead = true;
		PlayerDie();
	}
}

void Player_ManualMove() // 新しい手動移動関数として作成
{
	// カメラの前方向ベクトル
	float forwardX = GetCameraAtPosition().x - GetCameraPosition().x;
	float forwardZ = GetCameraAtPosition().z - GetCameraPosition().z;

	gp1_move = false; // 常に動いていないと更新

	if (!g_Player.m_isGround) // 地面についてないときに重力発動
	{
		g_Player.m_velocity.x += g_Player.m_acceleration.x;
		g_Player.m_velocity.y += g_Player.m_acceleration.y;
		g_Player.m_velocity.z += g_Player.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	// フラグがオフの時に1.0fになる
	if (g_Player.m_isGround &&
		!gp1_koyoteFlag)
	{
		g_Player.m_koyoteTime = 1.0f;
	}
	else
	{
		g_Player.m_koyoteTime -= 0.1f;
	}

	float len = sqrtf(forwardX * forwardX + forwardZ * forwardZ);
	if (len > 0.0f)
	{
		forwardX /= len;
		forwardZ /= len;
	}
	else
	{
		forwardX = 0.0f;
		forwardZ = 0.0f;
	}

	// カメラの右方向ベクトル
	float rightX = forwardZ;    // 右方向は前方向ベクトルを90度回転
	float rightZ = -forwardX;

	// 移動量初期化
	float moveX = 0.0f;
	float moveZ = 0.0f;

	float speed = 0.0f;
	float stickY = g_Controller[0].GetLeftStickY();
	if (fabs(stickY) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// ベクトルが逆だから移動が逆になる
		// 左スティック上方向 (+1.0f) で前進 (speed = -0.1f) に対応
		speed = stickY * 0.1f;
		gp1_move = true; // 動いている
	}


	if (Keyboard_IsKeyDown(KK_W))
	{
		speed = +0.1f;
		gp1_move = true; // 動いている
	}
	if (Keyboard_IsKeyDown(KK_S))
	{
		speed = -0.1f;
		gp1_move = true; // 動いている
	}

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	float stickX = g_Controller[0].GetLeftStickX();
	if (fabs(stickX) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// 左スティック左方向 (-1.0f) で左移動 (strafe = +0.1f) に対応
		strafe = stickX * 0.1f;
		gp1_move = true; // 動いている
	}

	if (Keyboard_IsKeyDown(KK_A))
	{
		strafe = -0.1f;  // 左
		gp1_move = true; // 動いている
	}
	if (Keyboard_IsKeyDown(KK_D))
	{
		strafe = +0.1f;  // 右
		gp1_move = true; // 動いている
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	if (g_Player.m_isGround)
	{
		// 地面にいるときは、入力方向へクイックに速度を合わせる
		// ただし、完全に上書きせず、現在の速度（滑り成分など）に加算する形にするのがベターです

		// 入力がないときは、今の速度を少しずつ減衰させる（摩擦の表現）
		if (fabs(moveX) < 0.001f && fabs(moveZ) < 0.001f)
		{
			g_Player.m_velocity.x *= 0.45f; // 摩擦で止まる
			g_Player.m_velocity.z *= 0.45f;
		}
		else
		{
			// 入力があるときは、入力方向に速度をセットする
			// ただし、滑っている力を消さないために「加算」に近い形にする
			g_Player.m_velocity.x += moveX * 0.3f; // 加速度的に足す
			g_Player.m_velocity.z += moveZ * 0.3f;

			// 最高速制限（これがないと無限に加速する）
			float maxSpeed = 0.15f * g_Player.m_moveMul;
			float currSpeed = sqrtf(g_Player.m_velocity.x * g_Player.m_velocity.x + g_Player.m_velocity.z * g_Player.m_velocity.z);
			if (currSpeed > maxSpeed)
			{
				g_Player.m_velocity.x = (g_Player.m_velocity.x / currSpeed) * maxSpeed;
				g_Player.m_velocity.z = (g_Player.m_velocity.z / currSpeed) * maxSpeed;
			}
		}
	}
	else
	{
		// 空中にいるときは制御を弱くする（または慣性を維持）
		g_Player.m_velocity.x += moveX * 0.05f;
		g_Player.m_velocity.z += moveZ * 0.05f;

		float maxSpeed = 0.1f * g_Player.m_moveMul;
		float currSpeed = sqrtf(g_Player.m_velocity.x * g_Player.m_velocity.x + g_Player.m_velocity.z * g_Player.m_velocity.z);
		if (currSpeed > maxSpeed)
		{
			g_Player.m_velocity.x = (g_Player.m_velocity.x / currSpeed) * maxSpeed;
			g_Player.m_velocity.z = (g_Player.m_velocity.z / currSpeed) * maxSpeed;
		}
	}

	// モデルの向きを移動方向に合わせる
	XMFLOAT3 moveDir = { g_Player.m_velocity.x, 0.0f, g_Player.m_velocity.z };
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
	if (length > 0.001f) // 移動しているときだけ向きを変える
	{
		// Y軸回転角を計算
		g_Player.m_rotation.y = atan2f(moveDir.x, moveDir.z); // atan2f(X,Z)でY回転
	}

	// Aボタンを押した && コヨーテタイムが0.0fより大きい
	//if (Keyboard_IsKeyDownTrigger(KK_SPACE) && g_Player.m_koyoteTime > 0.0f)
	if ((g_Controller[0].IsButtonPushed(ControllerButton::A_BUTTON) || Keyboard_IsKeyDown(KK_SPACE))
		&& g_Player.m_koyoteTime > 0.0f) //Aボタン**
	{
		g_Player.m_velocity.y = g_Player.m_jumpForce;
		g_Player.m_isGround = false;
		g_Player.m_koyoteTime = 0.0f;
		if(g_Player.m_isTransformed)
		{
			switch (g_Player.m_currentWT)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player.m_model, 300, 334, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player.m_model, 400, 450, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player.m_model, 240, 300, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player.m_model, 280, 350, 60.0f, false, 1.0f);
				break;
			}
		}
		else
		{	
			switch (g_setWTP1)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player.m_model, 300, 334, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player.m_model, 400, 450, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player.m_model, 240, 300, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player.m_model, 280, 350, 60.0f, false, 1.0f);
				break;
			}
		}
		g_Player1JumpPlaying = true;
		g_Player1CurrentAnim = 3; // ジャンプ 状態
	}
	else
	{
		g_Player.m_isGround = false;
	}

	if (!g_Player.m_isGround)
	{
		gp1_slopeSpeed.x *= 0.3f;
		gp1_slopeSpeed.z *= 0.3f;
		gp1_slopeSpeed.y *= 0.3f;
	}
	// 地面にいても、入力をしているときは少し滑りを抑えるなどの調整も可能
	if (fabs(moveX) > 0.01f || fabs(moveZ) > 0.01f)
	{
		gp1_slopeSpeed.x *= 0.95f;
		gp1_slopeSpeed.z *= 0.95f;
	}

	g_Player.m_position.x += (g_Player.m_velocity.x + gp1_slopeSpeed.x);
	g_Player.m_position.z += (g_Player.m_velocity.z + gp1_slopeSpeed.y);
	g_Player.m_position.y += (g_Player.m_velocity.y + gp1_slopeSpeed.z);
}

void PlayerDraw() 
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		0.01f,
		0.01f,
		0.01f );
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Player.m_rotation.x,
		g_Player.m_rotation.y + XM_PI,
		g_Player.m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		g_Player.m_position.x,
		g_Player.m_position.y - 1.0f,
		g_Player.m_position.z);
	if (g_Player.m_position.y < g_Player.m_position.y - 1.0f)
	{
		g_Player.m_position.y = g_Player.m_position.y - 0.99f;
	}
	

	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	Shader_SetBones(g_Player.m_model);
	//モデルの描画リクエスト
	ModelDraw(g_Player.m_model);

	if (g_Player.m_currentWeapon)
	{
		g_Player.m_currentWeapon->Draw();
	}
	//ModelDraw(g_modelP1);
}


XMFLOAT3 GetPlayerPosition()
{
	return g_Player.m_position;
}

void Player_Jump()
{
	g_Player.State = PLAYER_STATE::PLAYER_STATE_MOVE;
}

PLAYER* GetPlayer()
{
	return &g_Player;
}
float Player_GetHp() 
{
	return g_Player.m_currentHp; 
}
float Player_GetMaxHp()
{
	return g_Player.m_maxHp;
}
bool GetPlayer_IsAttacked()
{
	return g_Player.m_isAttacked;
}
void SetPlayer_IsAttacked(bool isAttacked)
{
    g_Player.m_isAttacked = isAttacked;
}
//武器を装備する
void PLAYER::EquipWeapon(std::unique_ptr<IWeapon> weapon)
{
	m_currentWeapon = nullptr;
	m_currentWeapon = std::move(weapon);
}

void PLAYER::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;
	if (m_isDead) return; //死亡していたら衝突処理を無視

	gp1_koyoteFlag = false; // 基本false

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
		// 攻撃の時
		if (info.other->m_tag == "Attack")
		{
			// 相手が武器オブジェクト持ってたら
			if (info.other->m_weaponPtr)
			{
				// 武器の衝突判定を呼び出す
				g_Player.m_isAttacked = true;
				info.other->m_weaponPtr->OnWeaponCollision(this);
			}
		}

		// 例えば壁・木だけコリジョン有効
		if (info.other->m_tag == "Wall" ||
			info.other->m_tag == "Tree" ||
			info.other->m_tag == "WallA")
		{
			auto INFO = info;

			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (INFO.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(INFO.normal.x) + fabs(INFO.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
			}
		}

		// 例えば壁・木だけコリジョン有効
		if (info.other->m_tag == "WALL" ||
			info.other->m_tag == "TREE")
		{
			auto INFO = info;

			INFO.normal.x *= -1;
			INFO.normal.y *= -1;
			INFO.normal.z *= -1;

			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (INFO.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(INFO.normal.x) + fabs(INFO.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
			}
		}

		if (info.other->m_tag == "Player2")
		{
			auto INFO = info;

			INFO.normal.x *= -1;
			INFO.normal.y *= -1;
			INFO.normal.z *= -1;

			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (INFO.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(INFO.normal.x) + fabs(INFO.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
			}
		}
		
		if (info.other->m_tag == "Lift" ||
			info.other->m_tag == "HILL")
		{
			auto INFO = info;

			INFO.normal.x *= -1;
			INFO.normal.y *= -1;
			INFO.normal.z *= -1;

			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (INFO.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(INFO.normal.x) + fabs(INFO.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
				m_velocity.y = CLIMB_SPEED;				
			}
		}
		
		if (info.other->m_tag == "Slope")
		{
			auto INFO = info;
			// 法線が自分を押し出す方向に向くように反転
			INFO.normal.x *= -1;
			INFO.normal.y *= -1;
			INFO.normal.z *= -1;

			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			// 坂道なら normal.y が 0 より大きければ地面とみなす
			if (INFO.normal.y > 0.1f)
			{
				m_isGround = true;
				if (m_velocity.y < 0) m_velocity.y = 0.0f;

				// --- gp_speed への計算 ---
				const float slideFriction = 0.15f;
				float slopeSeverity = 1.0f - INFO.normal.y;
				float slidePower = slopeSeverity * slideFriction;
				const float gravityEffect = 0.02f;

				// m_velocity ではなく gp_speed に加算
				gp1_slopeSpeed.x += INFO.normal.x * (slidePower + gravityEffect);
				gp1_slopeSpeed.z += INFO.normal.z * (slidePower + gravityEffect);

				// リミッター
				float maxSlide = 0.08f;
				float speedXZ = sqrtf(gp1_slopeSpeed.x * gp1_slopeSpeed.x + gp1_slopeSpeed.z * gp1_slopeSpeed.z);
				if (speedXZ > maxSlide)
				{
					gp1_slopeSpeed.x = (gp1_slopeSpeed.x / speedXZ) * maxSlide;
					gp1_slopeSpeed.z = (gp1_slopeSpeed.z / speedXZ) * maxSlide;
				}

				m_velocity.x *= 0.0f;
				m_velocity.z *= 0.0f;
			}
		}

		if (info.other->m_tag == "SlopeP1")
		{
			// 1. 押し出し（めり込み防止の基本）
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			if (info.normal.y > 0.1f)
			{
				m_isGround = true;
				if (m_velocity.y < 0) m_velocity.y = 0.0f;

				float climbResistance = 0.7f; // 0.8〜0.9 くらいで調整（小さいほど遅くなる）

				// 入力によって進もうとしている速度にブレーキをかける
				m_velocity.x *= climbResistance;
				m_velocity.z *= climbResistance;

				if (gp1_move)
				{
					m_position.y += 0.1f;
				}

				// 滑り計算（gp1_slopeSpeed）は使わないので 0 にリセット
				gp1_slopeSpeed = { 0.0f, 0.0f, 0.0f };
			}
		}

		if (info.other->m_tag == "SlopeP2")
		{
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			// 坂道なら normal.y が 0 より大きければ地面とみなす
			if (info.normal.y > 0.1f)
			{
				m_isGround = true;
				if (m_velocity.y < 0) m_velocity.y = 0.0f;
				gp1_koyoteFlag = true; // フラグをオンにする
				m_koyoteTime = 0.0f; // ジャンプできなくする

				// --- gp_speed への計算 ---
				const float slideFriction = 0.25f;
				float slopeSeverity = 1.0f - info.normal.y;
				float slidePower = slopeSeverity * slideFriction;
				const float gravityEffect = 0.02f;

				// m_velocity ではなく gp_speed に加算
				gp1_slopeSpeed.x += info.normal.x * (slidePower + gravityEffect);
				gp1_slopeSpeed.z += info.normal.z * (slidePower + gravityEffect);

				// リミッター
				float maxSlide = 0.08f;
				float speedXZ = sqrtf(gp1_slopeSpeed.x * gp1_slopeSpeed.x + gp1_slopeSpeed.z * gp1_slopeSpeed.z);
				if (speedXZ > maxSlide)
				{
					gp1_slopeSpeed.x = (gp1_slopeSpeed.x / speedXZ) * maxSlide;
					gp1_slopeSpeed.z = (gp1_slopeSpeed.z / speedXZ) * maxSlide;
				}

				m_velocity.x *= 0.0f;
				m_velocity.z *= 0.0f;
			}
		}

		if (info.other->m_tag == "BOGP2")
		{
			XMFLOAT3 bogPos = info.other->m_position;

			float dx = m_position.x - bogPos.x;
			float dz = m_position.z - bogPos.z;
			float distance = sqrtf(dx * dx + dz * dz);

			const float effectRadius = 5.0f;

			if (distance < effectRadius)
			{
				m_velocity.x *= 0.3f;
				m_velocity.z *= 0.3f;

				gp1_slopeSpeed.x *= 0.5f;
				gp1_slopeSpeed.z *= 0.5f;
			}
		}
	}
}

void PLAYER::EquipBaseWeapon()
{
	m_currentWT = m_baseWT;
	m_reservedWT[0] = WeaponTerrain::NONE;
	m_reservedWT[1] = WeaponTerrain::NONE;

	switch (m_baseWT)
	{
	case WeaponTerrain::SWORD_WALL:
		EquipWeapon(std::make_unique<Sword>(this, FALSE));
		g_changeP1 = 1;
		break;
	case WeaponTerrain::SPEAR_HILL:
		EquipWeapon(std::make_unique<Spear>(this, FALSE));
		g_changeP1 = 2;
		break;
	case WeaponTerrain::BOW_HILL:
		EquipWeapon(std::make_unique<Arrow>(this, FALSE));
		g_changeP1 = 3;
		break;
	case WeaponTerrain::HAMMER_:
		EquipWeapon(std::make_unique<Hammer>(this, FALSE));
		g_changeP1 = 4;
		break;
	case WeaponTerrain::SHURIKEN_:
		EquipWeapon(std::make_unique<Shuriken>(this, FALSE));
		g_changeP1 = 5;
		break;
	}
}
void PLAYER::RoundReset(XMFLOAT3 startPos)
{
	//�����I�ȏ�Ԃ̃��Z�b�g
    m_position = startPos;
    m_velocity = XMFLOAT3(0, 0, 0);
    m_rotation = XMFLOAT3(0, 0, 0); //�K�v�ɉ�����Y��]�������l��

    //�p�����[�^�̃��Z�b�g
    m_currentHp = m_maxHp; //�̗͑S��
    m_isDead = false;
    State = PLAYER_STATE_IDLE;

    //����ƕϐg��Ԃ��u��������v�ɖ߂�
    EquipBaseWeapon();

	g_Player1AttackPlaying = false;
	g_Player.SetReservedWT(0, WeaponTerrain::NONE);
	g_Player.SetReservedWT(1, WeaponTerrain::NONE);

	gp_itemSponer.ResetItem();
}

void SetWTP1(WeaponTerrain wt)
{
	g_setWTP1 = wt;
}
WeaponTerrain GetSetWTP1()
{
	return g_setWTP1;
}

bool GetChangeP1()
{
	return g_isChangeP1;
}



WeaponTerrain GetPlayerCurrentWT()
{
	return g_Player.m_currentWT;
}
void SetPlayer_IsTransformed(bool isTransformed)
{
	g_Player.m_isTransformed = isTransformed;
}
bool GetPlayer_IsTransformed()
{
	return g_Player.m_isTransformed;
}
int Player_GetTransformCount()
{
	return g_Player.m_transformCount;
}
int Player_GetItemCount()
{
	return g_Player.m_itemCount;

}
int Player_GetLoseCount()
{
	return g_Player.m_loseCount;

}
void Player_PlusTransformCount()
{
	g_Player.m_transformCount += 1;
}
void Player_PlusGetItemCount()
{
	g_Player.m_itemCount += 1;

}
void Player_PlusLoseCount()
{
	g_Player.m_loseCount += 1;

}
void Player_AllCountReset()
{
	g_Player.m_transformCount = 0;
	g_Player.m_itemCount = 0;
	g_Player.m_loseCount = 0;
	g_Player.m_score = 0;
}
void Player_PlusScore(int score)
{
	g_Player.m_score += score;
}
int Player_GetScore()
{
	return g_Player.m_score;
}

void Player_SetPlayerIsAttaking(int flg)
{
	g_Player1AttackPlaying = flg;
}
void Player_ResetMoveMul()
{
	g_Player.m_moveMul = 1.0f;
}
