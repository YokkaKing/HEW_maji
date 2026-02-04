/*
* ファイル名	Hp2.cpp
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include "Manager.h"
#include "sprite.h"
#include "keyboard.h"
#include "Hp2.h"
#include "fade.h"
#include "shader.h"
#include "player.h"
#include "player2.h"
#include <random>
#include <cmath>
#include <algorithm>
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureTimer =  NULL ;
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;
static	ID3D11ShaderResourceView* g_TextureHp_1P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureHp_2P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureGuide = NULL;
static  ID3D11Device* g_pDevice = nullptr;
static  ID3D11DeviceContext* g_pContext = nullptr;
HP_2 g_Hp;
HP_2P_2 g_Hp2;
TIMER_2 g_Timer;
static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);
void Hp2_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

    std::random_device rd;
    g_Rng.seed(rd());

    TexMetadata		metadata;
    ScratchImage	image;
    LoadFromWICFile(L"asset\\texture\\timer.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTimer);
    assert(&g_TextureTimer);
    LoadFromWICFile(L"asset\\texture\\number.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureNumber);
    assert(&g_TextureNumber);

	LoadFromWICFile(L"asset\\texture\\hp_bar.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
	CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_1P[0]);
	assert(&g_TextureHp_1P[0]);


    LoadFromWICFile(L"asset\\texture\\hp_red.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_1P[1]);
    assert(&g_TextureHp_1P[1]);
  

    LoadFromWICFile(L"asset\\texture\\hp.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_1P[2]);
    assert(&g_TextureHp_1P[2]);

 
    LoadFromWICFile(L"asset\\texture\\you.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_1P[3]);
    assert(&g_TextureHp_1P[3]);
    
   
    LoadFromWICFile(L"asset\\texture\\2p_hp_bar.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_2P[0]);
    assert(&g_TextureHp_2P[0]);


    LoadFromWICFile(L"asset\\texture\\2P_hp_red.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_2P[1]);
    assert(&g_TextureHp_2P[1]);


    LoadFromWICFile(L"asset\\texture\\2P_hp.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_2P[2]);
    assert(&g_TextureHp_2P[2]);


    LoadFromWICFile(L"asset\\texture\\enemy.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureHp_2P[3]);
    assert(&g_TextureHp_2P[3]);

    LoadFromWICFile(L"asset\\texture\\button_Ui.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureGuide);
    assert(&g_TextureHp_2P[3]);
	//フェードインのセット
    g_Hp.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_Hp.pos = { 500, 1006 };
    g_Hp.size = { 585*0.5, 41*0.6 };
    g_Hp.m_Hp = 0.0f;
    g_Hp.redHpLen = 100.0f;
	g_Hp.hpTimer = 2.0f;
    g_Hp.shakeOffset = { 0.0f, 0.0f };
    g_Hp.shakeTimer = 0.0f;
    g_Hp.shakeDuration = 0.6f;
    g_Hp.shakeMagnitude = 6.0f;
    g_Hp.prevHp = g_Hp.m_Hp;

    g_Hp2.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_Hp2.pos = { 500, 1006 };
    g_Hp2.size = { 585 * 0.5, 41 * 0.6 };
    g_Hp2.m_Hp = 0.0f;
    g_Hp2.redHpLen = 100.0f;
    g_Hp2.hpTimer = 2.0f;
    g_Hp2.shakeOffset = { 0.0f, 0.0f };
    g_Hp2.shakeTimer = 0.0f;
    g_Hp2.shakeDuration = 0.6f;
    g_Hp2.shakeMagnitude = 6.0f;
    g_Hp2.prevHp = g_Hp2.m_Hp;

    g_Timer.pos = XMFLOAT2(0, 0);
    g_Timer.size = XMFLOAT2(1648*0.5, 117*0.5);
    g_Timer.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Timer.time = 60.0f;
    g_Timer.frame = 1 / 60.0f;
}
void Hp2_Finalize()
{

    SAFE_RELEASE(g_TextureTimer);
    SAFE_RELEASE(g_TextureNumber);
    SAFE_RELEASE(g_TextureGuide);

    for (int i = 0; i < 4; i++)
    {
        SAFE_RELEASE(g_TextureHp_1P[i]);
        SAFE_RELEASE(g_TextureHp_2P[i]);

    }

}
void Hp2_Update()
{ 

    float prevHp1 = g_Hp.m_Hp;
    float prevHp2 = g_Hp2.m_Hp;

    g_Hp.m_Hp=Player_GetHp();  // get体力
    g_Hp2.m_Hp=Player2_GetHp();  // get体力
    g_Timer.time -= g_Timer.frame;

    if (GetPlayer_IsAttacked())
    {
        if (g_Hp.hpTimer <= 0)
        {
            g_Hp.redHpLen -= 0.4f;
            if (g_Hp.redHpLen < g_Hp.m_Hp)
            {
                g_Hp.redHpLen = g_Hp.m_Hp;
                SetPlayer_IsAttacked(false);
				g_Hp.hpTimer = 2.0f;
            }
        }
        else
        {
            g_Hp.hpTimer -= g_Timer.frame;
        }

    }
    if (GetPlayer2_IsAttacked())
    {
        if (g_Hp2.hpTimer <= 0)
        {
            g_Hp2.redHpLen -= 0.4f;
            if (g_Hp2.redHpLen < g_Hp2.m_Hp)
            {
                g_Hp2.redHpLen = g_Hp2.m_Hp;
                SetPlayer2_IsAttacked(false);
                g_Hp2.hpTimer = 2.0f;
            }
        }
        else
        {
            g_Hp2.hpTimer -= g_Timer.frame;
        }

    }
    if (g_Hp.prevHp > g_Hp.m_Hp)
    {
        // ダメージを計算
        float damage = g_Hp.prevHp - g_Hp.m_Hp;
        //ダメージをもとに揺れる強さを変える
        float mag = damage * 1.0f;
        mag = fmaxf(3.0f, fminf(mag, 14.0f));
        g_Hp.shakeMagnitude = mag;
        g_Hp.shakeDuration = 0.6f; // seconds
        g_Hp.shakeTimer = g_Hp.shakeDuration;
    }

    if (g_Hp2.prevHp > g_Hp2.m_Hp)
    {
        float damage = g_Hp2.prevHp - g_Hp2.m_Hp;
        float mag = damage * 1.0f;
        mag = fmaxf(3.0f, fminf(mag, 14.0f));
        g_Hp2.shakeMagnitude = mag;
        g_Hp2.shakeDuration = 0.6f;
        g_Hp2.shakeTimer = g_Hp2.shakeDuration;
    }
    if (g_Hp.shakeTimer > 0.0f)
    {
        float t = g_Hp.shakeTimer / g_Hp.shakeDuration; 
        float amp = g_Hp.shakeMagnitude * t; 
        float angle = g_Dist01(g_Rng) * 6.28318530718f;
        g_Hp.shakeOffset.x = cosf(angle) * amp;
        g_Hp.shakeOffset.y = sinf(angle) * amp * 0.5f; 
        g_Hp.shakeTimer -= g_Timer.frame;
        if (g_Hp.shakeTimer <= 0.0f)
        {
            g_Hp.shakeTimer = 0.0f;
            g_Hp.shakeOffset = { 0.0f, 0.0f };
        }
    }
    if (g_Hp2.shakeTimer > 0.0f)
    {
        float t = g_Hp2.shakeTimer / g_Hp2.shakeDuration;
        float amp = g_Hp2.shakeMagnitude * t;
        float angle = g_Dist01(g_Rng) * 6.28318530718f;
        g_Hp2.shakeOffset.x = cosf(angle) * amp;
        g_Hp2.shakeOffset.y = sinf(angle) * amp * 0.5f;
        g_Hp2.shakeTimer -= g_Timer.frame;
        if (g_Hp2.shakeTimer <= 0.0f)
        {
            g_Hp2.shakeTimer = 0.0f;
            g_Hp2.shakeOffset = { 0.0f, 0.0f };
        }
    }
    g_Hp.prevHp = g_Hp.m_Hp;
    g_Hp2.prevHp = g_Hp2.m_Hp;

   
}
void Hp2_Draw()
{
    Shader_Begin();
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
    g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2, 100);
    Shader_SetMatrix(XMMatrixOrthographicOffCenterLH(
        0.0f,
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        0.0f,
        0.0f,
        1.0f));
    Shader_SetWorldMatrix(XMMatrixIdentity());
	//タイマー描画
    g_pContext->PSSetShaderResources(0, 1, &g_TextureTimer);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Timer.pos, g_Timer.size, g_Timer.col);

	//数字描画
    int time[2];
    time[0] = (int)(g_Timer.time) / 10-1;
    time[1] = (int)(g_Timer.time) % 10-1;

    for (int i = 0; i < 2; i++)
    {
        g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2 - 20 + i * 40, 100);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureNumber);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Timer.pos, XMFLOAT2(156*0.5,156*0.5), g_Timer.col, time[i], 10, 1);
	}

    XMFLOAT2 basePos1 = XMFLOAT2(SCREEN_WIDTH / 2 - 240, 160);
    XMFLOAT2 drawPos1 = XMFLOAT2(basePos1.x + g_Hp2.shakeOffset.x, basePos1.y + g_Hp2.shakeOffset.y);


	//1P体力描画
    g_Hp2.pos = basePos1;
    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[0]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp2.size, g_Hp2.col, 1, 1, 1);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[1]); //赤い体力
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp2.size, g_Hp2.col, g_Hp2.redHpLen, true);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[2]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp2.size, g_Hp2.col, g_Hp2.m_Hp, true);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[3]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(XMFLOAT2(drawPos1.x + 125, drawPos1.y - 10), XMFLOAT2(63 * 0.6, 26 * 0.6), g_Hp2.col, 1, 1, 1);


	//2P体力描画
    XMFLOAT2 basePos2 = XMFLOAT2(SCREEN_WIDTH / 2 + 240, 160);
    XMFLOAT2 drawPos2 = XMFLOAT2(basePos2.x + g_Hp.shakeOffset.x, basePos2.y + g_Hp.shakeOffset.y);

    g_Hp.pos = basePos2;
    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[0]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp.size, g_Hp.col, 1, 1, 1);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[1]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp.size, g_Hp.col, g_Hp.redHpLen,false);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[2]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp.size, g_Hp.col, g_Hp.m_Hp, false);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[3]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(XMFLOAT2(drawPos2.x - 110, drawPos2.y - 10), XMFLOAT2(102 * 0.6, 26 * 0.6), g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureGuide);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(XMFLOAT2(200, SCREEN_HEIGHT - 150), XMFLOAT2(452 * 0.5, 261 * 0.5), g_Hp.col);
}

float Hp2_GetTime()
{
    return g_Timer.time;
}

void Hp2_SetTime(float time)
{
    g_Timer.time = time;
}

