/*
* ファイル名	collider.h
* タイトル	コライダー
* 作成者		久保木幹太
* 作成日		11月12日
* 更新日		11月18日
*/

#ifndef COLLIDER_H
#define COLLIDER_H

//================================================================
//  インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;
#include"component.h"
#include"gameObject.h"

//================================================================
//  コライダーの種類
//================================================================
enum class ColliderType
{
    Box,
    Sphere,
    Slope,
};

//================================================================
//  当たり判定の基底クラス
//================================================================
class Collider : public Component
{
public:
    ColliderType type = ColliderType::Box;
    bool isTrigger = false;
    GameObject* owner = nullptr;

    Collider(GameObject* o, ColliderType t) : owner(o), type(t) {};
    virtual ~Collider() = default;

    XMFLOAT3 WorldPosition() const
    {
        return owner->GetWorldPosition();
    }

    void SetEnable(bool enable) { m_enable = enable; }
    bool IsEnable() const { return m_enable; }

private:
    bool m_enable = true; // 当たり判定を取るか
};

//================================================================
//  ボックスの当たり判定のクラス
//================================================================
class BoxCollider : public Collider
{
public:
    DirectX::XMFLOAT3 size{ 1, 1, 1 };
    BoxCollider(GameObject* o, const XMFLOAT3& s)
    : Collider(o,ColliderType::Box), size(s) {}

    XMFLOAT3 HalfSize() const
    {
        return XMFLOAT3(size.x * 0.5, size.y * 0.5f, size.z * 0.5f);
    }
};

//================================================================
//  球の当たり判定のクラス
//================================================================
class SphereCollider : public Collider
{
public:
    float radius = 0.5f;
    SphereCollider(GameObject* o, float r)
        : Collider(o, ColliderType::Sphere), radius(r) {};
};

//================================================================
//  台形スロープ（坂道）の当たり判定クラス
//================================================================
class TrapezoidSlopeCollider : public Collider {
public:
    XMFLOAT3 start;      // 始点中央
    XMFLOAT3 end;        // 終点中央
    float startWidth;    // 始点の幅
    float endWidth;      // 終点の幅
    float thickness;     // 板の厚み（0.1など薄く設定可能）

    TrapezoidSlopeCollider(GameObject* o, XMFLOAT3 s, XMFLOAT3 e, float sw, float ew, float t = 0.1f)
        : Collider(o, ColliderType::Slope), start(s), end(e), startWidth(sw), endWidth(ew), thickness(t) {
    }
};

#endif // COLLIDER_H