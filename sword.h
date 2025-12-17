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
#include "gameObject.h" // GameObjectを継承する
#include "IWeapon.h"    // IWeaponインターフェースを実装する
#include "model.h"
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

class Sword : public GameObject, public IWeapon
{
public:
    // コンストラクタ
    Sword();

    // 外部のファクトリから生成された当たり判定コンポーネントとプロパティを引き継ぐ
    void SetObject(XMFLOAT3 pos, XMFLOAT3 scl, std::string tag, int lay);

    virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) override;
    virtual void Finalize() override;

    // 攻撃開始時に当たり判定を有効化する (メイン機能)
    virtual void StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;

    // 攻撃終了時に当たり判定を無効化する (メイン機能)
    virtual void EndAttack() override;

    // 武器ロジックの更新 (タイマーなど)
    virtual void Update(float deltaTime) override;

    virtual void Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) override;
    virtual bool ShouldEndAttack() const override;
    virtual bool IsAttacking() const override { return m_isAttacking; }

    void Update();

    // 衝突時の応答処理 (当たり判定が有効な場合のみヒット処理を行う)
    void OnCollision(const CollisionInfo& info) override;

private:
    // IWeapon の状態管理
    bool m_isAttacking = false;
    float m_attackTimer = 0.0f;
    const float m_attackDuration = 0.5f; // 攻撃時間

    XMFLOAT3 m_offset;   // 剣のオフセット (プレイヤー相対)

    MODEL* m_model;
};

#endif // SWORD_H