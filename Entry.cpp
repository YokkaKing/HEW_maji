
#include "Entry.h"
#include "keyboard.h"
#include "Controller.h"
#include "Manager.h"
#include "fade.h"
#include "sprite.h"
#include "shader.h"
#include "direct3d.h"
#include <cmath>

extern Controller g_Controller[2];

// カーソルとスロットの状態管理用
struct PlayerCursor {
    XMFLOAT2 pos;
    bool isSelected;
    int targetSlot; // -1: なし, 0: 1P枠, 1: 2P枠
};

static PlayerCursor g_Cursors[2];
static int g_PlayerToController[2] = { -1, -1 };
static ID3D11ShaderResourceView* g_EntryBgTexture = NULL;   // 背景用
static ID3D11ShaderResourceView* g_CardTexture[2] = { NULL, NULL };    // カード（スロット）用
static ID3D11ShaderResourceView* g_CursorTexture = NULL; static ID3D11DeviceContext* g_pContext = nullptr;
static ID3D11ShaderResourceView* g_ReadyTexture = NULL;
// 枠の配置定義
const float SLOT_WIDTH = 450.0f;
const float SLOT_HEIGHT = 650.0f;
static XMFLOAT2 g_SlotPos[2];

void Entry_Initialize(ID3D11Device* device, ID3D11DeviceContext* context) {
    g_pContext = context;
    float sw = (float)Direct3D_GetBackBufferWidth();
    float sh = (float)Direct3D_GetBackBufferHeight();
    g_SlotPos[0] = { sw * 0.28f, sh * 0.5f };
    g_SlotPos[1] = { sw * 0.72f, sh * 0.5f };
    for (int i = 0; i < 2; i++) {
        g_Cursors[i].pos = {sw/2.0f,sh/2.0f };
        g_Cursors[i].isSelected = false;
        g_Cursors[i].targetSlot = -1;
        g_PlayerToController[i] = -1;
        g_Controller[i].SetVibration(0.0f, 0.0f);
    }

    // 背景読み込み
    TexMetadata metadata;
    ScratchImage image;
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\selectBg_1.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &g_EntryBgTexture);
    }

    // 2. カード１の読み込み
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\1p_cursor.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &g_CardTexture[0]);
    }
    //カード２
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\2p_cursor.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &g_CardTexture[1]);
    }

    // 3. カーソルの読み込み (任意)
    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\shuriken.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &g_CursorTexture);
    }

    if (SUCCEEDED(LoadFromWICFile(L"asset\\texture\\Hp.png", WIC_FLAGS_NONE, &metadata, image))) {
        CreateShaderResourceView(device, image.GetImages(), image.GetImageCount(), metadata, &g_ReadyTexture);
    }
}

int GetControllerIndexFromPlayerNo(int playerNo) {
    if (playerNo < 0 || playerNo >= 2) return -1;
    return g_PlayerToController[playerNo];
}

void Entry_Update() {
    if (GetFadeState() != FADE_NONE) return;

    int readyCount = 0;

    int currentReady = 0;
    for (int i = 0; i < 2; i++) {
        if (g_Cursors[i].isSelected) currentReady++;
    }

    if (currentReady >= 2) {
        // Enterキー または コントローラーのYボタンで次へ
        bool nextScene = Keyboard_IsKeyDownTrigger(KK_ENTER);
        for (int i = 0; i < 2; i++) {
            if (g_Controller[i].IsButtonPushed(ControllerButton::Y_BUTTON)) nextScene = true;
        }

        if (nextScene) {
            XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
            SetFade(40.0f, color, FADE_OUT, SCENE_SELECT_WT);
            return;
        }
    }


    for (int i = 0; i < 2; i++) {
        //移動処理（決定していない場合のみ）
        if (!g_Cursors[i].isSelected) {
            float dx = g_Controller[i].GetLeftStickX();
            float dy = g_Controller[i].GetLeftStickY();
            if (i == 0) { // 1P: WASD
                if (Keyboard_IsKeyDown(KK_A)) dx = -1.0f;
                if (Keyboard_IsKeyDown(KK_D)) dx = 1.0f;
                if (Keyboard_IsKeyDown(KK_W)) dy = 1.0f;
                if (Keyboard_IsKeyDown(KK_S)) dy = -1.0f;
            }
            else { // 2P: Arrow Keys
                if (Keyboard_IsKeyDown(KK_LEFT)) dx = -1.0f;
                if (Keyboard_IsKeyDown(KK_RIGHT)) dx = 1.0f;
                if (Keyboard_IsKeyDown(KK_UP)) dy = 1.0f;
                if (Keyboard_IsKeyDown(KK_DOWN)) dy = -1.0f;
            }
            g_Cursors[i].pos.x += dx * 15.0f;
            g_Cursors[i].pos.y -= dy * 15.0f;
        }

        g_Cursors[i].targetSlot = -1;
        for (int s = 0; s < 2; s++) {
            if (abs(g_Cursors[i].pos.x - g_SlotPos[s].x) < SLOT_WIDTH / 2.0f &&
                abs(g_Cursors[i].pos.y - g_SlotPos[s].y) < SLOT_HEIGHT / 2.0f) {
                g_Cursors[i].targetSlot = s;
            }
        }

        //選択
        bool isDecideTriggered = g_Controller[i].IsButtonPushed(ControllerButton::A_BUTTON);
        if (i == 0 && Keyboard_IsKeyDownTrigger(KK_C)) isDecideTriggered = true;
        if (i == 1 && Keyboard_IsKeyDownTrigger(KK_P)) isDecideTriggered = true;

        if (isDecideTriggered) {
            int slot = g_Cursors[i].targetSlot;
            if (!g_Cursors[i].isSelected && slot != -1) {
                if (i == slot) {
                    g_Cursors[i].isSelected = true;
                    g_PlayerToController[slot] = i;
                    g_Controller[i].SetVibration(0.3f, 0.3f);
                }
            }
        }
            // Aボタン離したら振動止める
            if (g_Controller[i].IsButtonReleased(ControllerButton::A_BUTTON)) {
                g_Controller[i].SetVibration(0.0f, 0.0f);
            }

            //Bボタン：キャンセル
            bool isCancelTriggered = g_Controller[i].IsButtonPushed(ControllerButton::B_BUTTON);
            if (i == 0 && Keyboard_IsKeyDownTrigger(KK_V)) isCancelTriggered = true; // 1Pキャンセル例
            if (i == 1 && Keyboard_IsKeyDownTrigger(KK_L)) isCancelTriggered = true; // 2Pキャンセル例

            if (isCancelTriggered) {
                if (g_Cursors[i].isSelected) {
                    if (g_Cursors[i].targetSlot != -1) {
                        g_PlayerToController[g_Cursors[i].targetSlot] = -1;
                    }
                    g_Cursors[i].isSelected = false;
                }
                else {
                    SetScene(SCENE_TITLE);
                    return;
                }
            }
                if (g_Cursors[i].isSelected) readyCount++;
        }

        //Yボタン：全員準備完了で次へ
        if (readyCount >= 2) {
            // 両方のコントローラーの状態を確認
            if (g_Cursors[0].isSelected && g_Cursors[1].isSelected) {
                for (int i = 0; i < 2; i++) {
                    if (g_Controller[i].IsButtonPushed(ControllerButton::Y_BUTTON)) {
                        XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
                        SetFade(40.0f, color, FADE_OUT, SCENE_SELECT_WT);
                        return;
                    }
                }
            }
        }
}

void Entry_Draw() {
    if (!g_EntryBgTexture) return;

    Shader_Begin();
    const float SW = (float)Direct3D_GetBackBufferWidth();
    const float SH = (float)Direct3D_GetBackBufferHeight();
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(0.0f, SW, SH, 0.0f, 0.0f, 1.0f));
    Shader_SetWorldMatrix(XMMatrixIdentity());
    SetBlendState(BLENDSTATE_ALFA);

    // 背景
    g_pContext->PSSetShaderResources(0, 1, &g_EntryBgTexture);
    DrawSprite({ SW / 2, SH / 2 }, { SW, SH }, { 1, 1, 1, 1 });

    // スロットの描画（デバッグ用）
    for (int i = 0; i < 2; i++) {
        // スロットごとにテクスチャを切り替える
        if (g_CardTexture[i]) {
            g_pContext->PSSetShaderResources(0, 1, &g_CardTexture[i]);
        }
        XMFLOAT4 color = { 0.5f, 0.5f, 0.5f, 1.0f };
        for (int p = 0; p < 2; p++) {
            if (g_Cursors[p].targetSlot == i) color = { 1, 1, 1, 1 };
        }
        DrawSprite(g_SlotPos[i], { SLOT_WIDTH, SLOT_HEIGHT }, color);
    }

    // カーソルの描画
    if (g_CursorTexture) {
        g_pContext->PSSetShaderResources(0, 1, &g_CursorTexture);
    }

    for (int i = 0; i < 2; i++) {
        XMFLOAT4 cursorColor = (i == 0) ? XMFLOAT4(1.0f, 0.5f, 0.5f, 1.0f) : XMFLOAT4(0.5, 0.5, 1, 1);
        DrawSprite(g_Cursors[i].pos, { 64.0f, 64.0f }, cursorColor);
    }
    

    int readyCount = 0;
    for (int i = 0; i < 2; i++) {
        if (g_Cursors[i].isSelected) readyCount++;
    }

    if (readyCount >= 2 && g_ReadyTexture) {
        g_pContext->PSSetShaderResources(0, 1, &g_ReadyTexture);

        // 画面中央に表示 (例: 1280x720想定で 640, 360)
        // サイズは画像の大きさに合わせて調整してください
        DrawSprite({ SW / 2, SH / 2 }, { 800.0f, 200.0f }, { 1, 1, 1, 1 });
    }
}

void Entry_Finalize() {
    for (int i = 0; i < 2; i++) g_Controller[i].SetVibration(0.0f, 0.0f);
    SAFE_RELEASE(g_EntryBgTexture);
    SAFE_RELEASE(g_CardTexture[0]);
    SAFE_RELEASE(g_CardTexture[1]);
    SAFE_RELEASE(g_CursorTexture);
    SAFE_RELEASE(g_ReadyTexture);
}