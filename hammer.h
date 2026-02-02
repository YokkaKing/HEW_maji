/*
* ファイル名	hammer.h
* タイトル	ハンマー
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

#ifndef HAMMER_H
#define HAMMER_H

//================================================================
//	インクルード
//================================================================
#include<DirectXMath.h>
#include"IWeapon.h"
#include"model.h"
#include"managerCollider.h"
#include "keyboard.h"
#include "controller.h"
using namespace DirectX;

class Hammer : public IWeapon
{
private:
    int m_playerIndex = 0;      // 0 = P1, 1 = P2
   // int m_chargeKey = 0;        // P1: KK_C, P2: KK_P
    Keyboard_Keys m_chargeKey;
    ControllerButton::Button m_chargeButton; // コントローラー用 (X_BUTTON)
    XMFLOAT3 m_move = { 0,0,0 };  // g_moveHammer
public:
    std::shared_ptr<Collider> m_collider; // コライダーへの参照を保持

    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.5f;   // 攻撃の有効時間
    
    // プレイヤーから見てどこに位置するか
    XMFLOAT3 m_offset = { 0.0f, 0.0f, 0.5f };
    // 攻撃したときにどう動くか
    XMFLOAT3 m_animePosition = { 0.0f, 0.0f, 0.5f };
    XMFLOAT3 m_animeRotation = { 0.0f, 0.0f, 0.0f };

    FLOAT m_coolTime = 0.0f;

    float m_chargePower = 0.0f; // チャージ
    bool m_isCharging = false; // チャージしてるか
    const float MAX_CHARGE = 5.5f;
    enum CHARGE_STATE {
        CHARGE_NONE = 0,    // idle
        CHARGE_IN,          // initial 370->440 playing
        CHARGE_HOLD,        // stopped/held at frame 440
        CHARGE_MOVE_LOOP,   // 540->660 loop while moving during charge
        CHARGE_ATTACK_PLAY  // 440->539 playing when releasing (=attack)
    };
    CHARGE_STATE m_chargeState = CHARGE_NONE;
    bool m_wasCharging = false;
public:
    Hammer(GameObject* player, bool select);
    virtual ~Hammer();

    void Update() override;
    void Draw() override;
    void Attack() override;

    void OnWeaponCollision(GameObject* target) override;
};

#endif // HAMMER_H