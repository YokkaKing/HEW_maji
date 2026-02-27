//============================================
// ChargeEffect.cpp
// 
// ハンマーのチャージ中にエフェクトアニメーションを適用
// 作成日:2026/2/27
// 最終更新日:2026/2/27
//============================================
#include "ChargeEffect.h"
#include "shader.h"
#include "sprite.h"

void ChargeEffectManager::Initialize(ID3D11Device* device, ID3D11DeviceContext* context)
{
    m_pDevice = device;
    m_pContext = context;

    // --- 画像の分割数に合わせて調整 ---
    for (int i = 0; i < (int)ChargeType::HAMMER_C_MAX; i++) {
        m_columns[i] = 5;
        m_rows[i] = 5;
    }

    // --- テクスチャの読み込み ---
    auto LoadTex = [&](ChargeType type, const wchar_t* path) {
        TexMetadata metadata;
        ScratchImage image;
        HRESULT hr = LoadFromWICFile(path, WIC_FLAGS_FORCE_SRGB, &metadata, image);
        if (SUCCEEDED(hr)) {
            CreateShaderResourceView(m_pDevice, image.GetImages(), image.GetImageCount(), metadata, &m_pTextures[(int)type]);
        }
        };

    LoadTex(ChargeType::HAMMER_C_W, L"asset\\texture\\charge_white.png");
    LoadTex(ChargeType::HAMMER_C_B, L"asset\\texture\\charge_blue.png");
    LoadTex(ChargeType::HAMMER_C_G, L"asset\\texture\\charge_green.png");
    LoadTex(ChargeType::HAMMER_C_R, L"asset\\texture\\charge_red.png");
}

void ChargeEffectManager::Finalize()
{
    for (int i = 0; i < (int)ChargeType::HAMMER_C_MAX; i++)
    {
        if (m_pTextures[i]) {
            m_pTextures[i]->Release();
            m_pTextures[i] = nullptr;
        }
    }
}

void ChargeEffectManager::SetEffect(int playerIdx, XMFLOAT3 pos, XMFLOAT3 offset, ChargeType type, bool isActive)
{
    if (playerIdx < 0 || playerIdx >= 2) return;

    // タイプが変わったらフレームをリセット
    if (m_instances[playerIdx].type != type) {
        m_instances[playerIdx].currentFrame = 0;
        m_instances[playerIdx].timer = 0.0f;
    }

    // 座標にオフセットを足して位置調整
    m_instances[playerIdx].position.x = pos.x + offset.x;
    m_instances[playerIdx].position.y = pos.y + offset.y;
    m_instances[playerIdx].position.z = pos.z + offset.z;

    m_instances[playerIdx].type = type;
    m_instances[playerIdx].isActive = isActive;
}

void ChargeEffectManager::Update(float deltaTime)
{
    for (int i = 0; i < 2; i++) {
        if (!m_instances[i].isActive || m_instances[i].type == ChargeType::HAMMER_C_NONE) continue;

        ChargeInstance& inst = m_instances[i];
        int typeIdx = (int)inst.type;

        // アニメーション更新 (30FPS)
        inst.timer += deltaTime;
        if (inst.timer >= (1.0f / 30.0f)) {
            inst.timer = 0.0f;
            inst.currentFrame++;
            if (inst.currentFrame >= (m_columns[typeIdx] * m_rows[typeIdx])) {
                inst.currentFrame = 0;
            }
        }
    }
}

void ChargeEffectManager::Draw(const XMMATRIX& viewMat, const XMMATRIX& projectionMat)
{
    // 両方のプレイヤーが非アクティブなら何もしない
    if (!m_instances[0].isActive && !m_instances[1].isActive) return;

    Shader_Begin();
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
    Shader_SetWorldMatrix(XMMatrixIdentity());

    for (int i = 0; i < 2; i++) {
        ChargeInstance& inst = m_instances[i];
        if (!inst.isActive || inst.type == ChargeType::HAMMER_C_NONE) continue;

        int typeIdx = (int)inst.type;
        if (!m_pTextures[typeIdx]) continue;

        // テクスチャとブレンド設定
        m_pContext->PSSetShaderResources(0, 1, &m_pTextures[typeIdx]);
        SetBlendState(BLENDSTATE_ALFA);

        // 3D座標 -> スクリーン座標変換
        XMVECTOR worldPos = XMLoadFloat3(&inst.position);
        XMVECTOR screenPosVec = XMVector3Project(
            worldPos, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0, 1,
            projectionMat, viewMat, XMMatrixIdentity()
        );

        XMFLOAT3 screenPos;
        XMStoreFloat3(&screenPos, screenPosVec);
        if (screenPos.z < 0.0f || screenPos.z > 1.0f) continue;

        // サイズと座標の決定
        XMFLOAT2 effectSize = XMFLOAT2(1920.0f * 0.2f, 1080.0f * 0.2f);
        XMFLOAT2 drawPos = XMFLOAT2(screenPos.x, screenPos.y);

        // 描画実行
        DrawSpriteEx(
            drawPos,
            effectSize,
            XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f),
            (float)inst.currentFrame,
            m_columns[typeIdx],
            m_rows[typeIdx]
        );
    }
}