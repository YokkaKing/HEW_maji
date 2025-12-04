// ===============================
// arrow.cpp
// ===============================
#include "arrow.h"
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"
#include"debug_ostream.h"

Arrow::Arrow()
    : center(0.0f, 0.0f, 0.0f),
    halfSize(0.1f, 0.1f, 0.5f), // 細長い当たり判定
    isActive(false),
    m_AttackFrameTimer(0),
    m_ChargeTimer(0),   //追加
    m_ChargeLevel(0),   //追加
    m_Damage(0.0f),     //追加
    m_Range(0.0f),      //追加
    m_model(nullptr),
    m_scale(0.2f, 0.2f, 0.2f),
    m_rotation(0.0f, 0.0f, 0.0f),
    m_offset(0.0f, 0.0f, 0.0f),
    m_velocity(0.0f, 0.0f, 0.0f)
{}

void Arrow::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_model = ModelLoad("asset\\model\\arrow.fbx");
    if (!m_model)
    {
        hal::dout << "ERROR: Failed to load arrow model.\n";
    }
}

void Arrow::Finalize()
{
    if (m_model)
    {
        ModelRelease(m_model);
        m_model = nullptr;
    }
}

// IWeapon::StartAttackの実装
// プレイヤーの位置と回転を受け取り、当たり判定を生成/有効化する
void Arrow::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    
}

// プレイヤーの向いている方向に矢を撃つ //追加
void Arrow::Shoot(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) //追加
{
    center = playerPosition;

    // プレイヤーの回転から前方向ベクトルを計算
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(playerRotation.x, playerRotation.y, playerRotation.z);
    XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);
    XMStoreFloat3(&m_velocity, forward);

    // 射程距離を速度に反映
    m_velocity.x *= m_Range;
    m_velocity.y *= m_Range;
    m_velocity.z *= m_Range;

    isActive = true;
    m_AttackFrameTimer = 0;
}

// Aボタン入力処理 //追加
void Arrow::HandleInput(bool isAPressed, bool isAReleased, const XMFLOAT3& playerPos, const XMFLOAT3& playerRot) //追加
{
    if (isAPressed)
    {
        // 押し続けている間チャージ
        m_ChargeTimer++;
    }

    if (isAReleased)
    {
        // チャージ段階判定（60FPS換算）
        if (m_ChargeTimer == 0)
        {
            // 即押し
            m_ChargeLevel = 0;
            m_Range = 0.5f;
            m_Damage = 3.0f;
        }
        else if (m_ChargeTimer < 120)
        { // 1〜2秒
            m_ChargeLevel = 1;
            m_Range = 1.0f;
            m_Damage = 6.0f;
        }
        else if (m_ChargeTimer < 180)
        { // 2〜3秒
            m_ChargeLevel = 2;
            m_Range = 2.0f;
            m_Damage = 12.0f;
        }
        else
        { // 3秒以上
            m_ChargeLevel = 3;
            m_Range = 4.0f;
            m_Damage = 12.0f;
        }

        // プレイヤーの向きに矢を撃つ
        Shoot(playerPos, playerRot);

        // チャージリセット
        m_ChargeTimer = 0;
    }
}

void Arrow::EndAttack()
{
    isActive = false;
    m_AttackFrameTimer = 0;
}

void Arrow::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    if (!m_model) return;

    XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX rotation = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
    XMMATRIX translation = XMMatrixTranslation(center.x, center.y, center.z);

    XMMATRIX world = scale * rotation * translation;
    XMMATRIX view = GetViewMatrix();
    XMMATRIX projection = GetProjectionMatrix();
    XMMATRIX wvp = world * view * projection;

    Shader_SetWorldMatrix(world);
    Shader_SetMatrix(wvp);

    ModelDraw(m_model);
}

void Arrow::Update(float deltaTime)
{
    if (isActive) 
    {
        m_AttackFrameTimer++;

        // 矢を速度ベクトルで移動
        center.x += m_velocity.x * deltaTime;
        center.y += m_velocity.y * deltaTime;
        center.z += m_velocity.z * deltaTime;
    }
}

bool Arrow::ShouldEndAttack() const
{
    return isActive && (m_AttackFrameTimer >= ATTACK_DURATION_FRAMES);
}

bool Arrow::IsAttacking() const
{
    return isActive;
}

bool Arrow::CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize)
{
    if (!isActive) return false;

    float arrowMinX = center.x - halfSize.x;
    float arrowMaxX = center.x + halfSize.x;
    float arrowMinY = center.y - halfSize.y;
    float arrowMaxY = center.y + halfSize.y;
    float arrowMinZ = center.z - halfSize.z;
    float arrowMaxZ = center.z + halfSize.z;

    float playerMinX = playerCenter.x - playerHalfSize.x;
    float playerMaxX = playerCenter.x + playerHalfSize.x;
    float playerMinY = playerCenter.y - playerHalfSize.y;
    float playerMaxY = playerCenter.y + playerHalfSize.y;
    float playerMinZ = playerCenter.z - playerHalfSize.z;
    float playerMaxZ = playerCenter.z + playerHalfSize.z;

    bool collisionX = (arrowMinX <= playerMaxX) && (arrowMaxX >= playerMinX);
    bool collisionY = (arrowMinY <= playerMaxY) && (arrowMaxY >= playerMinY);
    bool collisionZ = (arrowMinZ <= playerMaxZ) && (arrowMaxZ >= playerMinZ);

    return collisionX && collisionY && collisionZ;
}