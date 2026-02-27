/*
* ファイル名	shuriken.h
* タイトル	手裏剣
* 作成者		三橋拓斗
* 作成日		12月09日
* 更新日		12月09日
*/

#ifndef SHURIKEN_H
#define SHURIKEN_H

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

class Shuriken : public IWeapon
{
public:
    std::shared_ptr<Collider> m_collider; // コライダーへの参照を保持

    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    const float ATTACK_DURATION = 0.5f;   // 攻撃の有効時間

    // プレイヤーから見てどこに位置するか
    XMFLOAT3 m_offset = { 0.0f, -0.4f, 0.0f };
    // 攻撃したときにどう動くか
    XMFLOAT3 m_animePosition = { 0.0f, 0.0f, 0.5f };
    XMFLOAT3 m_animeRotation = { 0.0f, 0.0f, 0.0f };

   // FLOAT m_coolTime = 0.0f;
public:
    Shuriken(GameObject* player, bool select);
    virtual ~Shuriken();

    void Update() override;
    void Draw() override;
    void Attack() override;
    void ResetEffect(int select) override;
    void Throw(bool select);

    void OnWeaponCollision(GameObject* target) override;
private:
    int m_playerIndex = 0;      // 0 = 1P, 1 = 2P
    Keyboard_Keys m_reloadKey;  // リロードキー
    ControllerButton::Button m_reloadButton; // リロードボタン
};

class ShurikenShot : public GameObject
{
public:
    XMFLOAT3 m_velocity{};  // ベクトル
    bool m_isStuck = false; // 刺さっているか
    float m_stuckLife = 2.0f; // 刺さってからの寿命
    std::shared_ptr<BoxCollider> m_collider;
    float m_flyTimer = 3.0f; // 発射してからの寿命

    int m_bounceCount = 0; // 反射した回数を記録
    float m_bounceCooldown = 0.0f; // 反射してからのクールタイム

    bool m_selectPlayer = false;
public:
    void Start();
    void Update() override;
    void Draw() override;
    void OnCollision(const CollisionInfo& info)override;
};



#endif // SHURIKEN_H