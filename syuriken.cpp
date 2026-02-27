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
#include"Audio.h"
#include"syuriken.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
#include"HitEffect.h"
#include"controller.h"
/*********************************/


//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelShuriken[2] = { NULL, NULL };
PLAYER* g_PlayerShuriken1;
PLAYER2* g_PlayerShuriken2;
Shuriken* g_shuriken;
XMFLOAT3 g_moveShuriken[2];
extern Controller g_Controller[2];

Shuriken::Shuriken(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerShuriken1 = GetPlayer();
	g_PlayerShuriken2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー


	m_selectPlayer = select; // �v���C���[�ݒ� 1P��2P��
	m_playerIndex = (m_selectPlayer == FALSE) ? 0 : 1;

	m_reloadKey = (m_playerIndex == 0) ? KK_LEFTSHIFT : KK_RIGHTSHIFT;
	m_reloadButton = ControllerButton::Y_BUTTON;
	// ����ɐe�ւ̃|�C���^��ݒ�

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
	g_modelShuriken[0] = ModelLoad("asset\\model\\weapon_shuriken.fbx");
	g_modelShuriken[1] = ModelLoad("asset\\model\\block.fbx");
	/*********************************/
}

Shuriken::~Shuriken()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Shuriken::Attack()
{
	if (m_coolTime > 0.0f) return; // クールタイム中ならだめ
	PlayAudio(g_arrow_shuriken, false);
	Throw(m_selectPlayer);
	m_coolTime = 0.5f;
	
}

void Shuriken::Update()
{
	if (m_coolTime > 0.0f)
	{
		{
			m_coolTime -= 1.0f / 60.0f;
		}
	}
	else
	{
		m_coolTime = 0.0f;
	}

	//bool shouldReload = false;
	//if (Keyboard_IsKeyDown(m_reloadKey)) {
	//	shouldReload = true;
	//}
	//if (g_Controller[m_playerIndex].IsConnected()) {
	//	if (g_Controller[m_playerIndex].IsButtonPushed(m_reloadButton)) {
	//		shouldReload = true;
	//	}
	//}

	//// �����[�h���s
	//if (shouldReload) {
	//	Reload();
	//}
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
		m_weapon->m_scale.x*0.1f,
		m_weapon->m_scale.y * 0.1f,
		m_weapon->m_scale.z * 0.1f);
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
	float baseSpeed = 0.25f;

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

//================================================================
//	ShurikenShotクラス
//================================================================
void ShurikenShot::Start()
{
	m_tag = "Attack";
	m_bounceCooldown = 0.0f;

	XMFLOAT3 scale = { 0.25f, 0.125f, 0.25f };
	m_scale = scale;
	m_collider = AddComponent<BoxCollider>(this, scale);
	ManagerCollider::AddCollider(m_collider);
}

void ShurikenShot::Update()
{
	// クールタイムを減らす
	if (m_bounceCooldown > 0.0f)
	{
		m_bounceCooldown -= (1.0f / 60.0f);
	}

	// 手裏剣が刺さってたら
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
		m_scale.x*0.1f,
		m_scale.y*0.1f,
		m_scale.z*0.1f);
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
	if (!info.other || m_bounceCooldown > 0.0f) return;

	if (info.other->m_tag == "BOUNCE")
	{
		if (m_bounceCount < 1)
		{
			XMVECTOR V = XMLoadFloat3(&m_velocity);
			XMVECTOR N = XMLoadFloat3(&info.normal);

			// 手裏剣が進んでいる方向と法線の関係をチェック
			float dotVal = XMVectorGetX(XMVector3Dot(V, N));

			// 寛容度アップ：法線が自分と同じ向きなら反転させる（これだけで全方位対応）
			if (dotVal > 0)
			{
				N = XMVectorNegate(N);
				dotVal = -dotVal;
			}

			// --- 反射計算 ---
			XMVECTOR R = XMVectorSubtract(V, XMVectorScale(N, 2.0f * dotVal));
			XMStoreFloat3(&m_velocity, R);

			// --- ★押し出しを「もっと寛容」にする ---
			// 0.3fだとまだ壁に引っかかることがあるので、少し多めに弾き出す
			float pushDist = info.penetration + 0.5f;
			m_position.x += XMVectorGetX(N) * pushDist;
			m_position.z += XMVectorGetZ(N) * pushDist;

			m_bounceCount++;

			// ★連続反射しやすくするために、クールタイムを少し短く（0.15 -> 0.1）
			m_bounceCooldown = 0.1f;

			// 反射した瞬間にちょっとだけ速度を上げると「弾いた感」が出ます（お好みで）
			// V = XMVectorScale(R, 1.1f);
			// XMStoreFloat3(&m_velocity, V);

			return;
		}
		else
		{
			m_isDead = true;
			return;
		}
	}

	if (info.other->m_tag == "Attack") return; // 武器に当たっても無視
	if (!m_selectPlayer && info.other->m_tag == "Player") return; // 武器はなった本人は無視
	if (m_selectPlayer && info.other->m_tag == "Player2") return; // 武器はなった本人は無視
	if (info.other->m_tag == "Item") return;
	if (info.other->m_tag == "Slope1") return;
	if (info.other->m_tag == "Slope2") return;
	if (info.other->m_tag == "BOGP1") return;
	if (info.other->m_tag == "BOGP2") return;
	if (info.other->m_tag == "TREEP1") return;
	if (info.other->m_tag == "TREEP2") return;
	if (info.other->m_tag == "WATER") return;
	if (info.other->m_tag == "LAVA") return;

	m_velocity = { 0.0f, 0.0f, 0.0f };
	m_isStuck = true;

	//ヒットストップ用
	float stopTime = 0.0f;

	// 1Pか2Pか
	switch (m_selectPlayer)
	{
	case FALSE: // 1Pだったら
		if (info.other->m_tag == "Player2") // 相手がPlayer2の時のみ
		{
			SetPlayer2_IsAttacked(true);

			PlayAudio(g_damageSharp, false);
			Player_PlusScore(5.0f); // スコア加算

			info.other->TakeDamage(5.0f);
			m_isDead = true;
			g_Player2.m_isAttacked = true;

			//ヒットエフェクト
			XMFLOAT3 effectPos = info.other->m_position;
			effectPos.y -= 1.0f;
			HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::ZANGEKI);

			//ヒットバック計算式
			XMFLOAT3 dir = {
				info.other->m_position.x - this->m_position.x,
				0.1f,
				info.other->m_position.z - this->m_position.z
			};

			//P2に対してヒットアクションを発動
			//引数:方向vec, HS時間, KB距離
			g_Player2.m_hitAction.triggerHA(dir, stopTime, 0.1f);
		}
		break;

	case TRUE: // 2Pだったら
		if (info.other->m_tag == "Player") // 相手がPlayerの時のみ
		{
			SetPlayer_IsAttacked(true);
			PlayAudio(g_damageSharp, false);
			Player2_PlusScore(5.0f); // スコア加算
			info.other->TakeDamage(5.0f);
			m_isDead = true;
			g_Player.m_isAttacked = true;
			//ヒットエフェクト
			XMFLOAT3 effectPos = info.other->m_position;
			effectPos.y -= 1.0f;
			HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::ZANGEKI);

			//ヒットバック計算式
			XMFLOAT3 dir = {
				info.other->m_position.x - this->m_position.x,
				0.1f,
				info.other->m_position.z - this->m_position.z
			};
			//P1に対してヒットアクションを発動
			//引数:方向vec, HS時間, KB距離
			g_Player.m_hitAction.triggerHA(dir, stopTime, 0.1f);
		}
		break;
	}
}

void Shuriken::ResetEffect(int select)
{
	return;
}