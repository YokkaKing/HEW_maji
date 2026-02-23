#pragma once
//============================================
// HitEffect.h
// 
// 被撃時にエフェクトアニメーションを適用
// 作成日:2026/2/16
// 最終更新日:2026/2/16
//============================================
#include <d3d11.h>
#include <DirectXMath.h>
#include "direct3d.h"
using namespace DirectX;
#include <vector>

enum class EffectType
{
    DAGEKI = 0,
    ZANGEKI,
    TRANSFORM,

    EFFECT_MAX
};

struct EffectInstance {
    XMFLOAT3 position;
    int currentFrame;
    float timer;
    bool isActive;
    EffectType type;
};

class HitEffectManager {
public:
    static HitEffectManager& GetInstance() {
        static HitEffectManager instance;
        return instance;
    }
public:
    // 初期化（テクスチャの読み込み、定数バッファの作成など）
    void Initialize(ID3D11Device* device, ID3D11DeviceContext* context);

    // エフェクトの発生
    void HitEffect(XMFLOAT3 pos, EffectType type);

    // 更新と描画
    void Update(float deltaTime);
    void Draw(const XMMATRIX& viewMat, const XMMATRIX& projectionMat);
    void Finalize();

private:
    //HitEffectManager() : m_columns(1), m_rows(1), m_frameDuration(1.0f / 30.0f) {}
    //~HitEffectManager() { Finalize(); }

    //HitEffectManager(const HitEffectManager&) = delete;
    //void operator=(const HitEffectManager&) = delete;

    ID3D11Device* m_pDevice = nullptr;
    ID3D11DeviceContext* m_pContext = nullptr;
    static ID3D11ShaderResourceView* m_pTextureRV[(int)EffectType::EFFECT_MAX];
    static int m_columns[(int)EffectType::EFFECT_MAX];
    static int m_rows[(int)EffectType::EFFECT_MAX];
    float m_frameDuration;
    std::vector<EffectInstance> m_effects;
};