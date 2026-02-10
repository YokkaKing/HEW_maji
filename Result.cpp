/*
* ファイル名    Result.cpp
* タイトル      Result（リザルト画面）
*
* 仕様:
*  - ステージ + 初期武器モデル + 変身先モデル2つ を同時に描画
*  - UI/地形/アイテム/当たり判定/プレイヤー処理などは一切なし
*/

#include "Result.h"

#include "direct3d.h" // LIGHTOBJECT
#include "keyboard.h"
#include "fade.h"
#include "Camera.h"
#include "shader.h"

#include "Stage.h"
#include "Winner.h"

static LIGHTOBJECT s_Light;
static STAGE s_stage;

void Result_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    Camera_Initialize();

    s_stage.Initialize(pDevice, pContext);
    WinnerInitialize(pDevice, pContext);

    // ライト（暗すぎ防止）
    XMFLOAT4 para;
    para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    s_Light.SetAmbient(para);
    para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    s_Light.SetDiffuse(para);

    para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);
    float len = sqrtf(para.x * para.x + para.y * para.y + para.z * para.z);
    if (len > 0.0f)
    {
        para.x /= len;
        para.y /= len;
        para.z /= len;
    }
    s_Light.SetDirection(para);
}

void Result_Finalize()
{
    WinnerFinalize();
    s_stage.Finalize();
    Camera_Finalize();
}

void Result_Update()
{
    WinnerUpdate();

    if ((Keyboard_IsKeyDownTrigger(KK_ENTER)) && (GetFadeState() == FADE_NONE))
    {
        XMFLOAT4 color(0.0f, 0.0f, 0.0f, 1.0f);
        SetFade(40.0f, color, FADE_OUT, SCENE_TITLE);
    }

    Camera_Update();
}

void Result_Draw()
{
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);

    Camera_Draw();
    Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());

    s_stage.Draw();
    WinnerDraw();

    s_Light.SetEnable(FALSE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(FALSE);
}
