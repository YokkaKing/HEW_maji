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
#define CLIMB_SPEED (m_jumpForce / 2.0f)

//================================================================
//	インクルード
//================================================================
#include"keyboard.h"
#include"controller.h"
#include"Player2.h"
#include "Entry.h"
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
#include "Audio.h"
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
MODEL* deadModel2;
WeaponTerrain g_setWTP2; // プレイヤーの武器と地形情報
unsigned int g_changeP2;
static bool g_Player2AttackPlaying = false; // 攻撃ワンショット再生中フラグ
static bool g_Player2JumpPlaying = false; // ジャンプワンショット再生中フラグ
static int g_Player2CurrentAnim = -1; // 0: idle, 1: move, 2: attack 3:jump
bool g_isChangeP2;
XMFLOAT3 gp2_slopeSpeed;
bool gp2_roundReset;

static const float HIT_ANIM_DURATION = 0.35f;

bool gp2_move; // プレイヤーが動いているかのフラグ
bool gp2_koyoteFlag; // コヨーテタイムを回復するかどうか
static bool  g_Player2DeathAnimPlaying = false;
static float g_Player2DeathAnimTimer = 0.0f;
static const float PLAYER2_DEATH_ANIM_LEN = 60.0f / 60.0f;

void Player2Die()
{
	hal::dout << "Player2 died!" << std::endl;

	g_Player2.m_currentWeapon->ResetEffect(1);
	g_Player2.m_currentWeapon = nullptr;

	if (g_Player2.m_gameObject != nullptr)
	{
		g_Player2.m_gameObject->m_isEnable = false;
	}
	
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_IDLE;
	//g_Player2.m_isDeadFlag = true;
	g_Player2.m_model = deadModel2;
	ModelPlayClip(g_Player.m_model, 0, 60, 60.0f, false);

	// 念のため先頭フレーム確定
	ModelUpdateAnimation(g_Player2.m_model, 0.0f);

	g_Player2DeathAnimPlaying = true;
	g_Player2DeathAnimTimer = 0.0f;
	PlayAudio(g_ko, false);
	// ★フェードはManager側で「1秒スロウ後」に開始する
}
void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp2)
{
	g_pDevice2 = pDevice;
	g_pContext2 = pContext;
	deadModel2 = ModelLoad("asset\\model\\dead.fbx");

	if (INITIAL_MODEL_PATH_P2 == nullptr) {
		g_Player2.m_model = ModelLoad("asset\\model\\block.fbx"); // 確実に存在するファイル
	}
	else {
		g_Player2.m_model = ModelLoad(INITIAL_MODEL_PATH_P2);
	}
	g_Player2.m_position = XMFLOAT3(10.0f, 1.0f, 1.0f);
	g_Player2.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player2.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
	g_Player2.m_tag = "Player2";
	g_Player2.m_layer = 0;

	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;
	g_Player2AttackPlaying = false;
	g_Player2.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player2.FrictionRate = 0.98f;
	g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE;
	g_Player2.m_currentHp = g_Player2.m_maxHp;
	g_Player2.m_isDead = false;
	g_Player2.m_baseWT = setWTp2;
	g_Player2.m_isAttacked = false;
	g_Player2.m_isTransformed = false;
	g_Player2.m_moveMul = 1.0f;

	// プレイヤーの当たり判定の追加
	auto collider = g_Player2.AddComponent<BoxCollider>(&g_Player2, g_Player2.m_scale);
	ManagerCollider::AddCollider(collider);

	gp2_roundReset = false; // ラウンドがリセットされる

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
	gp2_slopeSpeed = { 0.0f, 0.0f, 0.0f };
	gp2_move = false; // 最初は動いてない
	gp2_koyoteFlag = false; // 最初はフラグをオフ
	g_isChangeP2 = false;
	// ===== 死亡アニメ状態の初期化（ラウンド開始ごと）=====
	g_Player2DeathAnimPlaying = false;
	g_Player2DeathAnimTimer = 0.0f;

	// 死亡状態を解除（使っている変数名に合わせて）
	g_Player2.m_isDead = false;
	// 描画を再開
	if (g_Player2.m_gameObject != nullptr)
	{
		g_Player2.m_gameObject->m_isEnable = true;
	}

	g_Player2CurrentAnim = -1;
	ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);

	ModelUpdateAnimation(g_Player2.m_model, 0.0f);
}
void Player2Finalize()
{
	ModelRelease(g_Player2.m_model);
}
void	Player2Update()
{
	int ctrlIdx = GetControllerIndexFromPlayerNo(1);

	// --- 揺れ処理の追加 ---
	// ダメージを検知
	float damage = g_Player2.m_lastHp - g_Player2.m_currentHp;
	if (damage > 0.0f)
	{
		// ダメージ量に応じて揺れの強さを設定 (例: ダメージの 0.05倍)
		g_Player2.m_shakeIntensity += damage * 0.02f;
		if (g_Player2.m_shakeIntensity > 1.0f)
		{
			g_Player2.m_shakeIntensity = 1.0f;
		}
	}
	g_Player2.m_lastHp = g_Player2.m_currentHp; // HPを保存

	// 揺れの減衰 (毎フレーム 90% に減らすなど)
	g_Player2.m_shakeIntensity *= 0.9f;
	if (g_Player2.m_shakeIntensity < 0.001f) g_Player2.m_shakeIntensity = 0.0f;

	TransformPlayer2();           // Eキーで進化タイプを選択
	ApplyTransformEffect2();   // 進化タイプに応じたパラメータを適用
	if (g_Player2.m_isAttacked && !g_Player2.m_isDead)
	{
		Player2_StartHitAnim();
		g_Player2.m_isAttacked = false; // ★ここで消す（連続発火防止）
	}

	if (g_Player2.m_hitAnimPlaying)
	{
		g_Player2.m_hitAnimTimer += 1.0f / 60.0f;
		bool finished = ModelConsumeClipFinished(g_Player2.m_model);

		if (g_Player2.m_hitAnimTimer >= HIT_ANIM_DURATION || finished)
		{
			g_Player2.m_hitAnimPlaying = false;
			g_Player2AttackPlaying = false;
			g_Player2JumpPlaying = false;
			g_Player2CurrentAnim = -1;
		}
		else
		{
			ModelUpdateAnimation(g_Player2.m_model, 1.0f / 60.0f);
			return;
		}
	}
	//ヒットアクション
	g_Player2.m_hitAction.Update(g_Player2.m_position);
	//ヒットストップ中ならこの関数自体を抜けるため今後の処理がすべてスキップされる
	if (g_Player2.m_hitAction.IsStopping())
	{
		//Player2_ManualMove();
		//ModelUpdateAnimation(g_Player2.m_model, 1.0f / 60.0f);
		return;
	}
	if (g_Player2.m_isDead && g_Player2DeathAnimPlaying)
	{
		ModelUpdateAnimation(g_Player2.m_model, 1.0f / 60.0f);

		g_Player2DeathAnimTimer += 1.0f / 60.0f;
		if (g_Player2DeathAnimTimer >= PLAYER2_DEATH_ANIM_LEN)
		{
			g_Player2DeathAnimPlaying = false;

			// ★最後のフレーム(60)で停止
			ModelPlayClip(g_Player2.m_model, 60, 60, 60.0f, true);
			ModelUpdateAnimation(g_Player2.m_model, 0.0f);

			// 非表示にしない
		}
		return;
	}
	if (g_Player2.m_isDead)return;	//死亡している場合は更新処理をスキップ
//================================================================
//	武器変更処理(一旦)
//================================================================

//================================================================
//	攻撃処理
//================================================================
	// CキーかAボタンで
	bool attackPushed = (ctrlIdx != 0 && g_Controller[ctrlIdx].GetRightTrigger() >= 0.9f);
	if (Keyboard_IsKeyDownTrigger(KK_P) || attackPushed)
	{
		// 武器があるか
		if (g_Player2.m_currentWeapon && !g_Player2AttackPlaying&&g_Player2.m_currentWeapon->GetCoolTime() ==0.0f&& !g_Player2.m_hitAnimPlaying)
		{
			g_Player2.m_currentWeapon->Attack(); // 攻撃
			if (g_Player2.m_isTransformed)
			{
				switch (g_Player2.m_currentWT)
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

	if (!g_Player2.m_hitAction.IsStopping())
	{
		Player2_ManualMove();
	}
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
	if ((g_Player2AttackPlaying || g_Player2JumpPlaying)&& !g_Player2.m_hitAnimPlaying)
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
						switch (g_Player2.m_currentWT)
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
							break;
						case WeaponTerrain::SHURIKEN_:
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
							ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 1.0f);
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
						switch (g_Player2.m_currentWT)
						{
						case WeaponTerrain::SWORD_WALL:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						case WeaponTerrain::SPEAR_HILL:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::BOW_HILL:
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
							break;
						case WeaponTerrain::HAMMER_:
							ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
							break;
						case WeaponTerrain::SHURIKEN_:
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
							ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
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
					switch (g_Player2.m_currentWT)
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player2.m_model, 120, 165, 60.0f, true, 1.5f);
						break;
					case WeaponTerrain::SPEAR_HILL:
						ModelPlayClip(g_Player2.m_model, 240, 360, 60.0f, true, 2.0f);
						break;
					case WeaponTerrain::BOW_HILL:
						ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 2.0f);
						break;
					case WeaponTerrain::HAMMER_:
						ModelPlayClip(g_Player2.m_model, 180, 240, 60.0f, true, 1.0f);
						break;
					case WeaponTerrain::SHURIKEN_:
						ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 2.0f);
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
						ModelPlayClip(g_Player2.m_model, 121, 150, 60.0f, true, 2.0f);
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
					switch (g_Player2.m_currentWT)
					{
					case WeaponTerrain::SWORD_WALL:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					case WeaponTerrain::SPEAR_HILL:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::BOW_HILL:
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
						break;
					case WeaponTerrain::HAMMER_:
						ModelPlayClip(g_Player2.m_model, 0, 120, 60.0f, true);
						break;
					case WeaponTerrain::SHURIKEN_:
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
						ModelPlayClip(g_Player2.m_model, 0, 60, 60.0f, true);
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
	int ctrlIdx = GetControllerIndexFromPlayerNo(1);
//	if (controllerIdx == -1) return;

	Controller& ctrl = g_Controller[ctrlIdx];

	// カメラの前方向ベクトル
	float forwardX = GetCamera2AtPosition().x - GetCamera2Position().x;
	float forwardZ = GetCamera2AtPosition().z - GetCamera2Position().z;

	gp2_move = false; // 常に動いていないと更新

	if (!g_Player2.m_isGround) // 地面についてないときに重力発動
	{
		g_Player2.m_velocity.x += g_Player2.m_acceleration.x;
		g_Player2.m_velocity.y += g_Player2.m_acceleration.y;
		g_Player2.m_velocity.z += g_Player2.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	if (g_Player2.m_isGround &&
		!gp2_koyoteFlag)
	{
		g_Player2.m_koyoteTime = 1.0f;
	}
	else
	{
		g_Player2.m_koyoteTime -= 0.1f;
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
	
	float stickY = ctrl.GetLeftStickY();
	if (fabs(stickY) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		speed = stickY * 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_U)) speed = +0.1f;
	if (Keyboard_IsKeyDown(KK_J)) speed = -0.1f;

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	float stickX = ctrl.GetLeftStickX();
	if (fabs(stickX) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// 左スティック左方向 (-1.0f) で左移動 (strafe = +0.1f) に対応
		strafe = stickX * 0.1f;
	}
	if (Keyboard_IsKeyDown(KK_H)) strafe = -0.1f;
	if (Keyboard_IsKeyDown(KK_K)) strafe = +0.1f;

	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	if (g_Player2.m_isGround)
	{
		// 入力がないときは、今の速度を少しずつ減衰させる（摩擦の表現）
		if (fabs(moveX) < 0.001f && fabs(moveZ) < 0.001f)
		{
			g_Player2.m_velocity.x *= 0.45f; // 摩擦で止まる
			g_Player2.m_velocity.z *= 0.45f;
		}
		else
		{
			// 入力があるときは、入力方向に速度をセットする
			// ただし、滑っている力を消さないために「加算」に近い形にする
			g_Player2.m_velocity.x += moveX * 0.3f; // 加速度的に足す
			g_Player2.m_velocity.z += moveZ * 0.3f;

			// 最高速制限（これがないと無限に加速する）
			float maxSpeed = 0.15f * g_Player2.m_moveMul;
			float currSpeed = sqrtf(g_Player2.m_velocity.x * g_Player2.m_velocity.x + g_Player2.m_velocity.z * g_Player2.m_velocity.z);
			if (currSpeed > maxSpeed)
			{
				g_Player2.m_velocity.x = (g_Player2.m_velocity.x / currSpeed) * maxSpeed;
				g_Player2.m_velocity.z = (g_Player2.m_velocity.z / currSpeed) * maxSpeed;
			}
		}
	}
	else
	{
		// 空中にいるときは制御を弱くする（または慣性を維持）
		g_Player2.m_velocity.x += moveX * 0.05f;
		g_Player2.m_velocity.z += moveZ * 0.05f;

		float maxSpeed = 0.1f * g_Player2.m_moveMul;
		float currSpeed = sqrtf(g_Player2.m_velocity.x * g_Player2.m_velocity.x + g_Player2.m_velocity.z * g_Player2.m_velocity.z);
		if (currSpeed > maxSpeed)
		{
			g_Player2.m_velocity.x = (g_Player2.m_velocity.x / currSpeed) * maxSpeed;
			g_Player2.m_velocity.z = (g_Player2.m_velocity.z / currSpeed) * maxSpeed;
		}
	}
	// モデルの向きを移動方向に合わせる
	XMFLOAT3 moveDir = { g_Player2.m_velocity.x, 0.0f, g_Player2.m_velocity.z };
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
	if (length > 0.001f) // 移動しているときだけ向きを変える
	{
		// Y軸回転角を計算
		g_Player2.m_rotation.y = atan2f(moveDir.x, moveDir.z); // atan2f(X,Z)でY回転
	}
	// スペース押した && コヨーテタイムが0.0fより大きい
	bool jumpPushed = Keyboard_IsKeyDown(KK_SPACE);
	if (ctrlIdx != -1 && g_Controller[ctrlIdx].IsButtonPushed(ControllerButton::A_BUTTON)) jumpPushed = true;
	if (jumpPushed && g_Player2.m_koyoteTime > 0.0f) //Aボタン**
	{
		g_Player2.m_velocity.y = g_Player2.m_jumpForce;
		g_Player2.m_isGround = false;
		g_Player2.m_koyoteTime = 0.0f;
		if (g_Player2.m_isTransformed)
		{
			switch (g_Player2.m_currentWT)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player2.m_model, 300, 335, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player2.m_model, 361, 420, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player2.m_model, 240, 300, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player2.m_model, 280, 319, 60.0f, false, 1.0f);
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
				ModelPlayClip(g_Player2.m_model, 181, 240, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player2.m_model, 240, 300, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player2.m_model, 280, 319, 60.0f, false, 1.0f);
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

	if (!g_Player2.m_isGround)
	{
		gp2_slopeSpeed.x *= 0.3f;
		gp2_slopeSpeed.z *= 0.3f;
		gp2_slopeSpeed.y *= 0.3f;
	}
	// 地面にいても、入力をしているときは少し滑りを抑えるなどの調整も可能
	if (fabs(moveX) > 0.01f || fabs(moveZ) > 0.01f)
	{
		gp2_slopeSpeed.x *= 0.95f;
		gp2_slopeSpeed.z *= 0.95f;
	}

	g_Player2.m_position.x += (g_Player2.m_velocity.x + gp2_slopeSpeed.x);
	g_Player2.m_position.z += (g_Player2.m_velocity.z + gp2_slopeSpeed.z);
	g_Player2.m_position.y += (g_Player2.m_velocity.y + gp2_slopeSpeed.y);
}

void	Player2Draw()
{
	// --- 揺れオフセットの計算 ---
	float offsetX = 0.0f;
	float offsetY = 0.0f;
	if (g_Player2.m_shakeIntensity > 0.0f)
	{
		// -1.0 ～ 1.0 のランダム値 * 強度
		offsetX = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * g_Player2.m_shakeIntensity;
		offsetY = ((float)rand() / RAND_MAX * 2.0f - 1.0f) * g_Player2.m_shakeIntensity;
	}

	// ワールド行列作成
	XMMATRIX scale = XMMatrixScaling(0.01f, 0.01f, 0.01f);
	XMMATRIX rotation = XMMatrixRotationRollPitchYaw(
		g_Player2.m_rotation.x,
		g_Player2.m_rotation.y + XM_PI,
		g_Player2.m_rotation.z);

	// ★ translation の計算時に offsetX, offsetY を加える
	XMMATRIX translation = XMMatrixTranslation(
		g_Player2.m_position.x + offsetX,
		g_Player2.m_position.y - 1.0f + offsetY,
		g_Player2.m_position.z);

	if (g_Player2.m_position.y < g_Player2.m_position.y - 1.0f)
	{
		g_Player2.m_position.y = g_Player2.m_position.y - 0.99f;
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
	if (m_currentWeapon != nullptr)
	{
		m_currentWeapon->ResetEffect(1);
	}
	m_currentWeapon = nullptr;
	m_currentWeapon = std::move(weapon);
}

void PLAYER2::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;
	if (m_isDead) return; //死亡していたら衝突処理を無視
	if (gp2_roundReset) return;

	gp2_koyoteFlag = false; // 基本false

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
				//g_Player2.m_isAttacked = true;
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

		// 例えば壁・木だけコリジョン有効
		if (info.other->m_tag == "FANCE")
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

		if (info.other->m_tag == "Slope")
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

				if (gp2_move)
				{
					m_position.y += 0.1f;
				}

				// 滑り計算（gp1_slopeSpeed）は使わないので 0 にリセット
				gp2_slopeSpeed = { 0.0f, 0.0f, 0.0f };
			}
		}

		if (info.other->m_tag == "SlopeP2")
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

				if (gp2_move)
				{
					m_position.y += 0.1f;
				}

				// 滑り計算（gp1_slopeSpeed）は使わないので 0 にリセット
				gp2_slopeSpeed = { 0.0f, 0.0f, 0.0f };
			}
		}

		if (info.other->m_tag == "SlopeP1")
		{
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			// 坂道なら normal.y が 0 より大きければ地面とみなす
			if (info.normal.y > 0.1f)
			{
				m_isGround = true;
				if (m_velocity.y < 0) m_velocity.y = 0.0f;
				gp2_koyoteFlag = true; // フラグをオンにする
				m_koyoteTime = 0.0f; // ジャンプできなくする

				// --- gp_speed への計算 ---
				const float slideFriction = 0.05f;
				float slopeSeverity = 1.0f - info.normal.y;
				float slidePower = slopeSeverity * slideFriction;
				const float gravityEffect = 0.02f;

				// m_velocity ではなく gp_speed に加算
				gp2_slopeSpeed.x += info.normal.x * (slidePower + gravityEffect);
				gp2_slopeSpeed.z += info.normal.z * (slidePower + gravityEffect);

				// リミッター
				float maxSlide = 0.08f;
				float speedXZ = sqrtf(gp2_slopeSpeed.x * gp2_slopeSpeed.x + gp2_slopeSpeed.z * gp2_slopeSpeed.z);
				if (speedXZ > maxSlide)
				{
					gp2_slopeSpeed.x = (gp2_slopeSpeed.x / speedXZ) * maxSlide;
					gp2_slopeSpeed.z = (gp2_slopeSpeed.z / speedXZ) * maxSlide;
				}

				m_velocity.x *= 0.0f;
				m_velocity.z *= 0.0f;
			}
		}

		if (info.other->m_tag == "BOGP1")
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

				gp2_slopeSpeed.x *= 0.5f;
				gp2_slopeSpeed.z *= 0.5f;
			}
		}

		if (info.other->m_tag == "WATER")
		{
			XMFLOAT3 bogPos = info.other->m_position;

			float dx = m_position.x - bogPos.x;
			float dz = m_position.z - bogPos.z;
			float distance = sqrtf(dx * dx + dz * dz);

			const float effectRadius = 5.5f;

			if (distance < effectRadius)
			{
				m_velocity.x *= 0.7f;
				m_velocity.z *= 0.7f;

				gp2_slopeSpeed.x *= 0.0f;
				gp2_slopeSpeed.z *= 0.0f;
			}
		}

		if (info.other->m_tag == "LAVA")
		{
			XMFLOAT3 bogPos = info.other->m_position;

			float dx = m_position.x - bogPos.x;
			float dz = m_position.z - bogPos.z;
			float distance = sqrtf(dx * dx + dz * dz);

			const float effectRadius = 3.0f;

			static float coolTime = 0.0f;

			if (distance < effectRadius)
			{
				m_velocity.x *= 0.3f;
				m_velocity.z *= 0.3f;

				gp2_slopeSpeed.x *= 0.5f;
				gp2_slopeSpeed.z *= 0.5f;

				coolTime += 1.0f / 60.0f;

				if (coolTime > 1.0f)
				{
					m_currentHp -= 3.0f;
					coolTime = 0.0f;
				}
			}
			else
			{
				coolTime = 0.0f;
			}
		}

		if (info.other->m_tag == "TREEP1")
		{
			m_velocity.x *= 0.4f;
			m_velocity.z *= 0.4f;

			gp2_slopeSpeed.x *= 0.5f;
			gp2_slopeSpeed.z *= 0.5f;
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
	gp2_roundReset = true;

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

void SetWTP2(WeaponTerrain wt)
{
	g_setWTP2 = wt;
}

WeaponTerrain GetSetWTP2()
{
	return g_setWTP2;
}

bool GetChangeP2()
{
	return g_isChangeP2;
}



bool GetPlayer2_IsAttacked()
{
	return g_Player2.m_isAttacked;
}
void SetPlayer2_IsAttacked(bool isAttacked)
{
	g_Player2.m_isAttacked = isAttacked;
}
void SetPlayer2_IsTransformed(bool isTransformed)
{
	g_Player2.m_isTransformed = isTransformed;
}
bool GetPlayer2_IsTransformed()
{
	return g_Player2.m_isTransformed;
}
int Player2_GetTransformCount()
{
	return g_Player2.m_transformCount;
}
int Player2_GetItemCount()
{
	return g_Player2.m_itemCount;

}
int Player2_GetLoseCount()
{
	return g_Player2.m_loseCount;

}
void Player2_PlusTransformCount()
{
	g_Player2.m_transformCount += 1;
}
void Player2_PlusGetItemCount()
{
	g_Player2.m_itemCount += 1;

}
void Player2_PlusLoseCount()
{
	g_Player2.m_loseCount += 1;

}
void Player2_AllCountReset()
{
	g_Player2.m_transformCount = 0;
	g_Player2.m_itemCount = 0;
	g_Player2.m_loseCount = 0;
	g_Player2.m_score = 0;
}
void Player2_PlusScore(int score)
{
	g_Player2.m_score += score;
}
int Player2_GetScore()
{
	return g_Player2.m_score;
}
void Player2_ResetMoveMul()
{
	g_Player2.m_moveMul = 1.0f;
}
void Player2_SetPlayerIsAttaking(int flg)
{
	g_Player2AttackPlaying = flg;
}
static void Player2_StartHitAnim()
{
	if (!g_Player2.m_model) return;

	const int HIT_START = 600;
	const int HIT_END = 660;

	if (!g_Player2.m_hitAnimPlaying)
	{
		if (g_Player2.m_isTransformed)
		{
			switch (g_Player2.m_currentWT)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player2.m_model, 230, 280, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player2.m_model, 641, 700, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player2.m_model, 151, 180, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player2.m_model, 541, 600, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player2.m_model, 211, 260, 60.0f, false, 1.0f);
				break;
			}
		}
		else
		{
			switch (g_setWTP2)
			{
			case WeaponTerrain::SWORD_WALL: // Sword
				ModelPlayClip(g_Player2.m_model, 230, 280, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::SPEAR_HILL: // spear
				ModelPlayClip(g_Player2.m_model, 641, 700, 60.0f, false, 2.0f);
				break;
			case WeaponTerrain::BOW_HILL: // arrow
				ModelPlayClip(g_Player2.m_model, 150, 180, 60.0f, false, 1.0f);
				break;
			case WeaponTerrain::HAMMER_: // hammer
				ModelPlayClip(g_Player2.m_model, 541, 600, 60.0f, false, 1.0f);
				break;

			case WeaponTerrain::SHURIKEN_: //shuriken
				ModelPlayClip(g_Player2.m_model, 211, 260, 60.0f, false, 1.0f);
				break;
			}
		}
		g_Player2.m_hitAnimPlaying = true;
		g_Player2.m_hitAnimTimer = 0.0f;
	}
}
void Player2_WarmupVisual()
{
	if (!g_Player2.m_model) return;
	ModelUpdateAnimation(g_Player2.m_model, 0.0f);
}
bool Player2_GetIsDead()
{
	return g_Player2.m_isDead;
}