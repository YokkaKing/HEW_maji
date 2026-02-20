/*
* ファイル名	sword.cpp
* タイトル	剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"sword.h"
#include"managerCollider.h"
#include"debug_ostream.h"
#include"model.h"
#include"Camera.h"
#include"Player.h"
#include"Player2.h"
#include"hitAction.h"
#include"HitEffect.h"
//================================================================
//	グローバル変数
//================================================================
MODEL* g_modelSword[2] = { NULL,NULL };
PLAYER* g_PlayerSword1;
PLAYER2* g_PlayerSword2;
XMFLOAT3 g_moveSword[2]; // 簡易アニメーション

Sword::Sword(GameObject* player, bool select) : IWeapon(player)
{
	g_PlayerSword1 = GetPlayer();
	g_PlayerSword2 = GetPlayer2();

	// 武器の当たり判定の作成
	m_weapon = std::make_unique<GameObject>();
	m_weapon->m_tag = "Attack";	// タグ
	m_weapon->m_layer = 0;		// レイヤー
	
	m_selectPlayer = select; // プレイヤー設定 1Pか2Pか

	// 武器に親へのポインタを設定
	m_weapon->m_weaponPtr = this;

	XMFLOAT3 scale = { 0.3f, 1.0f, 0.3f };
	XMFLOAT3 bukiScale = { 1.5f, 1.0f, 1.0f };
	m_collider = m_weapon->AddComponent<BoxCollider>(m_weapon.get(), bukiScale);

	m_weapon->m_scale = scale;
	m_weapon->m_rotation = { 0.0f, 0.0f, 0.0f };

	ManagerCollider::AddCollider(m_collider); // 登録

	m_collider->SetEnable(false); // 最初は当たり判定を無効化

	m_attackTimer = 0.0f;
	m_coolTime = 0.0f;

	m_damageFCount = 0.0f; // ダメージの経過時間
	m_damageFrame = { 0.2f, 0.3f }; // ダメージの有効フレーム

	g_moveSword[m_selectPlayer] = { 0.0f, 0.0f, 0.0f };

	g_modelSword[0] = ModelLoad("asset\\model\\FX_sword.fbx");
	m_fxAnim.Bind(g_modelSword[0]);
	g_modelSword[1] = ModelLoad("asset\\model\\block2.fbx");
}

Sword::~Sword()
{
	ManagerCollider::RemoveCollider(m_collider); // 削除
}

void Sword::Attack()
{
	if (m_isAttacking) return; // 攻撃してたら終わり
	if (m_coolTime > 0.0f) return;
	PlayAudio(g_sword, false);
	m_damageFCount = 0.0f;
	m_isAttacking = true; // 攻撃している
	m_attackTimer = 0.0f; // 攻撃タイマー初期化
	g_moveSword[m_selectPlayer] = {0.0f, 0.0f, 0.0f}; // 簡易アニメーションの初期化
	m_coolTime = 1.0f; // クールタイムの設定
	m_fxAnim.PlayFrames(1, 20, 30.0f, false, 1.0f);
	m_collider->SetEnable(true); // 当たり判定の有効

	// 多重ヒット帽子リストをリセット
	m_hitTargets.clear();
}

void Sword::Update()
{
	m_fxAnim.Update(1.0f / 60.0f);
	if (m_coolTime > 0.0f)
	{
		m_coolTime -= 1.0f / 60.0f; 
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

	if (m_attackTimer < (ATTACK_DURATION / 2) && m_isAttacking)
	{
		float progress = m_attackTimer / (ATTACK_DURATION / 2.0f);

		if (progress > 1.0f) progress = 1.0f;

		g_moveSword[m_selectPlayer].x = m_animePosition.x * progress;
		g_moveSword[m_selectPlayer].y = m_animePosition.y * progress;
		g_moveSword[m_selectPlayer].z = m_animePosition.z * progress;
	}
	else
	{
		g_moveSword[m_selectPlayer].x -= (m_animePosition.x / 30.0f);
		g_moveSword[m_selectPlayer].y -= (m_animePosition.y / 30.0f);
		g_moveSword[m_selectPlayer].z -= (m_animePosition.z / 30.0f);

		if (g_moveSword[m_selectPlayer].x < 0.0f)
		{
			g_moveSword[m_selectPlayer].x = 0.0f;
		}
		if (g_moveSword[m_selectPlayer].y < 0.0f)
		{
			g_moveSword[m_selectPlayer].y = 0.0f;
		}
		if (g_moveSword[m_selectPlayer].z < 0.0f)
		{
			g_moveSword[m_selectPlayer].z = 0.0f;
		}
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
			m_offset.x + g_moveSword[m_selectPlayer].x,
			m_offset.y + g_moveSword[m_selectPlayer].y,
			m_offset.z + g_moveSword[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerSword1->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset1);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerSword1->m_rotation;
		break;

	case TRUE:
		XMFLOAT3 offset2 =
		{
			m_offset.x + g_moveSword[m_selectPlayer].x,
			m_offset.y + g_moveSword[m_selectPlayer].y,
			m_offset.z + g_moveSword[m_selectPlayer].z
		};

		rotationMatrixY = XMMatrixRotationY(g_PlayerSword2->m_rotation.y);
		offsetVector = XMLoadFloat3(&offset2);
		rotatedOffset = XMVector3Transform(offsetVector, rotationMatrixY);
		playerPosition = XMLoadFloat3(&owner->m_position);
		swordPosition = XMVectorAdd(playerPosition, rotatedOffset);
		XMStoreFloat3(&m_weapon->m_position, swordPosition);

		m_weapon->m_rotation = g_PlayerSword2->m_rotation;
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
	if (m_isAttacking)
	{
		XMMATRIX	scale = XMMatrixScaling(
			m_weapon->m_scale.x * 0.04f,
			m_weapon->m_scale.y * 0.02f,
			m_weapon->m_scale.z * 0.02f);
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


		//Shader_SetBones(g_modelSword[0]);
		ModelDraw(g_modelSword[0]);
		
	}

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
		//ヒットストップ用P1,P2共通変数
		float stopTime = 0.3f;
		// 1Pか2Pか
		switch (m_selectPlayer)
		{
		case FALSE: // 1Pだったら
			if (target->m_tag == "Player2") // 相手がPlayer2の時のみ
			{
				PlayAudio(g_damageSharp, false);
				m_hitTargets.insert(target);

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
				target->TakeDamage(15.0f); // 仮に20ダメージ
				Player_PlusScore(15); // スコア加算
			}
			break;

		case TRUE: // 2Pだったら
			if (target->m_tag == "Player") // 相手がPlayerの時のみ
			{
				PlayAudio(g_damageSharp, false);
				m_hitTargets.insert(target);

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
				target->TakeDamage(15.0f);
				Player2_PlusScore(15); // スコア加算
			}
			break;
		}
	}
}