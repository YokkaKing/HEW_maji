#pragma once
// ===============================
// syuriken.h
// ===============================
#include <DirectXMath.h>
#include "IWeapon.h"
#include "model.h"
using namespace DirectX;

class Syuriken : public IWeapon {
public:
    Syuriken();

    // IWeapon の基本メソッド
    virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) override;
    virtual void Finalize() override;
    virtual void StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
    virtual void EndAttack() override;
    virtual void Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
    virtual void Update(float deltaTime) override;
    virtual bool ShouldEndAttack() const override;
    virtual bool IsAttacking() const override;

    // 手裏剣専用メソッド
    void Throw(const XMFLOAT3& startPos, const XMFLOAT3& velocity); // 投げる

    XMFLOAT3 GetCenter() { return center; }
    XMFLOAT3 GetHalfSize() { return halfSize; }

    bool CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize);

private:
    XMFLOAT3 center;    // BOXの中心座標
    XMFLOAT3 halfSize;  // BOXの半寸法（小さめ）
    bool isActive;      // 攻撃中かどうか

    int m_AttackFrameTimer;
    const int ATTACK_DURATION_FRAMES = 90; // 手裏剣は飛び続けるので長め

    // モデル関連
    MODEL* m_model;
    XMFLOAT3 m_scale;
    XMFLOAT3 m_rotation;
    XMFLOAT3 m_offset;

    // 飛び道具の速度
    XMFLOAT3 m_velocity;
};