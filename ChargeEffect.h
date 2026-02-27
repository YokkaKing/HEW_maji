#pragma once
//============================================
// ChargeEffect.h
// 
// ハンマーのチャージ中にエフェクトアニメーションを適用
// 作成日:2026/2/27
// 最終更新日:2026/2/27
//============================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;
#include <vector>

enum class ChargeType
{
    HAMMER_C_NONE = 0,
    HAMMER_C_W,
    HAMMER_C_B,
    HAMMER_C_G,
    HAMMER_C_R,

    HAMMER_C_MAX
};

struct ChargeInstance {
    XMFLOAT3 position;
    int currentFrame;
    float timer;
    bool isActive;
    ChargeType type;
};

class ChargeEffectManager {
public:
    static ChargeEffectManager& GetInstance() {
        static ChargeEffectManager instance;
        return instance;
    }

    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);
    void Finalize();
    void Update(float deltaTime);
    void Draw(const XMMATRIX& viewMat, const XMMATRIX& projectionMat);

    // ハンマーから呼ぶ更新関数
    void SetEffect(int playerIdx, XMFLOAT3 pos, XMFLOAT3 offset, ChargeType type, bool isActive);

private:
    ChargeEffectManager() {}

    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;

    // テクスチャと分割数
    ID3D11ShaderResourceView* m_pTextures[(int)ChargeType::HAMMER_C_MAX] = { nullptr };
    int m_columns[(int)ChargeType::HAMMER_C_MAX] = { 0 };
    int m_rows[(int)ChargeType::HAMMER_C_MAX] = { 0 };

    // 1P用と2P用のエフェクトインスタンス
    ChargeInstance m_instances[2] = {};
};