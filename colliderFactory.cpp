/*
* ファイル名	colliderFactory.cpp
* タイトル	コライダー作成
* 作成者		久保木幹太
* 作成日		11月17日
* 更新日		12月02日
*/

//================================================================
//  インクルード
//================================================================
#include"ColliderFactory.h"
#include"debug_ostream.h"

// BoxColliderオブジェクトの作成
GameObject* ColliderFactory::CreateBoxObject(
    XMFLOAT3 pos,
    const XMFLOAT3& size,
    const std::string& tag,
    int layer)
{
    GameObject* obj = new GameObject();
    obj->m_position = pos;
    obj->m_tag = tag;
    obj->m_layer = layer;

    // owner と size をコンストラクタで渡す
    auto collider = obj->AddComponent<BoxCollider>(obj, size);

    ManagerCollider::AddCollider(collider);

    return obj;
}

// SphereColliderオブジェクトの作成
GameObject* ColliderFactory::CreateSphereObject(
    XMFLOAT3 pos,
    float radius,
    const std::string& tag,
    int layer)
{
    GameObject* obj = new GameObject();
    obj->m_position = pos;
    obj->m_tag = tag;
    obj->m_layer = layer;

    // owner と radius をコンストラクタで渡す
    auto collider = obj->AddComponent<SphereCollider>(obj, radius);

    ManagerCollider::AddCollider(collider);

    return obj;
}

// SlopeColliderオブジェクトの作成
GameObject* ColliderFactory::CreateTrapezoidSlopeObject(
    XMFLOAT3 start,
    XMFLOAT3 end,
    float startWidth,
    float endWidth,
    float thickness,
    const std::string& tag,
    int layer)
{
    GameObject* obj = new GameObject();

    obj->m_position = XMFLOAT3(0.0f, 0.0f, 0.0f);

    obj->m_tag = tag;
    obj->m_layer = 0;
    obj->m_isStatic = true;

    auto col = obj->AddComponent<TrapezoidSlopeCollider>(obj, start, end, startWidth, endWidth, thickness);
    ManagerCollider::AddCollider(col);
    return obj;
}