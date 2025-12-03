#pragma once
// ===============================
// sword.h
// ===============================
#include <DirectXMath.h>
#include "IWeapon.h"
#include "model.h"
using namespace DirectX;

// class Sword { の代わりに IWeapon を継承
class Sword : public IWeapon
{ // ★ IWeaponを継承
public:
    Sword();
    // IWeaponで要求される純粋仮想関数を全てオーバーライド

    // IWeapon 
    virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) override;
    virtual void Finalize() override;
    virtual void StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
    virtual void EndAttack() override;
    virtual void Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
    virtual void Update(float deltaTime) override;
    virtual bool ShouldEndAttack() const override;
    virtual bool IsAttacking() const override;

    void Update(XMFLOAT3& playerPos); // プレイヤー位置に追従 (既存のUpdateは別名か削除を推奨)


    bool IsActive() { return isActive; } // 既存のヘルパー関数
    XMFLOAT3 GetCenter() { return center; }
    XMFLOAT3 GetHalfSize() { return halfSize; }

    // 敵との衝突判定
    bool CheckCollision(XMFLOAT3& playerCenter, XMFLOAT3& playerHalfSize);

    // Aボタン一回押しで攻撃する処理
    void HandleInput(bool isAPressed, bool isAReleased,const XMFLOAT3& playerPos, const XMFLOAT3& playerRot); //追加

    // ダメージと射程を外部から参照できるようにする
    float GetDamage() const { return m_Damage; } //追加
    float GetRange() const { return m_Range; }   //追加

private:
    XMFLOAT3 center;   // BOXの中心座標
    XMFLOAT3 halfSize; // BOXの半寸法
    bool isActive;     // 攻撃中フラグ

    // 攻撃タイマーを管理するため、IWeaponの抽象化されていないSwordにメンバーを追加
    int m_AttackFrameTimer;
    const int ATTACK_DURATION_FRAMES = 30; // 30フレームで攻撃終了と仮定


    //剣のモデル
    MODEL* m_model;

    XMFLOAT3 m_scale;    // 剣のスケール
    XMFLOAT3 m_rotation; // 剣の回転
    XMFLOAT3 m_offset;   // 剣のオフセット (プレイヤー相対)

    // ダメージと射程
    float m_Damage; //追加
    float m_Range;  //追加
};