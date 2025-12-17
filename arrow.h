#pragma once
// ===============================
// arrow.h
// ===============================
#include <DirectXMath.h>
#include "IWeapon.h"
#include "model.h"
using namespace DirectX;

//class Arrow : public IWeapon
//{
//public:
//    Arrow();
//
//    // IWeapon の基本メソッド
//    virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) override;
//    virtual void Finalize() override;
//    virtual void StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
//    virtual void EndAttack() override;
//    virtual void Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
//    virtual void Update(float deltaTime) override;
//    virtual bool ShouldEndAttack() const override;
//    virtual bool IsAttacking() const override;
//
//    virtual WEAPON_TYPE GetWeaponType() const override { return WEAPON_TYPE::ARROW; }
//      // プレイヤーの向いている方向に矢を撃つ //追加
//    void Shoot(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation); //追加
//
//    // 入力処理（Aボタン押下／長押しチャージ） //追加
//    void HandleInput(bool isAPressed, bool isAReleased,
//                     const XMFLOAT3& playerPos, const XMFLOAT3& playerRot); //追加
//    // 追加メソッド
//    XMFLOAT3 GetCenter() { return center; }
//    XMFLOAT3 GetHalfSize() { return halfSize; }
//    float GetDamage() const { return m_Damage; } //ダメージ//追加
//    float GetRange() const { return m_Range; }   //射程//追加
//
//    bool CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize);
//
//private:
//    // 当たり判定
//    XMFLOAT3 center;    // BOXの中心座標
//    XMFLOAT3 halfSize;  // BOXの半寸法（矢は細長い）
//    bool isActive;      // 攻撃中かどうか
//
//    // 攻撃管理
//    int m_AttackFrameTimer;
//    const int ATTACK_DURATION_FRAMES = 120; // 矢は飛び続けるので長め
//
//    // チャージ管理
//    int m_ChargeTimer;   // チャージ時間（フレーム） //追加
//    int m_ChargeLevel;   // チャージ段階（0〜3） //追加
//
//    // ダメージと射程
//    float m_Damage; //追加
//    float m_Range;  //追加
//
//    // モデル関連
//    MODEL* m_model;
//    XMFLOAT3 m_scale;
//    XMFLOAT3 m_rotation;
//    XMFLOAT3 m_offset;
//
//    // 矢の速度ベクトル
//    XMFLOAT3 m_velocity;
//
//    XMFLOAT3 m_startPosition;
//};