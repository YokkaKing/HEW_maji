// ===============================
// spear.cpp
// ===============================
#include "spear.h"
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"
#include"debug_ostream.h"

//Spear::Spear()
//    : center(0.0f, 0.0f, 0.0f),
//    halfSize(0.2f, 0.2f, 1.0f), // 槍は細長い判定
//    isActive(false),
//    m_AttackFrameTimer(0),
//    m_model(nullptr),
//    m_scale(0.3f, 0.3f, 0.3f),
//    m_rotation(0.0f, 0.0f, 0.0f),
//    m_offset(0.7f, 0.0f, 0.0f),
//    m_forward(0.0f, 0.0f, 1.0f),
//    m_velocity(0.0f, 0.0f, 0.0f),
//    m_isThrown(false),
//    m_Damage(0.0f),   //追加: ダメージ初期化
//    m_Range(0.0f),    //追加: 射程初期化
//    m_ChargeTimer(0)  //追加: チャージタイマー初期化
//{}
//
//void Spear::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
//{
//    m_model = ModelLoad("asset\\model\\spear.fbx");
//    if (!m_model)
//    {
//        hal::dout << "ERROR: Failed to load spear model.\n";
//    }
//}
//
//void Spear::Finalize()
//{
//    if (m_model)
//    {
//        ModelRelease(m_model);
//        m_model = nullptr;
//    }
//}
//
//void Spear::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
//{
//    isActive = true;
//    m_AttackFrameTimer = 0;
//    center = playerPosition;
//    m_isThrown = false; // 突き攻撃
//}
//
//void Spear::EndAttack()
//{
//    isActive = false;
//    m_AttackFrameTimer = 0;
//}
//
//void Spear::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
//{
//    if (!m_model) return;
//
//    XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
//    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
//    XMMATRIX translation = XMMatrixTranslation(center.x, center.y, center.z);
//
//    XMMATRIX world = scale * rotation * translation;
//    XMMATRIX view = GetViewMatrix();
//    XMMATRIX projection = GetProjectionMatrix();
//    XMMATRIX wvp = world * view * projection;
//
//    Shader_SetWorldMatrix(world);
//    Shader_SetMatrix(wvp);
//
//    ModelDraw(m_model);
//}
//
//void Spear::Update(float deltaTime)
//{
//    if (isActive)
//    {
//        m_AttackFrameTimer++;
//
//        if (m_isThrown)
//        {
//            // 投げ槍は速度で移動
//            center.x += m_velocity.x * deltaTime;
//            center.y += m_velocity.y * deltaTime;
//            center.z += m_velocity.z * deltaTime;
//        }
//        else
//        {
//            // 突き槍は前方向に突き出す
//            center.x += m_forward.x * 0.2f;
//            center.y += m_forward.y * 0.2f;
//            center.z += m_forward.z * 0.2f;
//        }
//    }
//}
//
//bool Spear::ShouldEndAttack() const
//{
//    return isActive && (m_AttackFrameTimer >= ATTACK_DURATION_FRAMES);
//}
//
//bool Spear::IsAttacking() const
//{
//    return isActive;
//}
//
//void Spear::Stab(const XMFLOAT3& startPos, const XMFLOAT3& forward)
//{
//    center = startPos;
//    m_forward = forward;
//    isActive = true;
//    m_isThrown = false;
//    m_AttackFrameTimer = 0;
//}
//
//void Spear::Launch(const XMFLOAT3& startPos, const XMFLOAT3& velocity)
//{
//    center = startPos;
//    m_velocity = velocity;
//    isActive = true;
//    m_isThrown = true;
//    m_AttackFrameTimer = 0;
//}
//
//// Aボタン入力処理 //追加
//void Spear::HandleInput(bool isAPressed, bool isAReleased,const XMFLOAT3& playerPos, const XMFLOAT3& playerRot) //追加
//{
//    if (isAPressed)
//    {
//        // 押し続けている間チャージ
//        m_ChargeTimer++;
//    }
//
//    if (isAReleased)
//    {
//        if (m_ChargeTimer < 120)
//        { // 2秒未満 → 即押し攻撃
//            m_Range = 2.0f;   //射程2m
//            m_Damage = 7.0f;  //ダメージ7
//        }
//        else
//        { // 2秒以上チャージ
//            m_Range = 6.0f;   //射程6m
//            m_Damage = 20.0f; //ダメージ20
//
//          //// 槍をプレイヤー位置から前方に発射
//          //  center = playerPos;
//          //  XMMATRIX rot = XMMatrixRotationRollPitchYaw(playerRot.x, playerRot.y, playerRot.z);
//          //  XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);
//          //  XMStoreFloat3(&m_velocity, forward);
//        }  
//
//        // 槍をプレイヤー位置から前方に発射
//        center = playerPos;
//        XMMATRIX rot = XMMatrixRotationRollPitchYaw(playerRot.x, playerRot.y, playerRot.z);
//        XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);
//        XMStoreFloat3(&m_velocity, forward);
//
//        // 射程を速度に反映
//        m_velocity.x *= m_Range;
//        m_velocity.y *= m_Range;
//        m_velocity.z *= m_Range;
//
//        isActive = true;
//        m_isThrown = true;
//        m_AttackFrameTimer = 0;
//
//        // チャージリセット
//        m_ChargeTimer = 0;
//    }
//}
//
//bool Spear::CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize)
//{
//    if (!isActive) return false;
//
//    float spearMinX = center.x - halfSize.x;
//    float spearMaxX = center.x + halfSize.x;
//    float spearMinY = center.y - halfSize.y;
//    float spearMaxY = center.y + halfSize.y;
//    float spearMinZ = center.z - halfSize.z;
//    float spearMaxZ = center.z + halfSize.z;
//
//    float playerMinX = playerCenter.x - playerHalfSize.x;
//    float playerMaxX = playerCenter.x + playerHalfSize.x;
//    float playerMinY = playerCenter.y - playerHalfSize.y;
//    float playerMaxY = playerCenter.y + playerHalfSize.y;
//    float playerMinZ = playerCenter.z - playerHalfSize.z;
//    float playerMaxZ = playerCenter.z + playerHalfSize.z;
//
//    bool collisionX = (spearMinX <= playerMaxX) && (spearMaxX >= playerMinX);
//    bool collisionY = (spearMinY <= playerMaxY) && (spearMaxY >= playerMinY);
//    bool collisionZ = (spearMinZ <= playerMaxZ) && (spearMaxZ >= playerMinZ);
//
//    return collisionX && collisionY && collisionZ;
//}