
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
#include"Evolution.h"
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

	XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(40.0f, color, FADE_OUT, SCENE_GAME);

}

void PlayerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp1)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH_P1);
	//g_modelP1 = ModelLoad("asset\\model\\block.fbx");

	g_Player.m_position = XMFLOAT3(-10.0f, 0.5f, 1.0f);
	g_Player.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Player.m_tag = "Player";
	g_Player.m_layer = 0;

	g_Player.State = PLAYER_STATE::PLAYER_STATE_MOVE;

	g_Player.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player.FrictionRate = 0.98f;
	g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;
	g_Player.m_currentHp = g_Player.m_maxHp;
	g_Player.m_isDead = false;
	g_Player.m_baseWT = setWTp1;
	g_Player.m_isAttacked = false;
	g_Player.m_isTransformed = false;

	auto collider = g_Player.AddComponent<BoxCollider>(&g_Player, g_Player.m_scale);
	ManagerCollider::AddCollider(collider);

	// のちのちセレクト画面から分岐できるようにする
	// 自分をownerとして武器を生成

	g_changeP1 = 0;
	g_setWTP1 = setWTp1;
	if (g_setWTP1 == WeaponTerrain::SWORD_WALL)
	{
		g_Player.EquipWeapon(std::make_unique<Sword>(&g_Player, FALSE));
		g_Player.m_model = ModelLoad("asset\\model\\default_sword.fbx");
		g_changeP1 = 1;
	}
	else if (g_setWTP1 == WeaponTerrain::SPEAR_HILL)
	{
		g_Player.EquipWeapon(std::make_unique<Spear>(&g_Player, FALSE));
		g_Player.m_model = ModelLoad("asset\\model\\default_spear.fbx");
		g_changeP1 = 2;
	}
	
	else if (g_setWTP1 == WeaponTerrain::BOW_HILL)
	{		
		g_Player.EquipWeapon(std::make_unique<Arrow>(&g_Player, FALSE));
		g_Player.m_model = ModelLoad("asset\\model\\default_bow.fbx");
		g_changeP1 = 3;
	}
	else if (g_setWTP1 == WeaponTerrain::HAMMER_)
	{
		g_Player.EquipWeapon(std::make_unique<Hammer>(&g_Player, FALSE));
		g_Player.m_model = ModelLoad("asset\\model\\default_hammer.fbx");
		g_changeP1 = 4;
	}
	else if (g_setWTP1 == WeaponTerrain::SHURIKEN_)
	{
		g_Player.EquipWeapon(std::make_unique<Shuriken>(&g_Player, FALSE));
		g_Player.m_model = ModelLoad("asset\\model\\char_shuriken_motion.fbx");
		g_changeP1 = 5;
	
	}

	g_Player.EquipBaseWeapon(); //���E���h�����p�ɏ���������đ���
	
}
void PlayerFinalize()
{
	ModelRelease(g_Player.m_model);
}
void	PlayerUpdate()
{
	EvolvePlayer();
	// こいつの中でscaleが1.0fに固定されている
	ApplyEvolutionEffect();   // 進化タイプに応じたパラメータを適用
	if (g_Player.m_isDead)return;	//死亡している場合は更新処理をスキップ

//================================================================
//	武器変更処理(一旦)
//================================================================
	int slotToUse = -1; 
	if (Keyboard_IsKeyDownTrigger(KK_D1))
	{
		slotToUse = 0;
		g_Player.m_isTransformed = true;
	}
	if (Keyboard_IsKeyDownTrigger(KK_D0))
	{
		slotToUse = 1;
		g_Player.m_isTransformed = true;
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
			generateWT_Apply(data, &g_Player, &g_Player2, g_pDevice, g_pContext);
			TerrainSet(reserved, FALSE);
			switch (reserved) {
			case WeaponTerrain::SWORD_WALL: 
				g_changeP1 = 1;
				g_Player.m_model = ModelLoad("asset\\model\\char_sword_motion.fbx"); 
				g_Player.EquipWeapon(std::make_unique<Sword>(&g_Player, FALSE));
				break;
			case WeaponTerrain::SPEAR_HILL:
				g_changeP1 = 2;
				g_Player.m_model = ModelLoad("asset\\model\\spear.fbx"); 
				g_Player.EquipWeapon(std::make_unique<Spear>(&g_Player, FALSE));
				break;
			case WeaponTerrain::BOW_HILL:   
				g_changeP1 = 3;
				g_Player.m_model = ModelLoad("asset\\model\\char_bow_motion_b.fbx");
				g_Player.EquipWeapon(std::make_unique<Arrow>(&g_Player, FALSE));
				break;
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
			g_Player.SetCurrentWT(reserved);
			g_Player.SetReservedWT(slotToUse, WeaponTerrain::NONE);
		}

	}

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
				switch (g_changeP1)
				{
				case 1: // Sword
					ModelPlayClip(g_Player.m_model, 167, 227, 60.0f, false, 2.0f);
					break;
				case 2: // spear
					ModelPlayClip(g_Player.m_model, 500, 600, 60.0f, false, 4.0f);
					break;
				case 3: // arrow
					ModelPlayClip(g_Player.m_model, 460, 580, 60.0f, false, 4.0f);
					break;
				case 4: // hammer
					ModelPlayClip(g_Player.m_model, 360, 539, 60.0f, false, 2.0f);
					break;
				case 5: //shuriken
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
						switch (g_changeP1) //移動
						{
						case 1:
							ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case 2:
							ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case 3:
							ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
							break;
						case 4:
							ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case 5:
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
						switch (g_changeP1)
						{
						case 1:
							ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
							break;
						case 2:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case 3:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case 4:
							ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
							break;
						case 5:
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
					switch (g_changeP1) //移動
					{
					case 1:
						ModelPlayClip(g_Player.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case 2: // spear
						ModelPlayClip(g_Player.m_model, 240, 360, 60.0f, true, 1.5f);
						break;
					case 3: // hammer
						ModelPlayClip(g_Player.m_model, 181, 240, 60.0f, true, 1.0f);
						break;
					case 4: // arrow
						ModelPlayClip(g_Player.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case 5:
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
					switch (g_changeP1) //待機
					{
					case 1:
						ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, true);
						break;
					case 2: // spear
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case 3: // hammer
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case 4: // arrow
						ModelPlayClip(g_Player.m_model, 0, 120, 60.0f, true);
						break;
					case 5:
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

	if (!g_Player.m_isGround) // 地面についてないときに重力発動
	{
		g_Player.m_velocity.x += g_Player.m_acceleration.x;
		g_Player.m_velocity.y += g_Player.m_acceleration.y;
		g_Player.m_velocity.z += g_Player.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	if (g_Player.m_isGround)
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
	}


	if (Keyboard_IsKeyDown(KK_W))
	{
		speed = +0.1f;
	}
	if (Keyboard_IsKeyDown(KK_S))
	{
		speed = -0.1f;
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
	}

	if (Keyboard_IsKeyDown(KK_A))
	{
		strafe = -0.1f;  // 左
	}
	if (Keyboard_IsKeyDown(KK_D))
	{
		strafe = +0.1f;  // 右
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	g_Player.m_velocity.x = moveX * g_Player.m_moveMul;
	g_Player.m_velocity.z = moveZ * g_Player.m_moveMul;

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
	if (g_Controller[0].IsButtonPushed(ControllerButton::A_BUTTON) && g_Player.m_koyoteTime > 0.0f) //Aボタン**
	{
		g_Player.m_velocity.y = g_Player.m_jumpForce;
		g_Player.m_isGround = false;
		g_Player.m_koyoteTime = 0.0f;
		if(g_Player.m_isTransformed)
		{
			switch (g_changeP1)
			{
			case 1: // Sword
				ModelPlayClip(g_Player.m_model, 300, 334, 60.0f, false, 1.0f);
				break;
			case 2: // spear
				ModelPlayClip(g_Player.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case 3: // arrow
				ModelPlayClip(g_Player.m_model, 240, 300, 60.0f, false, 1.0f);
				break;
			case 4: // hammer
				ModelPlayClip(g_Player.m_model, 240, 300, 60.0f, false, 1.0f);
				break;
			case 5: //shuriken
				ModelPlayClip(g_Player.m_model, 280, 320, 60.0f, false, 1.0f);
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

	g_Player.m_position.x += g_Player.m_velocity.x;
	g_Player.m_position.z += g_Player.m_velocity.z;
	g_Player.m_position.y += g_Player.m_velocity.y;
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
	if (g_setWTP1 == WeaponTerrain::SPEAR_HILL||g_changeP1 == 2) //移動
	{
		XMMATRIX	translation = XMMatrixTranslation(
			g_Player.m_position.x,
			g_Player.m_position.y - 0.3f,
			g_Player.m_position.z);
	}
	XMMATRIX	translation = XMMatrixTranslation(
		g_Player.m_position.x,
		g_Player.m_position.y - 0.6f,
		g_Player.m_position.z);
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
	m_currentWeapon = std::move(weapon);
}

void PLAYER::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;
	if (m_isDead) return; //死亡していたら衝突処理を無視

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
			info.other->m_tag == "Tree")
		{
			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (info.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(info.normal.x) + fabs(info.normal.z);
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
		else
		{
			return; // 他は無視
		}
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
}

void PLAYER::EquipBaseWeapon()
{
	// ���݂̕�����N���A
	m_currentWeapon = nullptr;

	// ���݂̕ϐg��Ԃ��x�[�X�ɖ߂�
	m_currentWT = m_baseWT;
	m_reservedWT[2] = WeaponTerrain::NONE;

	// �x�[�X����ɉ����đ������� & �A�j���[�V�����ݒ�
	if (m_baseWT == WeaponTerrain::SWORD_WALL)
	{
		EquipWeapon(std::make_unique<Sword>(this, FALSE)); // P1�Ȃ̂�FALSE
		//extern int g_changeP1; // �O���[�o���ϐ����Q��
		g_changeP1 = 0;        // Sword�p�A�j���[�V����ID
	}
	else if (m_baseWT == WeaponTerrain::SPEAR_HILL)
	{
		EquipWeapon(std::make_unique<Spear>(this, FALSE));
		//extern int g_changeP1;
		g_changeP1 = 1;
	}
	else if (m_baseWT == WeaponTerrain::BOW_HILL)
	{
		EquipWeapon(std::make_unique<Arrow>(this, FALSE));
		//extern int g_changeP1;
		g_changeP1 = 3;
	}
	else if (m_baseWT == WeaponTerrain::HAMMER_)
	{
		EquipWeapon(std::make_unique<Hammer>(this, FALSE));
		//extern int g_changeP1;
		g_changeP1 = 2;
	}
	else if (m_baseWT == WeaponTerrain::SHURIKEN_)
	{
		EquipWeapon(std::make_unique<Shuriken>(this, FALSE));
		//extern int g_changeP1;
		g_changeP1 = 4;
	}
}

WeaponTerrain GetSetWTP1()
{
	return g_setWTP1;
}

//�f�o�b�O�R�[�h
/*
g_changeP1++;
if (g_changeP1 >= 5)
{
	g_changeP1 = 0;
}
switch (g_changeP1)
{
case 0: //sword
	g_Player.EquipWeapon(std::make_unique<Sword>(&g_Player, FALSE));
	g_setWTP1 = WeaponTerrain::SWORD_WALL;
	TerrainSet(WeaponTerrain::SWORD_WALL, FALSE);
	g_Player.m_model = ModelLoad("asset\\model\\char_sword_motion_b.fbx");
	break;

case 1: //spear
	g_Player.EquipWeapon(std::make_unique<Spear>(&g_Player, FALSE));
	g_setWTP1 = WeaponTerrain::SPEAR_HILL;
	TerrainSet(WeaponTerrain::SPEAR_HILL, FALSE);
	//g_Player.m_model = ModelLoad("asset\\model\\char_spear_motion_b.fbx");
	break;

case 2: //hammer
	g_Player.EquipWeapon(std::make_unique<Hammer>(&g_Player, FALSE));
	g_setWTP1 = WeaponTerrain::HAMMER_;
	TerrainSet(WeaponTerrain::HAMMER_, FALSE);
//	g_Player.m_model = ModelLoad("asset\\model\\char_hammer_motion_b.fbx");
	break;

case 3: //arrow
	g_Player.EquipWeapon(std::make_unique<Arrow>(&g_Player, FALSE));
	g_setWTP1 = WeaponTerrain::BOW_HILL;
	TerrainSet(WeaponTerrain::BOW_HILL, FALSE);
	//g_Player.m_model = ModelLoad("asset\\model\\char_arrow_motion_b.fbx");
	break;

case 4: //shuriken
	g_Player.EquipWeapon(std::make_unique<Shuriken>(&g_Player, FALSE));
	g_setWTP1 = WeaponTerrain::SHURIKEN_;
	TerrainSet(WeaponTerrain::SHURIKEN_, FALSE);
	g_Player.m_model = ModelLoad("asset\\model\\char_shuriken_motion_b.fbx");
	break;

default:
	break;
}
*/
