//============================================
// HitEffect.cpp
// 
// 被撃時にエフェクトアニメーションを適用
// 作成日:2026/2/16
// 最終更新日:2026/2/16
//============================================
#include "HitEffect.h"
#include "shader.h"
#include "sprite.h"

ID3D11ShaderResourceView* HitEffectManager::m_pTextureRV[(int)EffectType::EFFECT_MAX] = { nullptr };
int HitEffectManager::m_columns[(int)EffectType::EFFECT_MAX] = { 0 };
int HitEffectManager::m_rows[(int)EffectType::EFFECT_MAX] = { 0 };
int HitEffectManager::m_ChargeColumns[(int)ChargeType::HAMMER_C_MAX] = { 0 };
int HitEffectManager::m_ChargeRows[(int)ChargeType::HAMMER_C_MAX] = { 0 };

void HitEffectManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
	m_pDevice = device;
	m_pContext = context;

    m_columns[(int)EffectType::DAGEKI] = 5;
    m_rows[(int)EffectType::DAGEKI] = 5;

    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\HitEffect_sprite02.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)EffectType::DAGEKI]);
        assert(&m_pTextureRV[(int)EffectType::DAGEKI]);
    }

    m_columns[(int)EffectType::ZANGEKI] = 5;
    m_rows[(int)EffectType::ZANGEKI] = 7;

    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\Zangeki_Effect02.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)EffectType::ZANGEKI]);
        assert(&m_pTextureRV[(int)EffectType::ZANGEKI]);

    }

    m_columns[(int)EffectType::TRANSFORM] = 5;
    m_rows[(int)EffectType::TRANSFORM] = 7;

    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\TransformAnim2D.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)EffectType::TRANSFORM]);
        assert(&m_pTextureRV[(int)EffectType::TRANSFORM]);
    }

    //===============================================
    //  ハンマーのチャージエフェクト
    //===============================================
    for (int i = 0; i < (int)ChargeType::HAMMER_C_MAX; i++)
    {
        m_ChargeColumns[i] = 5;
        m_ChargeRows[i] = 5;
    }

    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\charge_white.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)ChargeType::HAMMER_C_W]);
        assert(&m_pTextureRV[(int)ChargeType::HAMMER_C_W]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\charge_blue.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)ChargeType::HAMMER_C_B]);
        assert(&m_pTextureRV[(int)ChargeType::HAMMER_C_B]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\charge_green.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)ChargeType::HAMMER_C_G]);
        assert(&m_pTextureRV[(int)ChargeType::HAMMER_C_G]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\charge_red.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &m_pTextureRV[(int)ChargeType::HAMMER_C_R]);
        assert(&m_pTextureRV[(int)ChargeType::HAMMER_C_R]);
    }

}

void HitEffectManager::Update(float deltaTime)
{
    for (auto it = m_effects.begin(); it != m_effects.end(); ) {
        it->timer += deltaTime;
        if (it->timer >= m_frameDuration) {
            it->timer = 0.0f;
            if (it->type == EffectType::ZANGEKI)
            {
				it->currentFrame += 3.0f;
            }
            else
            {
                it->currentFrame++;
            }
          
        }

        // その種類の最大フレーム数と比較
        int maxFrame = m_columns[(int)it->type] * m_rows[(int)it->type];
        if (it->currentFrame >= maxFrame) {
            it = m_effects.erase(it);
        }
        else {
            ++it;
        }
    }

    for (int i = 0; i < 2; i++)
    {
        if (!m_chargeEffects[i].isActive) continue;

        m_chargeEffects[i].timer += deltaTime;
        if (m_chargeEffects[i].timer >= m_frameDuration) {
            m_chargeEffects[i].timer = 0.0f;
            m_chargeEffects[i].currentFrame++;
            // ループさせる
            int typeIdx = (int)m_chargeEffects[i].type;
            if (m_chargeEffects[i].currentFrame >= m_ChargeColumns[typeIdx] * m_ChargeRows[typeIdx]) {
                m_chargeEffects[i].currentFrame = 0;
            }
        }

    }
}

void HitEffectManager::Finalize()
{
    for (int i = 0; i < 2; i++)
    {
        if (m_pTextureRV[i]) {
            m_pTextureRV[i]->Release();
            m_pTextureRV[i] = nullptr;
        }
    }
}

void HitEffectManager::Draw(const XMMATRIX& viewMat, const XMMATRIX& projectionMat)
{
    if (m_effects.empty()) return;

    //スクリーンサイズの取得
    Shader_Begin();
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    //正投影行列を設定
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0.0f,
        0.0f,
        1.0f));

    //ワールド行列は基本 identity (2D描画のため)
    Shader_SetWorldMatrix(XMMatrixIdentity());


    for (const auto& effect : m_effects) {
        int typeIndex = (int)effect.type;

        //テクスチャとブレンドステートの設定
        m_pContext->PSSetShaderResources(0, 1, &m_pTextureRV[typeIndex]);
        SetBlendState(BLENDSTATE_ALFA);

        // --- 3D座標を2Dスクリーン座標に変換 ---
        XMVECTOR worldPos = XMLoadFloat3(&effect.position);
        XMVECTOR screenPosVec = XMVector3Project(
            worldPos,
            0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1,
            projectionMat, viewMat, XMMatrixIdentity()
        );

        XMFLOAT3 screenPos;
        XMStoreFloat3(&screenPos, screenPosVec);

        // Z値がカメラの後ろにある場合は描画しない
        if (screenPos.z < 0.0f || screenPos.z > 1.0f) continue;

        // --- 2Dスプライトとして描画  ---
        // エフェクトの表示サイズ (ピクセル単位で指定)
        XMFLOAT2 effectSize = XMFLOAT2(1920.0f * 0.2, 1080.0f * 0.2);
        XMFLOAT2 drawPos = XMFLOAT2(screenPos.x, screenPos.y);
        XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        // DrawSpriteEx を使用してアニメーションのコマを指定
        // 引数: 位置, サイズ, 色, 現在のフレーム, 横の分割数, 縦の分割数
        if (typeIndex == (int)EffectType::ZANGEKI)
        {
            DrawSpriteEx(drawPos, XMFLOAT2(effectSize.x*3.0f,effectSize.y*3.0f), color, effect.currentFrame, m_columns[typeIndex], m_rows[typeIndex],45.0f);
        }
        else
        {
            DrawSpriteEx(drawPos, effectSize, color, effect.currentFrame, m_columns[typeIndex], m_rows[typeIndex]);
        }
        if (typeIndex == (int)EffectType::TRANSFORM)
        {
            float transformScale = 2.0f;
            DrawSpriteEx(
                drawPos, XMFLOAT2(effectSize.x * transformScale, effectSize.y * transformScale),
                color, (float)effect.currentFrame, m_columns[typeIndex], m_rows[typeIndex]
            );
        }
    }

    //===================================================
    //  チャージエフェクト描画
    //===================================================
    for (int i = 0; i < 2; i++)
    {
        EffectInstance& effect = m_chargeEffects[i];

        if (!effect.isActive || effect.cType == ChargeType::HAMMER_C_NONE) continue;

        int typeIndex = (int)effect.type;

        //テクスチャとブレンドステートの設定
        // ループ内で各プレイヤーのチャージ段階に応じたテクスチャをセット
        m_pContext->PSSetShaderResources(0, 1, &m_pTextureRV[typeIndex]);
        SetBlendState(BLENDSTATE_ALFA);

        // --- 3D座標を2Dスクリーン座標に変換 ---
        XMVECTOR worldPos = XMLoadFloat3(&effect.position);
        XMVECTOR screenPosVec = XMVector3Project(
            worldPos,
            0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1,
            projectionMat, viewMat, XMMatrixIdentity()
        );

        XMFLOAT3 screenPos;
        XMStoreFloat3(&screenPos, screenPosVec);

        // カメラの後ろにある場合は描画しない
        if (screenPos.z < 0.0f || screenPos.z > 1.0f) continue;

        // --- 2Dスプライトとして描画  ---
        // エフェクトの表示サイズ (ピクセル単位で指定)
        XMFLOAT2 effectSize = XMFLOAT2(1920.0f * 0.2, 1080.0f * 0.2);
        XMFLOAT2 drawPos = XMFLOAT2(screenPos.x, screenPos.y);
        XMFLOAT4 color = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);

        // スプライト描画
        DrawSpriteEx(
            drawPos,
            effectSize,
            color,
            effect.currentFrame,
            m_ChargeColumns[typeIndex],
            m_ChargeRows[typeIndex]
        );
    }


}

void HitEffectManager::HitEffect(XMFLOAT3 pos, EffectType type)
{
    EffectInstance newEffect;
    newEffect.position = pos;

    newEffect.position.y += 0.2f;
    newEffect.currentFrame = 0.0f;
    newEffect.timer = 0.0f;
    newEffect.isActive = true;
    newEffect.type = type;
    m_effects.push_back(newEffect);
}

void HitEffectManager::UpdateChargeEffect(int playerIdx, XMFLOAT3 pos, ChargeType type, bool isActive)
{
    if (playerIdx < 0 || playerIdx >= 2) return;

    m_chargeEffects[playerIdx].position = pos;
    m_chargeEffects[playerIdx].cType = type; 
    m_chargeEffects[playerIdx].isActive = isActive;

    // アニメーション更新
    if (isActive) {
        m_chargeEffects[playerIdx].timer += 1.0f / 60.0f;
        if (m_chargeEffects[playerIdx].timer >= 0.05f) {
            m_chargeEffects[playerIdx].timer = 0;
            m_chargeEffects[playerIdx].currentFrame++;
            int maxFrame = m_ChargeColumns[(int)type] * m_ChargeRows[(int)type];
            if (m_chargeEffects[playerIdx].currentFrame >= maxFrame) {
                m_chargeEffects[playerIdx].currentFrame = 0;
            }
        }
    }
}

