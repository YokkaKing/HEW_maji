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
//#include"keyboard.h"
#include"Controller.h"
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

Controller g_Controller(0); //ID 0のコントローラーを使用

void PlayerDie()
{
	hal::dout << "Player died!" << std::endl;
	//死亡処理

	// 例: プレイヤーを非表示にする
	g_Player.m_gameObject->m_isEnable = false;

	// 例: 入力を受け付けないようにする（状態をIDLEにするなど）
	g_Player.State = PLAYER_STATE::PLAYER_STATE_IDLE;
}

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
	g_Player.m_currentHp = g_Player.m_maxHp;
	g_Player.m_isDead = false;

	g_Player.SetObject(g_Player.m_position, g_Player.m_scale, "Player", 0);
	EvolutionInitialize();

	//Sword* newSword = new Sword();
	//newSword->Initialize(pDevice, pContext);
	//g_Player2.EquipWeapon(newSword);
}
void PlayerFinalize()
{
	ModelRelease(g_Player.m_model);
	//武器の解放
	if (g_Player.m_currentWeapon)
	{
		delete g_Player.m_currentWeapon;
		g_Player.m_currentWeapon = nullptr;
	}
}
void	PlayerUpdate()
{
	g_Controller.Update();//毎フレームコントローラーの状態を更新

	EvolvePlayer();           // Eキーで進化タイプを選択（一度だけ実行）
	ApplyEvolutionEffect();   // 進化タイプに応じたパラメータを適用
	if (g_Player.m_isDead)return;	//死亡している場合は更新処理をスキップ
	//装備中の武器を更新する
	if (g_Player.m_currentWeapon)
	{
		g_Player.m_currentWeapon->Update(1.0f / 60.0f); // 1/60秒で更新
		//攻撃終了判定
		if (g_Player.m_currentWeapon->ShouldEndAttack())
		{
			g_Player.m_currentWeapon->EndAttack();
		}
	}
	//攻撃入力のチェック (例: KK_Oキー)
	if (g_Controller.IsButtonPushed(ControllerButton::X_BUTTON))//xボタン
	{
		if (g_Player.m_currentWeapon && !g_Player.m_currentWeapon->IsAttacking())
		{
			g_Player.m_currentWeapon->StartAttack(g_Player.m_position, g_Player.m_rotation);
		}
	}

	Player_ManualMove();
	//死亡判定
	if (g_Player.m_currentHp <= 0.0f && !g_Player.m_isDead)
	{
		g_Player.m_isDead = true;
		PlayerDie();
	}
}

void Player_ManualMove() // 新しい手動移動関数として作成
{
	// カメラの前方向ベクトル
	float forwardX = GetCameraAtPosition().x - GetCameraPosition().x;
	float forwardZ = GetCameraAtPosition().z - GetCameraPosition().z;

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
	if (len > 0.0f)
	{
		forwardX /= len;
		forwardZ /= len;
	}
	else
	{
		forwardX = 0.0f;
		forwardZ = 0.0f;
	}

	// カメラの右方向ベクトル
	float rightX = forwardZ;    // 右方向は前方向ベクトルを90度回転
	float rightZ = -forwardX;

	// 移動量初期化
	float moveX = 0.0f;
	float moveZ = 0.0f;

	float speed = 0.0f;
	float stickY = g_Controller.GetLeftStickY();
	if (fabs(stickY) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// ベクトルが逆だから移動が逆になる
		// 左スティック上方向 (+1.0f) で前進 (speed = -0.1f) に対応
		speed = stickY * 0.1f;
	}

	moveX += forwardX * speed;
	moveZ += forwardZ * speed;

	// 横移動
	float strafe = 0.0f;
	float stickX = g_Controller.GetLeftStickX();
	if (fabs(stickX) > 0.05f) // デッドゾーンを設定 (必要に応じて調整)
	{
		// 左スティック左方向 (-1.0f) で左移動 (strafe = +0.1f) に対応
		strafe = stickX * 0.1f;
	}
	moveX += rightX * strafe;
	moveZ += rightZ * strafe;

	// 最終速度
	g_Player.m_velocity.x = moveX;
	g_Player.m_velocity.z = moveZ;

	// Aボタンを押した && コヨーテタイムが0.0fより大きい
	if (g_Controller.IsButtonPushed(ControllerButton::A_BUTTON) && g_Player.m_koyoteTime > 0.0f) //Aボタン**
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

void PLAYER::TakeDamage(float damage)
{
	if (m_isDead) return;

	m_currentHp -= damage;
	
	// デバッグ出力でダメージ表示
	hal::dout << "Player took " << damage << " damage. HP remaining: " << m_currentHp << std::endl;
	

}

//武器を装備する
void PLAYER::EquipWeapon(IWeapon* weapon)
{
	// 古い武器があれば解放する
	if (m_currentWeapon)
	{
		delete m_currentWeapon;
	}
	m_currentWeapon = weapon;
}

//攻撃を試みる
void PLAYER::TryAttack(const XMFLOAT3& direction)
{
	if (m_currentWeapon && !m_currentWeapon->IsAttacking())
	{
		m_currentWeapon->StartAttack(m_position, m_rotation);
		// 攻撃が成功した場合、クールダウンはAttack内で設定される
	}
	else if (m_currentWeapon)
	{
		// デバッグ出力: クールダウン中
		hal::dout<< "Attack on cooldown!" << std::endl; 
	}
}

void PLAYER::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;
	if (m_isDead) return; //死亡していたら衝突処理を無視

	// --- まずタグで相手を識別 ---
	if (info.other)
	{
		//敵の攻撃に当たった場合
		if (info.other->m_tag == "Player2Attack")
		{
			//ダメージを受ける
			TakeDamage(10.0f); // 10ダメージを与える（数値は適宜調整）
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

		if (info.other->m_tag == "Player2")
		{
			auto INFO = info;

			INFO.normal.x *= -1;
			INFO.normal.y *= -1;
			INFO.normal.z *= -1;

			//================================================================
			//	押し戻し
			//================================================================
			m_position.x += INFO.normal.x * INFO.penetration;
			m_position.y += INFO.normal.y * INFO.penetration;
			m_position.z += INFO.normal.z * INFO.penetration;

			//================================================================
			//	地面判定
			//================================================================
			if (INFO.normal.y > 0.7f)
			{
				m_isGround = true;
				m_velocity.y = 0;
			}

			//================================================================
			//	壁判定
			//================================================================
			float horiz = fabs(INFO.normal.x) + fabs(INFO.normal.z);
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

void PLAYER::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
{
	GameObject* obj = ColliderFactory::CreateBoxObject(
		pos,
		scl,
		tag,
		lay
	);

	m_position = obj->m_position;
	m_scale = obj->m_scale;
	m_tag = obj->m_tag;
	m_layer = obj->m_layer;

	for (auto& col : obj->GetColliders<>())
	{
		col->owner = this;
		this->components.push_back(col);
	}

	delete obj;
}