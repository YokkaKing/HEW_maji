/*
* ファイル名	sword.cpp
* タイトル	    剣
* 作成者		三橋拓斗
* 作成日		12月09日
*  更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"sword.h"
#include"managerCollider.h"
#include"debug_ostream.h"

/*********** テストコード **********/
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
/*********************************/

//================================================================
//	グローバル変数
//================================================================
MODEL* g_model = NULL;
PLAYER* g_Player1;
PLAYER2* g_Player2;

Sword::Sword(GameObject* player, bool select) : IWeapon(player)
{
	g_Player1 = GetPlayer();
	g_Player2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー
	
	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.3f, 1.0f, 0.3f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	/*********** テストコード **********/
	g_model = ModelLoad("asset\\model\\block.fbx");
	/*********************************/
}

Sword::~Sword()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Sword::Attack()
{
	if (m_isAttacking) return; // 攻撃してたら終わり

	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化

	m_collider->SetEnable(true); // 当たり判定の有効

	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Sword::Update()
{
	XMMATRIX rotationMatrixY = XMMatrixRotationY(g_Player1->m_rotation.y);

	XMVECTOR offsetVector = XMLoadFloat3(&m_offset);
	XMVECTOR rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);

	XMVECTOR playerPosition = XMLoadFloat3(&owner->m_position);
	XMVECTOR swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
	XMStoreFloat3(&m_weapon->m_position, swordPosition);

	m_weapon->m_rotation = g_Player1->m_rotation;

	switch (m_selectPlayer)
	{
	case FALSE:
		break;

	case TRUE:
		rotationMatrixY = XMMatrixRotationY(g_Player2->m_rotation.y);

		offsetVector = XMLoadFloat3(&m_offset);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);

		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_Player2->m_rotation;
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
		}
	}
}

void Sword::Draw()
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

	//モデルの描画リクエスト
	ModelDraw(g_model);
}

void Sword::OnWeaponCollision(GameObject* target)
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
				target->TakeDamage(20.0f); // 仮に20ダメージ
			}
			break;

		case TRUE: // 2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				m_hitTargets.insert(target);
				target->TakeDamage(20.0f);
			}
			break;
		}
	}
}