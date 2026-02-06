/*
* ファイル名	SELECT_TRANSFORM_UI.cpp
* タイトル	タイトル
* 作成者		カンジェウォン
* 作成日		02月05日
* 更新日		02月05日
*/

//================================================================
//	インクルード
//================================================================
#include "Select_Transform_Ui.h"
#include "transformManager.h"
#include"Manager.h"
#include"sprite.h"
#include"keyboard.h"
#include"fade.h"
#include"shader.h"
#include "player.h"
#include "player2.h"
#include "Audio.h"
#include <random>
#include <cmath>
#include <algorithm>
#pragma region "アニメーション用関数"
namespace
{
    inline float Clamp01(float v) { return (v < 0.0f) ? 0.0f : (v > 1.0f ? 1.0f : v); }

	// ０－＞１に対してコサインイージング
    inline float EaseCos01(float t)
    {
        t = Clamp01(t);
        return 0.5f - 0.5f * cosf(t * XM_PI);
    }

    struct CardAnimPerCard
    {
        float focus01 = 0.0f;     // 0=unfocus, 1=focus
        float from = 0.0f;
        float to = 0.0f;
        float t = 1.0f;          // progress 0..1
    };

    struct CardAnimState
    {
        // appear animation
        float appearT = 1.0f;
        float appearDur = 0.77f;
        float appearOffsetY = 40.0f;

        // selection pop
        bool  popActive = false;
        float popT = 1.0f;
        float popDur = 0.22f;
        int   lastReady = 0;
        bool  fadeOthersActive = false;
        float fadeOthersT = 1.0f;     // 0..1
        float fadeOthersDur = 0.35f;  // ← フェード速度（遅くしたいなら増やす）
        int lastFocused = 0;
        CardAnimPerCard card[2];
    };

    static CardAnimState g_CardAnim[2];

    inline void StartFocusAnim(CardAnimPerCard& c, float target)
    {
        target = Clamp01(target);
        c.from = c.focus01;
        c.to = target;
        c.t = 0.0f;
    }

    inline void StepFocusAnim(CardAnimPerCard& c, float dt, float dur)
    {
        if (c.t >= 1.0f) return;
        c.t += (dur <= 0.0f) ? 1.0f : (dt / dur);
        float e = EaseCos01(c.t);
        c.focus01 = c.from + (c.to - c.from) * e;
    }

    inline float Lerp(float a, float b, float t) { return a + (b - a) * t; }
    inline XMFLOAT2 Lerp2(const XMFLOAT2& a, const XMFLOAT2& b, float t)
    {
        return XMFLOAT2(Lerp(a.x, b.x, t), Lerp(a.y, b.y, t));
    }
}
#pragma endregion
//================================================================
//	グローバル変数
//================================================================
static	ID3D11ShaderResourceView* g_TextureBg[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureNumber = NULL;
static	ID3D11ShaderResourceView* g_TextureTimer[2] = { NULL };
static	ID3D11ShaderResourceView* g_TextureButton = NULL;
static	ID3D11ShaderResourceView* g_TextureEffect = NULL;

//================================================================
//	初期化
//================================================================
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;
SELECT_TRANSFORM_UI g_Ui[2];
SELECT_TRANSFORM_TIMER g_Timer;


static std::mt19937 g_Rng;
static std::uniform_real_distribution<float> g_Dist01(0.0f, 1.0f);
void SelectTransformUi_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;

    std::random_device rd;
    g_Rng.seed(rd());

    TexMetadata		metadata;
    ScratchImage	image;
    LoadFromWICFile(L"asset\\texture\\Transform_Timer.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTimer[0]);
    assert(&g_TextureTimer[0]);

    LoadFromWICFile(L"asset\\texture\\Transform_Timer2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureTimer[1]);
    assert(&g_TextureTimer[1]);

    LoadFromWICFile(L"asset\\texture\\Transform_Select.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBg[0]);
    assert(&g_TextureBg[0]);

    LoadFromWICFile(L"asset\\texture\\Transform_Select2.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBg[1]);
    assert(&g_TextureBg[1]);

    LoadFromWICFile(L"asset\\texture\\number.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureNumber);
    assert(&g_TextureNumber);

    LoadFromWICFile(L"asset\\texture\\choose_button.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
    CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureButton);
    assert(&g_TextureButton);


    for (int i = 0; i < 2; i++)
    {
        g_Ui[i].col = { 1.0f, 1.0f, 1.0f, 1.0f };
        g_Ui[i].pos = { 0, 0 };
        g_Ui[i].size = { 1920,1080 };
        g_Ui[i].isUsed = false;
        g_Ui[i].SelectNum = 0;
        g_Ui[i].Texcoord = XMFLOAT2(0.0f, 0.0f);

    }

    

	g_Timer.col = { 1.0f, 1.0f, 1.0f, 1.0f };
	g_Timer.pos = { 0, 0 };
	g_Timer.size = { 300, 120 };
	g_Timer.time = 20.0f;
	g_Timer.frame = 0.0f;

}
void SelectTransformUi_Finalize()
{
    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_TextureBg[i]);
		SAFE_RELEASE(g_TextureTimer[i]);
    }
    SAFE_RELEASE(g_TextureNumber);
    SAFE_RELEASE(g_TextureButton);
    SAFE_RELEASE(g_TextureEffect);

}
void SelectTransformUi_Update()
{
    for (int i = 0; i < 2; i++)
    {
        if (g_Ui[i].isUsed)
        {
            if (g_Ui[i].Texcoord.x == 0.0f)
            {
                PlayAudio(g_select_transform, false);
            }
            const float SCREEN_WIDTH = (float)Direct3D_GetBackBufferWidth();
            const float SCREEN_HEIGHT = (float)Direct3D_GetBackBufferHeight();

            if (g_Ui[i].Texcoord.x >= 1.0f)
            {
                g_Ui[i].Texcoord.x = 1.0f;
                g_Ui[i].Texcoord.y = 1.0f;
          

            }
            else
            {
                g_Ui[i].Texcoord.x += 0.1f;
                g_Ui[i].Texcoord.y += 0.1f;
                g_Ui[i].size = XMFLOAT2(SCREEN_WIDTH * g_Ui[i].Texcoord.x, (SCREEN_HEIGHT + 50) * g_Ui[i].Texcoord.y);
                g_Ui[i].pos = XMFLOAT2((SCREEN_WIDTH / 2) * g_Ui[i].Texcoord.x, (SCREEN_HEIGHT / 2 - 25) * g_Ui[i].Texcoord.x);

            }
           
        }      
    }
   
  
}
void SelectTransformUi_Draw()
{
    for (int i = 0; i < 2; i++)
    {
        if (g_Ui[i].isUsed)
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
            g_pContext->PSSetShaderResources(0, 1, &g_TextureTimer[g_Ui[i].SelectNum]);
            SetBlendState(BLENDSTATE_ALFA);
            DrawSprite(g_Timer.pos, g_Timer.size, g_Timer.col);



            g_pContext->PSSetShaderResources(0, 1, &g_TextureBg[g_Ui[i].SelectNum]);
            SetBlendState(BLENDSTATE_ALFA);
            DrawSpriteAnimation(g_Ui[i].pos, g_Ui[i].size, g_Ui[i].col, g_Ui[i].Texcoord);





            g_pContext->PSSetShaderResources(0, 1, &g_TextureButton);
            SetBlendState(BLENDSTATE_ALFA);
            DrawSprite(XMFLOAT2(SCREEN_WIDTH / 2 - 100, SCREEN_HEIGHT - 100), XMFLOAT2(263, 100), g_Ui[i].col);

            //数字描画
            int time[2];
            time[0] = (int)(g_Timer.time) / 10 - 1;
            time[1] = (int)(g_Timer.time) % 10 - 1;
            float timerScale = 156 * 0.8f;
            for (int i = 0; i < 2; i++)
            {
                g_Timer.pos = XMFLOAT2(SCREEN_WIDTH / 2 - timerScale / 4 + i * timerScale / 2, 100);
                g_pContext->PSSetShaderResources(0, 1, &g_TextureNumber);
                SetBlendState(BLENDSTATE_ALFA);
                DrawSpriteEx(g_Timer.pos, XMFLOAT2(timerScale, timerScale), g_Timer.col, time[i], 10, 1);
            }
        }
    }
}

void SetTransformUi_IsUsed(bool flg,int num)
{
    g_Ui[num].isUsed = flg;
    if (flg)
    {
        SelectTransformUi_CardAnim_Reset(num);
        
    }
}
void SetTransformUi_time(float time)
{
	g_Timer.time = time;
}
void SetTransformUi_SelectNum(int num)
{
	g_Ui[num].SelectNum = num;
}

void SelectTransformUi_CardAnim_Reset(int playerIndex)
{
    if (playerIndex < 0 || playerIndex > 1) return;
    auto& st = g_CardAnim[playerIndex];
    st.fadeOthersActive = false;
    st.fadeOthersT = 1.0f;
    st.appearT = 0.0f;
    st.popActive = false;
    st.popT = 1.0f;
    st.lastReady = 0;

    st.lastFocused = 0;

    for (int i = 0; i < 2; i++)
    {
        st.card[i].focus01 = 0.0f;
        st.card[i].from = 0.0f;
        st.card[i].to = 0.0f;
        st.card[i].t = 1.0f;
    }
    // default focus on left card
    StartFocusAnim(st.card[0], 1.0f);
    StartFocusAnim(st.card[1], 0.0f);
   
}
void SelectTransformUi_CardAnim_Update(int playerIndex, int focusedIndex, bool isReady, float deltaTime)
{
    if (playerIndex < 0 || playerIndex > 1) return;
    auto& st = g_CardAnim[playerIndex];

    // 現れる時のアニメ
    if (st.appearT < 1.0f)
    {
        st.appearT += (st.appearDur <= 0.0f) ? 1.0f : (deltaTime / st.appearDur);
        if (st.appearT > 1.0f) st.appearT = 1.0f;
    }

    // 選択を変える
    focusedIndex = (focusedIndex == 1) ? 1 : 0;
    if (focusedIndex != st.lastFocused)
    {
        StartFocusAnim(st.card[0], (focusedIndex == 0) ? 1.0f : 0.0f);
        StartFocusAnim(st.card[1], (focusedIndex == 1) ? 1.0f : 0.0f);
        st.lastFocused = focusedIndex;
    }

    // スムーズに大きくなる
    const float focusDur = 0.18f;
    StepFocusAnim(st.card[0], deltaTime, focusDur);
    StepFocusAnim(st.card[1], deltaTime, focusDur);

    
    int readyNow = isReady ? 1 : 0;
    if (readyNow == 1 && st.lastReady == 0)
    {
        // pop for selected card
        st.popActive = true;
        st.popT = 0.0f;

        // fade-out for other card
        st.fadeOthersActive = true;
        st.fadeOthersT = 0.0f;
    }
    st.lastReady = readyNow;

    // step fade-out timer
    if (st.fadeOthersActive)
    {
        st.fadeOthersT += (st.fadeOthersDur <= 0.0f) ? 1.0f : (deltaTime / st.fadeOthersDur);
        if (st.fadeOthersT >= 1.0f)
        {
            st.fadeOthersT = 1.0f;
            st.fadeOthersActive = false; // 終了（フェード値は保持される）
        }
    }

    if (st.popActive)
    {
        st.popT += (st.popDur <= 0.0f) ? 1.0f : (deltaTime / st.popDur);
        if (st.popT >= 1.0f)
        {
            st.popT = 1.0f;
            st.popActive = false;
        }
    }
}

SELECT_TRANSFORM_CARD SelectTransformUi_GetCardParam(
    int playerIndex,
    int cardIndex,
    XMFLOAT2 basePos,
    float cardSpacing,
    XMFLOAT2 sizeUnfocus,
    XMFLOAT2 sizeFocus)
{
    SELECT_TRANSFORM_CARD out{};
    out.col = XMFLOAT4(1, 1, 1, 1);

    if (playerIndex < 0 || playerIndex > 1) playerIndex = 0;
    if (cardIndex < 0 || cardIndex > 1) cardIndex = 0;

    auto& st = g_CardAnim[playerIndex];

    // base position: left/right
    out.pos = XMFLOAT2(basePos.x + (cardIndex == 0 ? -cardSpacing : cardSpacing), basePos.y);

    // appear effect (fade + from bottom)
    float a = EaseCos01(EaseCos01(st.appearT));
    float y = Lerp(st.appearOffsetY, 0.0f, a);
    out.pos.y += y;
    out.col.w *= a;

    // focus smooth sizing
    float f01 = st.card[cardIndex].focus01;
    out.size = Lerp2(sizeUnfocus, sizeFocus, f01);

    // selection pop: only focused card pops
    if (st.popActive && cardIndex == st.lastFocused)
    {
        float p = Clamp01(st.popT);
        float factor = 1.0f;

        if (p < 0.5f)
        {
            float e = EaseCos01(p * 2.0f);
            factor = Lerp(1.0f, 0.82f, e);
        }
        else
        {
            float e = EaseCos01((p - 0.5f) * 2.0f);
            factor = Lerp(0.82f, 1.18f, e);
        }
        out.size = XMFLOAT2(out.size.x * factor, out.size.y * factor);
    }
    if ((st.fadeOthersActive || st.lastReady == 1) && cardIndex != st.lastFocused)
    {
        // 0 -> 1 で fade進行。最後は完全に消すと見づらければ 0.2f くらい残してもOK
        float f = EaseCos01(st.fadeOthersT);        // 0..1
        float alphaMul = Lerp(1.0f, 0.0f, f);       // 最終0.0（完全消し）
        // float alphaMul = Lerp(1.0f, 0.2f, f);    // ← ちょい残し版（おすすめ）
        out.col.w *= alphaMul;
    }
    return out;
}