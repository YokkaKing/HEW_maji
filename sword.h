/*
* ファイル名	sword.h
* タイトル	    剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
//*/

#ifndef SWORD_H
#define SWORD_H

//================================================================
//  インクルード
//================================================================
#include"gameObject.h" // GameObjectを継承する
#include"IWeapon.h"
#include"collider.h"
#include"model.h"
#include<d3d11.h>
#include<DirectXMath.h>
using namespace DirectX;

class Sword : public IWeapon
{
public:
    std::shared_ptr<Collider> m_collider; // コライダーへの参照を保持

    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.5f;   // 攻撃の有効時間

    // プレイヤーから見てどこに位置するか
    XMFLOAT3 m_offset = { 0.5f, 0.0f, 0.5f };

public:
    Sword(GameObject* player, bool select);
    virtual ~Sword();

    void Update() override;
    void Draw() override;
    void Attack() override;

    void OnWeaponCollision(GameObject* target) override;
};

#endif // SWORD_H