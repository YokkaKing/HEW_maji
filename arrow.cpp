/*
* ファイル名	arrow.cpp
* タイトル	弓
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"arrow.h"
#include "Entry.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
#include"Controller.h"

//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelArrow[2] = { NULL, NULL };
PLAYER* g_PlayerArrow1;
PLAYER2* g_PlayerArrow2;
XMFLOAT3 g_moveArrow[2]; // 簡易アニメーション
extern Controller g_Controller[2];

Arrow::Arrow(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerArrow1 = GetPlayer();
	g_PlayerArrow2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー

	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.2f, 0.8f, 0.2f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	g_moveArrow[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 0.0f;

	/*********** テストコード **********/
	g_modelArrow[0] = ModelLoad("asset\\model\\weapon_arrow.fbx");
	g_modelArrow[1] = ModelLoad("asset\\model\\block2.fbx");
	/*********************************/
}

Arrow::~Arrow()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Arrow::Attack()
{
	
}

void Arrow::Update()
{
	int controlIdx = GetControllerIndexFromPlayerNo(m_selectPlayer);
	if (controlIdx == -1) return;

	if (m_coolTime > 0.0f)
	{
		{
			m_coolTime -= 1.0f / 60.0f;
			if (m_coolTime < 0.0f) m_coolTime = 0.0f;
		}
	}
	else
	{
		m_coolTime = 0.0f;
	}

	if (!m_selectPlayer)
	{
		if (Keyboard_IsKeyDown(KK_C) || g_Controller[controlIdx].GetRightTrigger() >= 0.9f)
		{
			// 攻撃中じゃなければチャージできる
			if (!m_isAttacking && m_coolTime <= 0.0f)
			{
				m_isCharging = true;
				m_chargePower += (1.0f / 60.0f);
				if (m_chargePower > MAX_CHARGE) m_chargePower = MAX_CHARGE;
				if (g_Controller[0].IsConnected()) {
					float intensity = (m_chargePower / MAX_CHARGE) * 0.4f;
					g_Controller[0].SetVibration(intensity, intensity);
				}
			}
		}
		else if (m_isCharging)
		{
			if (g_Controller[controlIdx].IsConnected()) g_Controller[controlIdx].SetVibration(0.0f, 0.0f);
			// キーを離した瞬間に投げる
			Throw(m_chargePower, m_selectPlayer);
			m_isCharging = false;
			m_chargePower = 0.0f;
			PlayAudio(g_arrow_shuriken, false);
			// 投げた後のクールタイム
			if (GetPlayer_IsTransformed())
			{
				m_coolTime = 1.2f;
			}
			else
			{
				m_coolTime = 1.5f;
			}
		}
	}

	if (m_selectPlayer)
	{
		if (Keyboard_IsKeyDown(KK_P) ||g_Controller[controlIdx].GetRightTrigger() >= 0.9f)
		{
			// 攻撃中じゃなければチャージできる
			if (!m_isAttacking && m_coolTime <= 0.0f)
			{
				m_isCharging = true;
				m_chargePower += (1.0f / 60.0f);
				if (m_chargePower > MAX_CHARGE) m_chargePower = MAX_CHARGE;
				if (g_Controller[1].IsConnected()) {
					float intensity = (m_chargePower / MAX_CHARGE) * 0.4f;
					g_Controller[1].SetVibration(intensity, intensity);
				}
			}
		}
		else if (m_isCharging)
		{
			if (g_Controller[controlIdx].IsConnected()) g_Controller[controlIdx].SetVibration(0.0f, 0.0f);
			// キーを離した瞬間に投げる
			Throw(m_chargePower, m_selectPlayer);
			m_isCharging = false;
			m_chargePower = 0.0f;
			PlayAudio(g_arrow_shuriken, false);
			// 投げた後のクールタイム
			if (GetPlayer2_IsTransformed())
			{
				m_coolTime = 1.2f;
			}
			else
			{
				m_coolTime = 1.5f;
			}
		}
	}
	const float mul = (m_isCharging || m_isAttacking) ? 0.3f : 1.0f;
	if (m_selectPlayer == FALSE)
	{
		if (g_PlayerArrow1) g_PlayerArrow1->m_moveMul = mul;
	}
	else
	{
		if (g_PlayerArrow2) g_PlayerArrow2->m_moveMul = mul;
	}

	MODEL* model = nullptr;
	bool isMoving = false;
	if (m_selectPlayer == 0)
	{
		PLAYER* p = g_PlayerArrow1;
		if (p)
		{
			model = p->m_model;
			float mv = sqrtf(p->m_velocity.x * p->m_velocity.x + p->m_velocity.z * p->m_velocity.z);
			isMoving = (mv > 0.001f);
		}
	}
	else
	{
		PLAYER2* p = g_PlayerArrow2;

		if (p)
		{
			model = p->m_model;
			float mv = sqrtf(p->m_velocity.x * p->m_velocity.x + p->m_velocity.z * p->m_velocity.z);
			isMoving = (mv > 0.001f);
		}
	}
	if (m_isCharging && !m_wasCharging)
	{
		if (model)
		{
			ModelPlayClip(model, 301, 374, 60.0f, false, 1.0f);
		}
		m_chargeState = CHARGE_IN;
	}
	if (m_isCharging && isMoving)
	{
		if (m_chargeState != CHARGE_MOVE_LOOP)
		{
			if (model)
			{
				ModelPlayClip(model, 241, 300, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_MOVE_LOOP;
		}
	}
	if (m_isCharging && !isMoving)
	{
		if (m_chargeState == CHARGE_MOVE_LOOP)
		{
			if (model) {
				ModelPlayClip(model, 374, 374, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_HOLD;
		}
		else if (m_chargeState == CHARGE_IN)
		{

			if (model && ModelConsumeClipFinished(model))
			{
				ModelPlayClip(model, 374, 374, 60.0f, true, 1.0f);
				m_chargeState = CHARGE_HOLD;
			}
		}
		else if (m_chargeState == CHARGE_NONE)
		{
			// nothing
		}
	}

	// ����p�t���O���g���ă`���[�W����

	m_wasCharging = m_isCharging;

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
			m_offset.x + g_moveArrow[m_selectPlayer].x,
			m_offset.y + g_moveArrow[m_selectPlayer].y,
			m_offset.z + g_moveArrow[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerArrow1->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset1);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerArrow1->m_rotation;
		break;

	case TRUE:
		XMFLOAT3 offset2 =
		{
			m_offset.x + g_moveArrow[m_selectPlayer].x,
			m_offset.y + g_moveArrow[m_selectPlayer].y,
			m_offset.z + g_moveArrow[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerArrow2->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset2);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerArrow2->m_rotation;
		break;

	default:
		break;
	}
}

void Arrow::Draw()
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

	//ModelDraw(g_modelArrow[0]);
}

void Arrow::OnWeaponCollision(GameObject* target)
{

}

void Arrow::ResetEffect(int select)
{
	return;
}

void Arrow::Throw(float power, bool select)
{
	ArrowShot* shot = new ArrowShot();
	PlayAudio(g_arrow_shuriken, false);
	shot->m_position = m_weapon->m_position;
	shot->m_rotation = m_weapon->m_rotation;
	shot->m_selectPlayer = select;
	shot->m_chargePower = power;

	// --- エイムアシスト実装開始 ---
	// 1. ターゲット（敵プレイヤー）を特定
	GameObject* target = nullptr;
	if (select == false) { // 1Pが投げた場合 (falseは通常1P)
		target = (GameObject*)g_PlayerArrow2;
	}
	else {               // 2Pが投げた場合
		target = (GameObject*)g_PlayerArrow1;
	}

	if (target) {
		// 2. 自分から敵への方向ベクトルを計算 (XZ平面)
		XMVECTOR myPos = XMLoadFloat3(&shot->m_position);
		XMVECTOR targetPos = XMLoadFloat3(&target->m_position);
		XMVECTOR toTarget = XMVectorSubtract(targetPos, myPos);
		toTarget = XMVectorSetY(toTarget, 0.0f); // 高低差は無視

		// 距離が極端に近くないかチェックしてから正規化
		XMVECTOR lengthSq = XMVector3LengthSq(toTarget);
		float lenSq;
		XMStoreFloat(&lenSq, lengthSq);

		if (lenSq > 0.0001f) {
			toTarget = XMVector3Normalize(toTarget);

			// 3. 自分の現在の正面ベクトルを計算 (回転角yから算出)
			float currentRy = shot->m_rotation.y;
			XMVECTOR myForward = XMVectorSet(sinf(currentRy), 0.0f, cosf(currentRy), 0.0f);

			// 4. 角度差（ドット積）を計算
			XMVECTOR dotVec = XMVector3Dot(myForward, toTarget);
			float dot = 0.0f;
			XMStoreFloat(&dot, dotVec);

			// cos(15度) ≒ 0.9659
			// ドット積が0.9659より大きければ、敵が正面15度以内にいる
			if (dot > 0.9659f) {
				// 5. エイムアシスト発動：矢の回転角をターゲットの方向へ書き換える
				shot->m_rotation.y = atan2f(XMVectorGetX(toTarget), XMVectorGetZ(toTarget));
			}
		}
	}
	// --- エイムアシスト実装終了 ---

	// 飛ばす方向を計算 (補正された shot->m_rotation.y を使用)
	float baseSpeed = 0.4f;
	float finalSpeed = baseSpeed * (1.0f + power);
	float ry = shot->m_rotation.y;
	shot->m_velocity.x = sinf(ry) * finalSpeed;
	shot->m_velocity.y = 0.0f;
	shot->m_velocity.z = cosf(ry) * finalSpeed;

	extern std::vector<GameObject*> g_gameObjects;
	g_gameObjects.push_back(shot);
	shot->Start();

	// アニメーション制御
	MODEL* model = nullptr;
	bool isMoving = false;
	if (m_selectPlayer == false)
	{
		PLAYER* player = g_PlayerArrow1;
		if (player)
		{
			model = player->m_model;
			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 374, 418, 60.0f, false, 2.0f);
		}
	}
	else
	{
		PLAYER2* player = g_PlayerArrow2;
		if (player)
		{
			model = player->m_model;
			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 374, 418, 60.0f, false, 2.0f);
		}
	}
}

//================================================================
//	ArrowShotクラス
//================================================================
void ArrowShot::Start()
{
	m_tag = "Attack";

	XMFLOAT3 scale = { 0.2f, 0.2f, 0.2f };
	m_scale = scale;
	m_collider = AddComponent<BoxCollider>(this, scale);
	ManagerCollider::AddCollider(m_collider);
}

void ArrowShot::Update()
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

		m_velocity.y -= 0.001f; // 重力
		// 大きいと重い、小さいとふわっとする

		m_position.x += m_velocity.x;
		m_position.y += m_velocity.y;
		m_position.z += m_velocity.z;

		// 常に先端が飛んでる方向を向く
		m_rotation.x = atan2f(-m_velocity.y, sqrtf(m_velocity.x * m_velocity.x + m_velocity.z * m_velocity.z));
	}
}

void ArrowShot::Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		m_scale.x*0.1f,
		m_scale.y*0.1f,
		m_scale.z*0.1f);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		m_rotation.x * -8.0f,
		m_rotation.y + XM_PI,
		m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		m_position.x,
		m_position.y - 0.5f,
		m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	ModelDraw(g_modelArrow[0]);
}

void ArrowShot::OnCollision(const CollisionInfo& info)
{
	// 刺さってたら何もなし
	if (m_isStuck) return;

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
	float stopTime1 = 0.1f;
	float stopTime2 = 0.2f;
	float stopTime3 = 0.3f;
	float stopTime4 = 0.5f;

	// 1Pか2Pか
	switch (m_selectPlayer)
	{
	case FALSE: // 1Pだったら
		if (info.other->m_tag == "Player2") // 相手がPlayer2の時のみ
		{
			SetPlayer2_IsAttacked(true);

			float damage = 5.0f;

			PlayAudio(g_damageSharp, false);
			if (m_chargePower < 0.5f)
			{
				damage = 5.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 10.0f;
				}

				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player2.m_hitAction.triggerHA(dir, stopTime1, 0.05f);

				Player_PlusScore(damage);
			}
			else if (m_chargePower < 1.0f)
			{
				damage = 10.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 15.0f;
				}

				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player2.m_hitAction.triggerHA(dir, stopTime2, 0.07f);
				Player_PlusScore(damage);
			}
			else if (m_chargePower < 2.0f)
			{
				damage = 15.0f;
				if (GetPlayer_IsTransformed())
				{
					damage = 20.0f;
				}

				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};
				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player2.m_hitAction.triggerHA(dir, stopTime3, 0.09f);
				Player_PlusScore(damage);
			}
			else if (m_chargePower > 2.0f)
			{
				damage = 20.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 25.0f;
				}
				info.other->TakeDamage(damage);

				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player2.m_hitAction.triggerHA(dir, stopTime4, 0.1f);
				Player_PlusScore(damage);
			}
			m_isDead = true;
			g_Player2.m_isAttacked = true;

		}
		break;

	case TRUE: // 2Pだったら
		if (info.other->m_tag == "Player") // 相手がPlayerの時のみ
		{
			SetPlayer_IsAttacked(true);

			float damage = 5.0f;

			PlayAudio(g_damageSharp, false);
			if (m_chargePower < 0.5f)
			{
				damage = 5.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 10.0f;
				}

				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player.m_hitAction.triggerHA(dir, stopTime1, 0.05f);
				Player2_PlusScore(damage);
			}
			else if (m_chargePower < 1.0f)
			{
				damage = 10.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 15.0f;
				}

				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player.m_hitAction.triggerHA(dir, stopTime2, 0.07f);

				Player2_PlusScore(damage);
			}
			else if (m_chargePower < 2.0f)
			{
				damage = 15.0f;
				if (GetPlayer_IsTransformed())
				{
					damage = 20.0f;
				}
				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player.m_hitAction.triggerHA(dir, stopTime3, 0.09f);
				Player2_PlusScore(damage);
			}
			else if (m_chargePower > 2.0f)
			{
				damage = 20.0f;

				if (GetPlayer_IsTransformed())
				{
					damage = 25.0f;
				}

				info.other->TakeDamage(damage);
				//ヒットバック計算式
				XMFLOAT3 dir = {
					info.other->m_position.x - this->m_position.x,
					0.1f,
					info.other->m_position.z - this->m_position.z
				};

				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player.m_hitAction.triggerHA(dir, stopTime4, 0.1f);
				Player2_PlusScore(damage);
			}
			m_isDead = true;
			g_Player.m_isAttacked = true;
		}
		break;
	}
}