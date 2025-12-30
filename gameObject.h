/*
* ファイル名	gameObject.h
* タイトル	ゲームオブジェクト
* 作成者		久保木幹太
* 作成日		11月12日
* 更新日		11月12日
*/

#ifndef GAME_OBJECT_H
#define GAME_OBJECT_H

//================================================================
//  インクルード
//================================================================
#include<vector>
#include<memory>
#include<DirectXMath.h>
using namespace DirectX;

#include"component.h"
//#include"managerCollider.h"
#include"field.h"
#include<string>
#include"model.h"
#include"debug_ostream.h"

class CollisionInfo;
class Collider;
class IWeapon;

//=======================================
// GameObject（コンポーネント管理）
//=======================================
class GameObject
{
public:
    XMFLOAT3 m_position{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_oldPosition{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_rotation{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_scale{ 1.0f, 1.0f, 1.0f };
    XMFLOAT3 m_velocity{ 0.0f, 0.0f, 0.0f };
    XMFLOAT3 m_acceleration{ 0.0f, 0.0f, 0.0f };
    FIELD m_type = FIELD_BOX; // ブロックの種類

    FLOAT m_maxHp = 100.0f; // 最大体力
    FLOAT m_currentHp;	    // 現在の体力

    std::string m_tag = "Untagged";
    int m_layer = 0;

    MODEL* m_model = nullptr;
    bool m_isGround = false;
    FLOAT m_koyoteTime = 0.0f;
    // 武器のためのやつ
    IWeapon* m_weaponPtr = nullptr;

    GameObject* m_gameObject = nullptr;

    bool m_isDead = false;

    std::vector<std::shared_ptr<Component>> components;
    bool m_isEnable = true;
public:
    template <typename T, typename... Args>
    std::shared_ptr<T> AddComponent(Args&&... args)
    {
        auto comp = std::make_shared<T>(std::forward<Args>(args)...);
        comp->owner = this;
        components.push_back(comp);
        return comp;
    }

    template <typename T>
    std::shared_ptr<T> GetComponent() const
    {
        for (auto& c : components)
            if (auto casted = std::dynamic_pointer_cast<T>(c))
                return casted;
        return nullptr;
    }

    template <typename T = Collider>
    std::vector<std::shared_ptr<T>> GetColliders() const
    {
        std::vector<std::shared_ptr<T>> result;

        for (auto& c : components)
        {
            // Collider のみ取得
            if (auto col = std::dynamic_pointer_cast<T>(c))
                result.push_back(col);
        }
        return result;
    }

    // 体力を減らす
    void TakeDamage(float damage)
    {
        m_currentHp -= damage;
    }

    XMFLOAT3 GetWorldPosition() const { return m_position; }

    virtual void OnCollision(const CollisionInfo& info) {}

    void Move(float dx, float dy, float dz)
    {
        m_position.x += dx;
        m_position.y += dy;
        m_position.z += dz;
    }

    virtual void Update() {};

    virtual void Draw() {}
};

#endif // GAME_OBJECT_H