#pragma once
#include <d3d11.h>
#include <DirectXMath.h>
using namespace DirectX;

// 武器の基本機能を提供するインターフェース
class IWeapon
{
public:
    // 純粋仮想関数 (派生クラスで必ず実装が必要)
    virtual ~IWeapon() = default;

    virtual void Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) = 0;
    virtual void Finalize() = 0;

    // 攻撃開始 (プレイヤーの位置と回転を受け取る)
    virtual void StartAttack(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) = 0;

    // 攻撃終了
    virtual void EndAttack() = 0;

    // 毎フレームの更新 (タイマーなど)
    virtual void Update(float deltaTime) = 0;

    // 描画 (プレイヤーの位置と回転を受け取る)
    virtual void Draw(const XMFLOAT3& playerPosition, const XMFLOAT3& playerRotation) = 0;

    // 攻撃が終了すべきかどうか (例: タイマー切れ)
    virtual bool ShouldEndAttack() const = 0;

    // 現在攻撃中かどうか
    virtual bool IsAttacking() const = 0;

    // その他、当たり判定情報などを取得する関数をIWeaponに移動することを検討
};
