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
using namespace DirectX;

class Hammer : public IWeapon
{
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
public:
    Hammer(GameObject* player, bool select);
    virtual ~Hammer();

    void Update() override;
    void Draw() override;
    void Attack() override;

    void OnWeaponCollision(GameObject* target) override;
};

#endif // HAMMER_H