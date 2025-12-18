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
    m_ChargeTimer(0),   
    m_ChargeLevel(0),   
    m_Damage(0.0f),     
    m_Range(0.0f),      
    m_model(nullptr),
    m_scale(0.2f, 0.2f, 0.2f),
    m_rotation(0.0f, 0.0f, 0.0f),
    m_offset(0.0f, 0.0f, 0.0f),
    m_velocity(0.0f, 0.0f, 0.0f),
    m_startPosition(0.0f, 0.0f, 0.0f)
{}


void Arrow::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    m_model = ModelLoad("asset\\model\\char_bow.fbx");
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
void Arrow::Shoot(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    center = playerPosition;
    m_startPosition = center;//発射地点

    // プレイヤーの回転から前方向ベクトルを計算
    XMMATRIX rot = XMMatrixRotationRollPitchYaw(playerRotation.x, playerRotation.y, playerRotation.z);
    XMVECTOR forward = XMVector3TransformNormal(XMVectorSet(0, 0, 1, 0), rot);
    XMStoreFloat3(&m_velocity, forward);

    // 射程距離を速度に反映
    const float ARROW_FIXED_SPEED = 0.2f; //1フレームあたりの固定移動量
    m_velocity.x *= ARROW_FIXED_SPEED;
    m_velocity.y *= ARROW_FIXED_SPEED;
    m_velocity.z *= ARROW_FIXED_SPEED;

    // デバッグ出力
    hal::dout << "Arrow Shot! Damage: " << m_Damage
        << ", Range: " << m_Range
        << ", Initial Velocity X: " << m_velocity.x << std::endl;

    isActive = true;
    m_AttackFrameTimer = 0;
}

// Aボタン入力処理 //追加
void Arrow::HandleInput(bool isAPressed, bool isAReleased, const XMFLOAT3& playerPos, const XMFLOAT3& playerRot) //追加
{

    const int MAX_CHARGE_TIME = 180; // 最大チャージ時間を3秒(180フレーム)とする
    // 即押し(0フレーム)の基準値
    const float BASE_RANGE = 0.5f;
    const float BASE_DAMAGE = 3.0f;
    // 最大チャージ(MAX_CHARGE_TIMEフレーム)での最大値
    const float MAX_RANGE = 4.0f;
    const float MAX_DAMAGE = 12.0f;

    if (isAPressed)
    {
        // 押し続けている間チャージ
        m_ChargeTimer++;

        // チャージタイマーを最大値でクランプ（これ以上チャージしない）
        if (m_ChargeTimer > MAX_CHARGE_TIME)
        {
            m_ChargeTimer = MAX_CHARGE_TIME;
        }

        // --- フレームごとにパラメータを計算し、段々たまるようにする ---
        float chargeRatio = (float)m_ChargeTimer / MAX_CHARGE_TIME;

        // 即押し（チャージタイマーが0）の場合、chargeRatioは0になるため、BASE値が適用される
        if (m_ChargeTimer == 0) {
            m_Range = BASE_RANGE;
            m_Damage = BASE_DAMAGE;
        }
        else {
            // 線形補間（Lerp）: BASE値からMAX値まで、chargeRatioに応じて滑らかに増加
            m_Range = BASE_RANGE + (MAX_RANGE - BASE_RANGE) * chargeRatio;
            m_Damage = BASE_DAMAGE + (MAX_DAMAGE - BASE_DAMAGE) * chargeRatio;

            // チャージレベルの表示用更新（任意: 1, 60, 120, 180フレームでレベル1, 2, 3, 4）
            if (m_ChargeTimer >= 1 && m_ChargeTimer < 60) m_ChargeLevel = 1;
            else if (m_ChargeTimer < 120) m_ChargeLevel = 2;
            else if (m_ChargeTimer < 180) m_ChargeLevel = 3;
            else m_ChargeLevel = 4; // 3秒以上
        }

    }

    if (isAReleased)
    {
        // プレイヤーの向きに矢を撃つ
        Shoot(playerPos, playerRot);

        // チャージリセット
        m_ChargeTimer = 0;
        m_ChargeLevel = 0; // リセット
 
    }

}

void Arrow::EndAttack()
{
    isActive = false;
    m_AttackFrameTimer = 0;
    m_Range = 0.0f;
    m_Damage = 0.0f;
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
        center.x += m_velocity.x *1.0f;
        center.y += m_velocity.y *1.0f;
        center.z += m_velocity.z *1.0f;

        float dx = center.x - m_startPosition.x;
        float dy = center.y - m_startPosition.y;
        float dz = center.z - m_startPosition.z;
        // 距離の2乗を計算
        float distanceSq = dx * dx + dy * dy + dz * dz;
        if (distanceSq > m_Range * m_Range)
        {
            hal::dout << "Arrow End! Reached Max Range: " << m_Range << std::endl;
            EndAttack();
        }
        // デバッグ出力
        // 矢が動いていることを確認するため、座標を出力
        hal::dout << "Arrow Position: " << center.x << ", " << center.y << ", " << center.z << std::endl;
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