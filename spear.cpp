/*
* ファイル名	spear.cpp
* タイトル	剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"spear.h"
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
MODEL* g_modelSpear[2] = { NULL, NULL };
PLAYER* g_PlayerSpear1;
PLAYER2* g_PlayerSpear2;
XMFLOAT3 g_moveSpear[2]; // 簡易アニメーション

Spear::Spear(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerSpear1 = GetPlayer();
	g_PlayerSpear2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー

	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.3f, 0.3f, 1.0f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	g_moveSpear[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 0.0f;

	/*********** テストコード **********/
	g_modelSpear[0] = ModelLoad("asset\\model\\FX_spear.fbx");
	m_fxAnim.Bind(g_modelSpear[0]);
	g_modelSpear[1] = ModelLoad("asset\\model\\block2.fbx");
	/*********************************/
}

Spear::~Spear()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Spear::Attack()
{
	if (m_isAttacking) return; // 攻撃してたら終わり
	if (m_coolTime > 0.0f) return;
	m_weapon->m_scale.x = 0.1f;
	m_weapon->m_scale.y = 0.1f;
	m_weapon->m_scale.z = 0.1f;
	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化
	g_moveSpear[m_selectPlayer] = {0.0f, 0.0f, 0.0f};
	m_coolTime = 1.5f;
	m_fxAnim.PlayFrames(1, 60, 60.0f, false, 1.0f);
	m_collider->SetEnable(true); // 当たり判定の有効
	m_weapon->m_delay = 0.1f;
	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Spear::Update()
{
	m_fxAnim.Update(1.0f / 60.0f);
	if (m_coolTime > 0.0f)
	{
		{
			m_coolTime -= 1.0f / 60.0f;
		}
	}

	if (Keyboard_IsKeyDown(KK_LEFTSHIFT))
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
		// キーを離した瞬間に投げる
		Throw(m_chargePower, m_selectPlayer);
		m_isCharging = false;
		m_chargePower = 0.0f;

		// 投げた後のクールタイム
		m_coolTime = 1.5f;
	}

	if (m_attackTimer < (ATTACK_DURATION / 2) && m_isAttacking)
	{
		float progress = m_attackTimer / (ATTACK_DURATION / 2.0f);

		if (progress > 1.0f) progress = 1.0f;

		g_moveSpear[m_selectPlayer].x = m_animePosition.x * progress;
		g_moveSpear[m_selectPlayer].y = m_animePosition.y * progress;
		g_moveSpear[m_selectPlayer].z = m_animePosition.z * progress;
	}
	else
	{
		g_moveSpear[m_selectPlayer].x -= (m_animePosition.x / 30.0f);
		g_moveSpear[m_selectPlayer].y -= (m_animePosition.y / 30.0f);
		g_moveSpear[m_selectPlayer].z -= (m_animePosition.z / 30.0f);

		if (g_moveSpear[m_selectPlayer].x < 0.0f) g_moveSpear[m_selectPlayer].x = 0.0f;
		if (g_moveSpear[m_selectPlayer].y < 0.0f) g_moveSpear[m_selectPlayer].y = 0.0f;
		if (g_moveSpear[m_selectPlayer].z < 0.0f) g_moveSpear[m_selectPlayer].z = 0.0f;
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
			m_offset.x + g_moveSpear[m_selectPlayer].x,
			m_offset.y + g_moveSpear[m_selectPlayer].y,
			m_offset.z + g_moveSpear[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerSpear1->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset1);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerSpear1->m_rotation;
		break;

	case TRUE:
		XMFLOAT3 offset2 =
		{
			m_offset.x + g_moveSpear[m_selectPlayer].x,
			m_offset.y + g_moveSpear[m_selectPlayer].y,
			m_offset.z + g_moveSpear[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerSpear2->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset2);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerSpear2->m_rotation;
		break;

	default:
		break;
	}

	// 攻撃してるとき
	if (m_isAttacking)
	{

		if (m_weapon->m_delay <= 0)
		{
			m_attackTimer += (1.0f / 60.0f);
			// 攻撃の有効時間が終わったら
			if (m_attackTimer >= ATTACK_DURATION)
			{
				m_isAttacking = false; // 攻撃終了
				m_collider->SetEnable(false); // 当たり判定止める
			}
			if (m_weapon->m_scale.x <= 0.5)
			{
				m_weapon->m_scale.x += 0.1f; // 攻撃中は少し細くする
				m_weapon->m_scale.y += 0.1f; // 攻撃中は少し細くする
				m_weapon->m_scale.z += 0.1f; // 攻撃中は少し細くする
			}
		}
		else
		{
			m_weapon->m_delay -= m_weapon->m_frame;
			m_weapon->m_scale.x = 0.0f;
			m_weapon->m_scale.y = 0.0f;
			m_weapon->m_scale.z = 0.0f;
		}
	}

}

void Spear::Draw()
{
	if (m_isAttacking)
	{
		XMMATRIX	scale = XMMatrixScaling(
			m_weapon->m_scale.x * 0.1f,
			m_weapon->m_scale.y * 0.1f,
			m_weapon->m_scale.z * 0.1f);
		XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
			m_weapon->m_rotation.x,
			m_weapon->m_rotation.y + XM_PI,
			m_weapon->m_rotation.z);
		XMMATRIX	translation = XMMatrixTranslation(
			m_weapon->m_position.x,
			m_weapon->m_position.y,
			m_weapon->m_position.z);
		XMMATRIX world = scale * rotation * translation;


		XMMATRIX fxWorld = m_fxAnim.GetDeltaMatrix() * world;

		Shader_SetWorldMatrix(fxWorld);
		ModelDraw(g_modelSpear[0]);
	}
	
}

void Spear::OnWeaponCollision(GameObject* target)
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
				target->TakeDamage(15.0f); // 仮に20ダメージ
			}
			break;

		case TRUE: // 2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				m_hitTargets.insert(target);
				target->TakeDamage(15.0f);
			}
			break;
		}
	}
}

void Spear::Throw(float power, bool select)
{
	SpearShot* shot = new SpearShot();

	shot->m_position = m_weapon->m_position;
	shot->m_rotation = m_weapon->m_rotation;
	shot->m_selectPlayer = select;

	// 飛ばす方向を計算
	float baseSpeed = 0.25f;
	float finalSpeed = baseSpeed * (1.0f + power);
	float ry = shot->m_rotation.y;
	shot->m_velocity.x = sinf(ry) * finalSpeed;
	shot->m_velocity.y = 0.0f;
	shot->m_velocity.z = cosf(ry) * finalSpeed;

	extern std::vector<GameObject*> g_gameObjects;
	g_gameObjects.push_back(shot);
	shot->Start();
}

//================================================================
//	SpearShotクラス
//================================================================
void SpearShot::Start()
{
	m_tag = "Attack";

	XMFLOAT3 scale = { 0.3f, 0.3f, 1.0f };
	m_scale = scale;
	m_collider = AddComponent<BoxCollider>(this, scale);
	ManagerCollider::AddCollider(m_collider);
}

void SpearShot::Update()
{
	// 槍が刺さってたら
	if (m_isStuck)
	{
		m_stuckLife -= (1.0f / 60.0f);
		// タイマーを減らす
		if (m_stuckLife <= 0.0f)
		{
			m_isDead = true;
		}
	}
	else // まだ飛んでたら
	{
		// 飛ばしてからの寿命
		m_flyTimer -= (1.0f / 60.0f);
		if (m_flyTimer <= 0.0f)
		{
			m_isDead = true;
		}

		m_velocity.y -= 0.0025f; // 重力
		// 大きいと重い、小さいとふわっとする

		m_position.x += m_velocity.x;
		m_position.y += m_velocity.y;
		m_position.z += m_velocity.z;

		// 常に先端が飛んでる方向を向く
		m_rotation.x = atan2f(-m_velocity.y, sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z));
	}
}

void SpearShot::Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		m_scale.x,
		m_scale.y,
		m_scale.z);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		m_rotation.x,
		m_rotation.y,
		m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		m_position.x,
		m_position.y,
		m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	ModelDraw(g_modelSpear[0]);
}

void SpearShot::OnCollision(const CollisionInfo& info)
{
	// 刺さってたら何もなし
	if (m_isStuck) return;

	if (info.other->m_tag == "Attack") return; // 武器に当たっても無視
	if (!m_selectPlayer && info.other->m_tag == "Player") return; // 武器はなった本人は無視
	if (m_selectPlayer && info.other->m_tag == "Player2") return; // 武器はなった本人は無視

	m_velocity = { 0.0f, 0.0f, 0.0f };
	m_isStuck = true;

	// 1Pか2Pか
	switch (m_selectPlayer)
	{
	case FALSE: // 1Pだったら
		if (info.other->m_tag == "Player2") // 相手がPlayer2の時のみ
		{
			info.other->TakeDamage(15.0f); // 仮に20ダメージ
			m_isDead = true;
		}
		break;

	case TRUE: // 2Pだったら
		if (info.other->m_tag == "Player") // 相手がPlayerの時のみ
		{
			info.other->TakeDamage(15.0f);
			m_isDead = true;
		}
		break;
	}
}