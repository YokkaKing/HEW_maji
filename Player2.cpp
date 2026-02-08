/*
* ファイル名	Player2.cpp
* タイトル	プレイヤー2
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月02日
*/
//================================================================
//	マクロ定義
//================================================================
//#define JUMP_FORCE (0.15f)
#define CLIMB_SPEED (m_moveSpeed / 2.0f)

//================================================================
//	インクルード
//================================================================
#include"keyboard.h"
#include"controller.h"
#include"Player2.h"
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
#include<memory>
#include"terrain.h"
#include"generateWT.h"
//================================================================
//	グローバル変数
//================================================================
//ボールオブジェクト
PLAYER2	g_Player2;
ID3D11Device* g_pDevice2;
ID3D11DeviceContext* g_pContext2;
extern Controller g_Controller[2]; //ID 0のコントローラーを使用
extern const char* INITIAL_MODEL_PATH_P2;
MODEL* g_modelP2;
WeaponTerrain g_setWTP2; // プレイヤーの武器と地形情報
unsigned int g_changeP2;
static bool g_Player2AttackPlaying = false; // 攻撃ワンショット再生中フラグ
static bool g_Player2JumpPlaying = false; // ジャンプワンショット再生中フラグ
static int g_Player2CurrentAnim = 0; // 0: idle, 1: move, 2: attack 3:jump

void Player2Die()
{
	hal::dout << "Player2 died!" << std::endl;
	// ここにゲームオーバー画面への遷移、リスポーン処理など
	//プレイヤーを非表示にする
	if (g_Player2.m_gameObject != nullptr)
	{
		g_Player2.m_gameObject->m_isEnable = false;
	}
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_IDLE;
	
	//フェードアウトさせてシーンを切り替える
	XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(40.0f, color, FADE_OUT, SCENE_GAME);
}

void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp2)
{
	g_pDevice2 = pDevice;
	g_pContext2 = pContext;
	if (INITIAL_MODEL_PATH_P2 == nullptr) {
		g_Player2.m_model = ModelLoad("asset\\model\\block.fbx"); // 確実に存在するファイル
	}
	else {
		g_Player2.m_model = ModelLoad(INITIAL_MODEL_PATH_P2);
	}
	g_Player2.m_position = XMFLOAT3(10.0f, 0.5f, 1.0f);
	g_Player2.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player2.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Player2.m_tag = "Player2";
	g_Player2.m_layer = 0;

	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;

	g_Player2.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player2.FrictionRate = 0.98f;
	g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE;
	g_Player2.m_currentHp = g_Player2.m_maxHp;
	g_Player2.m_isDead = false;
	g_Player2.m_baseWT = setWTp2;
	g_Player2.m_isAttacked = false;
	g_Player2.m_isTransformed = false;

	// プレイヤーの当たり判定の追加
	auto collider = g_Player2.AddComponent<BoxCollider>(&g_Player2, g_Player2.m_scale);
	ManagerCollider::AddCollider(collider);

	// のちのちセレクト画面から分岐できるようにする
	// 自分をownerとして武器を生成
	g_changeP2 = 0;
	g_setWTP2 = setWTp2;

	//届いた第3引数の中身に応じて条件式で判定、生成するクラスを変える
	//他の武器も同様に生成し、terrainのinitializeでも同じ処理の必要あり
	if (g_setWTP2 == WeaponTerrain::SWORD_WALL)
	{
		g_Player2.m_model = ModelLoad("asset\\model\\default_sword.fbx");
	}
	else if (g_setWTP2 == WeaponTerrain::SPEAR_HILL)
	{
		g_Player2.m_model = ModelLoad("asset\\model\\default_spear.fbx");
	}
	else if (g_setWTP2 == WeaponTerrain::BOW_HILL)
	{
		g_Player2.m_model = ModelLoad("asset\\model\\default_bow.fbx");
	}
	else if (g_setWTP2 == WeaponTerrain::HAMMER_)
	{
		g_Player2.m_model = ModelLoad("asset\\model\\default_hammer.fbx");
	}
	else if (g_setWTP2 == WeaponTerrain::SHURIKEN_)
	{
		g_Player2.m_model = ModelLoad("asset\\model\\default_shuriken.fbx");
	}

	g_Player2.EquipBaseWeapon();
}
void Player2Finalize()
{
	ModelRelease(g_Player2.m_model);
}
void	Player2Update()
{
	TransformPlayer2();           // Eキーで進化タイプを選択
	ApplyTransformEffect2();   // 進化タイプに応じたパラメータを適用
	if (g_Player2.m_isDead)return;	//死亡している場合は更新処理をスキップ
	
//================================================================
//	武器変更処理(一旦)
//================================================================
	/*
	int slotToUse = -1;
	if (Keyboard_IsKeyDownTrigger(KK_D2) && !GetIsUsedA_P2())
	{
		slotToUse = 0;
		g_Player2.m_isTransformed = true;
	}
	if (Keyboard_IsKeyDownTrigger(KK_D9) && !GetIsUsedB_P2())
	{
		slotToUse = 1;
		g_Player2.m_isTransformed = true;
	}

	if (slotToUse != -1)
	{
		// 予約されている変身先を取得
		WeaponTerrain reserved = g_Player2.GetReservedWT(slotToUse);

		// 選択（予約）済みであり、かつ現在変身中でない（または NONE でない）場合
		if (reserved != WeaponTerrain::NONE)
		{
			// 地形の生成（P2用なので第二引数はTRUE）
			TerrainSet(reserved, TRUE);

			//下にある攻撃処理のアニメーションの順と合わせる
			switch (reserved) {
			case WeaponTerrain::SWORD_WALL:
				g_changeP2 = 1;
				g_Player2.EquipWeapon(std::make_unique<Sword>(&g_Player2, TRUE));
				break;
			case WeaponTerrain::SPEAR_HILL:
				g_changeP2 = 2;
				g_Player2.EquipWeapon(std::make_unique<Spear>(&g_Player2, TRUE));
				break;
			case WeaponTerrain::BOW_HILL:
				g_changeP2 = 3;
				g_Player2.EquipWeapon(std::make_unique<Arrow>(&g_Player2, TRUE));
				break;
			case WeaponTerrain::HAMMER_:
				g_changeP2 = 4;
				g_Player2.EquipWeapon(std::make_unique<Hammer>(&g_Player2, TRUE));
				break;
			case WeaponTerrain::SHURIKEN_:
				g_changeP2 = 5;
				g_Player2.EquipWeapon(std::make_unique<Shuriken>(&g_Player2, TRUE));
				break;
			}
			g_setWTP2 = reserved;
			// g_Player2.SetCurrentWT(reserved);
		}
	}
	*/

//================================================================
//	攻撃処理
//================================================================
	// CキーかAボタンで
	if (Keyboard_IsKeyDownTrigger(KK_P) || g_Controller[1].IsButtonPushed(ControllerButton::X_BUTTON))
	{
		// 武器があるか
		if (g_Player2.m_currentWeapon && !g_Player2AttackPlaying)
		{
			g_Player2.m_currentWeapon->Attack(); // 攻撃
			if (g_Player2.m_isTransformed)
			{
				switch (g_changeP2)
				{
				case 1: // Sword
					ModelPlayClip(g_Player2.m_model, 167, 227, 60.0f, false, 2.0f);
					break;
				case 2: // spear
					ModelPlayClip(g_Player2.m_model, 500, 600, 60.0f, false, 4.0f);
					break;
				case 3: // arrow
					ModelPlayClip(g_Player2.m_model, 460, 580, 60.0f, false, 4.0f);
					break;
				case 4: // hammer
					ModelPlayClip(g_Player2.m_model, 360, 539, 60.0f, false, 2.0f);
					break;
				case 5: //shuriken
					ModelPlayClip(g_Player2.m_model, 151, 210, 60.0f, false, 4.0f);
					break;
				}
			}
			else
			{
				switch (g_setWTP2)
				{
				case WeaponTerrain::SWORD_WALL: // Sword
					ModelPlayClip(g_Player2.m_model, 167, 227, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SPEAR_HILL: // spear
					ModelPlayClip(g_Player2.m_model, 500, 600, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::BOW_HILL: // arrow
					ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, false, 4.0f);
					break;
				case WeaponTerrain::HAMMER_: // hammer
					ModelPlayClip(g_Player2.m_model, 360, 539, 60.0f, false, 2.0f);
					break;
				case WeaponTerrain::SHURIKEN_: //shuriken
					ModelPlayClip(g_Player2.m_model, 151, 210, 60.0f, false, 4.0f);
					break;
				}
			}
			
			
			g_Player2AttackPlaying = true;
			g_Player2CurrentAnim = 2; // attack 状態
		}

		hal::dout << "Playerから攻撃した！\n";
	}

//================================================================
//	武器の更新
//================================================================
	if (g_Player2.m_currentWeapon)
	{
		g_Player2.m_currentWeapon->Update();
	}

	Player2_ManualMove();
	//死亡判定
	if (g_Player2.m_currentHp <= 0.0f && !g_Player2.m_isDead)
	{
		g_Player2.m_isDead = true;
		Player2Die();
	}
	//================================================================
	// アニメーション処理
	// ================================================================
	// 移動速度判定
	float moveSpeed = sqrtf(g_Player2.m_velocity.x * g_Player2.m_velocity.x +
		g_Player2.m_velocity.z * g_Player2.m_velocity.z);
	bool isMoving = (moveSpeed > 0.001f);

	// アニメーション状態管理：
	//  - 攻撃ワンショット再生中はその完了を監視し、完了したら移動/待機ループへ復帰
	//  - 攻撃中でなければ移動/待機のループアニメを確実に再生しておく
	if (g_Player2AttackPlaying || g_Player2JumpPlaying)
	{
		// ワンショットクリップが終了したか確認
		if (ModelConsumeClipFinished(g_Player2.m_model))
		{
			// 攻撃アニメ完了: フラグ解除して適切なループへ戻す
			g_Player2AttackPlaying = false;
			g_Player2JumpPlaying = false;
			if (isMoving)
			{
				// 移動ループ
				if (g_Player2CurrentAnim != 1)
				{
					if (g_Player2.m_isTransformed)
					{
						switch (g_changeP2) //移動
						{
						case 1:
							ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case 2:
							ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case 3:
							ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, true, 1.0f);
							break;
						case 4:
							ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case 5:
							ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 1.0f);
							break;
						}
					}
					else
					{
						switch (g_setWTP2) //移動
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 1.0f);
							break;
						}
					}
					
					g_Player2CurrentAnim = 1;
				}
			}
			else
			{
				// 待機ループ（0~60）
				if (g_Player2CurrentAnim != 0)
				{
					if (g_Player2.m_isTransformed)
					{
						switch (g_changeP2) //移動
						{
						case 1:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						case 2:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case 3:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case 4:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case 5:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						}
					}
					else
					{
						switch (g_setWTP2) //移動
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::SHURIKEN_:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						}
					}
					

					g_Player2CurrentAnim = 0;
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
			if (g_Player2CurrentAnim != 1)
			{
				if (g_Player2.m_isTransformed)
				{
					switch (g_changeP2) //移動
					{
					case 1:
						ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case 2:
						ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 2.0f);
						break;
					case 3:
						ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, true, 1.0f);
						break;
					case 4:
						ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case 5:
						ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 1.0f);
						break;
					}
				}
				else
				{
					switch (g_setWTP2) //移動
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::SPEAR_HILL:
						ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 2.0f);
						break;
					case WeaponTerrain::BOW_HILL:
						ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, true, 2.0f);
						break;
					case WeaponTerrain::HAMMER_:
						ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 2.0f);
						break;
					}
				}
				
				g_Player2CurrentAnim = 1;
			}
		}
		else
		{
			if (g_Player2CurrentAnim != 0)
			{
				if (g_Player2.m_isTransformed)
				{
					switch (g_changeP2)
					{
					case 1:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					case 2:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case 3:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case 4:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case 5:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					}
				}
				else
				{
					switch (g_setWTP2)
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					case WeaponTerrain::SPEAR_HILL:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::BOW_HILL:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::HAMMER_:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					}
				}
				
				g_Player2CurrentAnim = 0;
			}
		}
	}

	// アニメーション時間の進行は Update 側で一度だけ行う（フレーム固定レート環境を想定して 1/60 を使用）
	// deltaTime が利用可能ならそちらを使ってください（例: ModelUpdateAnimation(g_Player.m_model, deltaTime);）
	ModelUpdateAnimation(g_Player2.m_model, 1.0f / 60.0f);
	if (g_Player2.m_currentHp <= 0.0f && !g_Player2.m_isDead)
	{
		g_Player2.m_isDead = true;
		Player2Die();
	}

	if (Keyboard_IsKeyDownTrigger(KK_D2) || Keyboard_IsKeyDownTrigger(KK_D9))
	{
		g_Player2CurrentAnim = 0;
	}
}

void Player2_ManualMove()
{
	// カメラの前方向ベクトル
	float forwardX = GetCamera2Position().x - GetCamera2AtPosition().x;
	float forwardZ = GetCamera2Position().z - GetCamera2AtPosition().z;

	if (!g_Player2.m_isGround) // 地面についてないときに重力発動
	{
		g_Player2.m_velocity.x += g_Player2.m_acceleration.x;
		g_Player2.m_velocity.y += g_Player2.m_acceleration.y;
		g_Player2.m_velocity.z += g_Player2.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	if (g_Player2.m_isGround)
	{
		g_Player2.m_koyoteTime = 1.0f;
	}
	else
	{
		g_Player2.m_koyoteTime -= 0.1f;
	}

	float len = sqrtf(forwardX * forwardX + forwardZ * forwardZ);
	forwardX /= len;
	forwardZ /= len;

	// カメラの右方向ベクトル
	float rightX = forwardZ;    // 右方向は前方向ベクトルを90度回転
	float rightZ = -forwardX;

	// 移動量初期化
	float moveX = 0.0f;
	float moveZ = 0.0f;
	float speed = 0.0f;
	float stickY = g_Controller[1].GetLeftStickY();
	if (fabs(stickY) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// ベクトルが逆だから移動が逆になる
		// 左スティック上方向 (+1.0f) で前進 (speed = -0.1f) に対応
		speed = stickY * 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_U))
	{
		speed = -0.1f;
	}
	if (Keyboard_IsKeyDown(KK_J))
	{
		speed = +0.1f;
	}

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	float stickX = g_Controller[1].GetLeftStickX();
	if (fabs(stickX) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// 左スティック左方向 (-1.0f) で左移動 (strafe = +0.1f) に対応
		strafe = stickX * 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_H))
	{
		strafe = +0.1f;  // 左
	}
	if (Keyboard_IsKeyDown(KK_K))
	{
		strafe = -0.1f;  // 右
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	g_Player2.m_velocity.x = moveX*g_Player2.m_moveMul;
	g_Player2.m_velocity.z = moveZ*g_Player2.m_moveMul;

	// モデルの向きを移動方向に合わせる
	XMFLOAT3 moveDir = { g_Player2.m_velocity.x, 0.0f, g_Player2.m_velocity.z };
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
	if (length > 0.001f) // 移動しているときだけ向きを変える
	{
		// Y軸回転角を計算
		g_Player2.m_rotation.y = atan2f(moveDir.x, moveDir.z); // atan2f(X,Z)でY回転
	}

	// スペース押した && コヨーテタイムが0.0fより大きい
	//if (Keyboard_IsKeyDownTrigger(KK_SPACE) && g_Player2.m_koyoteTime > 0.0f)
	if (g_Controller[1].IsButtonPushed(ControllerButton::A_BUTTON) && g_Player2.m_koyoteTime > 0.0f) //Aボタン**
	{
		g_Player2.m_velocity.y = g_Player2.m_jumpForce;
		g_Player2.m_isGround = false;
		g_Player2.m_koyoteTime = 0.0f;
		if (g_Player2.m_isTransformed)
		{
			switch (g_changeP2)
			{
			case 1: // Sword
				ModelPlayClip(g_Player2.m_model, 300, 335, 60.0f, false, 1.0f);
				break;
			case 2: // spear
				ModelPlayClip(g_Player2.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case 3: // arrow
				ModelPlayClip(g_Player2.m_model, 240, 300, 60.0f, false, 1.0f);
				break;
			case 4: // hammer
				ModelPlayClip(g_Player2.m_model, 240, 300, 60.0f, false, 1.0f);
				break;
			case 5: //shuriken
				ModelPlayClip(g_Player2.m_model, 280, 350, 60.0f, false, 1.0f);
				break;
			}
		}
		else
		{
			switch (g_setWTP2)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player2.m_model, 300, 335, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player2.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player2.m_model, 400, 450, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player2.m_model, 240, 300, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player2.m_model, 280, 320, 60.0f, false, 1.0f);
				break;
			}
		}
		g_Player2JumpPlaying = true;
		g_Player2CurrentAnim = 3; // ジャンプ 状態
	}
	else
	{
		g_Player2.m_isGround = false;
	}

	g_Player2.m_position.x += g_Player2.m_velocity.x;
	g_Player2.m_position.z += g_Player2.m_velocity.z;
	g_Player2.m_position.y += g_Player2.m_velocity.y;
}

void	Player2Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		0.01f,
		0.01f,
		0.01f);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Player2.m_rotation.x,
		g_Player2.m_rotation.y+ XM_PI,
		g_Player2.m_rotation.z);

	XMMATRIX	translation = XMMatrixTranslation(
		g_Player2.m_position.x,
		g_Player2.m_position.y - 1.0f,
		g_Player2.m_position.z);

	if (g_setWTP2 == WeaponTerrain::SPEAR_HILL || g_changeP2 == 2) //移動
	{
		    translation = XMMatrixTranslation(
			g_Player2.m_position.x,
			g_Player2.m_position.y - 0.3f,
			g_Player2.m_position.z);
	}

	XMMATRIX	world = scale * rotation * translation;
	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	Shader_SetBones(g_Player2.m_model);
	//モデルの描画リクエスト
	ModelDraw(g_Player2.m_model);

	if (g_Player2.m_currentWeapon)
	{
		g_Player2.m_currentWeapon->Draw();
	}
}

XMFLOAT3 GetPlayer2Position()
{
	return g_Player2.m_position;
}

void Player2_Jump()
{
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;
}
float Player2_GetHp()
{
	return g_Player2.m_currentHp;
}
float Player2_GetMaxHp()
{	
	return g_Player2.m_maxHp;
}
PLAYER2* GetPlayer2()
{
	return &g_Player2;
}

// 武器を装備する
void PLAYER2::EquipWeapon(std::unique_ptr<IWeapon> weapon)
{
	m_currentWeapon = nullptr;
	m_currentWeapon = std::move(weapon);
}

void PLAYER2::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
		if (info.other->m_tag == "Attack")
		{
			// 相手が武器オブジェクト持ってたら
			if (info.other->m_weaponPtr)
			{	
				// 武器の衝突判定を呼び出す
				info.other->m_weaponPtr->OnWeaponCollision(this);
				g_Player2.m_isAttacked = true;
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

		if (info.other->m_tag == "Player")
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

void PLAYER2::RoundReset(XMFLOAT3 startPos)
{
	//物理的な状態のリセット
	m_position = startPos;
	m_velocity = XMFLOAT3(0, 0, 0);
	m_rotation = XMFLOAT3(0, 0, 0); //必要に応じてY回転を初期値に

	//パラメータのリセット
	m_currentHp = m_maxHp; //体力全快
	m_isDead = false;
	State = PLAYER2_STATE_IDLE;

	//武器と変身状態を「初期武器」に戻す
	EquipBaseWeapon();

	g_Player2AttackPlaying = false;
	g_Player2.SetReservedWT(0, WeaponTerrain::NONE);
	g_Player2.SetReservedWT(1, WeaponTerrain::NONE);
}

void PLAYER2::EquipBaseWeapon()
{
	// 現在の変身状態をベースに戻す
	m_currentWT = m_baseWT;
	m_reservedWT[0] = WeaponTerrain::NONE;
	m_reservedWT[1] = WeaponTerrain::NONE;

	switch (m_baseWT)
	{
	case WeaponTerrain::SWORD_WALL:
		EquipWeapon(std::make_unique<Sword>(this, TRUE));
		g_changeP2 = 1;
		break;
	case WeaponTerrain::SPEAR_HILL:
		EquipWeapon(std::make_unique<Spear>(this, TRUE));
		g_changeP2 = 2;
		break;
	case WeaponTerrain::BOW_HILL:
		EquipWeapon(std::make_unique<Arrow>(this, TRUE));
		g_changeP2 = 3;
		break;
	case WeaponTerrain::HAMMER_:
		EquipWeapon(std::make_unique<Hammer>(this, TRUE));
		g_changeP2 = 4;
		break;
	case WeaponTerrain::SHURIKEN_:
		EquipWeapon(std::make_unique<Shuriken>(this, TRUE));
		g_changeP2 = 5;
		break;
	}
}

WeaponTerrain GetSetWTP2()
{
	return g_setWTP2;
}
void SetWTP2(WeaponTerrain wt)
{
	g_setWTP2 = wt;
}

bool GetPlayer2_IsAttacked()
{
	return g_Player2.m_isAttacked;
}
void SetPlayer2_IsAttacked(bool isAttacked)
{
	g_Player2.m_isAttacked = isAttacked;
}