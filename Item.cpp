/*
* ファイル名	Item.cpp
* タイトル	アイテム制御
* 作成者		久保木幹太
* 作成日		12月31日
* 更新日		12月31日
*/

//================================================================
//	インクルード
//================================================================
#include"Item.h"
#include"model.h"
#include"Player.h"
#include"Player2.h"
#include"managerCollider.h"

//================================================================
//	マクロ定義
//================================================================
#define SPONE_TIME (10.0f)

//================================================================
//	グローバル変数
//================================================================
MODEL* gi_model[2];

//================================================================
//	ITEM_SPONERクラス
//================================================================
void ITEM_SPONER::Initialize()
{
	m_position = { 0.0f, 0.0f, 0.0f };
	m_count = 0.0f;

	gi_model[0] = ModelLoad("asset\\model\\potion1.fbx");
	gi_model[1] = ModelLoad("asset\\model\\potion2.fbx");
}
void ITEM_SPONER::Finalize()
{
	if (gi_model[0] != nullptr)
	{
		ModelRelease(gi_model[0]);
		gi_model[0] = nullptr; // 解放済みであることを明示
	}

	if (gi_model[1] != nullptr) 
	{
		ModelRelease(gi_model[1]);
		gi_model[1] = nullptr; // 二重解放を防止
	}
}
void ITEM_SPONER::Update()
{
	m_count += (1.0f / 60.0f);

	// 一定時間たったら
	if (m_count > SPONE_TIME)
	{
		m_position = WherePosition();

		Spwan(); // アイテムを投下

		m_count = 0.0f;
	}
}
void ITEM_SPONER::Spwan()
{
	ITEM* item = new ITEM();

	item->m_position = m_position;
	item->m_rotation = m_rotation;

	extern std::vector<GameObject*> g_gameObjects;
	g_gameObjects.push_back(item); // アイテムを登録

	item->Set(); // アイテムのセットを呼び出し
}
// どこにスポナーを作ればいいかを判断する
XMFLOAT3 ITEM_SPONER::WherePosition()
{
	// Player1とPlayer2のどっちが体力が少ないか
	bool nearPlayer = (Player_GetHp < Player2_GetHp) ? false :
		(Player_GetHp > Player2_GetHp) ? true : false;

	// プレイヤーの範囲10マス以内くらいに落ちてくるようにする
	if (nearPlayer)
	{
		return XMFLOAT3(
			GetPlayer2Position().x + (rand() % 4),
			10.0f,
			GetPlayer2Position().z + (rand() % 4));
	}
	else
	{
		return XMFLOAT3(
			GetPlayerPosition().x + (rand() % 2),
			10.0f,
			GetPlayerPosition().z + (rand() % 2));
	}
}

//================================================================
//	ITEMクラス
//================================================================
void ITEM::Set()
{
	m_acceleration = XMFLOAT3(0.0f, -9.8f / 600.0f * 0.5f, 0.0f);

	// どのアイテムかを決める
	int set = (int)(rand() % 2);

	switch (set)
	{
	case 0:
		m_itemType = ITEM_TYPE::CHANGE_RECOVERY;
		break;

	case 1:
		m_itemType = ITEM_TYPE::HEALTH_RECOVERY;
		break;
	}

	m_tag = "Item";
	m_isScale = false;
	m_isDead = false;

	XMFLOAT3 scale = { 0.4f, 0.4f, 0.4f };
	m_scale = scale;
	m_collider = AddComponent<BoxCollider>(this, scale);
	ManagerCollider::AddCollider(m_collider);
}
void ITEM::Update()
{
	if (!m_isGround)
	{
		m_velocity.y += m_acceleration.y;
	}
	m_rotation.y += 0.1f;

	float scaleStep = 0.01f;

	if (m_isScale) 
	{
		m_scale.x = m_scale.y = m_scale.z += scaleStep;
	}
	else
	{
		m_scale.x = m_scale.y = m_scale.z -= scaleStep;
	}

	// 0.65を超えたら小さくする(false)、0.15を下回ったら大きくする(true)
	if (m_scale.x > 0.65f)
	{
		m_isScale = false;
	}
	else if (m_scale.x < 0.15f) 
	{
		m_isScale = true;
	}

	m_position.x += m_velocity.x;
	m_position.z += m_velocity.z;
	m_position.y += m_velocity.y;
}
void ITEM::Draw()
{
	if (m_isDead) return;

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

	if (m_itemType == ITEM_TYPE::CHANGE_RECOVERY)
	{
		ModelDraw(gi_model[0]);
	}
	else if (m_itemType == ITEM_TYPE::HEALTH_RECOVERY)
	{
		ModelDraw(gi_model[1]);
	}
}
void ITEM::OnCollision(const CollisionInfo& info)
{
	if (!info.isHit) return;

	if (info.other)
	{
		// とりあえず地面判定のやつ(Player達以外)
		if (info.other->m_tag == "Wall" ||
			info.other->m_tag == "Tree" ||
			info.other->m_tag == "Lift" ||
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
			}
		}

		// プレイヤーに当たったら回復
		// 一旦こっち側で処理を作る
		if (info.other->m_tag == "Player")
		{
			if (m_itemType == ITEM_TYPE::HEALTH_RECOVERY)
			{
				float health = 100.0f - Player_GetHp();

				// Player1が体力マックス-30.0f以下だったら30.0f回復
				if (health > 30.0f)
				{
					info.other->TakeDamage(-30.0f);
				}
				else
				{
					// そうじゃなければHPマックスまで回復
					info.other->TakeDamage(-health);
				}

				m_isDead = true; // 消滅
			}
		}

		if (info.other->m_tag == "Player2")
		{
			if (m_itemType == ITEM_TYPE::HEALTH_RECOVERY)
			{
				float health = 100.0f - Player2_GetHp();

				// Player1が体力マックス-30.0f以下だったら30.0f回復
				if (health > 30.0f)
				{
					info.other->TakeDamage(-30.0f);
				}
				else
				{
					// そうじゃなければHPマックスまで回復
					info.other->TakeDamage(-health);
				}

				m_isDead = true; // 消滅
			}
		}
	}
}