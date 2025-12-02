/*
* ファイル名	Player.cpp
* タイトル	プレイヤー
* 作成者		久保木幹太
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
// #include	"Controller.h"
#include"Player.h"
#include"Camera.h"
#include"shader.h"
#include"Evolution.h"
#include"colliderFactory.h"
#include"debug_ostream.h"

//================================================================
//	グローバル変数
//================================================================
//ボールオブジェクト
PLAYER	g_Player;
ID3D11Device* g_pDevice;
ID3D11DeviceContext* g_pContext;

void PlayerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

	g_Player.m_model = ModelLoad("asset\\model\\test.fbx");

	g_Player.m_position = XMFLOAT3(0.0f, 0.5f, 1.0f);
	g_Player.m_rotation = XMFLOAT3(0.0f, 0.0f, 0.0f);
	g_Player.m_velocity = XMFLOAT3(0.0f, 0.0f, 0.0f);
	
	g_Player.m_scale = XMFLOAT3(0.5f, 0.5f, 0.5f);

	g_Player.State = PLAYER_STATE::PLAYER_STATE_MOVE;

	g_Player.m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);
	g_Player.FrictionRate = 0.98f;
	g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;

	g_Player.SetObject(g_Player.m_position, g_Player.m_scale, "Player", 0);
	EvolutionInitialize();
}
void PlayerFinalize()
{
	ModelRelease(g_Player.m_model);
}
void	PlayerUpdate()
{
	EvolvePlayer();           // Eキーで進化タイプを選択（一度だけ実行）
	ApplyEvolutionEffect();   // 進化タイプに応じたパラメータを適用
	Player_ManualMove();
}

void Player_ManualMove() // 新しい手動移動関数として作成
{
	g_Player.m_gameObject->m_position = g_Player.m_position;

	// カメラの前方向ベクトル
	float forwardX = GetCameraPosition().x - GetCameraAtPosition().x;
	float forwardZ = GetCameraPosition().z - GetCameraAtPosition().z;

	if (!g_Player.m_isGround) // 地面についてないときに重力発動
	{
		g_Player.m_velocity.x += g_Player.m_acceleration.x;
		g_Player.m_velocity.y += g_Player.m_acceleration.y;
		g_Player.m_velocity.z += g_Player.m_acceleration.z;
	}

	// 地面についているときにコヨーテタイムが1.0fになる
	if (g_Player.m_isGround)
	{
		g_Player.m_koyoteTime = 1.0f;
	}
	else
	{
		g_Player.m_koyoteTime -= 0.1f;
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
	if (Keyboard_IsKeyDown(KK_W))
	{
		// ベクトルが逆だから移動が逆になる
		speed = -0.1f;
	}
	if (Keyboard_IsKeyDown(KK_S))
	{
		speed = 0.1f;
	}

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	if (Keyboard_IsKeyDown(KK_A))
	{
		strafe = +0.1f;  // 左
	}
	if (Keyboard_IsKeyDown(KK_D))
	{
		strafe = -0.1f;  // 右
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	g_Player.m_velocity.x = moveX;
	g_Player.m_velocity.z = moveZ;

	// スペース押した && コヨーテタイムが0.0fより大きい
	if (Keyboard_IsKeyDownTrigger(KK_SPACE) && g_Player.m_koyoteTime > 0.0f)
	{
		g_Player.m_velocity.y = JUMP_FORCE;
		g_Player.m_isGround = false;
		g_Player.m_koyoteTime = 0.0f;
	}
	else
	{
		g_Player.m_isGround = false;
	}

	g_Player.m_position.x += g_Player.m_velocity.x;
	g_Player.m_position.z += g_Player.m_velocity.z;
	g_Player.m_position.y += g_Player.m_velocity.y;
}

void PlayerDraw() 
{
	//ワールド行列作成
	XMMATRIX	scale = XMMatrixScaling(
		1.0f,
		1.0f,
		1.0f);
	XMMATRIX	rotation = XMMatrixRotationRollPitchYaw(
		g_Player.m_rotation.x,
		g_Player.m_rotation.y,
		g_Player.m_rotation.z);
	XMMATRIX	translation = XMMatrixTranslation(
		g_Player.m_position.x,
		g_Player.m_position.y,
		g_Player.m_position.z);
	XMMATRIX	world = scale * rotation * translation;

	//変換行列作成
	XMMATRIX	view = GetViewMatrix();
	XMMATRIX	projection = GetProjectionMatrix();
	XMMATRIX	wvp = world * view * projection;

	//シェーダーへ行列をセット
	Shader_SetWorldMatrix(world);
	//Shader_SetMatrix(wvp);

	//モデルの描画リクエスト
	ModelDraw(g_Player.m_model);

}

XMFLOAT3 GetPlayerPosition()
{
	return g_Player.m_position;
}

void Player_Jump()
{
	g_Player.State = PLAYER_STATE::PLAYER_STATE_MOVE;
}

PLAYER* GetPlayer()
{
	return &g_Player;
}

void PLAYER::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
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

void PLAYER::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
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
		col->owner = &g_Player;
	}
}
