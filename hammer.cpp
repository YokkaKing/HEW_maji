// ===============================
// hammer.cpp
// ===============================
#include "hammer.h"
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"

Hammer::Hammer()
    : center(0.0f, 0.0f, 0.0f),
    halfSize(0.7f, 0.7f, 0.3f), // ハンマーは広めの判定
    isActive(false),
    m_AttackFrameTimer(0),
    m_model(nullptr),
    m_scale(0.4f, 0.4f, 0.4f),
    m_rotation(0.0f, XM_PIDIV2, 0.0f), // 下向きに振り下ろすイメージ
    m_offset(0.6f, 0.2f, 0.0f),         // プレイヤーの右手側に配置
    m_Damage(0.0f),     //追加: ダメージ初期化
    m_Range(1.0f),      //追加: 射程は常に1m
    m_ChargeTimer(0),   //追加: チャージタイマー初期化
    m_ChargeLevel(0)    //追加: チャージ段階初期化
{}

void Hammer::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_model = ModelLoad("asset\\model\\hammer.fbx");
    if (!m_model)
    {
        hal::dout << "ERROR: Failed to load hammer model.\n";
    }
}

void Hammer::Finalize()
{
    if (m_model)
    {
        ModelRelease(m_model);
        m_model = nullptr;
    }
}

void Hammer::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    isActive = true;
    m_AttackFrameTimer = 0;

    // プレイヤー位置に追従
    center.x = playerPosition.x + m_offset.x;
    center.y = playerPosition.y + m_offset.y;
    center.z = playerPosition.z + m_offset.z;
}

void Hammer::EndAttack()
{
    isActive = false;
    m_AttackFrameTimer = 0;
}

void Hammer::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    if (!m_model) return;

    XMMATRIX scale = XMMatrixScaling(m_scale.x, m_scale.y, m_scale.z);
    XMMATRIX hammer_rotation = XMMatrixRotationRollPitchYaw(m_rotation.x, m_rotation.y, m_rotation.z);
    XMMATRIX player_rotation = XMMatrixRotationRollPitchYaw(playerRotation.x, playerRotation.y, playerRotation.z);
    XMMATRIX offset_translation = XMMatrixTranslation(m_offset.x, m_offset.y, m_offset.z);
    XMMATRIX player_translation = XMMatrixTranslation(playerPosition.x, playerPosition.y, playerPosition.z);

    XMMATRIX world = scale * hammer_rotation * offset_translation * player_rotation * player_translation;

    XMMATRIX view = GetViewMatrix();
    XMMATRIX projection = GetProjectionMatrix();
    XMMATRIX wvp = world * view * projection;

    Shader_SetWorldMatrix(world);
    Shader_SetMatrix(wvp);

    ModelDraw(m_model);
}

void Hammer::Update(float deltaTime)
{
    if (isActive)
    {
        m_AttackFrameTimer++;
        // 振り下ろし動作を簡易的に表現（Y方向に少し下げる）
        center.y -= 0.01f;
    }
}

bool Hammer::ShouldEndAttack() const
{
    return isActive && (m_AttackFrameTimer >= ATTACK_DURATION_FRAMES);
}

bool Hammer::IsAttacking() const
{
    return isActive;
}

// Aボタン入力処理
void Hammer::HandleInput(bool isAPressed, bool isAReleased,const XMFLOAT3& playerPos, const XMFLOAT3& playerRot) //追加
{
    if (isAPressed)
    {
        // 押し続けている間チャージ
        m_ChargeTimer++;
    }

    if (isAReleased)
    {
        // チャージ段階判定（60FPS換算）
        if (m_ChargeTimer >= 150 && m_ChargeTimer < 210)
        { // 2.5秒〜3.5秒未満
            m_ChargeLevel = 1;
            m_Damage = 20.0f;
        }
        else if (m_ChargeTimer >= 210 && m_ChargeTimer < 270)
        { // 3.5秒〜4.5秒未満
            m_ChargeLevel = 2;
            m_Damage = 30.0f;
        }
        else if (m_ChargeTimer >= 270 && m_ChargeTimer < 330)
        { // 4.5秒〜5.5秒未満
            m_ChargeLevel = 3;
            m_Damage = 40.0f;
        }
        else if (m_ChargeTimer >= 330)
        { // 5.5秒以上
            m_ChargeLevel = 4;
            m_Damage = 70.0f;
        }

        // 射程は常に1m
        m_Range = 1.0f;

        // 攻撃開始
        isActive = true;
        m_AttackFrameTimer = 0;

        center.x = playerPos.x + m_offset.x;
        center.y = playerPos.y + m_offset.y;
        center.z = playerPos.z + m_offset.z;

        // チャージリセット
        m_ChargeTimer = 0;
    }
}

bool Hammer::CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize)
{
    if (!isActive) return false;

    float hammerMinX = center.x - halfSize.x;
    float hammerMaxX = center.x + halfSize.x;
    float hammerMinY = center.y - halfSize.y;
    float hammerMaxY = center.y + halfSize.y;
    float hammerMinZ = center.z - halfSize.z;
    float hammerMaxZ = center.z + halfSize.z;

    float playerMinX = playerCenter.x - playerHalfSize.x;
    float playerMaxX = playerCenter.x + playerHalfSize.x;
    float playerMinY = playerCenter.y - playerHalfSize.y;
    float playerMaxY = playerCenter.y + playerHalfSize.y;
    float playerMinZ = playerCenter.z - playerHalfSize.z;
    float playerMaxZ = playerCenter.z + playerHalfSize.z;

    bool collisionX = (hammerMinX <= playerMaxX) && (hammerMaxX >= playerMinX);
    bool collisionY = (hammerMinY <= playerMaxY) && (hammerMaxY >= playerMinY);
    bool collisionZ = (hammerMinZ <= playerMaxZ) && (hammerMaxZ >= playerMinZ);

    return collisionX && collisionY && collisionZ;
}