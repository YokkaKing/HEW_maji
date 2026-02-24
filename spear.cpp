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
#include"Audio.h"
#include"spear.h"
#include "Entry.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
#include "HitEffect.h"
#include"controller.h"
/*********************************/


//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelSpear[2] = { NULL, NULL };
PLAYER* g_PlayerSpear1;
PLAYER2* g_PlayerSpear2;

XMFLOAT3 g_moveSpear[2]; // �ȈՃA�j���[�V����
extern Controller g_Controller[2];

Spear::Spear(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerSpear1 = GetPlayer();
	g_PlayerSpear2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー


	m_selectPlayer = select; // �v���C���[�ݒ� 1P��2P��
	m_playerIndex = (m_selectPlayer == FALSE) ? 0 : 1;
	m_chargeKey = (m_playerIndex == 0) ? KK_C : KK_P;
	m_chargeButton = ControllerButton::X_BUTTON;


	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.45f, 0.45f, 1.0f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), scale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;

	g_moveSpear[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 0.0f;

	m_damageFCount = 0.0f; // ダメージの経過時間
	m_damageFrame = { 0.2f, 0.35f }; // ダメージの有効フレーム

	/*********** テストコード **********/
	g_modelSpear[0] = ModelLoad("asset\\model\\weapon_spear.fbx");
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
	if (!m_isAttack) return;
	m_damageFCount = 0.0f; // ダメージ経過時間をリセット
	PlayAudio(g_spear, false);
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
	m_isAttack = false;

	MODEL* model = nullptr;
	bool isMoving = false;

	if (m_selectPlayer == FALSE)
	{
		PLAYER* player = g_PlayerSpear1;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 420, 479, 60.0f, false, 3.0f);
		}
	}
	else
	{
		PLAYER2* player = g_PlayerSpear2;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
				ModelPlayClip(model, 420, 479, 60.0f, false, 3.0f);

		}
	}
	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Spear::Update()
{
	int controlIdx = GetControllerIndexFromPlayerNo(m_selectPlayer);
	if (controlIdx == -1) return;

	m_fxAnim.Update(1.0f / 60.0f);
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

	if (m_isAttacking)
	{
		m_damageFCount += 1.0f / 60.0f;
	}
	else
	{
		m_damageFCount = 0.0f;
	}

	// ダメージ経過時間が範囲内なら攻撃できる
	if (m_damageFCount > m_damageFrame.x &&
		m_damageFCount < m_damageFrame.y)
	{
		if (!m_collider.get()->IsEnable())
		{
			m_collider.get()->SetEnable(true); // 攻撃有効	
		}
	}
	else
	{
		if (m_collider.get()->IsEnable())
		{
			m_collider.get()->SetEnable(false); // 攻撃無効
		}
	}

	bool inputCharge = false;

	if (Keyboard_IsKeyDown(m_chargeKey) || Keyboard_IsKeyDown(KK_C)) {
		inputCharge = true;

	}
	// �R���g���[���[�`�F�b�N
	if (g_Controller[m_playerIndex].IsConnected()) {
		if (g_Controller[m_playerIndex].IsButtonDown(m_chargeButton)) {
			inputCharge = true;


		}
	}
	if (!m_selectPlayer)
	{
		if (Keyboard_IsKeyDown(KK_C) || g_Controller[0].IsButtonDown(ControllerButton::X_BUTTON))
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
			if (m_chargePower <= 1.0f)
			{
				m_isAttack = true;
				Attack();
				m_isCharging = false;
			}
			else
			{
				// キーを離した瞬間に投げる
				Throw(m_chargePower, m_selectPlayer);
				m_isCharging = false;
				m_chargePower = 0.0f;

				// 投げた後のクールタイム
				m_coolTime = 1.5f;
			}
		}
	}

	if (m_selectPlayer)
	{
		if (Keyboard_IsKeyDown(KK_P) || g_Controller[1].IsButtonPushed(ControllerButton::X_BUTTON))
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
			if (m_chargePower <= 1.0f)
			{
				m_isAttack = true;
				Attack();
				m_isCharging = false;
			}
			else
			{
				// キーを離した瞬間に投げる
				Throw(m_chargePower, m_selectPlayer);
				m_isCharging = false;
				m_chargePower = 0.0f;

				// 投げた後のクールタイム
				m_coolTime = 1.5f;
			}
		}
	}
	const float mul = (m_isCharging || m_isAttacking) ? 0.3f : 1.0f;
	if (m_selectPlayer == FALSE)
	{
		if (g_PlayerSpear1) g_PlayerSpear1->m_moveMul = mul;
	}
	else
	{
		if (g_PlayerSpear2) g_PlayerSpear2->m_moveMul = mul;
	}
	MODEL* model = nullptr;
	bool isMoving = false;
	if (m_playerIndex == 0)
	{
		PLAYER* p = g_PlayerSpear1;
		if (p)
		{
			model = p->m_model;
			float mv = sqrtf(p->m_velocity.x * p->m_velocity.x + p->m_velocity.z * p->m_velocity.z);
			isMoving = (mv > 0.001f);
		}
	}
	else
	{
		PLAYER2* p = g_PlayerSpear2;
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
			ModelPlayClip(model, 761, 810, 60.0f, false, 1.0f);
		}
		m_chargeState = CHARGE_IN;
	}
	if (m_isCharging && isMoving)
	{
		if (m_chargeState != CHARGE_MOVE_LOOP)
		{
			if (model)
			{
				ModelPlayClip(model, 500, 640, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_MOVE_LOOP;
		}
	}
	if (m_isCharging && !isMoving)
	{
		if (m_chargeState == CHARGE_MOVE_LOOP)
		{
			if (model) {
				ModelPlayClip(model, 810, 810, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_HOLD;
		}
		else if (m_chargeState == CHARGE_IN)
		{

			if (model && ModelConsumeClipFinished(model))
			{
				ModelPlayClip(model, 810, 810, 60.0f, true, 1.0f);
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
			m_weapon->m_scale.x * 0.03f,
			m_weapon->m_scale.y * 0.03f,
			m_weapon->m_scale.z * 0.03f);
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
		//ヒットストップ用P1,P2共通変数
		float stopTime = 0.5f;
		// 1Pか2Pか
		switch (m_selectPlayer)
		{
		case FALSE: // 攻撃者が1Pだったら
			if (target->m_tag == "Player2") // 相手がPlayer2の時のみ
			{
				SetPlayer2_IsAttacked(true);
				PlayAudio(g_damageSharp);
				m_hitTargets.insert(target);
				target->TakeDamage(100.0f); // 仮に20ダメージ

				//ヒットエフェクト
				XMFLOAT3 effectPos = target->m_position;
				effectPos.y -= 1.0f;
				HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::ZANGEKI);

				//ヒットバック計算式
				XMFLOAT3 dir = {
					target->m_position.x - owner->m_position.x,
					0.1f,
					target->m_position.z - owner->m_position.z
				};
				//P2に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player2.m_hitAction.triggerHA(dir, stopTime, 0.1);
				//攻撃時に攻撃者側にもヒットストップを入れる
				//時間だけを止めたいため、方向ベクトルとパワーの値は0に
				g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime, 0.0f);
			}
			break;

		case TRUE: // 攻撃者が2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				SetPlayer_IsAttacked(true);
				PlayAudio(g_damageSharp);

				m_hitTargets.insert(target);
				target->TakeDamage(100.0f);

				//ヒットエフェクト
				XMFLOAT3 effectPos = target->m_position;
				effectPos.y -= 1.0f;
				HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::ZANGEKI);

				//ヒットバック計算式
				XMFLOAT3 dir = {
					target->m_position.x - owner->m_position.x,
					0.1f,
					target->m_position.z - owner->m_position.z
				};

				//P1に対してヒットアクションを発動
				//引数:方向vec, HS時間, KB距離
				g_Player.m_hitAction.triggerHA(dir, stopTime, 0.1f);
				//攻撃時に攻撃者側にもヒットストップを入れる
				//時間だけを止めたいため、方向ベクトルとパワーの値は0に
				g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime, 0.0f);
			}
			break;
		}
	}
}

void Spear::Throw(float power, bool select)
{
	PlayAudio(g_spear, false);

	SpearShot* shot = new SpearShot();

	shot->m_position = m_weapon->m_position;
	shot->m_rotation = m_weapon->m_rotation;
	shot->m_selectPlayer = select;

	// 飛ばす方向を計算
	float baseSpeed = 0.4f;
	float finalSpeed = baseSpeed * (1.0f + power);
	float ry = shot->m_rotation.y;
	shot->m_velocity.x = sinf(ry) * finalSpeed;
	shot->m_velocity.y = 0.0f;
	shot->m_velocity.z = cosf(ry) * finalSpeed;
	MODEL* model = nullptr;
	bool isMoving = false;

	if (m_selectPlayer == FALSE)
	{
		PLAYER* player = g_PlayerSpear1;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 810, 879, 60.0f, false, 2.0f);
		}
	}
	else
	{
		PLAYER2* player = g_PlayerSpear2;
		if (player)
		{
			model = player->m_model;

			float mv = sqrtf(player->m_velocity.x * player->m_velocity.x +
				player->m_velocity.z * player->m_velocity.z);
			isMoving = (mv > 0.001f);
			ModelPlayClip(model, 810, 880, 60.0f, false, 2.0f);

		}
	}
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

	XMFLOAT3 scale = {1.0f,1.0f,1.0f };
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
		m_scale.x * 0.015f,
		m_scale.y * 0.015f,
		m_scale.z * 0.015f);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		m_rotation.x * -10.0f,
		m_rotation.y + XM_PI,
		m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		m_position.x,
		m_position.y - 0.6f,
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

	//ヒットストップ時間
	float stopTime = 0.2f;

	// 1Pか2Pか
	switch (m_selectPlayer)
	{
	case FALSE: // 1Pだったら
		if (info.other->m_tag == "Player2") // 相手がPlayer2の時のみ
		{
			SetPlayer2_IsAttacked(true);

			PlayAudio(g_damageSharp, false);
			info.other->TakeDamage(15.0f); // 仮に20ダメージ
			Player_PlusScore(15.0f);
			m_isDead = true;

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
			PlayAudio(g_damageSharp, false);
			info.other->TakeDamage(15.0f);
			Player2_PlusScore(15.0f);
			SetPlayer_IsAttacked(true);

			m_isDead = true;

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