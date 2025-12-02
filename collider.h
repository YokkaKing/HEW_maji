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

#endif // COLLIDER_H