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
#include"controller.h"
#include"Player2.h"
#include"Camera.h"
#include"shader.h"
#include"Evolution.h"
#include"colliderFactory.h"
#include"debug_ostream.h"
#include"fade.h"
#include"sword.h"
#include"spear.h"
#include"hammer.h"
#include"arrow.h"
#include"syuriken.h"
#include<memory>

//================================================================
//	グローバル変数
//================================================================
//ボールオブジェクト
PLAYER2	g_Player2;
ID3D11Device* g_pDevice2;
ID3D11DeviceContext* g_pContext2;
Controller g_Controller2(0); //ID 0のコントローラーを使用
MODEL* g_modelP2;
unsigned int g_changeP2;

void Player2Die()
{
	hal::dout << "Player2 died!" << std::endl;
	// ここにゲームオーバー画面への遷移、リスポーン処理など
	//プレイヤーを非表示にする
	if (g_Player2.m_gameObject != nullptr)
	{
		g_Player2.m_gameObject->m_isEnable = false;
	}
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_IDLE;
	
	//フェードアウトさせてシーンを切り替える
	XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
	SetFade(40.0f, color, FADE_OUT, SCENE_RESULT);
}

void Player2Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext, WeaponTerrain setWTp2)
{
	g_pDevice2 = pDevice;
	g_pContext2 = pContext;

	g_Player2.m_model = ModelLoad("asset\\model\\char_bow.fbx");
	g_modelP2 = ModelLoad("asset\\model\\block.fbx");

	g_Player2.m_position = XMFLOAT3(2.0f, 0.5f, 2.0f);
	g_Player2.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player2.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);

	g_Player2.m_scale = XMFLOAT3(0.6f, 1.0f, 0.6f);
	g_Player2.m_tag = "Player2";
	g_Player2.m_layer = 0;

	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;

	g_Player2.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player2.FrictionRate = 0.98f;
	g_Player2.m_currentHp = g_Player2.m_maxHp;
	g_Player2.m_isDead = false;

	// プレイヤーの当たり判定の追加
	auto collider = g_Player2.AddComponent<BoxCollider>(&g_Player2, g_Player2.m_scale);
	ManagerCollider::AddCollider(collider);

	// のちのちセレクト画面から分岐できるようにする
	// 自分をownerとして武器を生成
	g_Player2.m_currentWeapon = std::make_unique<Sword>(&g_Player2, TRUE); // 2Pです
	g_changeP2 = 0;

	EvolutionInitialize();
}
void Player2Finalize()
{
	ModelRelease(g_Player2.m_model);
}
void	Player2Update()
{
	g_Controller2.Update();

	EvolvePlayer2();           // Eキーで進化タイプを選択
	ApplyEvolutionEffect2();   // 進化タイプに応じたパラメータを適用
	if (g_Player2.m_isDead)return;	//死亡している場合は更新処理をスキップ
	
	//================================================================
//	武器変更処理(一旦)
//================================================================
	if (Keyboard_IsKeyDownTrigger(KK_D2))
	{
		g_changeP2++;

		if (g_changeP2 >= 5)
		{
			g_changeP2 = 0;
		}

		switch (g_changeP2)
		{
		case 0:
			g_Player2.EquipWeapon(std::make_unique<Sword>(&g_Player2, FALSE));
			break;

		case 1:
			g_Player2.EquipWeapon(std::make_unique<Spear>(&g_Player2, FALSE));
			break;

		case 2:
			// g_Player2.EquipWeapon(std::make_unique<Hammer>(&g_Player, FALSE));
			break;

		case 3:
			// g_Player2.EquipWeapon(std::make_unique<Arrow>(&g_Player, FALSE));
			break;

		case 4:
			// g_Player2.EquipWeapon(std::make_unique<Shuriken>(&g_Player, FALSE));
			break;

		default:
			break;
		}
	}

//================================================================
//	攻撃処理
//================================================================
	// CキーかAボタンで
	if (Keyboard_IsKeyDownTrigger(KK_P) || g_Controller2.IsButtonPushed(ControllerButton::B_BUTTON))
	{
		// 武器があるか
		if (g_Player2.m_currentWeapon)
		{
			g_Player2.m_currentWeapon->Attack(); // 攻撃
		}

		hal::dout << "Playerから攻撃した！\n";
	}

//================================================================
//	武器の更新
//================================================================
	if (g_Player2.m_currentWeapon)
	{
		g_Player2.m_currentWeapon->Update();
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
	// カメラの前方向ベクトル
	float forwardX = GetCamera2Position().x - GetCamera2AtPosition().x;
	float forwardZ = GetCamera2Position().z - GetCamera2AtPosition().z;

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
		speed = -0.1f;
	}
	if (Keyboard_IsKeyDown(KK_J))
	{
		speed = +0.1f;
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

	// モデルの向きを移動方向に合わせる
	XMFLOAT3 moveDir = { g_Player2.m_velocity.x, 0.0f, g_Player2.m_velocity.z };
	float length = sqrtf(moveDir.x * moveDir.x + moveDir.z * moveDir.z);
	if (length > 0.001f) // 移動しているときだけ向きを変える
	{
		// Y軸回転角を計算
		g_Player2.m_rotation.y = atan2f(moveDir.x, moveDir.z); // atan2f(X,Z)でY回転
	}

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
		0.05f,
		0.05f,
		0.05f);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Player2.m_rotation.x,
		g_Player2.m_rotation.y,
		g_Player2.m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		g_Player2.m_position.x,
		g_Player2.m_position.y - 0.25f,
		g_Player2.m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	//モデルの描画リクエスト
	ModelDraw(g_Player2.m_model);

	if (g_Player2.m_currentWeapon)
	{
		g_Player2.m_currentWeapon->Draw();
	}

	//ワールド行列作成
	scale = XMMatrixScaling(
		0.6f,
		1.0f,
		0.6f);
	rotation = XMMatrixRotationRollPitchYaw(
		g_Player2.m_rotation.x,
		g_Player2.m_rotation.y,
		g_Player2.m_rotation.z);
	translation = XMMatrixTranslation(
		g_Player2.m_position.x,
		g_Player2.m_position.y,
		g_Player2.m_position.z);
	world = scale * rotation * translation;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);

	ModelDraw(g_modelP2);
}

XMFLOAT3 GetPlayer2Position()
{
	return g_Player2.m_position;
}

void Player2_Jump()
{
	g_Player2.State = PLAYER2_STATE::PLAYER2_STATE_MOVE;
}
float Player2_GetHp()
{
	return g_Player2.m_currentHp;
}
float Player2_GetMaxHp()
{	
	return g_Player2.m_maxHp;
}
PLAYER2* GetPlayer2()
{
	return &g_Player2;
}

// 武器を装備する
void PLAYER2::EquipWeapon(std::unique_ptr<IWeapon> weapon)
{
	m_currentWeapon = std::move(weapon);
}

void PLAYER2::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
		if (info.other->m_tag == "Attack")
		{
			// 相手が武器オブジェクト持ってたら
			if (info.other->m_weaponPtr)
			{	
				// 武器の衝突判定を呼び出す
				info.other->m_weaponPtr->OnWeaponCollision(this);
			}
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

		if (info.other->m_tag == "Player")
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

		if (info.other->m_tag == "Lift" ||
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