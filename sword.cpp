// ===============================
//sword.cpp
// ===============================
#include "sword.h"
#include "direct3d.h"
#include "shader.h"
#include "Camera.h"

Sword::Sword()
    : center(0.0f, 0.0f, 0.0f),
    halfSize(0.5f, 0.5f, 0.05f),
    isActive(false),
    m_AttackFrameTimer(0), // タイマー初期化

    m_model(nullptr), //初期化
    m_scale(0.2f, 0.2f, 0.2f), //剣のスケールを小さめに設定
    m_rotation(0.0f, XM_PIDIV2, 0.0f), //剣をプレイヤーに合わせて回転させる
    m_offset(0.5f, 0.1f, 0.0f), //剣をプレイヤーの右側、少し上に配置
    m_Damage(0.0f), //追加: ダメージ初期化
    m_Range(0.0f)   //追加: 射程初期化
{
}

// IWeapon::Initializeの実装
void Sword::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    // モデルロードやリソースの初期化処理
    //m_model = ModelLoad("asset\\model\\sword.fbx");
    m_model = ModelLoad("asset\\model\\test.fbx");

    if (m_model == nullptr)
    {
        hal::dout << "ERROR: Failed to load sword model.\n";
    }
}

// IWeapon::Finalizeの実装
void Sword::Finalize()
{
    //モデルの解放
    if (m_model)
    {
        ModelRelease(m_model);
        m_model = nullptr;
    }
}

// IWeapon::StartAttackの実装
// プレイヤーの位置と回転を受け取り、当たり判定を生成/有効化する
void Sword::StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    // 剣の当たり判定を有効化
    isActive = true;
    m_AttackFrameTimer = 0; // タイマーリセット

    // プレイヤーの位置と回転を元に、剣の位置を決定
    // 今回の例では回転を使わず、プレイヤーの右側に固定（既存のUpdateロジックを利用）
    center.x = playerPosition.x + 0.5f;
    center.y = playerPosition.y;
    center.z = playerPosition.z;

}

// IWeapon::EndAttackの実装
void Sword::EndAttack()
{
    // 剣の当たり判定を無効化
    isActive = false;
    m_AttackFrameTimer = 0; // タイマーリセット
}

// IWeapon::Drawの実装
void Sword::Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation)
{
    if (!m_model) return;

    // ★ 剣のワールド行列を作成

    // 1. スケール
    XMMATRIX scale = XMMatrixScaling(
        m_scale.x,
        m_scale.y,
        m_scale.z);

    // 2. 剣自身の回転 (待機時や攻撃アニメーションの回転)
    XMMATRIX sword_rotation = XMMatrixRotationRollPitchYaw(
        m_rotation.x,
        m_rotation.y,
        m_rotation.z);

    // 3. プレイヤーの回転 (剣もプレイヤーと同時に回転する)
    XMMATRIX player_rotation = XMMatrixRotationRollPitchYaw(
        playerRotation.x,
        playerRotation.y,
        playerRotation.z);

    // 4. 剣のオフセット移動 (プレイヤーの中心からの相対位置)
    // プレイヤーの回転を考慮するため、オフセットを回転行列で変換する必要がある
    XMMATRIX offset_translation = XMMatrixTranslation(
        m_offset.x,
        m_offset.y,
        m_offset.z);

    // 5. プレイヤーのワールド空間での位置
    XMMATRIX player_translation = XMMatrixTranslation(
        playerPosition.x,
        playerPosition.y,
        playerPosition.z);

    // ワールド行列の結合順序
    // LocalScale -> LocalRotation -> LocalOffset(回転込) -> PlayerTranslation
    // (LocalOffset * PlayerRotation) でオフセットを回転させる
    XMMATRIX world = scale * sword_rotation * offset_translation * player_rotation * player_translation;


    // 変換行列作成
    XMMATRIX view = GetViewMatrix();
    XMMATRIX projection = GetProjectionMatrix();
    XMMATRIX wvp = world * view * projection;

    // シェーダーへ行列をセット
    Shader_SetWorldMatrix(world);
    Shader_SetMatrix(wvp);

    // モデルの描画リクエスト
    ModelDraw(m_model);
}

// IWeapon::Updateの実装
void Sword::Update(float deltaTime)
{
    // 武器自身の状態（タイマーなど）を更新する
    if (isActive)
    {
        m_AttackFrameTimer++;
    }

}

// IWeapon::ShouldEndAttackの実装
bool Sword::ShouldEndAttack() const
{
    // タイマーが規定のフレーム数を超えたら true
    return isActive && (m_AttackFrameTimer >= ATTACK_DURATION_FRAMES);
}

// IWeapon::IsAttackingの実装
bool Sword::IsAttacking() const
{
    return isActive;
}

void Sword::Update(XMFLOAT3& playerPos)
{
    // プレイヤー位置に追従 (既存の関数、IWeaponのUpdateとは別)
    center.x = playerPos.x + 0.5f;
    center.y = playerPos.y;
    center.z = playerPos.z;
}
/*
void Sword::StartAttack()
{
    isActive = true;
}

void Sword::EndAttack()
{
    isActive = false;
}
*/

// Aボタン一回押しで攻撃する処理 //追加
void Sword::HandleInput(bool isAPressed, bool isAReleased,const XMFLOAT3& playerPos, const XMFLOAT3& playerRot) //追加
{
    if (isAPressed)
    {
        // 即押しで攻撃開始
        isActive = true;
        m_AttackFrameTimer = 0;

        // プレイヤー位置に基づいて剣の位置を設定
        center.x = playerPos.x + 0.5f;
        center.y = playerPos.y;
        center.z = playerPos.z;

        // 射程とダメージを設定
        m_Range = 1.0f;   //追加: 射程1m
        m_Damage = 10.0f; //追加: ダメージ10
    }

    if (isAReleased)
    {
        // 攻撃終了
        EndAttack();
    }
       
}

bool Sword::CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize)
{
    if (!isActive) return false;

    // 剣の当たり判定BOXの最小・最大
    float swordMinX = center.x - halfSize.x;
    float swordMaxX = center.x + halfSize.x;
    float swordMinY = center.y - halfSize.y;
    float swordMaxY = center.y + halfSize.y;
    float swordMinZ = center.z - halfSize.z;
    float swordMaxZ = center.z + halfSize.z;

    // プレイヤーの当たり判定BOXの最小・最大
    float playerMinX = playerCenter.x - playerHalfSize.x;
    float playerMaxX = playerCenter.x + playerHalfSize.x;
    float playerMinY = playerCenter.y - playerHalfSize.y;
    float playerMaxY = playerCenter.y + playerHalfSize.y;
    float playerMinZ = playerCenter.z - playerHalfSize.z;
    float playerMaxZ = playerCenter.z + playerHalfSize.z;

    // AABB同士の衝突判定
    bool collisionX = (swordMinX <= playerMaxX) && (swordMaxX >= playerMinX);
    bool collisionY = (swordMinY <= playerMaxY) && (swordMaxY >= playerMinY);
    bool collisionZ = (swordMinZ <= playerMaxZ) && (swordMaxZ >= playerMinZ);

    return collisionX && collisionY && collisionZ;
}
