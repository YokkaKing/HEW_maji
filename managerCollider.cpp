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

void ManagerCollider::ClearCollider()
{
    colliders.clear();
}

//================================================================
//  衝突チェック（全オブジェクト）
//================================================================
void ManagerCollider::UpdateAllCollisions()
{
    // 判定を行う周囲の距離
    const float checkRadius = 5.0f;
    const float checkRadiusSq = checkRadius * checkRadius;

    for (size_t i = 0; i < colliders.size(); ++i)
    {
        auto a = colliders[i];
        if (!a->IsEnable() || !a->owner) continue;

        for (size_t j = i + 1; j < colliders.size(); ++j)
        {
            auto b = colliders[j];
            if (!b->IsEnable() || !b->owner) continue;

            // 静止物同士はスキップ
            if (a->owner->m_isStatic && b->owner->m_isStatic)
            {
                continue;
            }

            // 距離による早期フィルタリング
            // どちらかが動く物であれば、二点間の距離をチェックする
            XMVECTOR posA = XMLoadFloat3(&a->owner->m_position);
            XMVECTOR posB = XMLoadFloat3(&b->owner->m_position);
            XMVECTOR diff = XMVectorSubtract(posB, posA);
            float distSq = XMVector3LengthSq(diff).m128_f32[0];

            // 指定半径より遠ければスキップ
            if (distSq > checkRadiusSq)
            {
                continue;
            }

            // 範囲内の場合のみ詳細な判定を実行
            CollisionInfo info = CheckCollision(a.get(), b.get());

            if (info.isHit)
            {
                // a に衝突情報を渡す
                info.other = b->owner;
                if (a->owner)
                {
                    a->owner->OnCollision(info);
                }

                // b にも逆向きの情報を渡す
                info.other = a->owner;
                if (b->owner)
                {
                    b->owner->OnCollision(info);
                }
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
    {
        const BoxCollider* b1 = static_cast<const BoxCollider*>(collider1);
        const BoxCollider* b2 = static_cast<const BoxCollider*>(collider2);

        // 両方の箱が回転していない（ほぼ0）なら、今までの軽い方を呼ぶ
        if (AbsF(b1->owner->m_rotation.x) < 0.001f && AbsF(b1->owner->m_rotation.y) < 0.001f && AbsF(b1->owner->m_rotation.z) < 0.001f &&
            AbsF(b2->owner->m_rotation.x) < 0.001f && AbsF(b2->owner->m_rotation.y) < 0.001f && AbsF(b2->owner->m_rotation.z) < 0.001f)
        {
            return CheckBoxBox(b1, b2); // 軽い！
        }

        // どちらかが回転している時だけ OBB を使う
        return CheckBoxBoxOBB(b1, b2); // 槍などの時だけ発動
    }
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

    // Slope vs Sphere
    if (collider1->type == ColliderType::Sphere && collider2->type == ColliderType::Slope) {
        return CheckTrapezoidSlopeSphere(static_cast<const TrapezoidSlopeCollider*>(collider2), static_cast<const SphereCollider*>(collider1));
    }
    if (collider1->type == ColliderType::Slope && collider2->type == ColliderType::Sphere) {
        CollisionInfo info = CheckTrapezoidSlopeSphere(static_cast<const TrapezoidSlopeCollider*>(collider1), static_cast<const SphereCollider*>(collider2));
        InvertNormalize(info); return info;
    }

    // --- ボックス vs 台形 ---
    if (collider1->type == ColliderType::Box && collider2->type == ColliderType::Slope) {
        return CheckBoxTrapezoidSlope(static_cast<const BoxCollider*>(collider1), static_cast<const TrapezoidSlopeCollider*>(collider2));
    }

    if (collider1->type == ColliderType::Slope && collider2->type == ColliderType::Box) {
        CollisionInfo info = CheckBoxTrapezoidSlope(static_cast<const BoxCollider*>(collider2), static_cast<const TrapezoidSlopeCollider*>(collider1));
        InvertNormalize(info); return info;
    }

    // --- 台形 vs 台形 ---
    if (collider1->type == ColliderType::Slope && collider2->type == ColliderType::Slope) {
        return CheckTrapezoidSlopeTrapezoidSlope(static_cast<const TrapezoidSlopeCollider*>(collider1), static_cast<const TrapezoidSlopeCollider*>(collider2));
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

//================================================================
//  Box vs Box(回転付き)
//================================================================
CollisionInfo ManagerCollider::CheckBoxBoxOBB(const BoxCollider* box1, const BoxCollider* box2)
{
    CollisionInfo info;

    // 1. 各ボックスのワールド座標、サイズ、回転を取得
    XMFLOAT3 worldPosA = box1->WorldPosition(); // 一度変数に受ける
    XMFLOAT3 worldPosB = box2->WorldPosition(); // これで実体（左辺値）ができる
    XMVECTOR posA = XMLoadFloat3(&worldPosA);   // その変数のアドレスを渡す
    XMVECTOR posB = XMLoadFloat3(&worldPosB);
    XMFLOAT3 sizeA = box1->HalfSize();
    XMFLOAT3 sizeB = box2->HalfSize();

    // 2. 回転行列を作成（GameObjectの回転から作成）
    XMMATRIX rotA = XMMatrixRotationRollPitchYaw(box1->owner->m_rotation.x, box1->owner->m_rotation.y, box1->owner->m_rotation.z);
    XMMATRIX rotB = XMMatrixRotationRollPitchYaw(box2->owner->m_rotation.x, box2->owner->m_rotation.y, box2->owner->m_rotation.z);

    // 3. 各箱の「向いている軸」を取得 (X, Y, Z軸を回転させる)
    XMVECTOR axesA[3] = {
        XMVector3TransformNormal(XMVectorSet(1,0,0,0), rotA),
        XMVector3TransformNormal(XMVectorSet(0,1,0,0), rotA),
        XMVector3TransformNormal(XMVectorSet(0,0,1,0), rotA)
    };
    XMVECTOR axesB[3] = {
        XMVector3TransformNormal(XMVectorSet(1,0,0,0), rotB),
        XMVector3TransformNormal(XMVectorSet(0,1,0,0), rotB),
        XMVector3TransformNormal(XMVectorSet(0,0,1,0), rotB)
    };

    // 4. 二つの箱の中心間ベクトル
    XMVECTOR L = XMVectorSubtract(posB, posA);

    // --- ここから本来は15軸の判定が必要ですが、まずは主要な「自分の各辺」の軸でチェック ---
    // (簡易版SAT: 軸Aの3方向、軸Bの3方向の合計6軸で判定)
    float overlapMin = 1000000.0f;
    XMVECTOR hitNormal = XMVectorZero();

    auto CheckAxis = [&](XMVECTOR axis) -> bool {
        // 軸がほぼゼロならスキップ
        if (XMVector3LengthSq(axis).m128_f32[0] < 0.001f) return true;
        axis = XMVector3Normalize(axis);

        float rA = fabsf(XMVectorGetX(XMVector3Dot(axesA[0], axis)) * sizeA.x) +
            fabsf(XMVectorGetX(XMVector3Dot(axesA[1], axis)) * sizeA.y) +
            fabsf(XMVectorGetX(XMVector3Dot(axesA[2], axis)) * sizeA.z);

        // 箱Bの投影半径
        float rB = fabsf(XMVectorGetX(XMVector3Dot(axesB[0], axis)) * sizeB.x) +
            fabsf(XMVectorGetX(XMVector3Dot(axesB[1], axis)) * sizeB.y) +
            fabsf(XMVectorGetX(XMVector3Dot(axesB[2], axis)) * sizeB.z);

        // 中心間の投影距離
        float distance = fabsf(XMVectorGetX(XMVector3Dot(L, axis)));

        float overlap = rA + rB - distance;
        if (overlap <= 0.0f) return false;

        if (overlap < overlapMin) {
            overlapMin = overlap;
            hitNormal = axis;
        }
        return true;
        };

    // 箱Aの各軸、箱Bの各軸でチェック
    if (!CheckAxis(axesA[0])) return info;
    if (!CheckAxis(axesA[1])) return info;
    if (!CheckAxis(axesA[2])) return info;
    if (!CheckAxis(axesB[0])) return info;
    if (!CheckAxis(axesB[1])) return info;
    if (!CheckAxis(axesB[2])) return info;

    // すべての軸で重なっていたら衝突！
    info.isHit = true;
    info.penetration = overlapMin;
    XMStoreFloat3(&info.normal, hitNormal);

    return info;
}

CollisionInfo ManagerCollider::CheckTrapezoidSlopeSphere(const TrapezoidSlopeCollider* slope, const SphereCollider* sphere)
{
    CollisionInfo info;
    XMVECTOR objPos = XMLoadFloat3(&slope->owner->m_position);
    // start と end に、親の座標を足し算して「現在のワールド座標」にする
    XMVECTOR S = XMVectorAdd(objPos, XMLoadFloat3(&slope->start));
    XMVECTOR E = XMVectorAdd(objPos, XMLoadFloat3(&slope->end));
    XMVECTOR C = XMLoadFloat3(&sphere->owner->m_position);

    XMVECTOR lineVec = XMVectorSubtract(E, S);
    float lineLenSq = XMVector3LengthSq(lineVec).m128_f32[0];
    if (lineLenSq < 1e-6f) return info;

    // 1. 線分上の比率 t (0~1)
    XMVECTOR v = XMVectorSubtract(C, S);
    float t = (std::max)(0.0f, (std::min)(1.0f, XMVector3Dot(v, lineVec).m128_f32[0] / lineLenSq));

    // 2. その地点の幅と中心点
    float currentWidth = slope->startWidth + (slope->endWidth - slope->startWidth) * t;
    XMVECTOR P = XMVectorAdd(S, XMVectorScale(lineVec, t));

    // 3. 右方向(幅)ベクトルと上方向(厚み)ベクトルの算出
    XMVECTOR upDir = XMVectorSet(0, 1, 0, 0);
    XMVECTOR forwardDir = XMVector3Normalize(lineVec);
    if (fabsf(XMVector3Dot(forwardDir, upDir).m128_f32[0]) > 0.99f) upDir = XMVectorSet(0, 0, 1, 0);
    XMVECTOR rightDir = XMVector3Normalize(XMVector3Cross(forwardDir, upDir));
    upDir = XMVector3Normalize(XMVector3Cross(rightDir, forwardDir)); // 正確な上方向

    // 4. 板の範囲内での最近接点 P_final を求める
    XMVECTOR vToC = XMVectorSubtract(C, P);
    float distR = XMVector3Dot(vToC, rightDir).m128_f32[0];
    float distU = XMVector3Dot(vToC, upDir).m128_f32[0];

    // 幅と厚みの範囲内にクランプ
    distR = (std::max)(-currentWidth * 0.5f, (std::min)(currentWidth * 0.5f, distR));
    distU = (std::max)(-slope->thickness * 0.5f, (std::min)(slope->thickness * 0.5f, distU));

    XMVECTOR P_final = XMVectorAdd(P, XMVectorAdd(XMVectorScale(rightDir, distR), XMVectorScale(upDir, distU)));

    // 5. 衝突判定
    XMVECTOR diff = XMVectorSubtract(C, P_final);
    float dist = XMVector3Length(diff).m128_f32[0];
    if (dist < sphere->radius) {
        info.isHit = true;
        info.other = slope->owner;
        info.penetration = sphere->radius - dist;
        XMStoreFloat3(&info.normal, XMVector3Normalize(diff));
        XMStoreFloat3(&info.depth, XMVectorScale(XMLoadFloat3(&info.normal), info.penetration));
    }
    return info;
}

CollisionInfo ManagerCollider::CheckBoxTrapezoidSlope(const BoxCollider* box, const TrapezoidSlopeCollider* slope)
{
    CollisionInfo info;
    XMFLOAT3 boxHalf = box->HalfSize();

    // --- ここで行列を合成する ---
    XMMATRIX mScale = XMMatrixScaling(box->owner->m_scale.x, box->owner->m_scale.y, box->owner->m_scale.z);
    XMMATRIX mRot = XMMatrixRotationRollPitchYaw(box->owner->m_rotation.x, box->owner->m_rotation.y, box->owner->m_rotation.z);
    XMMATRIX mTrans = XMMatrixTranslation(box->owner->m_position.x, box->owner->m_position.y, box->owner->m_position.z);

    // ワールド行列 = スケール * 回転 * 移動
    XMMATRIX boxWorld = mScale * mRot * mTrans;
    // ----------------------------

    // ボックスのローカル座標における8頂点
    XMFLOAT3 corners[8] = {
        {-boxHalf.x, -boxHalf.y, -boxHalf.z}, {boxHalf.x, -boxHalf.y, -boxHalf.z},
        {-boxHalf.x,  boxHalf.y, -boxHalf.z}, {boxHalf.x,  boxHalf.y, -boxHalf.z},
        {-boxHalf.x, -boxHalf.y,  boxHalf.z}, {boxHalf.x, -boxHalf.y,  boxHalf.z},
        {-boxHalf.x,  boxHalf.y,  boxHalf.z}, {boxHalf.x,  boxHalf.y,  boxHalf.z}
    };

    float maxPen = -1.0f;
    // 8つの頂点が台形の中にめり込んでいるか調べる
    for (int i = 0; i < 8; i++) {
        // ローカル頂点をワールド座標に変換
        XMVECTOR worldCorner = XMVector3Transform(XMLoadFloat3(&corners[i]), boxWorld);

        // 以前作成した CheckTrapezoidSlopeSphere のロジックを流用するため
        // 一時的にこの頂点を中心とする半径0の「点」として判定を行う
        // ※実際には球を作らず、判定ロジックだけを抽出して呼ぶのがスマートです

        XMVECTOR objPos = XMLoadFloat3(&slope->owner->m_position);
        // start と end に、親の座標を足し算して「現在のワールド座標」にする
        XMVECTOR S = XMVectorAdd(objPos, XMLoadFloat3(&slope->start));
        XMVECTOR E = XMVectorAdd(objPos, XMLoadFloat3(&slope->end));
        XMVECTOR lineVec = XMVectorSubtract(E, S);
        float lineLenSq = XMVector3LengthSq(lineVec).m128_f32[0];
        if (lineLenSq < 1e-6f) continue;

        XMVECTOR v = XMVectorSubtract(worldCorner, S);
        float t = (std::max)(0.0f, (std::min)(1.0f, XMVector3Dot(v, lineVec).m128_f32[0] / lineLenSq));
        float currentWidth = slope->startWidth + (slope->endWidth - slope->startWidth) * t;
        XMVECTOR P = XMVectorAdd(S, XMVectorScale(lineVec, t));

        // 方向ベクトルの算出（CheckTrapezoidSlopeSphereと同じ）
        XMVECTOR upDir = XMVectorSet(0, 1, 0, 0);
        XMVECTOR forwardDir = XMVector3Normalize(lineVec);
        if (fabsf(XMVector3Dot(forwardDir, upDir).m128_f32[0]) > 0.99f) upDir = XMVectorSet(0, 0, 1, 0);
        XMVECTOR rightDir = XMVector3Normalize(XMVector3Cross(forwardDir, upDir));
        upDir = XMVector3Normalize(XMVector3Cross(rightDir, forwardDir));

        XMVECTOR vToC = XMVectorSubtract(worldCorner, P);
        float distR = XMVector3Dot(vToC, rightDir).m128_f32[0];
        float distU = XMVector3Dot(vToC, upDir).m128_f32[0];

        // 台形の範囲内（幅・厚み）に点があるかチェック
        if (fabsf(distR) <= currentWidth * 0.5f && fabsf(distU) <= slope->thickness * 0.5f) {
            // 最も近い面への押し戻し距離を計算
            float penR = (currentWidth * 0.5f) - fabsf(distR);
            float penU = (slope->thickness * 0.5f) - fabsf(distU);
            float penetration = (std::min)(penR, penU);

            if (penetration > maxPen) {
                maxPen = penetration;
                info.isHit = true;
                info.penetration = penetration;
                // 法線は単純化して上方向（坂の面）とする
                XMStoreFloat3(&info.normal, upDir);
                XMStoreFloat3(&info.depth, XMVectorScale(upDir, penetration));
            }
        }
    }

    info.other = const_cast<GameObject*>(slope->owner);
    return info;
}

CollisionInfo ManagerCollider::CheckTrapezoidSlopeTrapezoidSlope(const TrapezoidSlopeCollider* slope1, const TrapezoidSlopeCollider* slope2)
{
    CollisionInfo info;

    // slope1の各端点を算出
    auto GetCorners = [](const TrapezoidSlopeCollider* s, XMVECTOR* outCorners) {
        XMVECTOR S = XMLoadFloat3(&s->start);
        XMVECTOR E = XMLoadFloat3(&s->end);
        XMVECTOR dir = XMVector3Normalize(XMVectorSubtract(E, S));
        XMVECTOR up = XMVectorSet(0, 1, 0, 0);
        if (fabsf(XMVector3Dot(dir, up).m128_f32[0]) > 0.99f) up = XMVectorSet(0, 0, 1, 0);
        XMVECTOR right = XMVector3Normalize(XMVector3Cross(dir, up));
        up = XMVector3Normalize(XMVector3Cross(right, dir));

        float sw = s->startWidth * 0.5f;
        float ew = s->endWidth * 0.5f;
        float th = s->thickness * 0.5f;

        // 始点側4点, 終点側4点
        outCorners[0] = S + right * sw + up * th; outCorners[1] = S - right * sw + up * th;
        outCorners[2] = S + right * sw - up * th; outCorners[3] = S - right * sw - up * th;
        outCorners[4] = E + right * ew + up * th; outCorners[5] = E - right * ew + up * th;
        outCorners[6] = E + right * ew - up * th; outCorners[7] = E - right * ew - up * th;
        };

    XMVECTOR corners1[8];
    GetCorners(slope1, corners1);

    float maxPen = -1.0f;
    for (int i = 0; i < 8; i++) {
        SphereCollider point(nullptr, 0.0f);
        point.owner = new GameObject();
        XMStoreFloat3(&point.owner->m_position, corners1[i]);
        CollisionInfo cinfo = CheckTrapezoidSlopeSphere(slope2, &point);
        delete point.owner;

        if (cinfo.isHit && cinfo.penetration > maxPen) {
            info = cinfo;
            maxPen = cinfo.penetration;
        }
    }

    info.other = slope2->owner;
    return info;
}