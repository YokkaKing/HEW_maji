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
#include "Transform.h"
#include <random>
#include <cmath>
#include <algorithm>
#include <utility> // std::swap

//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureTimer =  NULL ;
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;
static	ID3D11ShaderResourceView* g_TextureHp_1P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureHp_2P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureGuide = NULL;
static	ID3D11ShaderResourceView* g_TextureTransform_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransform_2P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransformNow_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransformNow_2P[6] = { NULL };
static	ID3D11ShaderResourceView* g_Texture_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_Texture_2P[6] = { NULL };
static  ID3D11Device* g_pDevice = nullptr;
static  ID3D11DeviceContext* g_pContext = nullptr;
HP_2 g_Hp;
HP_2P_2 g_Hp2;
TIMER_2 g_Timer;
STATUS_1P_2 g_Status1;
STATUS_2P_2 g_Status2;
static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);

static int WTToUIIndex(WeaponTerrain wt)
{
    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL:  return 0;
    case WeaponTerrain::SPEAR_HILL:  return 1;
    case WeaponTerrain::BOW_HILL:    return 2;
    case WeaponTerrain::HAMMER_:     return 3;
    case WeaponTerrain::SHURIKEN_:   return 4;
    default:                         return 5; // NONEなどはデフォルト
    }
}

// UIだけで「左/中央/右」を入れ替えるためのスロット
struct UITransformSlotsInt
{
    int left;     // 1(左)  = 変身候補A
    int cur;      // 2(中央)= 現在(最初はdefault=5)
    int right;    // 3(右)  = 変身候補B
    int prevType; // 前フレームのTransformType
    bool inited;
};
static UITransformSlotsInt g_UITrP1 = {}; // Player1（右に出す）
static UITransformSlotsInt g_UITrP2 = {}; // Player2（左に出す）

void Hp2_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
	g_pDevice = pDevice;
	g_pContext = pContext;

    std::random_device rd;
    g_Rng.seed(rd());
#pragma region hp/timer/number
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

#pragma endregion

#pragma region status_1P
    LoadFromWICFile(L"asset\\texture\\sword_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[0]);
    assert(&g_TextureTransformNow_1P[0]);

    LoadFromWICFile(L"asset\\texture\\spear_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[1]);
    assert(&g_TextureTransformNow_1P[1]);

    LoadFromWICFile(L"asset\\texture\\bow_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[2]);
    assert(&g_TextureTransformNow_1P[2]);

    LoadFromWICFile(L"asset\\texture\\hammer_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[3]);
    assert(&g_TextureTransformNow_1P[3]);

    LoadFromWICFile(L"asset\\texture\\shuriken_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[4]);
    assert(&g_TextureTransformNow_1P[4]);

    LoadFromWICFile(L"asset\\texture\\default_select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_1P[5]);
    assert(&g_TextureTransformNow_1P[5]);


    LoadFromWICFile(L"asset\\texture\\sword_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[0]);
    assert(&g_TextureTransform_1P[0]);

    LoadFromWICFile(L"asset\\texture\\spear_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[1]);
    assert(&g_TextureTransform_1P[1]);
    LoadFromWICFile(L"asset\\texture\\bow_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[2]);
    assert(&g_TextureTransform_1P[2]);

    LoadFromWICFile(L"asset\\texture\\hammer_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[3]);
    assert(&g_TextureTransform_1P[3]);

    LoadFromWICFile(L"asset\\texture\\shuriken_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[4]);
    assert(&g_TextureTransform_1P[4]);

    LoadFromWICFile(L"asset\\texture\\default_noselect.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_1P[5]);
    assert(&g_TextureTransform_1P[5]);
#pragma endregion

#pragma region status_2P
    LoadFromWICFile(L"asset\\texture\\sword_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[0]);
    assert(&g_TextureTransformNow_2P[0]);

    LoadFromWICFile(L"asset\\texture\\spear_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[1]);
    assert(&g_TextureTransformNow_2P[1]);

    LoadFromWICFile(L"asset\\texture\\bow_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[2]);
    assert(&g_TextureTransformNow_2P[2]);

    LoadFromWICFile(L"asset\\texture\\hammer_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[3]);
    assert(&g_TextureTransformNow_2P[3]);

    LoadFromWICFile(L"asset\\texture\\shuriken_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[4]);
    assert(&g_TextureTransformNow_2P[4]);

    LoadFromWICFile(L"asset\\texture\\default_select_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransformNow_2P[5]);
    assert(&g_TextureTransformNow_2P[5]);


    LoadFromWICFile(L"asset\\texture\\sword_noselect_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[0]);
    assert(&g_TextureTransform_2P[0]);

    LoadFromWICFile(L"asset\\texture\\spear_noselect_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[1]);
    assert(&g_TextureTransform_2P[1]);
    LoadFromWICFile(L"asset\\texture\\bow_noselect_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[2]);
    assert(&g_TextureTransform_2P[2]);
    LoadFromWICFile(L"asset\\texture\\hammer_noselect_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[3]);
    assert(&g_TextureTransform_2P[3]);
    LoadFromWICFile(L"asset\\texture\\shuriken_noselect_2P.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[4]);
    assert(&g_TextureTransform_2P[4]);

    LoadFromWICFile(L"asset\\texture\\default_noselect_2p.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTransform_2P[5]);
    assert(&g_TextureTransform_2P[5]);
#pragma endregion
    //フェードインのセット
    g_Hp.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_Hp.pos = { 500, 1006 };
    g_Hp.size = { 585 * 0.5, 41 * 0.6 };
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
    g_Timer.size = XMFLOAT2(1648 * 0.5, 117 * 0.5);
    g_Timer.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Timer.time = 60.0f;
    g_Timer.frame = 1 / 59.0f;

    g_Status2.pos[0] = XMFLOAT2(620, 100);
    g_Status2.pos[1] = XMFLOAT2(712, 87);
    g_Status2.pos[2] = XMFLOAT2(800, 100);
    g_Status1.size[0] = XMFLOAT2(300 * 0.55, 300 * 0.55); //今の状態
    g_Status1.size[1] = XMFLOAT2(300 * 0.5, 300 * 0.5); //今の状態じゃない
    g_Status1.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Status1.nowType = 5;
    for (int i = 0; i < 2; i++)
    {
        g_Status1.nextType[i] = 5;
    }


    g_Status1.pos[0] = XMFLOAT2(1110, 100);
    g_Status1.pos[1] = XMFLOAT2(1198, 87);
    g_Status1.pos[2] = XMFLOAT2(1290, 100);
    g_Status2.size[0] = XMFLOAT2(300 * 0.55, 300 * 0.55); //今の状態
    g_Status2.size[1] = XMFLOAT2(300 * 0.5, 300 * 0.5); //今の状態じゃない
    g_Status2.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Status2.nowType = 5;
    for (int i = 0; i < 2; i++)
    {
        g_Status2.nextType[i] = 5;
    }
    g_UITrP1.inited = false;
    g_UITrP1.inited = false;
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
    for (int i = 0; i < 6; i++)
    {
        SAFE_RELEASE(g_TextureTransform_1P[i]);
        SAFE_RELEASE(g_TextureTransform_2P[i]);
        SAFE_RELEASE(g_TextureTransformNow_1P[i]);
        SAFE_RELEASE(g_TextureTransformNow_2P[i]);

    }

}
void Hp2_Update()
{
    float hpPrev1 = g_Hp.prevHp;
    float hpPrev2 = g_Hp2.prevHp;

    g_Hp.m_Hp = Player_GetHp();
    g_Hp2.m_Hp = Player2_GetHp();

    if (g_Timer.time > 0.0f)
    {
        g_Timer.time -= g_Timer.frame;
    }
    else
    {
        g_Timer.time = 0.0f;
    }

    const float DAMAGE_DELAY = 2.0f;
    const float RED_SHRINK_PER_FRAME = 0.4f;
    // --- Player1（右側に出す） ---
    {
        int nowType = (int)g_Player.TransformType; // Player1

        if (!g_UITrP1.inited)
        {
            g_UITrP1.left = WTToUIIndex(GetTransform_P1(0));
            g_UITrP1.cur = 5; // default
            g_UITrP1.right = WTToUIIndex(GetTransform_P1(1));
            g_UITrP1.prevType = nowType;
            g_UITrP1.inited = true;
        }

        // ① swap（変身/解除の瞬間だけ）
        if (g_UITrP1.prevType != nowType)
        {
            if (g_UITrP1.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_A)
                std::swap(g_UITrP1.left, g_UITrP1.cur);
            else if (g_UITrP1.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_B)
                std::swap(g_UITrP1.right, g_UITrP1.cur);
            else if (g_UITrP1.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_A &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
                std::swap(g_UITrP1.left, g_UITrP1.cur);
            else if (g_UITrP1.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_B &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
                std::swap(g_UITrP1.right, g_UITrP1.cur);

            g_UITrP1.prevType = nowType;
        }

        // ② NONEなら中央default固定＆左右候補同期（解除で中央が確実にdefaultへ）
        if (nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
        {
            g_UITrP1.cur = 5;
            g_UITrP1.left = WTToUIIndex(GetTransform_P1(0));
            g_UITrP1.right = WTToUIIndex(GetTransform_P1(1));
        }
    }

    // --- Player2（左側に出す） ---
    {
        int nowType = (int)g_Player2.TransformType; // Player2

        if (!g_UITrP2.inited)
        {
            g_UITrP2.left = WTToUIIndex(GetTransform_P2(0));
            g_UITrP2.cur = 5;
            g_UITrP2.right = WTToUIIndex(GetTransform_P2(1));
            g_UITrP2.prevType = nowType;
            g_UITrP2.inited = true;
        }

        if (g_UITrP2.prevType != nowType)
        {
            if (g_UITrP2.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_A)
                std::swap(g_UITrP2.left, g_UITrP2.cur);
            else if (g_UITrP2.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_B)
                std::swap(g_UITrP2.right, g_UITrP2.cur);
            else if (g_UITrP2.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_A &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
                std::swap(g_UITrP2.left, g_UITrP2.cur);
            else if (g_UITrP2.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_B &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
                std::swap(g_UITrP2.right, g_UITrP2.cur);

            g_UITrP2.prevType = nowType;
        }

        if (nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
        {
            g_UITrP2.cur = 5;
            g_UITrP2.left = WTToUIIndex(GetTransform_P2(0));
            g_UITrP2.right = WTToUIIndex(GetTransform_P2(1));
        }
    }

    // 左＝Player2、右＝Player1 に反映
    g_Status2.nextType[0] = g_UITrP2.left;
    g_Status2.nowType = g_UITrP2.cur;
    g_Status2.nextType[1] = g_UITrP2.right;

    g_Status1.nextType[0] = g_UITrP1.left;
    g_Status1.nowType = g_UITrP1.cur;
    g_Status1.nextType[1] = g_UITrP1.right;
    // 1P
    {
        float hpNow = g_Hp.m_Hp;

        if (hpNow < hpPrev1)
        {
            g_Hp.hpTimer = DAMAGE_DELAY;
            if (g_Hp.redHpLen < hpPrev1) g_Hp.redHpLen = hpPrev1;
            SetPlayer_IsAttacked(false);
        }
        else if (hpNow > hpPrev1)
        {
            if (g_Hp.redHpLen < hpNow) g_Hp.redHpLen = hpNow;
        }

        if (g_Hp.hpTimer > 0.0f)
        {
            g_Hp.hpTimer -= g_Timer.frame;
            if (g_Hp.hpTimer < 0.0f) g_Hp.hpTimer = 0.0f;
        }
        else
        {
            if (g_Hp.redHpLen > hpNow)
            {
                g_Hp.redHpLen -= RED_SHRINK_PER_FRAME;
                if (g_Hp.redHpLen < hpNow) g_Hp.redHpLen = hpNow;
            }
            else
            {
                g_Hp.redHpLen = hpNow;
            }
        }
    }

    // 2P
    {
        float hpNow = g_Hp2.m_Hp;

        if (hpNow < hpPrev2)
        {
            g_Hp2.hpTimer = DAMAGE_DELAY;
            if (g_Hp2.redHpLen < hpPrev2) g_Hp2.redHpLen = hpPrev2;
            SetPlayer2_IsAttacked(false);
        }
        else if (hpNow > hpPrev2)
        {
            if (g_Hp2.redHpLen < hpNow) g_Hp2.redHpLen = hpNow;
        }

        if (g_Hp2.hpTimer > 0.0f)
        {
            g_Hp2.hpTimer -= g_Timer.frame;
            if (g_Hp2.hpTimer < 0.0f) g_Hp2.hpTimer = 0.0f;
        }
        else
        {
            if (g_Hp2.redHpLen > hpNow)
            {
                g_Hp2.redHpLen -= RED_SHRINK_PER_FRAME;
                if (g_Hp2.redHpLen < hpNow) g_Hp2.redHpLen = hpNow;
            }
            else
            {
                g_Hp2.redHpLen = hpNow;
            }
        }
    }

    // 揺れ（元のまま）
    if (g_Hp.prevHp > g_Hp.m_Hp)
    {
        float damage = g_Hp.prevHp - g_Hp.m_Hp;
        float mag = damage * 1.0f;
        mag = fmaxf(3.0f, fminf(mag, 14.0f));
        g_Hp.shakeMagnitude = mag;
        g_Hp.shakeDuration = 0.6f;
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

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_1P[g_Status2.nextType[0]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[0], g_Status2.size[1], g_Status2.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_1P[g_Status2.nextType[1]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[2], g_Status2.size[1], g_Status2.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransformNow_1P[g_Status2.nowType]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[1], g_Status2.size[0], g_Status2.col);

    // 右（Player1）
    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_2P[g_Status1.nextType[0]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[0], g_Status1.size[1], g_Status1.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_2P[g_Status1.nextType[1]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[2], g_Status1.size[1], g_Status1.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransformNow_2P[g_Status1.nowType]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[1], g_Status1.size[0], g_Status1.col);
}

float Hp2_GetTime()
{
    return g_Timer.time;
}

void Hp2_SetTime(float time)
{
    g_Timer.time = time;
}

