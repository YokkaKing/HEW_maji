#pragma once
// ===============================
// spear.h
// ===============================
#include <DirectXMath.h>
#include "IWeapon.h"
#include "model.h"
using namespace DirectX;

//class Spear : public IWeapon
//{
//public:
//    Spear();
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
//    // 槍専用メソッド
//    void Stab(const XMFLOAT3& startPos, const XMFLOAT3& forward);   // 突き攻撃
//    void Launch(const XMFLOAT3& startPos, const XMFLOAT3& velocity); // 投げ槍
//
//    XMFLOAT3 GetCenter() { return center; }
//    XMFLOAT3 GetHalfSize() { return halfSize; }
//
//    bool CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize);
//
//    // Aボタン入力処理
//    void HandleInput(bool isAPressed, bool isAReleased,const XMFLOAT3& playerPos, const XMFLOAT3& playerRot); //追加
//
//    // ダメージと射程を外部から参照できるようにする
//    float GetDamage() const { return m_Damage; } //追加
//    float GetRange() const { return m_Range; }   //追加
//
//private:
//    XMFLOAT3 center;    // BOXの中心座標
//    XMFLOAT3 halfSize;  // BOXの半寸法（細長い）
//    bool isActive;      // 攻撃中かどうか
//
//    int m_AttackFrameTimer;
//    const int ATTACK_DURATION_FRAMES = 60; // 槍の攻撃時間
//
//    // モデル関連
//    MODEL* m_model;
//    XMFLOAT3 m_scale;
//    XMFLOAT3 m_rotation;
//    XMFLOAT3 m_offset;
//
//    // 槍の動き
//    XMFLOAT3 m_forward;   // 突き方向
//    XMFLOAT3 m_velocity;  // 投げ槍の速度
//    bool m_isThrown;      // 投げ槍かどうか
//
//    // ダメージと射程
//    float m_Damage; //追加
//    float m_Range;  //追加
//
//    // チャージ用タイマー
//    int m_ChargeTimer; //追加
//};