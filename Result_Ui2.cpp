/*
* ファイル名	Result_Ui.cpp
* タイトル	タイトル
* 作成者		カンジェウォン
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"Result_Ui2.h"
#include"fade.h"
#include"shader.h"
#include "player.h"
#include "player2.h"
#include "Transform.h"
#include "Audio.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <utility> 
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureResult_Ui_2P[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureChallenge_Ui_2P[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureScore_Ui_2P[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureScore_Bg = NULL;
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;

static	ID3D11ShaderResourceView* g_TextureWin = NULL;
static	ID3D11ShaderResourceView* g_TextureLose = NULL;
static	ID3D11ShaderResourceView* g_Texturecheck_2P = NULL;
static	ID3D11ShaderResourceView* g_TextureChallenge_Bg = NULL;
static	ID3D11ShaderResourceView* g_TextureChallenge[3] = { NULL };

#define UI_YOKO (6)
#define UI_TATE (5)
#define UI_MAX (18)



static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
RESULT_UI_2P g_Result_Ui2;
CHALLENGE_UI_2P g_Ch_Ui2;
SCORE_UI_2P g_Score_Ui2;
static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);
static float result_frame[5] = { 0.0f ,0.0f ,0.0f ,0.0f ,0.0f };
static float challenge_frame[8] = { 0.0f ,0.0f ,0.0f ,0.0f ,0.0f,0.0f ,0.0f ,0.0f };
static float score_frame[5] = { 0.0f ,0.0f ,0.0f ,0.0f ,0.0f };

static float result_timer;
static float challenge_timer;
static float score_timer;
static float timer = 1.0f / 60.0f;




void Result2_Ui_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    std::random_device rd;
    g_Rng.seed(rd());
    //フェードインのセット
    g_Result_Ui2.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    for (int i = 0; i < 2; i++)
    {
        g_Result_Ui2.pos[i] = { 0,0 };
        g_Result_Ui2.size[i] = { 0,0 };
    }

    for (int i = 0; i < 8; i++)
    {
        g_Ch_Ui2.pos[i] = { 0,0 };
        g_Ch_Ui2.size[i] = { 0,0 };
        g_Ch_Ui2.col[i] = { 1.0f, 1.0f, 1.0f, 0.0f };
    }


    for (int i = 0; i < 7; i++)
    {
        g_Score_Ui2.pos[i] = { 0,0 };
        g_Score_Ui2.size[i] = { 0,0 };
        g_Score_Ui2.col[i] = { 1.0f, 1.0f, 1.0f, 0.0f };
    }
    g_Score_Ui2.score = 0;
    result_timer = 0.0f;
    challenge_timer = 0.0f;
    score_timer = 0.0f;

    g_Result_Ui2.ui_used = false;
    g_Result_Ui2.isEnd = false;
    g_Ch_Ui2.ui_used = false;
    g_Ch_Ui2.isEnd = false;
    g_Score_Ui2.ui_used = false;
    g_Score_Ui2.isEnd = false;

    for (int i = 0; i < 5; i++)
    {
        result_frame[i] = 0.0f;
        score_frame[i] = 0.0f;

    }
    for (int i = 0; i < 8; i++)
    {
        challenge_frame[i] = 0.0f;
    }

    TexMetadata		metadata;
    ScratchImage	image;
#pragma region Result_Ui

    LoadFromWICFile(L"asset\\texture\\Result_Player2_FadeIn.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureResult_Ui_2P[0]);
    assert(&g_TextureResult_Ui_2P[0]);

    LoadFromWICFile(L"asset\\texture\\Result_Player2_FadeOut.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureResult_Ui_2P[1]);
    assert(&g_TextureResult_Ui_2P[1]);

    LoadFromWICFile(L"asset\\texture\\Result_Win.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureWin);
    assert(&g_TextureWin);

    LoadFromWICFile(L"asset\\texture\\Result_Lose.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureLose);
    assert(&g_TextureLose);
#pragma endregion

#pragma region Challenge_Ui
    LoadFromWICFile(L"asset\\texture\\Challenge_Player2_FadeIn.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge_Ui_2P[0]);
    assert(&g_TextureChallenge_Ui_2P[0]);

    LoadFromWICFile(L"asset\\texture\\Challenge_Player2_FadeOut.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge_Ui_2P[1]);
    assert(&g_TextureChallenge_Ui_2P[1]);

    LoadFromWICFile(L"asset\\texture\\Challenge_Bg.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge_Bg);
    assert(&g_TextureChallenge_Bg);

    LoadFromWICFile(L"asset\\texture\\Challenge_1.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge[0]);
    assert(&g_TextureChallenge[0]);

    LoadFromWICFile(L"asset\\texture\\Challenge_2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge[1]);
    assert(&g_TextureChallenge[1]);

    LoadFromWICFile(L"asset\\texture\\Challenge_3.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureChallenge[2]);
    assert(&g_TextureChallenge[2]);

    LoadFromWICFile(L"asset\\texture\\Challenge_Check_Player2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_Texturecheck_2P);
    assert(&g_Texturecheck_2P);
#pragma endregion

#pragma region Score_Ui

    LoadFromWICFile(L"asset\\texture\\Score_Player2_FadeIn.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureScore_Ui_2P[0]);
    assert(&g_TextureScore_Ui_2P[0]);

    LoadFromWICFile(L"asset\\texture\\Score_Player2_FadeOut.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureScore_Ui_2P[1]);
    assert(&g_TextureScore_Ui_2P[1]);
    LoadFromWICFile(L"asset\\texture\\Score_Bg.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureScore_Bg);
    assert(&g_TextureScore_Bg);
    LoadFromWICFile(L"asset\\texture\\number.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureNumber);
    assert(&g_TextureNumber);
#pragma endregion
}
void Result2_Ui_Finalize()
{
    for (int i = 0; i < 2; i++)
    {
       
        SAFE_RELEASE(g_TextureResult_Ui_2P[i]);
      
        SAFE_RELEASE(g_TextureChallenge_Ui_2P[i]);
       
        SAFE_RELEASE(g_TextureScore_Ui_2P[i]);
    }

    SAFE_RELEASE(g_TextureWin);
    SAFE_RELEASE(g_TextureLose);
   
    SAFE_RELEASE(g_Texturecheck_2P);
    SAFE_RELEASE(g_TextureChallenge_Bg);
    SAFE_RELEASE(g_TextureScore_Bg);

    SAFE_RELEASE(g_Texturecheck_2P);
    for (int i = 0; i < 3; i++)
    {
        SAFE_RELEASE(g_TextureChallenge[i]);
    }

}
void Result2_Ui_Update()
{
    if (g_Result_Ui2.ui_used)
    {
        g_Result_Ui2.size[0] = { 2000,530 };
        g_Result_Ui2.pos[0] = { 0,0 };
        g_Result_Ui2.size[1] = { 600,300 };
        g_Result_Ui2.pos[1] = { 0,0 };
        if (result_frame[0] < UI_MAX)
        {
            result_frame[0] += 1.0f;
        }
        else
        {
            result_frame[0] = UI_MAX;
            result_timer = result_timer + timer;
        }
        if (result_frame[0] >= UI_MAX && result_timer >= 1.0f)
        {
            if (result_frame[1] < 28.0f)
            {
                result_frame[1] += 0.5f;
            }
            else
            {
                result_frame[1] = 28.0f;
            }
        }
    }
    if (g_Result_Ui2.isEnd)
    {
        if (result_frame[2] < UI_MAX)
        {
            result_frame[2] += 1.0f;
        }
        else
        {
            g_Result_Ui2.isEnd = false;
            result_timer = 0;
        }
    }
    if (g_Ch_Ui2.ui_used)
    {
        g_Ch_Ui2.size[0] = { 2000,530 };
        g_Ch_Ui2.col[0] = { 1.0f,1.0f,1.0f,1.0f };
        if (challenge_frame[0] < UI_MAX)
        {
            challenge_frame[0] += 1.0f;
        }
        else
        {
            challenge_frame[0] = UI_MAX;

        }
        if (challenge_frame[0] >= UI_MAX)
        {
            g_Ch_Ui2.size[1] = { 1000 * 0.8,416 * 0.8 };
            if (g_Ch_Ui2.col[1].w < 1.0f)
            {
                g_Ch_Ui2.col[1].w += 0.05f;
            }
            else
            {
                g_Ch_Ui2.col[1].w = 1.0f;
            }
            if (g_Ch_Ui2.col[1].w >= 1.0f)
            {
                g_Ch_Ui2.size[2] = { 630 * 0.7,89 * 0.7 };
                g_Ch_Ui2.col[2].w += 0.05f;
                if (g_Ch_Ui2.col[2].w >= 1.0f)
                {
                    g_Ch_Ui2.size[3] = { 630 * 0.7,89 * 0.7 };
                    g_Ch_Ui2.col[3].w += 0.05f;
                    if (g_Ch_Ui2.col[3].w >= 1.0f)
                    {
                        g_Ch_Ui2.size[4] = { 630 * 0.7,89 * 0.7 };
                        g_Ch_Ui2.col[4].w += 0.05f;
                        if (g_Ch_Ui2.col[4].w >= 1.0f)
                        {
                            challenge_timer = challenge_timer + timer;

                            if (Player2_GetItemCount() >= 1 && challenge_timer >= 1.0f)
                            {

                                g_Ch_Ui2.size[5] = { 86 * 0.7,86 * 0.7 };
                                g_Ch_Ui2.col[5] = { 1.0f,1.0f,1.0f,1.0f };
                                if (challenge_frame[5] < 10.0f)
                                {
                                    challenge_frame[5] += 1.0f;
                                }
                                else
                                {
                                    challenge_frame[5] = 10.0f;
                                }
                            }
                            if (Player2_GetTransformCount() >= 3 && challenge_timer >= 1.3f)
                            {
                                g_Ch_Ui2.size[6] = { 86 * 0.7,86 * 0.7 };
                                g_Ch_Ui2.col[6] = { 1.0f,1.0f,1.0f,1.0f };
                                if (challenge_frame[6] < 10.0f)
                                {
                                    challenge_frame[6] += 1.0f;
                                }
                                else
                                {
                                    challenge_frame[6] = 10.0f;
                                }
                            }
                            if (Player2_GetLoseCount() <= 0 && challenge_timer >= 1.6f)
                            {
                                g_Ch_Ui2.size[7] = { 86 * 0.7,86 * 0.7 };
                                g_Ch_Ui2.col[7] = { 1.0f,1.0f,1.0f,1.0f };
                                if (challenge_frame[7] < 10.0f)
                                {
                                    challenge_frame[7] += 1.0f;
                                }
                                else
                                {
                                    challenge_frame[7] = 10.0f;
                                }

                            }



                        }
                    }
                }

            }



        }
    }
    if (g_Ch_Ui2.isEnd)
    {
        if (challenge_frame[1] < UI_MAX)
        {
            challenge_frame[1] += 1.0f;
        }
        else
        {
            g_Ch_Ui2.isEnd = false;
            challenge_timer = 0;
        }
    }
    if (g_Score_Ui2.ui_used)
    {
        g_Score_Ui2.size[0] = { 2000,530 };
        g_Score_Ui2.col[0] = { 1.0f,1.0f,1.0f,1.0f };
        if (score_frame[0] < UI_MAX)
        {
            score_frame[0] += 1.0f;
        }
        else
        {
            score_frame[0] = UI_MAX;
        }
        if (score_frame[0] >= UI_MAX)
        {
            g_Score_Ui2.size[1] = { 558,260 };
            if (g_Score_Ui2.col[1].w < 1.0f)
            {
                g_Score_Ui2.col[1].w += 0.05f;
            }
            else
            {
                g_Score_Ui2.col[1].w = 1.0f;
            }
            if (g_Score_Ui2.col[1].w >= 1.0f)
            {
                if (g_Score_Ui2.score < Player2_GetScore())
                {
                    if (Player2_GetScore() < 200)
                    {
                        g_Score_Ui2.score += 2;
                    }
                    else if (Player2_GetScore() < 500)
                    {
                        g_Score_Ui2.score += 5;
                    }
                    else if (Player2_GetScore() < 1000)
                    {
                        g_Score_Ui2.score += 10;
                    }
                    else
                    {
                        g_Score_Ui2.score += 20;
                    }
                }
                else
                {
                    g_Score_Ui2.score = Player2_GetScore();
                }
            }
        }
    }

}

void Result2_Ui_Draw()
{

    Shader_Begin();
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0.0f,
        0.0f,
        1.0f));
    Shader_SetWorldMatrix(XMMatrixIdentity());

    if (g_Result_Ui2.ui_used)
    {
        g_Result_Ui2.pos[0] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureResult_Ui_2P[0]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Result_Ui2.pos[0], g_Result_Ui2.size[0], g_Result_Ui2.col, result_frame[0], UI_YOKO, UI_TATE, -3.0f);
        //
        if (g_Result_Ui2.win_used)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_TextureWin);
            SetBlendState(BLENDSTATE_ALFA);
            g_Result_Ui2.pos[1] = { SCREEN_WIDTH / 2 - 200.0f ,SCREEN_HEIGHT / 2 + 70.0f };
            DrawSpriteEx(g_Result_Ui2.pos[1], g_Result_Ui2.size[1], g_Result_Ui2.col, result_frame[1], 5, 6, -3.0f);
        }
        if (g_Result_Ui2.lose_used)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_TextureLose);
            SetBlendState(BLENDSTATE_ALFA);
            g_Result_Ui2.pos[1] = { SCREEN_WIDTH / 2 - 200.0f ,SCREEN_HEIGHT / 2 + 70.0f };
            DrawSpriteEx(g_Result_Ui2.pos[1], g_Result_Ui2.size[1], g_Result_Ui2.col, result_frame[1], 5, 6, -3.0f);
        }
    }
    if (g_Result_Ui2.isEnd)
    {
        g_Result_Ui2.pos[1] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureResult_Ui_2P[1]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Result_Ui2.pos[1], g_Result_Ui2.size[0], g_Result_Ui2.col, result_frame[2], UI_YOKO, UI_TATE, -3.0f);
        g_Ch_Ui2.pos[1] = { SCREEN_WIDTH / 2 - 350.0f ,SCREEN_HEIGHT / 2 + 100.0f };
        g_Ch_Ui2.pos[2] = { SCREEN_WIDTH / 2 - 350.0f ,SCREEN_HEIGHT / 2 + 30.0f };
        g_Ch_Ui2.pos[3] = { SCREEN_WIDTH / 2 - 350.0f ,SCREEN_HEIGHT / 2 + 100.0f };
        g_Ch_Ui2.pos[4] = { SCREEN_WIDTH / 2 - 350.0f ,SCREEN_HEIGHT / 2 + 170.0f };

    }
    if (g_Ch_Ui2.ui_used)
    {

        g_Ch_Ui2.pos[0] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge_Ui_2P[0]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[0], g_Ch_Ui2.size[0], g_Ch_Ui2.col[0], challenge_frame[0], UI_YOKO, UI_TATE, -3.0f);

        for (int i = 0; i < 5; i++)
        {
            if (g_Ch_Ui2.col[i].w > 0.0f && g_Ch_Ui2.pos[i].x < SCREEN_WIDTH / 2 - 300.0f)
            {
                g_Ch_Ui2.pos[i].x += 3.0f;

            }
        }

        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge_Bg);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[1], g_Ch_Ui2.size[1], g_Ch_Ui2.col[1], 1, 1, 1, -3.0f);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge[0]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[2], g_Ch_Ui2.size[2], g_Ch_Ui2.col[2], 1, 1, 1, -3.0f);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge[1]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[3], g_Ch_Ui2.size[3], g_Ch_Ui2.col[3], 1, 1, 1, -3.0f);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge[2]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[4], g_Ch_Ui2.size[4], g_Ch_Ui2.col[4], 1, 1, 1, -3.0f);

        for (int i = 5; i < 8; i++)
        {
            g_Ch_Ui2.pos[5] = { SCREEN_WIDTH / 2 - 490.0f ,SCREEN_HEIGHT / 2 + 30.0f };
            g_Ch_Ui2.pos[6] = { SCREEN_WIDTH / 2 - 490.0f ,SCREEN_HEIGHT / 2 + 100.0f };
            g_Ch_Ui2.pos[7] = { SCREEN_WIDTH / 2 - 490.0f ,SCREEN_HEIGHT / 2 + 170.0f };
            g_pContext->PSSetShaderResources(0, 1, &g_Texturecheck_2P);
            SetBlendState(BLENDSTATE_ALFA);
            DrawSpriteEx(g_Ch_Ui2.pos[i], g_Ch_Ui2.size[i], g_Ch_Ui2.col[i], challenge_frame[i], 4, 3, -3.0f);
        }
    }
    if (g_Ch_Ui2.isEnd)
    {
        g_Ch_Ui2.pos[0] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureChallenge_Ui_2P[1]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Ch_Ui2.pos[0], g_Ch_Ui2.size[0], g_Ch_Ui2.col[0], challenge_frame[1], UI_YOKO, UI_TATE, -3.0f);
        g_Score_Ui2.pos[1] = { SCREEN_WIDTH / 2 - 250.0f ,SCREEN_HEIGHT / 2 + 80.0f };
    }
    if (g_Score_Ui2.ui_used)
    {
        g_Score_Ui2.pos[0] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureScore_Ui_2P[0]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Score_Ui2.pos[0], g_Score_Ui2.size[0], g_Score_Ui2.col[0], score_frame[0], UI_YOKO, UI_TATE, -3.0f);


        if (g_Score_Ui2.col[1].w > 0.0f && g_Score_Ui2.pos[1].x < SCREEN_WIDTH / 2 - 300.0f)
        {
            g_Score_Ui2.pos[1].x += 3.0f;

        }


        g_pContext->PSSetShaderResources(0, 1, &g_TextureScore_Bg);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Score_Ui2.pos[1], g_Score_Ui2.size[1], g_Score_Ui2.col[1], 1, 1, 1, -3.0f);
        if (g_Score_Ui2.col[1].w >= 1.0f)
        {
            int score = g_Score_Ui2.score;
            int digits[4] = { 0,0,0,0 };
            digits[0] = score / 1000 - 1;           // 千の位
            digits[1] = score / 100 - 1;    // 百の位
            digits[2] = score / 10 - 1;           // 十の位
            digits[3] = score % 10 - 1;           // 一の位
            for (int i = 1; i < 5; i++)
            {
                g_Score_Ui2.pos[i + 1] = { SCREEN_WIDTH / 2 - 300.0f + (i - 2) * 90.0f ,SCREEN_HEIGHT / 2 + 90.0f };
                g_Score_Ui2.size[i + 1] = { 156.0f * 1.1f, 156.0f * 1.1f };
                g_Score_Ui2.col[i + 1] = { 1.0f,1.0f,1.0f,1.0f };
                g_pContext->PSSetShaderResources(0, 1, &g_TextureNumber);
                SetBlendState(BLENDSTATE_ALFA);
                DrawSpriteEx(g_Score_Ui2.pos[i + 1], g_Score_Ui2.size[i + 1], g_Score_Ui2.col[i + 1], (float)digits[i - 1], 10, 1, -3.0f);
            }
        }
    }
    if (g_Score_Ui2.isEnd)
    {
        g_Score_Ui2.pos[0] = { SCREEN_WIDTH / 2,SCREEN_HEIGHT / 2 + 50.0f };
        g_pContext->PSSetShaderResources(0, 1, &g_TextureScore_Ui_2P[1]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Score_Ui2.pos[0], g_Score_Ui2.size[0], g_Score_Ui2.col[0], score_frame[1], UI_YOKO, UI_TATE, -3.0f);
    }


}
void Result2_SetupResultUi(bool flg, int winner)
{
    if (!g_Result_Ui2.ui_used)
    {
        if (winner == 1)
        {
            g_Result_Ui2.win_used = flg;
        }
        else
        {
            g_Result_Ui2.lose_used = flg;
        }
        g_Result_Ui2.ui_used = flg;
    }


}
void Result2_SetResultIsEnd(bool flg)
{
    g_Result_Ui2.win_used = false;
    g_Result_Ui2.lose_used = false;
    g_Result_Ui2.ui_used = false;
    g_Result_Ui2.isEnd = flg;
}
void Result2_SetChallengeUi(bool flg)
{
    g_Ch_Ui2.ui_used = flg;
}
void Result2_SetChallengeIsEnd(bool flg)
{
    g_Ch_Ui2.ui_used = false;
    g_Ch_Ui2.isEnd = flg;
}
void Result2_SetScoreUi(bool flg)
{
    g_Score_Ui2.ui_used = flg;
}
void Result2_SetScoreIsEnd(bool flg)
{
    g_Score_Ui2.ui_used = false;
    g_Score_Ui2.isEnd = flg;
}

// ==============================
// 攻撃開始Ready（ResultSystemが呼ぶ）
// ==============================

bool Result2_IsResultAttackReady()
{
    return (g_Result_Ui2.ui_used && result_frame[1] >= 18.0f);
}
bool Result2_IsChallengeAttackReady()
{
    return (g_Ch_Ui2.ui_used && g_Ch_Ui2.col[1].w >= 1.0f);
}
bool Result2_IsScoreAttackReady()
{
    return (g_Score_Ui2.ui_used &&
        g_Score_Ui2.col[1].w >= 1.0f &&
        g_Score_Ui2.score >= Player_GetScore());
}
bool Result2_IsAnyUiEnding()
{
    return (g_Result_Ui2.isEnd || g_Ch_Ui2.isEnd || g_Score_Ui2.isEnd);
}

