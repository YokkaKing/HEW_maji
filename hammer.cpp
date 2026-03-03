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
#include "Entry.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"keyboard.h"
#include"controller.h"
#include"Manager.h"
#include"hitAction.h"
#include"HitEffect.h"
#include"ChargeEffect.h"
//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelHammer[2] = { NULL, NULL };
PLAYER* g_PlayerHammer1;
PLAYER2* g_PlayerHammer2;
XMFLOAT3 g_moveHammer[2]; // 簡易アニメーション
extern Controller g_Controller[2];


static int GetHammerChargeSoundStageByPower(float p)
{
	if (p < 2.4f) return 0;
	if (p < 3.5f) return 1;
	if (p < 4.5f) return 2;
	if (p < 5.5f) return 3;
	return 4; // 最大段階（ループ）
}

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
	m_chargeButton = ControllerButton::X_BUTTON;

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
	m_coolTime = 0.0f;
	m_chargeSoundStage = -1;
	m_chargeLoopPlaying = false;
	m_charge5Played = false;
	m_charge5WaitTimer = 0.0f;
	m_damageFCount = 0.0f; // ダメージの経過時間
	m_damageFrame = { 0.38f, 0.5f }; // ダメージの有効フレーム

	m_move = { 0.0f, 0.0f, 0.0f };

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
	if (m_isCharging) return;
	if (!m_isAttack) return;

	m_damageFCount = 0.0f;
	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化
	m_move = { 0.0f, 0.0f, 0.0f };
	m_coolTime = 1.5f;
	m_isAttack = false;

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
			ModelPlayClip(model, 280, 419, 60.0f, false, 2.0f);
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
			ModelPlayClip(model, 280, 420, 60.0f, false, 2.0f);
		}
	}

	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Hammer::Update()
{
	int controlIdx = GetControllerIndexFromPlayerNo(m_selectPlayer);
	if (controlIdx == -1) return;

	if (m_coolTime > 0.0f) {
		m_coolTime -= 1.0f / 60.0f;
		if (m_coolTime < 0.0f) m_coolTime = 0.0f;
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
	if (Keyboard_IsKeyDown(m_chargeKey)|| g_Controller[controlIdx].GetRightTrigger() >= 0.9f)
	{
		inputCharge = true;
	}
	//if (m_playerIndex == 0) inputCharge = Keyboard_IsKeyDown(KK_C);//<< キー設定
	//else                    inputCharge = Keyboard_IsKeyDown(KK_P);//<< キー設定
	// コントローラー入力チェック (Xボタン)
	if (g_Controller[m_playerIndex].IsConnected()) {
		if (g_Controller[m_playerIndex].IsButtonDown(m_chargeButton)) {
			inputCharge = true;
		}
	}

	if (inputCharge)
	{
		if (!m_isAttacking && m_coolTime <= 0.0f)
		{
			m_isCharging = true;
			m_chargePower += (1.0f / 60.0f);
			if (m_chargePower > MAX_CHARGE) m_chargePower = MAX_CHARGE;
			if (g_Controller[m_playerIndex].IsConnected())
			{
				float intensity = m_chargePower / MAX_CHARGE;
				g_Controller[m_playerIndex].SetVibration(intensity, intensity);
			}
		}
	}

	else if (m_isCharging)
	{
	
		StopAudio(g_charge1);
		StopAudio(g_charge2);
		StopAudio(g_charge3);
		StopAudio(g_charge4);
		StopAudio(g_charge5);

		m_chargeLoopPlaying = false;
		m_charge5Played = false;
		m_charge5WaitTimer = 0.0f;
		m_chargeSoundStage = -1;

		// そのあと攻撃音
		PlayAudio(g_hammer, false);
		if (g_Controller[m_playerIndex].IsConnected()) {
			g_Controller[m_playerIndex].SetVibration(0.0f, 0.0f);
		}

		PlayAudio(g_hammer, false);
		m_isAttack = true;
		// キーを離した瞬間攻撃
		m_isCharging = false;
		Attack();
	}
	const float mul = (m_isCharging || m_isAttacking) ? 0.3f : 1.0f;
	const float dt = 1.0f / 60.0f;

	if (m_isCharging)
	{
		const int newStage = GetHammerChargeSoundStageByPower(m_chargePower);

		// 段階が変わった瞬間だけ鳴らす
		if (newStage != m_chargeSoundStage)
		{
			// ループ中だったら止める
			if (m_chargeLoopPlaying)
			{
				StopAudio(g_charge5);
				m_chargeLoopPlaying = false;
			}

			// 最大段階用の待機状態をリセット
			m_charge5Played = false;
			m_charge5WaitTimer = 0.0f;

			switch (newStage)
			{
			case 0:
				PlayAudio(g_charge1, false);
				break;
			case 1:
				PlayAudio(g_charge2, false);
				break;
			case 2:
				PlayAudio(g_charge3, false);
				break;
			case 3:
				PlayAudio(g_charge4, false);
				break;
			case 4:
				// 最大段階に入った瞬間は g_charge5 を1回だけ鳴らす
				PlayAudio(g_charge5, false);
				m_charge5Played = true;

				// g_charge5 の長さ分待ってからループへ
				// ここは実音の長さに合わせて調整（例: 0.8秒）
				m_charge5WaitTimer = 0.8f;
				break;
			}

			m_chargeSoundStage = newStage;
		}

		// 最大段階にいる間：g_charge5後に 2~3秒ループへ移行
		if (m_chargeSoundStage == 4 && m_charge5Played && !m_chargeLoopPlaying)
		{
			if (m_charge5WaitTimer > 0.0f)
			{
				m_charge5WaitTimer -= dt;
				if (m_charge5WaitTimer <= 0.0f)
				{
					PlayAudioLoopSection(g_charge5, 1.5f, 3.0f);
					m_chargeLoopPlaying = true;
				}
			}
		}
	}
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
			ModelPlayClip(model, 241, 280, 60.0f, false, 1.0f);
		}
		m_chargeSoundStage = -1;
		m_chargeLoopPlaying = false;
		m_charge5Played = false;
		m_charge5WaitTimer = 0.0f;
		m_chargeState = CHARGE_IN;
	}
	if (m_isCharging && isMoving)
	{
		if (m_chargeState != CHARGE_MOVE_LOOP)
		{
			if (model) {
				ModelPlayClip(model, 421, 540, 60.0f, true, 2.0f);
			}
			m_chargeState = CHARGE_MOVE_LOOP;
		}
	}
	if (m_isCharging && !isMoving)
	{
		if (m_chargeState == CHARGE_MOVE_LOOP)
		{
			
			if (model) {
				ModelPlayClip(model, 280, 280, 60.0f, true, 1.0f);
			}
			m_chargeState = CHARGE_HOLD;
		}
		else if (m_chargeState == CHARGE_IN)
		{
			
			if (model && ModelConsumeClipFinished(model))
			{
				
				ModelPlayClip(model, 280, 280, 60.0f, true, 1.0f);
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

	ChargeType currentCType = ChargeType::HAMMER_C_NONE;

	if (m_isCharging && m_chargePower > 0.0f)
	{
		if (m_chargePower < 2.4f)
		{
			currentCType = ChargeType::HAMMER_C_W;
		}
		else if (m_chargePower < 3.5f)
		{
			currentCType = ChargeType::HAMMER_C_B;
		}
		else if (m_chargePower < 4.5f)
		{
			currentCType = ChargeType::HAMMER_C_G;
		}
		else if (m_chargePower < 5.5f)
		{
			currentCType = ChargeType::HAMMER_C_G;
		}
		else if (m_chargePower >= 5.5f)
		{
			currentCType = ChargeType::HAMMER_C_R;
		}
	}

	//===============================================
	//	ハンマーのチャージエフェクト
	//===============================================
	if (m_isCharging)
	{
		//プレイヤー識別
		int playerIdx = (int)m_selectPlayer;

		//エフェクトの位置をプレイヤーの位置に合わせる
		XMFLOAT3 effectPos = owner->m_position;
		XMFLOAT3 effectOffset = { 0.0f, -0.6f, 0.0f }; // 止まった状態でのチャージエフェクトオフセット

		//Managerにエフェクトの状態を送信
		ChargeEffectManager::GetInstance().SetEffect(
			playerIdx,
			effectPos,
			effectOffset,
			currentCType,
			m_isCharging
		);
	}
	else
	{
		ChargeEffectManager::GetInstance().SetEffect(
			(int)m_selectPlayer,
			owner->m_position,
			{ 0.0f, 0.0f, 0.0f },
			ChargeType::HAMMER_C_NONE,
			false
		);
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
		//ヒットストップ用P1,P2共通変数
		float stopTime1 = 0.3f;
		float stopTime2 = 0.5f;
		float stopTime3 = 0.8f;
		float stopTime4 = 1.2f;
		// 1Pか2Pか
		switch (m_selectPlayer)
		{
		case FALSE: // 1Pだったら
			if (target->m_tag == "Player2") // 相手がPlayer2の時のみ
			{
				float damage = 20.0f;

				PlayAudio(g_damageHammer, false);
				SetPlayer2_IsAttacked(true);
				m_hitTargets.insert(target);

				if (m_chargePower < 2.4f)
				{
					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player2.m_hitAction.triggerHA(dir, stopTime1, 0.1f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime1, 0.0f);
					Player_PlusScore(damage);
				}
				else if (m_chargePower < 3.5f)
				{
					damage = 25.0f;

					if (GetPlayer_IsTransformed())
					{
						damage = 30.0f;
					}

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player2.m_hitAction.triggerHA(dir, stopTime2, 0.2f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime2, 0.0f);
					Player_PlusScore(damage);
				}
				else if (m_chargePower < 4.5f)
				{
					damage = 30.0f;

					if (GetPlayer_IsTransformed())
					{
						damage = 40.0f;
					}

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player2.m_hitAction.triggerHA(dir, stopTime3, 0.3f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime3, 0.0f);
					Player_PlusScore(damage);
				}
				else if (m_chargePower < 5.5f)
				{
					damage = 40.0f;

					if (GetPlayer_IsTransformed())
					{
						damage = 50.0f;
					}

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player2.m_hitAction.triggerHA(dir, stopTime3, 0.3f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime3, 0.0f);
					Player_PlusScore(damage);
				}
				else if (m_chargePower >= 5.5f)
				{
					damage = 50.0f;

					if (GetPlayer_IsTransformed())
					{
						damage = 70.0f;
					}

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};
					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player2.m_hitAction.triggerHA(dir, stopTime4, 0.6f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime4, 0.0f);
					Player_PlusScore(damage);
				}
			}
			break;

		case TRUE: // 2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				float damage = 20.0f;

				PlayAudio(g_damageHammer, false);
				SetPlayer_IsAttacked(true);

				m_hitTargets.insert(target);

				if (m_chargePower < 2.4f)
				{
					target->TakeDamage(damage);

					PlayAudio(g_damageHammer);


					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);


					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player.m_hitAction.triggerHA(dir, stopTime1, 0.1f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime1, 0.0f);
					Player2_PlusScore(damage);
				}
				else if (m_chargePower < 3.5f)
				{
					damage = 25.0f;

					if (GetPlayer2_IsTransformed())
					{
						damage = 30.0f;
					}

					PlayAudio(g_damageHammer);

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player.m_hitAction.triggerHA(dir, stopTime2, 0.2f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime2, 0.0f);
					Player2_PlusScore(damage);
				}
				else if (m_chargePower < 4.5f)
				{
					damage = 30.0f;

					if (GetPlayer2_IsTransformed())
					{
						damage = 40.0f;
					}

					PlayAudio(g_damageHammer);

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player.m_hitAction.triggerHA(dir, stopTime3, 0.3f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime3, 0.0f);
					Player2_PlusScore(damage);
				}
				else if (m_chargePower < 5.5f)
				{
					damage = 40.0f;

					if (GetPlayer2_IsTransformed())
					{
						damage = 50.0f;
					}

					PlayAudio(g_damageHammer);

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player.m_hitAction.triggerHA(dir, stopTime3, 0.3f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime3, 0.0f);
					Player2_PlusScore(damage);
				}
				else if (m_chargePower >= 5.5f)
				{
					damage = 50.0f;

					if (GetPlayer2_IsTransformed())
					{
						damage = 70.0f;
					}

					PlayAudio(g_damageHammer);

					target->TakeDamage(damage);

					//ヒットエフェクト
					XMFLOAT3 effectPos = target->m_position;
					effectPos.y -= 1.0f;
					HitEffectManager::GetInstance().HitEffect(effectPos, EffectType::DAGEKI);

					//ヒットバック計算式
					XMFLOAT3 dir = {
						target->m_position.x - owner->m_position.x,
						0.1f,
						target->m_position.z - owner->m_position.z
					};

					//P2に対してヒットアクションを発動
					//引数:方向vec, HS時間, KB距離
					g_Player.m_hitAction.triggerHA(dir, stopTime4, 0.6f);
					//攻撃時に攻撃者側にもヒットストップを入れる
					//時間だけを止めたいため、方向ベクトルとパワーの値は0に
					g_Player2.m_hitAction.triggerHA({ 0.0f, 0.0f, 0.0f }, stopTime4, 0.0f);
					Player2_PlusScore(damage);
				}
			}
			break;
		}
	}
}

void Hammer::ResetEffect(int select)
{
	ChargeEffectManager::GetInstance().SetEffect(
		select,
		owner->m_position,
		{ 0.0f, 0.0f, 0.0f },
		ChargeType::HAMMER_C_NONE,
		false
	);
}