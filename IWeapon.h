/*
* ファイル名 IWeapon.h
* タイトル   私武器
* 作成者     ☆†◇糸井◇†☆
* 作成日     未定
* 更新日     未定
*/

#ifndef IWEAPON_H
#define IWEAPON_H

//================================================================
//	インクルード
//================================================================
#include<d3d11.h>
#include<DirectXMath.h>
#include"gameObject.h"
#include <memory>
#include <unordered_set>
using namespace DirectX;

enum class WEAPON_TYPE
{
    NONE = 0,
    ARROW,      // 弓矢 (チャージが必要)
    SWORD,      // 剣 (瞬時攻撃)
    SHURIKEN,   // 手裏剣 (投擲)
    SPEAR       // 槍 (突き)
};

// 武器の基本機能を提供するインターフェース
class IWeapon
{
public:
    // 武器を所有するGameObject(プレイヤー)
    GameObject* owner = nullptr;
    // 武器自身のゲームオブジェクト
    std::unique_ptr<GameObject> m_weapon;

    // 既にヒットしたGameObjectを記録するセット(多重ヒット防止)
    std::unordered_set<GameObject*> m_hitTargets;
    FLOAT m_coolTime;
    float m_damage = 10.0f; // 武器の基本ダメージ量
    bool m_selectPlayer = false; // 1Pか2Pか false=1P true=2P
    float m_damageFCount = 0.0f; // ダメージの経過時間
    XMFLOAT2 m_damageFrame = { 0.0f, 0.0f }; // 何フレームから何フレームが攻撃か

public:
    IWeapon(GameObject* o) : owner(o) {}
    virtual ~IWeapon() = default;

    virtual void Update() = 0;
    virtual void Draw() = 0;

    // 攻撃開始時にコライダーを有効化
    virtual void Attack() = 0;

    // 衝突応答関数
    virtual void OnWeaponCollision(GameObject* target) = 0;
    void SetCoolTime(FLOAT coolTime)
    {
        m_coolTime = coolTime;
    }
    FLOAT GetCoolTime()
    {
        return m_coolTime;
    }
};


#endif // IWEAPON_H