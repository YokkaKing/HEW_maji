#include "PlayerUI.h"
#include "Player.h"
#include "Player2.h"
#include "Camera.h"
#include "sprite.h"
#include "shader.h"
#include "direct3d.h"

static ID3D11DeviceContext* g_pContext;
static ID3D11ShaderResourceView* g_TagTexture[2] = { NULL, NULL };

void PlayerUI::Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext) {
    g_pContext = pContext;
    TexMetadata metadata;
    ScratchImage image;

    // １pタグの読み込み
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\1p.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TagTexture[0]);
    }
    //２pタグ
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\2p.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TagTexture[1]);
    }
}

void PlayerUI::Finalize() {
    // 終了処理
    if (g_TagTexture[0]) g_TagTexture[0]->Release();
    if (g_TagTexture[1]) g_TagTexture[1]->Release();
    g_TagTexture[0] = nullptr;
    g_TagTexture[1] = nullptr;
}

void PlayerUI::Update() {
    // アニメーション（上下にふわふわさせる等）が必要ならここに記述
}

DirectX::XMFLOAT2 PlayerUI::ProjectWorldToScreen(DirectX::XMFLOAT3 worldPos, DirectX::XMMATRIX view, DirectX::XMMATRIX proj) {
    using namespace DirectX;

    XMVECTOR vPos = XMLoadFloat3(&worldPos);
    XMMATRIX world = XMMatrixIdentity();

    // 画面サイズ（Game.cppのビューポート設定に合わせるのが理想）
    float screenW = 1920.0f;
    float screenH = 1080.0f;

    // ここで引数の view, proj を使うことで、分割画面に対応できる
    XMVECTOR screenPos = XMVector3Project(vPos, 0, 0, screenW, screenH, 0.0f, 1.0f, proj, view, world);

    XMFLOAT3 res;
    XMStoreFloat3(&res, screenPos);
    return { res.x, res.y };
}

void PlayerUI::Draw(bool isPlayer1) {
    Shader_Begin(); // シェーダーの使用開始を宣言

    // 画面サイズを取得
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    XMMATRIX ortho = XMMatrixOrthographicOffCenterLH(
        0.0f, SCREEN_WIDTH, SCREEN_HEIGHT, 0.0f, 0.0f, 1.0f);
    Shader_SetMatrix(ortho);
    Shader_SetWorldMatrix(XMMatrixIdentity());

    SetBlendState(BLENDSTATE_ALFA);
    SetDepthTest(FALSE);

    //座標計算
    XMMATRIX view = isPlayer1 ? GetViewMatrix() : GetViewMatrix2();
    XMMATRIX proj = isPlayer1 ? GetProjectionMatrix() : GetProjectionMatrix2();

    XMFLOAT2 size = { 100.0f, 100.0f };
    XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

    //プレイヤー1のタグ
    if (g_TagTexture[0]) {
        g_pContext->PSSetShaderResources(0, 1, &g_TagTexture[0]);

        XMFLOAT3 pos1 = g_Player.m_position;
        pos1.y += 0.5f;

        // ビューポートに合わせたスクリーン座標を計算
        XMFLOAT2 screenPos1 = ProjectWorldToScreen(pos1, view, proj);

        // 分割画面時の表示位置補正 (もしズレる場合)
        // 1Pは左半分、2Pは右半分に表示されるようにDrawSpriteExを呼ぶ
       // DrawSpriteEx(screenPos1, size, color, 0, 1, 1);
    }

    //プレイヤー2のタグ
    if (g_TagTexture[1]) {
        g_pContext->PSSetShaderResources(0, 1, &g_TagTexture[1]);

        XMFLOAT3 pos2 = g_Player2.m_position;
        pos2.y += 0.5f;

        XMFLOAT2 screenPos2 = ProjectWorldToScreen(pos2, view, proj);
        //DrawSpriteEx(screenPos2, size, color, 0, 1, 1);
    }


}