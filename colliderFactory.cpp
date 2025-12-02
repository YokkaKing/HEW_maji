/*
* ファイル名	colliderFactory.cpp
* タイトル	コライダー作成
* 作成者		久保木幹太
* 作成日		11月17日
* 更新日		11月17日
*/

//================================================================
//  インクルード
//================================================================
#include "ColliderFactory.h"
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