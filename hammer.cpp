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
#include"hammer.h"
#include"debug_ostream.h"

/*********** テストコード **********/
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
/*********************************/

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

	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 1.5f, 1.0f, 1.0f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	g_moveHammer[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
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
	if (m_chargePower < 2.5f) return;
	if (m_isCharging) return;

	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化
	g_moveHammer[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 1.5f;

	m_collider->SetEnable(true); // 当たり判定の有効

	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Hammer::Update()
{
	if (m_coolTime > 0.0f)
	{
		{
			m_coolTime -= 1.0f / 60.0f;
		}
	}

	if (Keyboard_IsKeyDown(KK_C))
	{
		// 攻撃中じゃなければチャージできる
		if (!m_isAttacking && m_coolTime <= 0.0f)
		{
			m_isCharging = true;
			m_chargePower += (1.0f / 60.0f);
			if (m_chargePower > MAX_CHARGE) m_chargePower = MAX_CHARGE;
		}
	}
	else if (m_isCharging)
	{
		// キーを離した瞬間攻撃
		m_isCharging = false;
		Attack();
	}

	if (m_attackTimer < (ATTACK_DURATION / 2) && m_isAttacking)
	{
		float progress = m_attackTimer / (ATTACK_DURATION / 2.0f);

		if (progress > 1.0f) progress = 1.0f;

		g_moveHammer[m_selectPlayer].x = m_animePosition.x * progress;
		g_moveHammer[m_selectPlayer].y = m_animePosition.y * progress;
		g_moveHammer[m_selectPlayer].z = m_animePosition.z * progress;
	}
	else
	{
		g_moveHammer[m_selectPlayer].x -= (m_animePosition.x / 30.0f);
		g_moveHammer[m_selectPlayer].y -= (m_animePosition.y / 30.0f);
		g_moveHammer[m_selectPlayer].z -= (m_animePosition.z / 30.0f);

		if (g_moveHammer[m_selectPlayer].x < 0.0f) g_moveHammer[m_selectPlayer].x = 0.0f;
		if (g_moveHammer[m_selectPlayer].y < 0.0f) g_moveHammer[m_selectPlayer].y = 0.0f;
		if (g_moveHammer[m_selectPlayer].z < 0.0f) g_moveHammer[m_selectPlayer].z = 0.0f;
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
			m_offset.x + g_moveHammer[m_selectPlayer].x,
			m_offset.y + g_moveHammer[m_selectPlayer].y,
			m_offset.z + g_moveHammer[m_selectPlayer].z
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
			m_offset.x + g_moveHammer[m_selectPlayer].x,
			m_offset.y + g_moveHammer[m_selectPlayer].y,
			m_offset.z + g_moveHammer[m_selectPlayer].z
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

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	if (m_isAttacking)
	{
		ModelDraw(g_modelHammer[1]);
	}
	else
	{
		ModelDraw(g_modelHammer[0]);
	}
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