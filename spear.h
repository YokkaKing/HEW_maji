/*
* ファイル名	spear.h
* タイトル	剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

#ifndef SPEAR_H
#define SPEAR_H

//================================================================
//	インクルード
//================================================================
#include<DirectXMath.h>
#include"IWeapon.h"
#include "effect_anim.h"
#include"model.h"
#include"managerCollider.h"
#include "keyboard.h"
#include "controller.h"
using namespace DirectX;

class Spear : public IWeapon
{
public:
    std::shared_ptr<Collider> m_collider; // コライダーへの参照を保持

    bool m_isAttacking = false;
    bool m_isAttack = false;
    float m_attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.5f;   // 攻撃の有効時間

    // プレイヤーから見てどこに位置するか
    XMFLOAT3 m_offset = { 0.0f, 0.25f, 0.0f };
    // 攻撃したときにどう動くか
    XMFLOAT3 m_animePosition = { 0.0f, 0.0f, 0.5f };
    XMFLOAT3 m_animeRotation = { 0.0f, 0.0f, 0.0f };

    FLOAT m_coolTime = 0.0f;

    float m_chargePower = 0.0f; // チャージ
    bool m_isCharging = false; // チャージしてるか
    const float MAX_CHARGE = 2.0f; // 最大2倍の飛距離
public:
    Spear(GameObject* player, bool select);
    virtual ~Spear();

    void Update() override;
    void Draw() override;
    void Attack() override;

    void Throw(float power, bool select);

    void OnWeaponCollision(GameObject* target) override;


private:
    EffectAnim m_fxAnim;
    int m_playerIndex = 0;      // 持ち主が1P(0)か2P(1)か
    Keyboard_Keys m_chargeKey;  // キーボード用
    ControllerButton::Button m_chargeButton; // コントローラー用
};

class SpearShot : public GameObject
{
public:
    XMFLOAT3 m_velocity{};  // ベクトル
    bool m_isStuck = false; // 刺さっているか
    float m_stuckLife = 2.0f; // 刺さってからの寿命
    std::shared_ptr<BoxCollider> m_collider;
    float m_flyTimer = 3.0f; // 発射してからの寿命

    bool m_selectPlayer = false;
public:
    void Start();
    void Update() override;
    void Draw() override;
    void OnCollision(const CollisionInfo& info)override;
};

#endif // SPEAR_H