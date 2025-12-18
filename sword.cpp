/*
* ファイル名	sword.cpp
* タイトル	    剣
* 作成者		三橋拓斗
* 作成日		12月09日
*  更新日		12月09日
*/

//================================================================
//	インクルード
//================================================================
#include "sword.h"
#include "debug_ostream.h"
#include "colliderFactory.h"
#include "collider.h"
#include "component.h"
#include "Player2.h"
#include "Player.h"

Sword::Sword()
{
    m_tag = "Sword";
}

void Sword::SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay)
{
    std::unique_ptr<GameObject> obj_ptr(
        ColliderFactory::CreateBoxObject(pos, scl, tag, lay)
    );
    GameObject* obj = obj_ptr.get();

    m_position = obj->m_position;
    m_scale = obj->m_scale;
    m_tag = obj->m_tag;
    m_layer = obj->m_layer;

    for (auto& col : obj->GetColliders<>())
    {
        col->owner = this;

        this->components.push_back(col);
    }
}

void Sword::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    if (m_isAttacking)
        return;

    m_isAttacking = true;
    m_attackTimer = 0.0f;
    m_isEnable = true;
    hal::dout << "Sword attack started." << std::endl;

    m_position = playerPosition;
    m_rotation = playerRotation;
}

void Sword::EndAttack()
{
    if (!m_isAttacking)
        return;

    m_isAttacking = false;
    m_attackTimer = 0.0f;
    m_isEnable = false;
    hal::dout << "Sword attack ended." << std::endl;
}

void Sword::OnCollision(const CollisionInfo& info)
{
    // 当たり判定が有効なとき（攻撃中）にのみ、衝突処理を行う
    if (m_isAttacking)
    {
        hal::dout << "Sword hit something! Tag: " << info.other->m_tag << std::endl;

        // 衝突相手がPlayer2であるかをタグでチェック
        if (info.other->m_tag == "Player2")
        {
            // GameObject* (info.other) を PLAYER2* 型に安全にキャスト
            PLAYER2* targetPlayer2 = dynamic_cast<PLAYER2*>(info.other);

            if (targetPlayer2)
            {
                // 10ダメージを与える
                targetPlayer2->TakeDamage(10.0f);

                hal::dout << "Sword hit Player2 and dealt 10 damage! HP left: "
                    << targetPlayer2->m_currentHp << std::endl;
            }
        }
        else
        {
            // Player2以外に当たった場合のデバッグ出力
            hal::dout << "Sword hit detected on target. Tag: " << info.other->m_tag << std::endl;
        }

    }
}

void Sword::Update()
{
    Update(1.0f / 60.0f);

    GameObject::Update();
}

void Sword::Update(float deltaTime)
{
    if (m_isAttacking)
    {
        m_attackTimer += deltaTime;
        if (ShouldEndAttack())
        {
            EndAttack();
        }
    }
}


void Sword::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    hal::dout << "Sword initialized." << std::endl;
}

void Sword::Finalize()
{
    hal::dout << "Sword finalized." << std::endl;
}

void Sword::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    m_position = playerPosition;
    m_rotation = playerRotation;

    if (m_model && m_isEnable)
    {
        // 剣のモデル描画API呼び出し
    }

    if (m_isAttacking)
    {
        auto colliders = this->GetColliders<Collider>();
    }
}

bool Sword::ShouldEndAttack() const
{
    return m_attackTimer >= m_attackDuration;
}