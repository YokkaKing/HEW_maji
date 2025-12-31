/*
* ファイル名	managerCollider.h
* タイトル	マネージャーコライダー
* 作成者		久保木幹太
* 作成日		11月12日
* 更新日		11月12日
*/

#ifndef MANAGER_COLLIDER_H
#define MANAGER_COLLIDER_H

//================================================================
//  インクルード
//================================================================
#include<memory>
#include<vector>
#include<DirectXMath.h>
using namespace DirectX;

#include"collider.h"

#include<cmath>
#include<algorithm>

class GameObject;

//=======================================
//  当たり判定を一括管理
//=======================================
enum class CollisionFace
{
    NONE = 0,
    LEFT,
    RIGHT,
    TOP,
    BOTTOM,
    FRONT,
    BACK
};

class CollisionInfo
{
public:
    bool isHit = FALSE;                         // 衝突したか
    CollisionFace face = CollisionFace::NONE;   // どの方向に当たったか
    XMFLOAT3 normal{ 0.0f, 0.0f, 0.0f };        // 衝突方向ベクトル
    FLOAT penetration{ 0.0f };
    XMFLOAT3 depth{ 0.0f, 0.0f, 0.0f };                   // めり込み量
    XMFLOAT3 contactPoint{ 0, 0, 0 };           // 衝突地点（最近接点）
    GameObject* other = nullptr;
};

class ManagerCollider
{
public:
    // Colliderの登録、削除
    static void AddCollider(std::shared_ptr<Collider> collider);
    static void RemoveCollider(std::shared_ptr<Collider> collider);
    static void ClearCollider();

    // 登録された全コライダー間の衝突チェック
    static void UpdateAllCollisions();

    // 単体の判定の関数(内部で複数回使用)
    static CollisionInfo CheckCollision(const Collider* collider1, const Collider* collider2);

public:
    static const std::vector<std::shared_ptr<Collider>>& GetColliders(); // colliderの読み取り専用


private:
    static std::vector<std::shared_ptr<Collider>> colliders;

private:
    static void InvertNormalize(CollisionInfo& info);

private:
    // 衝突判定（方向付き）
    static CollisionInfo CheckBoxBox(const BoxCollider* box1, const BoxCollider* box2);
    static CollisionInfo CheckSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2);
    static CollisionInfo CheckBoxSphere(const BoxCollider* box, const SphereCollider* sphere);

    // 衝突判定(回転対応)
    static CollisionInfo CheckBoxBoxOBB(const BoxCollider* box1, const BoxCollider* box2);
};

#endif // MANAGER_COLLIDER_H