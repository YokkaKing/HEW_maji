/*
* ファイル名	Hp.cpp
* タイトル	タイトル
* 作成者		久保木幹太
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"Hp.h"
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
static	ID3D11ShaderResourceView* g_TextureTimer =  NULL ;
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;
static	ID3D11ShaderResourceView* g_TextureLowHp = NULL;
static	ID3D11ShaderResourceView* g_TextureButton[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureItem[2] = {NULL};


static	ID3D11ShaderResourceView* g_TextureHp_1P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureHp_2P[4] = { NULL };
static	ID3D11ShaderResourceView* g_TextureGuide = NULL;   
static	ID3D11ShaderResourceView* g_TextureTransform_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransform_2P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransformNow_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_TextureTransformNow_2P[6] = { NULL };
static	ID3D11ShaderResourceView* g_Texture_1P[6] = { NULL };
static	ID3D11ShaderResourceView* g_Texture_2P[6] = { NULL };

const float RED_SHRINK_PER_FRAME = 0.4f;
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
HP g_Hp;
HP_2P g_Hp2;
TIMER g_Timer;
STATUS_1P g_Status1;
STATUS_2P g_Status2;
static float g_HpBlinkTime = 0.0f;
static float canTransformFrame = 0.0f;
static bool g_isItemAlarmUse = false;
static bool g_LowHp = false;
static int g_itemType = 0;
static float g_alarmAlpha = 0.0f;
static float g_alarmBlinkTime = 0.0f;
static float g_alarmTimer = 0.0f;
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

static UITransformSlotsInt g_UITr1P = {};
static UITransformSlotsInt g_UITr2P = {};
void Hp_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
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

    LoadFromWICFile(L"asset\\texture\\low_Hp.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureLowHp);
    assert(&g_TextureLowHp);

    LoadFromWICFile(L"asset\\texture\\L_button.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureButton[0]);
    assert(&g_TextureButton[0]);

    LoadFromWICFile(L"asset\\texture\\R_button.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureButton[1]);
    assert(&g_TextureButton[1]);

    LoadFromWICFile(L"asset\\texture\\TransformHeal_alarm.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureItem[0]);
    assert(&g_TextureItem[0]);

    LoadFromWICFile(L"asset\\texture\\Heal_alarm.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureItem[1]);
    assert(&g_TextureItem[1]);
	//フェードインのセット
    g_Hp.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_Hp.pos = { 400, 1006 };
    g_Hp.size = { 650*0.6, 41*0.7 };
    g_Hp.m_Hp = 0.0f;
    g_Hp.redHpLen = 100.0f;
	g_Hp.hpTimer = 2.0f;
    g_Hp.shakeOffset = { 0.0f, 0.0f };
    g_Hp.shakeTimer = 0.0f;
    g_Hp.shakeDuration = 0.6f;
    g_Hp.shakeMagnitude = 6.0f;
    g_Hp.prevHp = g_Hp.m_Hp;


    g_Hp2.col = { 1.0f, 1.0f, 1.0f, 1.0f };
    g_Hp2.pos = { 600, 1006 };
    g_Hp2.size = { 650 * 0.6, 41 * 0.7 };
    g_Hp2.m_Hp = 0.0f;
    g_Hp2.redHpLen = 100.0f;
    g_Hp2.hpTimer = 2.0f;
    g_Hp2.shakeOffset = { 0.0f, 0.0f };
    g_Hp2.shakeTimer = 0.0f;
    g_Hp2.shakeDuration = 0.6f;
    g_Hp2.shakeMagnitude = 6.0f;    
    g_Hp2.prevHp = g_Hp2.m_Hp;

    g_Timer.pos = XMFLOAT2(0, 0);
    g_Timer.size = XMFLOAT2(1648*0.6, 117*0.6);
    g_Timer.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Timer.time = 60.0f;
    g_Timer.frame = 1 / 59.0f;

	g_Status1.pos[0] = XMFLOAT2(560, 130);
    g_Status1.pos[1] = XMFLOAT2(680, 117);
    g_Status1.pos[2] = XMFLOAT2(800, 130);
	g_Status1.size[0] = XMFLOAT2(300 * 0.65, 300 * 0.65); //今の状態
    g_Status1.size[1] = XMFLOAT2(300 * 0.6, 300 * 0.6); //今の状態じゃない
	g_Status1.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Status1.nowType = 5;
    for (int i = 0; i < 2; i++)
    {
        g_Status1.nextType[i] = 5;
    }


    g_Status2.pos[0] = XMFLOAT2(1120, 130);
    g_Status2.pos[1] = XMFLOAT2(1240, 117);
    g_Status2.pos[2] = XMFLOAT2(1360, 130);
    g_Status2.size[0] = XMFLOAT2(300 * 0.65, 300 * 0.65); //今の状態
    g_Status2.size[1] = XMFLOAT2(300 * 0.6, 300 * 0.6); //今の状態じゃない
    g_Status2.col = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
    g_Status2.nowType = 5;
    for (int i = 0; i < 2; i++)
    {
        g_Status2.nextType[i] = 5;
    }
    g_UITr1P.inited = false;
    g_UITr2P.inited = false;
    g_HpBlinkTime = 0.0f;
    g_LowHp = false;
    canTransformFrame = 0.0f;
    g_isItemAlarmUse = false;
    g_itemType = 0;
    g_alarmAlpha = 0.0f;
    g_alarmTimer = 0.0f;
}
void Hp_Finalize()
{

    SAFE_RELEASE(g_TextureTimer);
    SAFE_RELEASE(g_TextureNumber);
    SAFE_RELEASE(g_TextureGuide);
    SAFE_RELEASE(g_TextureLowHp);

    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_TextureButton[i]);
		SAFE_RELEASE(g_TextureItem[i]);
		
    }
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
void Hp_Update()
{
    // いまのHP（前フレームとの差分でダメージ/回復を判定する）
    float hpPrev1 = g_Hp.prevHp;
    float hpPrev2 = g_Hp2.prevHp;
    g_Hp.m_Hp = Player_GetHp();
    g_Hp2.m_Hp = Player2_GetHp();
    if (g_Timer.time > 0.0f&&!Player_GetIsDead()&&!Player2_GetIsDead())
    {
        g_Timer.time -= g_Timer.frame;
    }
    else
    {
		g_Timer.time = 0.0f;
    }
    const float DAMAGE_DELAY = 2.0f;      // ダメージ後、赤HPが動き出すまで待つ秒数
    const float RED_SHRINK_PER_FRAME = 0.4f; // 赤HPが減る量（1フレームあたり）※元と同じ

    if (g_Hp.m_Hp <= 30)
    {
        g_LowHp = true;
    }
    if (g_LowHp)
    {
        g_HpBlinkTime += 0.04f;
    }
    if (g_isItemAlarmUse)
    {
		g_alarmBlinkTime += 0.04f;
        g_alarmTimer += 1.0f;
    }
 
    //========================
    // 1P 赤HP制御
    //========================
    {
        float hpNow = g_Hp.m_Hp;

        // ダメージを受けた瞬間：タイマー更新（2秒に戻す）
        if (hpNow < hpPrev1)
        {
            g_Hp.hpTimer = DAMAGE_DELAY;

            // 赤HPは「ダメージ前のHP」まで残しておきたい
            // std::maxの代わり：大きい方に合わせる
            if (g_Hp.redHpLen < hpPrev1) g_Hp.redHpLen = hpPrev1;

            // attackedフラグを使ってるなら、ここで戻してOK（放置すると挙動が変になりがち）
            SetPlayer_IsAttacked(false);
        }
        // 回復した瞬間：赤HPが取り残されないよう即追従
        else if (hpNow > hpPrev1)
        {
            if (g_Hp.redHpLen < hpNow) g_Hp.redHpLen = hpNow;
        }

        // 2秒間ダメージが無ければ、赤HPを少しずつ現在HPへ減らす
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
                // 念のため（回復などで逆転したら揃える）
                g_Hp.redHpLen = hpNow;
            }
        }
    }
    {
        int nowType = (int)g_Player.TransformType;

        if (!g_UITr1P.inited)
        {
            g_UITr1P.left = WTToUIIndex(GetTransform_P1(0));
            g_UITr1P.cur = 5; // ★中央は最初デフォルト
            g_UITr1P.right = WTToUIIndex(GetTransform_P1(1));
            g_UITr1P.prevType = nowType;
            g_UITr1P.inited = true;
        }

        // ① まず「状態変化（変身/解除）」が起きた瞬間だけswap
        if (g_UITr1P.prevType != nowType)
        {
            // NONE -> A（変身：左↔中央）
            if (g_UITr1P.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_A)
            {
                std::swap(g_UITr1P.left, g_UITr1P.cur);
            }
            // NONE -> B（変身：右↔中央）
            else if (g_UITr1P.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_B)
            {
                std::swap(g_UITr1P.right, g_UITr1P.cur);
            }
            // A -> NONE（解除：左↔中央を戻す）
            else if (g_UITr1P.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_A &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
            {
                std::swap(g_UITr1P.left, g_UITr1P.cur);
            }
            // B -> NONE（解除：右↔中央を戻す）
            else if (g_UITr1P.prevType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_B &&
                nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
            {
                std::swap(g_UITr1P.right, g_UITr1P.cur);
            }

            g_UITr1P.prevType = nowType;
        }

        // ② その後で、NONE状態なら「中央=default固定」「左右=候補同期」
        //    （解除した瞬間でも確実に中央がdefaultに戻る）
        if (nowType == (int)TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
        {
            g_UITr1P.cur = 5; // ★ここが超重要：解除されたら必ず中央はdefault
            g_UITr1P.left = WTToUIIndex(GetTransform_P1(0));
            g_UITr1P.right = WTToUIIndex(GetTransform_P1(1));
        }

        // ③ 描画用に反映
        g_Status1.nextType[0] = g_UITr1P.left;
        g_Status1.nowType = g_UITr1P.cur;
        g_Status1.nextType[1] = g_UITr1P.right;
    }
    // --- 2P ---
    {
        int nowType = (int)g_Player2.TransformType;

        if (!g_UITr2P.inited)
        {
            g_UITr2P.left = WTToUIIndex(GetTransform_P2(0));
            g_UITr2P.cur = 5;
            g_UITr2P.right = WTToUIIndex(GetTransform_P2(1));
            g_UITr2P.prevType = nowType;
            g_UITr2P.inited = true;
        }

        if (g_UITr2P.prevType != nowType)
        {
            if (g_UITr2P.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_A)
            {
                std::swap(g_UITr2P.left, g_UITr2P.cur);
            }
            else if (g_UITr2P.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_B)
            {
                std::swap(g_UITr2P.right, g_UITr2P.cur);
            }
            else if (g_UITr2P.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_A &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
            {
                std::swap(g_UITr2P.left, g_UITr2P.cur);
            }
            else if (g_UITr2P.prevType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_B &&
                nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
            {
                std::swap(g_UITr2P.right, g_UITr2P.cur);
            }

            g_UITr2P.prevType = nowType;
        }

        if (nowType == (int)TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
        {
            g_UITr2P.cur = 5; // ★解除されたら中央default固定
            g_UITr2P.left = WTToUIIndex(GetTransform_P2(0));
            g_UITr2P.right = WTToUIIndex(GetTransform_P2(1));
        }

        g_Status2.nextType[0] = g_UITr2P.left;
        g_Status2.nowType = g_UITr2P.cur;
        g_Status2.nextType[1] = g_UITr2P.right;
    }

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

    //========================
    // 揺れ（元コードのまま）
    //========================
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

    // 最後にprevHp更新
    g_Hp.prevHp = g_Hp.m_Hp;
    g_Hp2.prevHp = g_Hp2.m_Hp;
}

void Hp_Draw()
{
    Shader_Begin();
    const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
    const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();
    g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2, 130);
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
        g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2 - 20 + i * 50, 120);
        g_pContext->PSSetShaderResources(0, 1, &g_TextureNumber);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSpriteEx(g_Timer.pos, XMFLOAT2(156*0.6,156*0.6), g_Timer.col, time[i], 10, 1);
	}

    XMFLOAT2 basePos1 = XMFLOAT2(SCREEN_WIDTH / 2 - 300, 200);
    XMFLOAT2 drawPos1 = XMFLOAT2(basePos1.x + g_Hp.shakeOffset.x, basePos1.y + g_Hp.shakeOffset.y);


	//1P体力描画
    g_Hp.pos = basePos1;
    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[0]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp.size, g_Hp.col, 1, 1, 1);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[1]); //赤い体力
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp.size, g_Hp.col, g_Hp.redHpLen, true);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[2]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos1, g_Hp.size, g_Hp.col, g_Hp.m_Hp, true);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_1P[3]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(XMFLOAT2(drawPos1.x + 150, drawPos1.y - 10), XMFLOAT2(63 * 0.8, 26 * 0.8), g_Hp.col, 1, 1, 1);


	//2P体力描画
    XMFLOAT2 basePos2 = XMFLOAT2(SCREEN_WIDTH / 2 + 300, 200);
    XMFLOAT2 drawPos2 = XMFLOAT2(basePos2.x + g_Hp2.shakeOffset.x, basePos2.y + g_Hp2.shakeOffset.y);

    g_Hp2.pos = basePos2;
    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[0]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp2.size, g_Hp2.col, 1, 1, 1);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[1]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp2.size, g_Hp2.col, g_Hp2.redHpLen,false);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[2]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSpriteEx(drawPos2, g_Hp2.size, g_Hp2.col, g_Hp2.m_Hp, false);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureHp_2P[3]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(XMFLOAT2(drawPos2.x - 135, drawPos2.y - 10), XMFLOAT2(102 * 0.8, 26 * 0.8), g_Hp2.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureGuide);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(XMFLOAT2(250, SCREEN_HEIGHT - 200), XMFLOAT2(900 * 0.6, 520 * 0.6), g_Hp.col);

    XMFLOAT2 transformIconScale = XMFLOAT2(g_Status1.size[1].x + 150, g_Status1.size[1].y+60);


 



    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_1P[g_Status1.nextType[0]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[0], g_Status1.size[1], g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_1P[g_Status1.nextType[1]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[2], g_Status1.size[1], g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransformNow_1P[g_Status1.nowType]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status1.pos[1], g_Status1.size[0], g_Hp.col);


 

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_2P[g_Status2.nextType[0]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[0], g_Status2.size[1], g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransform_2P[g_Status2.nextType[1]]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[2], g_Status2.size[1], g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureTransformNow_2P[g_Status2.nowType]);
    SetBlendState(BLENDSTATE_ALFA);
    DrawSprite(g_Status2.pos[1], g_Status2.size[0], g_Hp.col);

    g_pContext->PSSetShaderResources(0, 1, &g_TextureLowHp);
    SetBlendState(BLENDSTATE_ALFA);
    // 色と位置・サイズを設定
   // cosで0.0～1.0を作る
    float blink = (cosf(g_HpBlinkTime) + 1.0f) * 0.5f;

    // 完全に消えると見づらいので、0.35～1.0くらいで点滅させる
    float alpha = 0.1f + blink * 0.9f;

    XMFLOAT4 col = { 1.0f, 1.0f, 1.0f, alpha };
    XMFLOAT2 pos = { SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2};
    XMFLOAT2 size = { SCREEN_WIDTH, SCREEN_HEIGHT };
    if (g_LowHp)
    {
        DrawSprite(pos, size, col);
    }
    static float scale = 0.3f;
    if (!GetIsUsedA_P1())
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureButton[0]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(XMFLOAT2(g_Status1.pos[0].x , g_Status1.pos[0].y - 50), XMFLOAT2(219 * scale, 105 * scale), g_Hp.col);
    }
    if (!GetIsUsedB_P1())
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureButton[1]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(XMFLOAT2(g_Status1.pos[2].x , g_Status1.pos[2].y - 50), XMFLOAT2(219 * scale, 105 * scale), g_Hp.col);
    }
    blink = (cosf(g_alarmBlinkTime) + 1.0f) * 0.5f;
    alpha = 0.1f + blink * 0.9f;
    if (g_isItemAlarmUse&&g_alarmTimer<=180.0f)
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureItem[g_itemType]);
        SetBlendState(BLENDSTATE_ALFA);
        DrawSprite(XMFLOAT2(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 -200), XMFLOAT2(600*1.2f, 600*1.2f), XMFLOAT4(1.0f,1.0f,1.0f, alpha));
    }
}

float Hp_GetTime()
{
    return g_Timer.time;
}

void Hp_SetTime(float time)
{
    g_Timer.time = time;
}
void SetIsItemAlarmUse(bool use,int type)
{
	g_isItemAlarmUse = use;
    g_itemType = type;
    g_alarmTimer = 0.0f;
}



