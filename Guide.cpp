#include "Guide.h"
#include "Player.h"
#include "Player2.h"
#include "Camera.h"
#include "sprite.h"
#include "shader.h"
#include "direct3d.h"
#include <cmath>

// テクスチャを配列に変更
static ID3D11ShaderResourceView* g_GuideTexture[2] = { nullptr, nullptr };

void Guide::Initialize(ID3D11Device* pDevice) {
    TexMetadata metadata;
    ScratchImage image;

    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\1p_arrow.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_GuideTexture[0]);
    }

    // 2P用の矢印
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\2p_arrow.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_GuideTexture[1]);
    }
}

void Guide::Finalize() {
    if (g_GuideTexture[0]) { g_GuideTexture[0]->Release(); g_GuideTexture[0] = nullptr; }
    if (g_GuideTexture[1]) { g_GuideTexture[1]->Release(); g_GuideTexture[1] = nullptr; }
}

DirectX::XMFLOAT2 Guide::ProjectWorldToScreen(DirectX::XMFLOAT3 worldPos, DirectX::XMMATRIX view, DirectX::XMMATRIX proj) {
    using namespace DirectX;
    XMVECTOR vPos = XMLoadFloat3(&worldPos);
    XMMATRIX world = XMMatrixIdentity();
    float screenW = (float)Direct3D_GetBackBufferWidth();
    float screenH = (float)Direct3D_GetBackBufferHeight();

    XMVECTOR screenPos = XMVector3Project(vPos, 0, 0, screenW, screenH, 0.0f, 1.0f, proj, view, world);
    XMFLOAT3 res;
    XMStoreFloat3(&res, screenPos);

    if (res.z < 0.0f || res.z > 1.0f) {
        res.x = screenW - res.x;
        res.y = screenH - res.y;

        if (res.x > 0 && res.x < screenW) res.x = -100.0f;
    }

    return { res.x, res.y };
}

void Guide::Draw(bool isPlayer1) {
    ID3D11ShaderResourceView* pCurrentTex = isPlayer1 ? g_GuideTexture[1] : g_GuideTexture[0];
    if (!pCurrentTex) return;

    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    //3D計算用準備
    using namespace DirectX;
    XMMATRIX view = isPlayer1 ? GetViewMatrix() : GetViewMatrix2();
    XMMATRIX proj = isPlayer1 ? GetProjectionMatrix() : GetProjectionMatrix2();
    XMFLOAT3 enemyPosF = isPlayer1 ? g_Player2.m_position : g_Player.m_position;
    XMVECTOR vEnemyPos = XMLoadFloat3(&enemyPosF);

    //スクリーン座標を計算
    XMFLOAT2 screenPos = ProjectWorldToScreen(enemyPosF, view, proj);

    //画面外判定
    float margin = 60.0f;
    bool isOffScreen = (screenPos.x < 0 || screenPos.x > SCREEN_WIDTH || screenPos.y < 0 || screenPos.y > SCREEN_HEIGHT);

    if (isOffScreen) {
        XMMATRIX invView = XMMatrixInverse(nullptr, view);
        XMVECTOR camPos = invView.r[3];
        XMVECTOR camRight = invView.r[0];
        XMVECTOR camUp = invView.r[1];

        XMVECTOR dirToEnemy = XMVector3Normalize(vEnemyPos - camPos);

        float dx = XMVectorGetX(XMVector3Dot(dirToEnemy, camRight));
        float dy = XMVectorGetX(XMVector3Dot(dirToEnemy, camUp));

        //角度計算
        float angle = atan2f(-dy, dx);
        angle -= 1.5708f;

        //表示位置
        float centerX = SCREEN_WIDTH / 2.0f;
        float centerY = SCREEN_HEIGHT / 2.0f;
        XMFLOAT2 edgePos;

        float absX = fabsf(dx);
        float absY = fabsf(dy);

        if (absX * (SCREEN_HEIGHT / 2.0f) > absY * (SCREEN_WIDTH / 2.0f)) {
            edgePos.x = (dx > 0) ? SCREEN_WIDTH - margin : margin;
            edgePos.y = centerY + ((-dy) * (centerX - margin) / absX);
        }
        else {
            edgePos.x = centerX + (dx * (centerY - margin) / absY);
            edgePos.y = ((-dy) > 0) ? SCREEN_HEIGHT - margin : margin;
        }

        //描画処理
        Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f));
        Shader_SetWorldMatrix(XMMatrixIdentity());
        SetBlendState(BLENDSTATE_ALFA);
        SetDepthTest(FALSE);

        XMFLOAT2 size = { 80.0f, 80.0f };
        XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };
        ID3D11DeviceContext* pContext = Direct3D_GetDeviceContext();
        pContext->PSSetShaderResources(0, 1, &pCurrentTex);

        DrawSpriteExRotation(edgePos, size, color, 0, 1, 1, angle);
    }
}