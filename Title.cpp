/*
* ファイル名	Title.cpp
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Audio.h"
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"Controller.h"
#include"Title.h"
#include"fade.h"
#include"shader.h"
#include "Entry.h"
#include "CameraIntroSequence.h"

#define TITLE_YOKO (5)
#define TITLE_TATE (10)

//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_Texture_Logo = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_Texture_TitleBg[2] = { NULL };	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_Texture_White = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_Texture_Button = NULL;	//テクスチャ１枚を表すオブジェクト
static	ID3D11ShaderResourceView* g_Texture_IconLane = NULL;	//テクスチャ１枚を表すオブジェクト


static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
extern Controller g_Controller[2];

static int g_titleVibTimerP1 = 0;
static int g_titleVibTimerP2 = 0;
static float frame = 0.0f;
static bool bgUsed = false;
static float g_pressBlinkTime = 0.0f;

// 3列分のスクロール位置
static float g_iconLaneX[3];

// 速度（+なら右、-なら左）
static float g_iconLaneSpeed[3] =
{
    -2.0f,  // 1列目：左
     1.5f,  // 2列目：右（逆）
    -2.8f   // 3列目：左
};

// 見た目調整
static const float ICON_LANE_ANGLE_DEG = 6.0f;
static float timer = 0.0f; // タイマー変数
static XMFLOAT4 WhiteCol = { 1.0f,1.0f,1.0f,0.0f };
void Title_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;
    g_titleVibTimerP1 = 0;
    g_titleVibTimerP2 = 0;

	//テクスチャ読み込みなど
	TexMetadata		metadata;
	ScratchImage	image;
	LoadFromWICFile(L"asset\\texture\\title_logo.png", WIC_FLAGS_NONE, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_Logo);
	assert(g_Texture_Logo);//読み込み失敗時にダイアログを表示

    LoadFromWICFile(L"asset\\texture\\titleBg_1.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_TitleBg[0]);
    assert(g_Texture_TitleBg[0]);

    LoadFromWICFile(L"asset\\texture\\titleBg_2.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_TitleBg[1]);

    assert(g_Texture_TitleBg[1]);
    LoadFromWICFile(L"asset\\texture\\SelectBg_1.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_White);
    assert(g_Texture_White);

    LoadFromWICFile(L"asset\\texture\\press_button.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_Button);
    assert(g_Texture_Button);

    LoadFromWICFile(L"asset\\texture\\icon.png", WIC_FLAGS_NONE, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texture_IconLane);
    assert(g_Texture_IconLane);

    SetCameraIntroPlayed(false);
	//フェードインのセット
	//XMFLOAT4	color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	//SetFade(60.0f, color, FADE_IN, SCENE_ENTRY);

    frame = 0.0f;
 
    bgUsed = false;
    WhiteCol = { 1.0f,1.0f,1.0f,0.0f };
    g_iconLaneX[0] = 0.0f;
    g_iconLaneX[1] = -380.0f;   // ずらす（適当に調整）
    g_iconLaneX[2] = -760.0f;   // さらにずらす
    g_pressBlinkTime = 0.0f;
    timer = 0.0f;
}
void Title_Finalize()
{
    g_Controller[0].SetVibration(0.0f, 0.0f);
    g_Controller[1].SetVibration(0.0f, 0.0f);
	//テクスチャの解放など
	SAFE_RELEASE(g_Texture_Logo);
    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_Texture_TitleBg[i]);
    }
   
    SAFE_RELEASE(g_Texture_White);
    SAFE_RELEASE(g_Texture_Button);
    SAFE_RELEASE(g_Texture_IconLane);



}
void Title_Update()
{ 
    //timer++;
    //if (timer >= 60)
    {
        float laneTexW = 1600.0f;
        float laneGap = 80.0f;               // Drawと同じ値
        float laneStep = laneTexW + laneGap; // Drawと同じ値

        for (int i = 0; i < 3; i++)
        {
            g_iconLaneX[i] += g_iconLaneSpeed[i];

            if (g_iconLaneSpeed[i] < 0.0f)
            {
                if (g_iconLaneX[i] <= -laneStep) // ← laneTexW → laneStep
                {
                    g_iconLaneX[i] += laneStep;  // ← laneTexW → laneStep
                }
            }
            else
            {
                if (g_iconLaneX[i] >= laneStep)  // ← laneTexW → laneStep
                {
                    g_iconLaneX[i] -= laneStep;  // ← laneTexW → laneStep
                }
            }
        }
        if (bgUsed)
        {
            g_pressBlinkTime += 0.04f;
        }
        if (frame < (TITLE_TATE * TITLE_YOKO) - 1)
        {
            frame += 0.8f;
        }
        else
        {
            frame = (TITLE_TATE * TITLE_YOKO) - 1;
        }
        if ((int)frame == (TITLE_TATE * TITLE_YOKO) - 40)
        {
            PlayAudio(g_titleSword, false);
        }
        if (frame > (TITLE_TATE * TITLE_YOKO) - 30 && !bgUsed)
        {
            if (WhiteCol.w <= 1.0f)
            {
                WhiteCol.w += 0.1f;
            }
            else
            {
                WhiteCol.w = 1.0f;
            }

        }
        if (WhiteCol.w >= 1.0f && !bgUsed)
        {
            bgUsed = true;
            PlayAudio(g_title, true);
        }
        if (WhiteCol.w >= 0.0f)
        {
            if (bgUsed)
                WhiteCol.w -= 0.1f;
        }
        else
        {
            WhiteCol.w -= 0.0f;
        }



        for (int playerNo = 0; playerNo < 2; playerNo++)
        {
            int ctrlIdx = GetControllerIndexFromPlayerNo(playerNo);
            if (ctrlIdx == -1) continue; // コントローラーが割り当てられていない場合はスキップ

            int& timer = (playerNo == 0) ? g_titleVibTimerP1 : g_titleVibTimerP2;

            if (timer > 0) {
                timer--;
                if (timer <= 0) {
                    g_Controller[ctrlIdx].SetVibration(0.0f, 0.0f);
                }
            }
        }
        //キー入力チェック
        //スタートボタンが押されたらシーンを切り替え
        //フェード処理中はキーを受け付けない
        bool isStartTriggered = Keyboard_IsKeyDownTrigger(KK_ENTER);
        for (int playerNo = 0; playerNo < 2; playerNo++)
        {
            int ctrlIdx = GetControllerIndexFromPlayerNo(playerNo);
            if (ctrlIdx != -1)
            {
                if (g_Controller[playerNo].IsButtonPushed(ControllerButton::A_BUTTON)) {
                    isStartTriggered = true;
                }
            }
            else
            {
                if (g_Controller[playerNo].IsButtonPushed(ControllerButton::A_BUTTON)) {
                    isStartTriggered = true;
                }
            }

        }
        if (isStartTriggered && (GetFadeState() == FADE_NONE))
        {
            for (int playerNo = 0; playerNo < 2; playerNo++)
            {
                int ctrlIdx = GetControllerIndexFromPlayerNo(playerNo);
                if (ctrlIdx != -1)
                {
                    g_Controller[ctrlIdx].SetVibration(0.7f, 0.7f);
                }
                else
                {
                    // エントリー前なら 0, 1 両方に送る
                    g_Controller[playerNo].SetVibration(0.7f, 0.7f);
                }
            }
            g_titleVibTimerP1 = 10;
            g_titleVibTimerP2 = 10;
            PlayAudio(g_fade, false);
            //フェードアウトさせてシーンを切り替える
            XMFLOAT4	color(0.0f, 0.0f, 0.0f, 1.0f);
            SetFade(40.0f, color, FADE_OUT, SCENE_ENTRY);
        }

    }

}
void Title_Draw()
{
    // シェーダーを描画パイプラインに設定
    Shader_Begin();

    // 画面サイズ取得
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

    // 頂点シェーダーに正射影行列を設定
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0.0f,
        0.0f,
        1.0f));




        // World 行列は単位行列を設定
        Shader_SetWorldMatrix(XMMatrixIdentity());

        g_pContext->PSSetShaderResources(0, 1, &g_Texture_White);
        SetBlendState(BLENDSTATE_ALFA);
        // 色と位置・サイズを設定
        XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, 1.0f };
        XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
        XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT + 200 };
        // 描画
        DrawSprite(pos, size, col);

        //if (timer >= 60)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_Texture_TitleBg[0]);
            // BlendState 設定
            SetBlendState(BLENDSTATE_ALFA);
            // 色と位置・サイズを設定
            col = { 1.0f, 1.0f, 1.0f, 1.0f };
            pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 90 };
            size = { SCREEN_WIDTH, SCREEN_HEIGHT + 180 };
            // 描画
            if (bgUsed)
            {
                DrawSprite(pos, size, col);
            }

            float deg = -6.0f;
            float rad = deg * 3.14159265f / 180.0f;

            // 画像の描画サイズ（実際に見せたいサイズ）
            XMFLOAT2 laneSize = { 1600.0f, 170.0f };
            float laneTexW = laneSize.x;

            // ★追加：間を置く
            float laneGap = 80.0f;               // ← 好きな間隔に調整
            float laneStep = laneTexW + laneGap; // ← これを並べ間隔に使う

            // 各列のY位置（3列）
            float laneY[3] =
            {
                120.0f,
                400.0f,
                680.0f
            };

            // 色（少し透明でも綺麗）
            XMFLOAT4 laneCol = { 1,1,1,0.9f };

            // テクスチャセット
            g_pContext->PSSetShaderResources(0, 1, &g_Texture_IconLane);
            SetBlendState(BLENDSTATE_ALFA);

            for (int row = 0; row < 3; row++)
            {
                for (int k = -1; k <= 2; k++)
                {
                    XMFLOAT2 pos;
                    pos.x = g_iconLaneX[row] + k * laneStep; // ← laneTexW じゃなく laneStep
                    pos.y = laneY[row];

                    DrawSpriteEx(pos, laneSize, laneCol, 1, 1, 1, rad);
                }
            }

            g_pContext->PSSetShaderResources(0, 1, &g_Texture_TitleBg[1]);
            // BlendState 設定
            SetBlendState(BLENDSTATE_ALFA);
            // 色と位置・サイズを設定
            col = { 1.0f, 1.0f, 1.0f, 1.0f };
            pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            size = { SCREEN_WIDTH, SCREEN_HEIGHT };
            // 描画
            if (bgUsed)
            {
                DrawSprite(pos, size, col);
            }
            // テクスチャをセット
            g_pContext->PSSetShaderResources(0, 1, &g_Texture_Logo);
            SetBlendState(BLENDSTATE_ALFA);
            // 色と位置・サイズを設定
            col = { 1.0f, 1.0f, 1.0f, 1.0f };
            pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            size = { SCREEN_WIDTH, SCREEN_HEIGHT };
            // 描画
            DrawSpriteEx(pos, size, col, (int)frame, TITLE_YOKO, TITLE_TATE);


            g_pContext->PSSetShaderResources(0, 1, &g_Texture_White);
            SetBlendState(BLENDSTATE_ALFA);
            // 色と位置・サイズを設定
            col = WhiteCol;
            pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 };
            size = { SCREEN_WIDTH, SCREEN_HEIGHT };
            // 描画
            DrawSprite(pos, size, WhiteCol);

            g_pContext->PSSetShaderResources(0, 1, &g_Texture_Button);
            SetBlendState(BLENDSTATE_ALFA);
            // 色と位置・サイズを設定
           // cosで0.0～1.0を作る
            float blink = (cosf(g_pressBlinkTime) + 1.0f) * 0.5f;

            // 完全に消えると見づらいので、0.35～1.0くらいで点滅させる
            float alpha = 0.1f + blink * 0.9f;

            col = { 1.0f, 1.0f, 1.0f, alpha };
            pos = { SCREEN_WIDTH / 2 + 300, SCREEN_HEIGHT / 2 + 300 };
            size = { 2223 * 0.35f, 273 * 0.35f };
            if (bgUsed)
            {
                DrawSpriteEx(pos, size, col, 1, 1, 1, -0.25f);
            }
        }
    
}

