/*
* ファイル名	colliderFactory.h
* タイトル	コライダー作成
* 作成者		久保木幹太
* 作成日		11月17日
* 更新日		11月17日
*/

#ifndef COLLIDER_FACTORY_H
#define COLLIDER_FACTORY_H

//================================================================
//	インクルード
//================================================================
#include"managerCollider.h"
#include"gameObject.h"
#include<DirectXMath.h>
#include<string>

class ColliderFactory
{
public:
    // BoxColliderオブジェクト作成
    static GameObject* CreateBoxObject(
        XMFLOAT3 pos,
        const DirectX::XMFLOAT3& size = { 1.0f,1.0f,1.0f },
        const std::string& tag = "Untagged",
        int layer = 0);

    // SphereColliderオブジェクト作成
    static  GameObject* CreateSphereObject(
        XMFLOAT3 pos,
        float radius = 1.0f,
        const std::string& tag = "Untagged",
        int layer = 0);

    // SlopeColliderオブジェクトの作成
    static GameObject* CreateTrapezoidSlopeObject(
        XMFLOAT3 start,
        XMFLOAT3 end,
        float startWidth,
        float endWidth,
        float thickness,
        const std::string& tag,
        int layer = 0);
};

#endif // COLLIDER_FACTORY_H