/*
* ファイル名	shuriken.cpp
* タイトル	手裏剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"syuriken.h"
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
MODEL* g_modelShuriken[2] = { NULL, NULL };
PLAYER* g_PlayerShuriken1;
PLAYER2* g_PlayerShuriken2;
XMFLOAT3 g_moveShuriken[2]; // 簡易アニメーション

Shuriken::Shuriken(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerShuriken1 = GetPlayer();
	g_PlayerShuriken2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー

	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.25f, 0.125f, 0.25f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	g_moveShuriken[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 0.0f;

	/*********** テストコード **********/
	g_modelShuriken[0] = ModelLoad("asset\\model\\block.fbx");
	g_modelShuriken[1] = ModelLoad("asset\\model\\block2.fbx");
	/*********************************/
}

Shuriken::~Shuriken()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Shuriken::Attack()
{
	if (m_restBullet <= 0) return; // 残弾数が無ければだめ
	if (m_coolTime > 0.0f) return; // クールタイム中ならだめ

	Throw(m_selectPlayer);
	m_coolTime = 0.5f;
	m_restBullet--;
}

void Shuriken::Update()
{
	if (m_coolTime > 0.0f)
	{
		{
			m_coolTime -= 1.0f / 60.0f;
		}
	}

	if (Keyboard_IsKeyDown(KK_LEFTSHIFT))
	{
		Reload();
	}

	// キャラに合わせて武器も回転
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
			m_offset.x + g_moveShuriken[m_selectPlayer].x,
			m_offset.y + g_moveShuriken[m_selectPlayer].y,
			m_offset.z + g_moveShuriken[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerShuriken1->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset1);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerShuriken1->m_rotation;
		break;

	case TRUE:
		XMFLOAT3 offset2 =
		{
			m_offset.x + g_moveShuriken[m_selectPlayer].x,
			m_offset.y + g_moveShuriken[m_selectPlayer].y,
			m_offset.z + g_moveShuriken[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerShuriken2->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset2);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerShuriken2->m_rotation;
		break;

	default:
		break;
	}
}

void Shuriken::Draw()
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

	ModelDraw(g_modelShuriken[0]);
}

void Shuriken::OnWeaponCollision(GameObject* target)
{

}

void Shuriken::Throw(bool select)
{
	// 飛ばす角度のオフセット（ラジアンに変換）
	// 45度 = PI / 4
	float angles[] = { -XM_PIDIV2 / 6, 0.0f, XM_PIDIV2 / 6 };
	float baseSpeed = 0.3f;

	for (int i = 0; i < 3; i++)
	{
		ShurikenShot* shot = new ShurikenShot();

		shot->m_position = m_weapon->m_position;
		shot->m_rotation = m_weapon->m_rotation;
		shot->m_selectPlayer = select;

		// 元の向き(y軸回転)にオフセットを加える
		float ry = shot->m_rotation.y + angles[i];

		shot->m_rotation.y = ry;

		// 飛ばす方向を計算
		shot->m_velocity.x = sinf(ry) * baseSpeed;
		shot->m_velocity.y = 0.0f;
		shot->m_velocity.z = cosf(ry) * baseSpeed;

		// ゲームオブジェクトリストに追加
		extern std::vector<GameObject*> g_gameObjects;
		g_gameObjects.push_back(shot);
		shot->Start();
	}
}

void Shuriken::Reload()
{
	m_restBullet = 6;
	m_coolTime = 0.5f;
}

//================================================================
//	ShurikenShotクラス
//================================================================
void ShurikenShot::Start()
{
	m_tag = "Attack";

	XMFLOAT3 scale = { 0.25f, 0.125f, 0.25f };
	m_scale = scale;
	m_collider = AddComponent<BoxCollider>(this, scale);
	ManagerCollider::AddCollider(m_collider);
}

void ShurikenShot::Update()
{
	// 矢が刺さってたら
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

		m_velocity.y = 0.0f; // 重力
		// 大きいと重い、小さいとふわっとする

		m_position.x += m_velocity.x;
		m_position.y += m_velocity.y;
		m_position.z += m_velocity.z;

		// 常に先端が飛んでる方向を向く
		m_rotation.x = atan2f(-m_velocity.y, sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z));
	}
}

void ShurikenShot::Draw()
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

	ModelDraw(g_modelShuriken[0]);
}

void ShurikenShot::OnCollision(const CollisionInfo& info)
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
			info.other->TakeDamage(15.0f);
			m_isDead = true;
			g_Player2.m_isAttacked = true;
		}
		break;

	case TRUE: // 2Pだったら
		if (info.other->m_tag == "Player") // 相手がPlayerの時のみ
		{
			info.other->TakeDamage(15.0f);
			m_isDead = true;
			g_Player.m_isAttacked = true;

		}
		break;
	}
}