/*
* ファイル名	Player2.cpp
* タイトル	プレイヤー2
* 作成者		鈴木豪
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	マクロ定義
//================================================================
#define JUMP_FORCE (0.15f)
#define CLIMB_SPEED (JUMP_FORCE / 2.0f)

//================================================================
//	インクルード
//================================================================
#include"keyboard.h"
#include"Player2.h"
#include"Camera.h"
#include"shader.h"
#include"Evolution.h"
#include"colliderFactory.h"
#include"debug_ostream.h"


//================================================================
//	グローバル変数
//================================================================
//ボールオブジェクト
PLAYER2	g_Player2;
ID3D11Device* g_pDevice2;
ID3D11DeviceContext* g_pContext2;

void Player2Die()
{
	hal::dout << "Player2 died!" << std::endl;
	// ここにゲームオーバー画面への遷移、リスポーン処理など

	g_Player2.m_gameObject->m_isEnable = false;
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_IDLE;
}

void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice2 = pDevice;
	g_pContext2 = pContext;

	g_Player2.m_model = ModelLoad("asset\\model\\ball.fbx");

	g_Player2.m_position = XMFLOAT3(2.0f, 0.5f, 2.0f);
	g_Player2.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player2.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);

	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;

	g_Player2.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);

	g_Player2.m_currentHp = g_Player2.m_maxHp;
	g_Player2.m_isDead = false;

	g_Player2.SetObject(g_Player2.m_position, g_Player2.m_scale, "Player2", 0);
	EvolutionInitialize();

	//Sword* newSword = new Sword();
	//newSword->Initialize(pDevice, pContext);
	//g_Player2.EquipWeapon(newSword);
}
void Player2Finalize()
{
	ModelRelease(g_Player2.m_model);
	if (g_Player2.m_currentWeapon)
	{
		delete g_Player2.m_currentWeapon;
		g_Player2.m_currentWeapon = nullptr;
	}
}
void	Player2Update()
{
	//武器の更新と攻撃終了判定
	if (g_Player2.m_currentWeapon)
	{
		g_Player2.m_currentWeapon->Update(1.0f / 60.0f);
		if (g_Player2.m_currentWeapon->ShouldEndAttack())
		{
			g_Player2.m_currentWeapon->EndAttack();
		}
	}

	//攻撃入力のチェック (例: KK_Oキー)
	if (Keyboard_IsKeyDownTrigger(KK_O))
	{
		if (g_Player2.m_currentWeapon && !g_Player2.m_currentWeapon->IsAttacking())
		{
			g_Player2.m_currentWeapon->StartAttack(g_Player2.m_position, g_Player2.m_rotation);
		}
	}
	Player2_ManualMove();
	//死亡判定
	if (g_Player2.m_currentHp <= 0.0f && !g_Player2.m_isDead)
	{
		g_Player2.m_isDead = true;
		Player2Die();
	}
}

void Player2_ManualMove()
{
	g_Player2.m_gameObject->m_position = g_Player2.m_position;

	// カメラの前方向ベクトル
	float forwardX = GetCameraPosition().x - GetCameraAtPosition().x;
	float forwardZ = GetCameraPosition().z - GetCameraAtPosition().z;

	if (!g_Player2.m_isGround) // 地面についてないときに重力発動
	{
		g_Player2.m_velocity.x += g_Player2.m_acceleration.x;
		g_Player2.m_velocity.y += g_Player2.m_acceleration.y;
		g_Player2.m_velocity.z += g_Player2.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	if (g_Player2.m_isGround)
	{
		g_Player2.m_koyoteTime = 1.0f;
	}
	else
	{
		g_Player2.m_koyoteTime -= 0.1f;
	}

	float len = sqrtf(forwardX * forwardX + forwardZ * forwardZ);
	forwardX /= len;
	forwardZ /= len;

	// カメラの右方向ベクトル
	float rightX = forwardZ;    // 右方向は前方向ベクトルを90度回転
	float rightZ = -forwardX;

	// 移動量初期化
	float moveX = 0.0f;
	float moveZ = 0.0f;

	float speed = 0.0f;
	if (Keyboard_IsKeyDown(KK_U))
	{
		// ベクトルが逆だから移動が逆になる
		speed = -0.1f;
	}
	if (Keyboard_IsKeyDown(KK_J))
	{
		speed = 0.1f;
	}

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	if (Keyboard_IsKeyDown(KK_H))
	{
		strafe = +0.1f;  // 左
	}
	if (Keyboard_IsKeyDown(KK_K))
	{
		strafe = -0.1f;  // 右
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	g_Player2.m_velocity.x = moveX;
	g_Player2.m_velocity.z = moveZ;

	// スペース押した && コヨーテタイムが0.0fより大きい
	if (Keyboard_IsKeyDownTrigger(KK_SPACE) && g_Player2.m_koyoteTime > 0.0f)
	{
		g_Player2.m_velocity.y = JUMP_FORCE;
		g_Player2.m_isGround = false;
		g_Player2.m_koyoteTime = 0.0f;
	}
	else
	{
		g_Player2.m_isGround = false;
	}

	g_Player2.m_position.x += g_Player2.m_velocity.x;
	g_Player2.m_position.z += g_Player2.m_velocity.z;
	g_Player2.m_position.y += g_Player2.m_velocity.y;
}

void	Player2Draw()
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		g_Player2.m_scale.x,
		g_Player2.m_scale.y,
		g_Player2.m_scale.z);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Player2.m_rotation.x,
		g_Player2.m_rotation.y,
		g_Player2.m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		g_Player2.m_position.x,
		g_Player2.m_position.y,
		g_Player2.m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//変換行列作成
	XMMATRIX	view = GetViewMatrix2();
	XMMATRIX	projection = GetProjectionMatrix2();
	XMMATRIX	wvp = world * view * projection;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);
	//Shader_SetMatrix(wvp);

	//モデルの描画リクエスト
	ModelDraw(g_Player2.m_model);
}

XMFLOAT3 GetPlayer2Position()
{
	return g_Player2.m_position;
}

void Player2_Jump()
{
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;
}

PLAYER2* GetPlayer2()
{
	return &g_Player2;
}
void PLAYER2::TakeDamage(float damage)
{
	if (m_isDead) return;

	m_currentHp -= damage;

	hal::dout << "Player2 took " << damage << " damage. HP remaining: " << m_currentHp << std::endl;
}

// 武器を装備する
void PLAYER2::EquipWeapon(IWeapon* weapon)
{
	if (m_currentWeapon)
	{
		delete m_currentWeapon;
	}
	m_currentWeapon = weapon;
}
void PLAYER2::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
		if (info.other->m_tag == "PlayerAttack")
		{
			TakeDamage(10.0f); // 10ダメージを与える
			return;
		}

		// 例えば壁・木だけコリジョン有効
		if (info.other->m_tag == "Wall" ||
			info.other->m_tag == "Tree")
		{
			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (info.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(info.normal.x) + fabs(info.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
			}
		}
		else if (info.other->m_tag == "Lift" ||
			info.other->m_tag == "HILL")
		{
			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += info.normal.x * info.penetration;
			m_position.y += info.normal.y * info.penetration;
			m_position.z += info.normal.z * info.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (info.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(info.normal.x) + fabs(info.normal.z);
			if (horiz > 0.7f)
			{
				m_velocity.x = 0;
				m_velocity.z = 0;
				m_velocity.y = CLIMB_SPEED;
			}
		}
		else
		{
			return; // 他は無視
		}
	}
}

void PLAYER2::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
{
	GameObject* obj = ColliderFactory::CreateBoxObject(
		pos,
		scl,
		tag,
		lay
	);

	m_gameObject = obj;

	for (auto& col : obj->GetColliders<>())
	{
		col->owner = &g_Player2;
	}
}