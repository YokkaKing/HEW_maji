/*
* ファイル名	managerCollider.cpp
* タイトル	マネージャーコライダー
* 作成者		久保木幹太
* 作成日		11月12日
* 更新日		11月12日
*/

//================================================================
//  インクルード
//================================================================
#include"managerCollider.h"
#include"gameObject.h"

#include"debug_ostream.h"
#include"camera.h"

//================================================================
//  静的メンバ定義
//================================================================
std::vector<std::shared_ptr<Collider>> ManagerCollider::colliders;

static inline float AbsF(float v)
{
    return v < 0.0f ? -v : v;
}

static inline float SafeInvSqrt(float v)
{
    return 1.0f / (sqrtf(v) + 1e-8f);
}

//================================================================
//  Colliderの登録 / 削除
//================================================================
void ManagerCollider::AddCollider(std::shared_ptr<Collider> collider)
{
    if (collider && std::find(colliders.begin(), colliders.end(), collider) == colliders.end())
        colliders.push_back(collider);
}

void ManagerCollider::RemoveCollider(std::shared_ptr<Collider> collider)
{
    colliders.erase(std::remove(colliders.begin(), colliders.end(), collider), colliders.end());
}

//================================================================
//  衝突チェック（全オブジェクト）
//================================================================
void ManagerCollider::UpdateAllCollisions()
{
    for (size_t i = 0; i < colliders.size(); ++i)
    {
        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            auto a = colliders[i];
            auto b = colliders[j];

            CollisionInfo info = CheckCollision(a.get(), b.get());

            if (info.isHit)
            {
                // a に衝突情報を渡す
                info.other = b->owner;
                if (a->owner)
                    a->owner->OnCollision(info);

                // b にも逆向きの情報を渡す
                info.other = a->owner;
                if (b->owner)
                    b->owner->OnCollision(info);
            }
        }
    }
}

//================================================================
//  colliderの読み取り専用
//================================================================
const std::vector<std::shared_ptr<Collider>>& ManagerCollider::GetColliders()
{
    return colliders;
}

//================================================================
//  単体衝突判定
//================================================================
CollisionInfo ManagerCollider::CheckCollision(const Collider* collider1, const Collider* collider2)
{
    if (!collider1 || !collider2 || collider1 == collider2) return {};

    if (collider1->type == ColliderType::Box && collider2->type == ColliderType::Box)
        return CheckBoxBox(static_cast<const BoxCollider*>(collider1), static_cast<const BoxCollider*>(collider2));

    if (collider1->type == ColliderType::Sphere && collider2->type == ColliderType::Sphere)
        return CheckSphereSphere(static_cast<const SphereCollider*>(collider1), static_cast<const SphereCollider*>(collider2));

    if (collider1->type == ColliderType::Box && collider2->type == ColliderType::Sphere)
        return CheckBoxSphere(static_cast<const BoxCollider*>(collider1), static_cast<const SphereCollider*>(collider2));

    if (collider1->type == ColliderType::Sphere && collider2->type == ColliderType::Box)
    {
        CollisionInfo info = CheckBoxSphere(static_cast<const BoxCollider*>(collider2), static_cast<const SphereCollider*>(collider1));
        // 法線は a->b の向きに反転
        ManagerCollider::InvertNormalize(info);
        return info;
    }

    return {};
}

//================================================================
//  法線を逆にする
//================================================================
void ManagerCollider::InvertNormalize(CollisionInfo& info)
{
    info.normal.x *= -1;
    info.normal.y *= -1;
    info.normal.z *= -1;
}

//================================================================
//  Box vs Box
//================================================================
CollisionInfo ManagerCollider::CheckBoxBox(const BoxCollider* box1, const BoxCollider* box2) {
    XMFLOAT3 pa = box1->WorldPosition();
    XMFLOAT3 pb = box2->WorldPosition();
    XMFLOAT3 ha = box1->HalfSize();
    XMFLOAT3 hb = box2->HalfSize();

    float dx = pb.x - pa.x;
    float px = ha.x + hb.x - AbsF(dx);
    if (px <= 0.0f) return CollisionInfo();

    float dy = pb.y - pa.y;
    float py = ha.y + hb.y - AbsF(dy);
    if (py <= 0.0f) return CollisionInfo();

    float dz = pb.z - pa.z;
    float pz = ha.z + hb.z - AbsF(dz);
    if (pz <= 0.0f) return CollisionInfo();

    CollisionInfo info;
    info.isHit = true;

    // 軸ごとの深さを保存
    info.depth = XMFLOAT3{ px, py, pz };

    // 最小軸を penetration に入れて normal を決める
    if (px <= py && px <= pz) {
        info.penetration = px;
        info.normal = XMFLOAT3{ (dx < 0.0f) ? -1.f : 1.f, 0.f, 0.f };
    }
    else if (py <= px && py <= pz) {
        info.penetration = py;
        info.normal = XMFLOAT3{ 0.f, (dy < 0.0f) ? -1.f : 1.f, 0.f };
    }
    else {
        info.penetration = pz;
        info.normal = XMFLOAT3{ 0.f, 0.f, (dz < 0.0f) ? -1.f : 1.f };
    }

    return info;
}

//================================================================
//  Sphere vs Sphere
//================================================================
CollisionInfo ManagerCollider::CheckSphereSphere(const SphereCollider* sphere1, const SphereCollider* sphere2)
{
    XMFLOAT3 pa = sphere1->WorldPosition();
    XMFLOAT3 pb = sphere2->WorldPosition();

    float dx = pb.x - pa.x;
    float dy = pb.y - pa.y;
    float dz = pb.z - pa.z;

    float distSq = dx * dx + dy * dy + dz * dz;
    float r = sphere1->radius + sphere2->radius;

    CollisionInfo info;

    if (distSq >= r * r)
        return info;

    info.isHit = true;

    float dist = sqrtf(distSq);

    // 通常ケース（距離 > 0）
    if (dist > 1e-6f)
    {
        float penetration = r - dist;

        info.penetration = penetration;
        info.normal = XMFLOAT3(dx / dist, dy / dist, dz / dist);
        info.depth = XMFLOAT3(penetration, penetration, penetration);

        return info;
    }

    // 完全に中心が一致（dist = 0）
    //  → Normal が求められないので仮の方向を作る
    info.penetration = r;
    info.normal = XMFLOAT3(1.f, 0.f, 0.f);   // どこでも良い（押し出し専用）
    info.depth = XMFLOAT3(r, r, r);

    return info;
}

//================================================================
//  Box vs Sphere
//================================================================
CollisionInfo ManagerCollider::CheckBoxSphere(const BoxCollider* b, const SphereCollider* s)
{
    XMFLOAT3 pb = b->WorldPosition();    // Box center
    XMFLOAT3 pc = s->WorldPosition();    // Sphere center
    XMFLOAT3 h = b->HalfSize();         // Box half-size

    // 最近接点を求める（クランプ）
    float cx = (pc.x < pb.x - h.x) ? (pb.x - h.x) :
        (pc.x > pb.x + h.x) ? (pb.x + h.x) : pc.x;

    float cy = (pc.y < pb.y - h.y) ? (pb.y - h.y) :
        (pc.y > pb.y + h.y) ? (pb.y + h.y) : pc.y;

    float cz = (pc.z < pb.z - h.z) ? (pb.z - h.z) :
        (pc.z > pb.z + h.z) ? (pb.z + h.z) : pc.z;

    // Sphere center → 最近接点 への距離
    float dx = pc.x - cx;
    float dy = pc.y - cy;
    float dz = pc.z - cz;

    float distSq = dx * dx + dy * dy + dz * dz;
    float r = s->radius;

    CollisionInfo info;

    if (distSq > r * r)
        return info;  // no hit

    info.isHit = true;

    float dist = sqrtf(distSq);

    // 通常ケース（距離 > 0）
    if (dist > 1e-6f)
    {
        float penetration = r - dist;

        info.penetration = penetration;
        info.normal = XMFLOAT3(dx / dist, dy / dist, dz / dist);
        info.depth = XMFLOAT3(penetration, penetration, penetration);

        return info;
    }

    // 完全に内部に入っている（中心が一致・非常に近い）
    float ax = fabsf(pc.x - pb.x) - h.x;
    float ay = fabsf(pc.y - pb.y) - h.y;
    float az = fabsf(pc.z - pb.z) - h.z;

    if (ax > ay && ax > az)
        info.normal = XMFLOAT3((pc.x < pb.x) ? -1.f : 1.f, 0.f, 0.f);
    else if (ay > az)
        info.normal = XMFLOAT3(0.f, (pc.y < pb.y) ? -1.f : 1.f, 0.f);
    else
        info.normal = XMFLOAT3(0.f, 0.f, (pc.z < pb.z) ? -1.f : 1.f);

    info.penetration = r;
    info.depth = XMFLOAT3(r, r, r);

    return info;
}