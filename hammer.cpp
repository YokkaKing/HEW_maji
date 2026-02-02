/*
* ファイル名	hammer.cpp
* タイトル	ハンマー
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"hammer.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
#include"Manager.h"

//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelHammer[2] = { NULL, NULL };
PLAYER* g_PlayerHammer1;
PLAYER2* g_PlayerHammer2;
XMFLOAT3 g_moveHammer[2]; // 簡易アニメーション

Hammer::Hammer(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerHammer1 = GetPlayer();
	g_PlayerHammer2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー
	m_chargeState = CHARGE_NONE;
	m_wasCharging = false;
	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか
	m_playerIndex = (m_selectPlayer == FALSE) ? 0 : 1;
	m_chargeKey = (m_playerIndex == 0) ? KK_C : KK_P; //<< キー設定

	m_move = { 0.0f, 0.0f, 0.0f };
	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 1.5f, 1.0f, 1.0f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	m_move = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 0.0f;

	/*********** テストコード **********/
	g_modelHammer[0] = ModelLoad("asset\\model\\block.fbx");
	g_modelHammer[1] = ModelLoad("asset\\model\\block2.fbx");
	/*********************************/
}

Hammer::~Hammer()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Hammer::Attack()
{
	if (m_isAttacking) return; // 攻撃してたら終わり
	if (m_coolTime > 0.0f) return;
	if (m_chargePower < 1.0f) return;
	if (m_isCharging) return;

	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化
	m_move = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 1.5f;

	m_collider->SetEnable(true); // 当たり判定の有効
	MODEL* model = nullptr;
	bool isMoving = false;

	if (m_selectPlayer == FALSE)
	{
		PLAYER* player = g_PlayerHammer1;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 440, 539, 60.0f, false, 2.0f);
		}
	}
	else
	{
		PLAYER2* player = g_PlayerHammer2;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 440, 539, 60.0f, false, 2.0f);
		}
	}

	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Hammer::Update()
{
	if (m_coolTime > 0.0f) {
		m_coolTime -= 1.0f / 60.0f;
		if (m_coolTime < 0.0f) m_coolTime = 0.0f;
	}
	
	bool inputCharge = false;
	if (m_playerIndex == 0) inputCharge = Keyboard_IsKeyDown(KK_C);//<< キー設定
	else                    inputCharge = Keyboard_IsKeyDown(KK_P);//<< キー設定


	if (inputCharge)
	{
		if (!m_isAttacking && m_coolTime <= 0.0f)
		{
			m_isCharging = true;
			m_chargePower += (1.0f / 60.0f);
			if (m_chargePower > MAX_CHARGE) m_chargePower = MAX_CHARGE;
		}
	}
	else if (m_isCharging)
	{
		PlayAudio(g_hammer, false);
		// キーを離した瞬間攻撃
		m_isCharging = false;
		Attack();
	}
	const float mul = (m_isCharging || m_isAttacking) ? 0.3f : 1.0f;

	if (m_selectPlayer == FALSE)
	{
		if (g_PlayerHammer1) g_PlayerHammer1->m_moveMul = mul;
	}
	else
	{
		if (g_PlayerHammer2) g_PlayerHammer2->m_moveMul = mul;
	}
	MODEL* model = nullptr;
	bool isMoving = false;
	if (m_playerIndex == 0)
	{
		PLAYER* p = g_PlayerHammer1;
		if (p)
		{
			model = p->m_model;
			float mv = sqrtf(p->m_velocity.x * p->m_velocity.x + p->m_velocity.z * p->m_velocity.z);
			isMoving = (mv > 0.001f);
		}
	}
	else
	{
		PLAYER2* p = g_PlayerHammer2;
		if (p)
		{
			model = p->m_model;
			float mv = sqrtf(p->m_velocity.x * p->m_velocity.x + p->m_velocity.z * p->m_velocity.z);
			isMoving = (mv > 0.001f);
		}
	}

	if (m_isCharging && !m_wasCharging)
	{
		
		if (model) {
			ModelPlayClip(model, 370, 440, 60.0f, false, 1.0f);
		}
		m_chargeState = CHARGE_IN;
	}
	if (m_isCharging && isMoving)
	{
		if (m_chargeState != CHARGE_MOVE_LOOP)
		{
			if (model) {
				ModelPlayClip(model, 540, 660, 60.0f, true, 2.0f);
			}
			m_chargeState = CHARGE_MOVE_LOOP;
		}
	}
	if (m_isCharging && !isMoving)
	{
		if (m_chargeState == CHARGE_MOVE_LOOP)
		{
			
			if (model) {
				ModelPlayClip(model, 440, 440, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_HOLD;
		}
		else if (m_chargeState == CHARGE_IN)
		{
			
			if (model && ModelConsumeClipFinished(model))
			{
				
				ModelPlayClip(model, 440, 440, 60.0f, true, 1.0f);
				m_chargeState = CHARGE_HOLD;
			}
		}
		else if (m_chargeState == CHARGE_NONE)
		{
			// nothing
		}
	}
	m_wasCharging = m_isCharging;
	if (m_attackTimer < (ATTACK_DURATION / 2) && m_isAttacking)
	{
		float progress = m_attackTimer / (ATTACK_DURATION / 2.0f);

		if (progress > 1.0f) progress = 1.0f;

		m_move.x = m_animePosition.x * progress;
		m_move.y = m_animePosition.y * progress;
		m_move.z = m_animePosition.z * progress;
	}
	else
	{
		m_move.x -= (m_animePosition.x / 30.0f);
		m_move.y -= (m_animePosition.y / 30.0f);
		m_move.z -= (m_animePosition.z / 30.0f);

		if (m_move.x < 0.0f) m_move.x = 0.0f;
		if (m_move.y < 0.0f) m_move.y = 0.0f;
		if (m_move.z < 0.0f) m_move.z = 0.0f;
	}

	XMMATRIX rotationMatrixY;
	XMVECTOR offsetVector;
	XMVECTOR rotatedOffset;
	XMVECTOR playerPosition;
	XMVECTOR swordPosition;

	switch (m_selectPlayer)
	{
	case FALSE:
		XMFLOAT3 offset1 =
		{
			m_offset.x + m_move.x,
			m_offset.y + m_move.y,
			m_offset.z + m_move.z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerHammer1->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset1);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerHammer1->m_rotation;
		break;

	case TRUE:
		XMFLOAT3 offset2 =
		{
			m_offset.x + m_move.x,
			m_offset.y + m_move.y,
			m_offset.z + m_move.z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerHammer2->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset2);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerHammer2->m_rotation;
		break;

	default:
		break;
	}

	// 攻撃してるとき
	if (m_isAttacking)
	{
		m_attackTimer += (1.0f / 60.0f);

		// 攻撃の有効時間が終わったら
		if (m_attackTimer >= ATTACK_DURATION)
		{
			m_isAttacking = false; // 攻撃終了
			m_collider->SetEnable(false); // 当たり判定止める
			m_chargePower = 0.0f;
			m_chargeState = CHARGE_NONE;
		}
	}
}

void Hammer::Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		m_weapon->m_scale.x,
		m_weapon->m_scale.y,
		m_weapon->m_scale.z);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		m_weapon->m_rotation.x,
		m_weapon->m_rotation.y,
		m_weapon->m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		m_weapon->m_position.x,
		m_weapon->m_position.y,
		m_weapon->m_position.z);
	XMMATRIX	world = scale * rotation * translation;
}

void Hammer::OnWeaponCollision(GameObject* target)
{
	// 自分のオーナーだったら飛ばす
	if (target == owner)
	{
		return;
	}

	// 多重ヒット防止、既に一回の攻撃でダメージを与えてたら
	if (m_hitTargets.count(target) > 0)
	{
		return;
	}

	if (m_isAttacking)
	{
		// 1Pか2Pか
		switch (m_selectPlayer)
		{
		case FALSE: // 1Pだったら
			if (target->m_tag == "Player2") // 相手がPlayer2の時のみ
			{
				PlayAudio(g_damageHammer, false);

				m_hitTargets.insert(target);

				if (m_chargePower < 3.5f)
				{
					target->TakeDamage(20.0f);
				}
				else if (m_chargePower < 4.5f)
				{
					target->TakeDamage(30.0f);
				}
				else if (m_chargePower < 5.5f)
				{
					target->TakeDamage(40.0f);
				}
				else if (m_chargePower >= 5.5f)
				{
					target->TakeDamage(70.0f);
				}
			}
			break;

		case TRUE: // 2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				PlayAudio(g_damageHammer, false);

				m_hitTargets.insert(target);

				if (m_chargePower < 3.5f)
				{
					target->TakeDamage(20.0f);
				}
				else if (m_chargePower < 4.5f)
				{
					target->TakeDamage(30.0f);
				}
				else if (m_chargePower < 5.5f)
				{
					target->TakeDamage(40.0f);
				}
				else if (m_chargePower >= 5.5f)
				{
					target->TakeDamage(70.0f);
				}
			}
			break;
		}
	}
}