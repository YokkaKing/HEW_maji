// ===============================
// syuriken.cpp
// ===============================
#include "syuriken.h"
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"
#include"debug_ostream.h"

Syuriken::Syuriken()
    : center(0.0f, 0.0f, 0.0f),
    halfSize(0.2f, 0.2f, 0.05f), // 手裏剣は薄くて小さい
    isActive(false),
    m_AttackFrameTimer(0),
    m_model(nullptr),
    m_scale(0.2f, 0.2f, 0.2f),
    m_rotation(0.0f, 0.0f, 0.0f),
    m_offset(0.5f, 0.2f, 0.0f),
    m_velocity(0.0f, 0.0f, 0.0f)
{}

void Syuriken::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_model = ModelLoad("asset\\model\\syuriken.fbx");
    if (!m_model) {
        hal::dout << "ERROR: Failed to load syuriken model.\n";
    }
}

void Syuriken::Finalize()
{
    if (m_model) {
        ModelRelease(m_model);
        m_model = nullptr;
    }
}

void Syuriken::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    isActive = true;
    m_AttackFrameTimer = 0;

    // プレイヤー位置から投げる
    center = playerPosition;
    m_velocity = XMFLOAT3(0.0f, 0.0f, 0.8f); // 仮に前方向へ飛ばす
}

void Syuriken::EndAttack()
{
    isActive = false;
    m_AttackFrameTimer = 0;
}

void Syuriken::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
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

void Syuriken::Update(float deltaTime)
{
    if (isActive) {
        m_AttackFrameTimer++;

        // 投げた方向に移動
        center.x += m_velocity.x * deltaTime;
        center.y += m_velocity.y * deltaTime;
        center.z += m_velocity.z * deltaTime;
    }
}

bool Syuriken::ShouldEndAttack() const
{
    return isActive && (m_AttackFrameTimer >= ATTACK_DURATION_FRAMES);
}

bool Syuriken::IsAttacking() const
{
    return isActive;
}

void Syuriken::Throw(const XMFLOAT3& startPos, const XMFLOAT3& velocity)
{
    center = startPos;
    m_velocity = velocity;
    isActive = true;
    m_AttackFrameTimer = 0;
}

bool Syuriken::CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize)
{
    if (!isActive) return false;

    float minX = center.x - halfSize.x;
    float maxX = center.x + halfSize.x;
    float minY = center.y - halfSize.y;
    float maxY = center.y + halfSize.y;
    float minZ = center.z - halfSize.z;
    float maxZ = center.z + halfSize.z;

    float playerMinX = playerCenter.x - playerHalfSize.x;
    float playerMaxX = playerCenter.x + playerHalfSize.x;
    float playerMinY = playerCenter.y - playerHalfSize.y;
    float playerMaxY = playerCenter.y + playerHalfSize.y;
    float playerMinZ = playerCenter.z - playerHalfSize.z;
    float playerMaxZ = playerCenter.z + playerHalfSize.z;

    bool collisionX = (minX <= playerMaxX) && (maxX >= playerMinX);
    bool collisionY = (minY <= playerMaxY) && (maxY >= playerMinY);
    bool collisionZ = (minZ <= playerMaxZ) && (maxZ >= playerMinZ);

    return collisionX && collisionY && collisionZ;
}