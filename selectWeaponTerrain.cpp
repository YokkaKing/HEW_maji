//============================================
// selectWeaponTerrain.cpp
// 
// 武器/地形セレクト画面
// 作成:2026/1/10
// 更新:2026/1/21
// 更新:2026/01/24 - カーソル移動/スケールアニメ追加
//============================================
#include "selectWeaponTerrain.h"
#include "Manager.h"
#include "keyboard.h"
#include "fade.h"
#include "shader.h"
#include "Camera.h"
#include "model.h"
#include <cmath>

// 選択スロット数
static const int selectCount = 5;
#pragma region UI関連定数
static ID3D11ShaderResourceView* g_TextureBG[2] = { NULL };	// 背景テクスチャ
static ID3D11ShaderResourceView* g_TextureUi_Card[3] = { NULL };
static ID3D11ShaderResourceView* g_TextureUi_Cursor[2] = { NULL };
static ID3D11ShaderResourceView* g_TextureUi_Button[2] = { NULL };
static ID3D11ShaderResourceView* g_TextureUI[selectCount] = { NULL };
static ID3D11Device* g_pDevice = nullptr;
static ID3D11DeviceContext* g_pContext = nullptr;

static inGameWTselect g_selectData;
static int g_cursorP1 = 0;
static int g_cursorP2 = 0;

static bool g_isP1Ready = false;
static bool g_isP2Ready = false;

// selectBG_3 用の SRV（左回転 / 右回転）
static ID3D11ShaderResourceView* g_TextureBG3_Left = nullptr;
static ID3D11ShaderResourceView* g_TextureBG3_Right = nullptr;
// 回転後のテクスチャの幅・高さ（ピクセル）を保持
static size_t g_BG3_LeftWidth = 0, g_BG3_LeftHeight = 0;
static size_t g_BG3_RightWidth = 0, g_BG3_RightHeight = 0;
// スクロールオフセット（ピクセル単位、左は上方向に増加、右は下方向に増加）
static float g_bg3OffsetLeft = 0.0f;
static float g_bg3OffsetRight = 0.0f;
// スクロール速度（ピクセル/秒）
static float g_bg3SpeedLeft = 80.0f;  // 整可
static float g_bg3SpeedRight = 80.0f; // 調整可


static MODEL* g_CenterSwordModel = nullptr;
// ------------------ カーソル / スケールアニメ用データ ------------------
// 各プレイヤー用カーソル状態
struct CursorState
{
    // 現在のX位置（ピクセル）と目標インデックス
    float posX = 0.0f;
    int index = 0;

    // 移動アニメーション
    bool moving = false;
    float moveTime = 0.0f;
    float moveDuration = 0.28f; // カーソル移動にかける秒数（調整可）
    float startX = 0.0f;
    float targetX = 0.0f;

    // スロットごとのスケールアニメ（0..1 を進める）
    // 選択されたスロットのみアニメを開始する方式
    // 実装は slotScale の値を直接保持
};
static CursorState g_cursorState[2];

static float g_cursorScale[2] = { 1.0f, 1.0f };
static bool  g_cursorScaleAnim[2] = { false, false };
static float g_cursorScaleTime[2] = { 0.0f, 0.0f };
static const float g_cursorScaleDuration = 0.1f; 
static const float g_cursorScaleMin = 0.70f; 
// 各スロットのスケール値 (1.0 = base, >1 = 拡大)、アニメは slotAnimProgress/dirで制御
static float g_slotScale[selectCount] = { 1.0f, 1.0f, 1.0f, 1.0f, 1.0f };
static bool  g_slotAnimating[selectCount] = { false };
static float g_slotAnimTime[selectCount] = { 0.0f };
static float g_slotAnimDuration = 0.20f; // スケールアニメーション時間 (秒)
static const float g_slotBaseScale = 1.0f;
static const float g_slotSelectedScale = 1.10f; // 選択時の最終スケール
MODEL* model;
// 固定: スロット配置関連
static float g_slotStartX = 600.0f;
static float g_slotSpacing = 170.0f;
static float g_slotPosY = 0.0f;

// フレーム時間（現在は固定 1/60f。実運用では deltaTime を渡すことを推奨）
static const float FRAME_DT = 1.0f / 60.0f;

// コサイン（または sin を用いた）イージング: 始めは速く、終わりで遅くなる ease-out (t: 0..1)
static inline float EaseOutCos(float t)
{
    if (t <= 0.0f) return 0.0f;
    if (t >= 1.0f) return 1.0f;
    // sin(t * PI/2) は開始が急で終わりでゆっくり
    return sinf(t * (XM_PI / 2.0f));
}

// スロットインデックス -> X 座標計算
static inline float SlotIndexToX(int index)
{
    return g_slotStartX + (index * g_slotSpacing);
}

// スロットのスケールアニメを開始する (grow==true で拡大, false で縮小)
static void StartSlotScaleAnim(int slotIndex, bool grow)
{
    if (slotIndex < 0 || slotIndex >= selectCount) return;
    g_slotAnimating[slotIndex] = true;
    g_slotAnimTime[slotIndex] = 0.0f;
    // 実際の補間は update 側で行う
}
// ------------------ スプライトアニメ用データ ------------------
// スプライトシート (sword idle / attack) 用 SRV
static ID3D11ShaderResourceView* g_TextureSwordIdle = nullptr;
static ID3D11ShaderResourceView* g_TextureSwordAttack = nullptr;
const int SWORD_SLOT_INDEX = 0;
// 単純なスプライトアニメ再生機構
struct SpriteAnim
{
    ID3D11ShaderResourceView* srv = nullptr; // SRV
    int cols = 1;     // 横フレーム数 (wc)
    int rows = 1;     // 縦フレーム数 (hc)
    int frameCount = 1; // 総フレーム数
    float frameTime = 0.1f; // 1フレームの秒数
    float acc = 0.0f;  // 経過時間蓄積
    int curFrame = 0;  // 現在のフレームインデックス
    bool loop = true;  // ループ再生するか
    bool playing = false; // 再生中フラグ

    // ループ再生開始
    void PlayLoop(int startFrame = 0)
    {
        loop = true;
        curFrame = startFrame;
        acc = 0.0f;
        playing = true;
    }
    // 1回再生（終了後 playing=false になる）
    void PlayOnce(int startFrame = 0)
    {
        loop = false;
        curFrame = startFrame;
        acc = 0.0f;
        playing = true;
    }
    // 停止
    void Stop()
    {
        playing = false;
        acc = 0.0f;
    }
    // 更新
    void Update(float dt)
    {
        if (!playing) return;
        acc += dt;
        while (acc >= frameTime)
        {
            acc -= frameTime;
            curFrame++;
            if (curFrame >= frameCount)
            {
                if (loop)
                {
                    curFrame = 0;
                }
                else
                {
                    curFrame = frameCount - 1;
                    playing = false;
                    break;
                }
            }
        }
    }
    // 現在のブロック番号（bno）取得
    int GetBno() const { return curFrame; }
};

// プレイヤーごとにアニメを保持（0 = P1, 1 = P2）
static SpriteAnim g_swordIdleAnim[2];
static SpriteAnim g_swordAttackAnim[2];
// 攻撃再生中フラグ（アニメーションが終わるまで Ready にしない）
static bool g_p1AttackPlaying = false;
static bool g_p2AttackPlaying = false;

// スプライトシートのレイアウト（必要に応じて変更してください）
static const int SWORD_IDLE_WC = 5;   // idle シートの列数
static const int SWORD_IDLE_HC = 6;   // idle シートの行数
static const int SWORD_IDLE_FRAMES = 26; // idle 総フレーム数
static const float SWORD_IDLE_FRAME_TIME = 1.0f / 30.0f; // 12 fps

static const int SWORD_ATTACK_WC = 5;
static const int SWORD_ATTACK_HC = 5;
static const int SWORD_ATTACK_FRAMES = 22;
static const float SWORD_ATTACK_FRAME_TIME = 1.0f / 30.0f; // 24 fps (例)



static ID3D11ShaderResourceView* g_TextureGoBg = nullptr;
static ID3D11ShaderResourceView* g_TextureGoBtn = nullptr;
enum GO_STATE
{
    GO_NONE = 0,
    GO_ANIMATING,
    GO_WAIT_FOR_A
};
static int g_goState = GO_NONE;
static float g_goBtnX = 0.0f;
static float g_goBtnY = 0.0f;
static float g_goBtnStartX = 0.0f;
static float g_goBtnTargetX = 0.0f;
static float g_goAnimDuration = 0.12f; 
static float g_goAnimTime = 0.0f;
static XMFLOAT2 g_goBtnSize = XMFLOAT2(500.0f*2, 231.0f * 2);
#pragma endregion
// ------------------ 初期化 ------------------
void selectWT_Initialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    g_pDevice = pDevice;
    g_pContext = pContext;
    g_isP1Ready = false;
    g_isP2Ready = false;
    g_p1AttackPlaying = false;
    g_p2AttackPlaying = false;

#pragma region スプライトアニメ初期化
    for (int p = 0; p < 2; ++p)
    {
        g_cursorScale[p] = 1.0f;
        g_cursorScaleAnim[p] = false;
        g_cursorScaleTime[p] = 0.0f;
    }
    g_cursorP1 = 0;
    g_cursorP2 = 0;
    g_cursorState[0].posX = SlotIndexToX(g_cursorP1);
    g_cursorState[0].index = g_cursorP1;
    g_cursorState[1].posX = SlotIndexToX(g_cursorP2);
    g_cursorState[1].index = g_cursorP2;
    g_goState = GO_NONE;
    g_goAnimTime = 0.0f;
    g_goBtnX = 0.0f;
    g_goBtnY = 0.0f;
    g_goBtnStartX = 0.0f;
    g_goBtnTargetX = 0.0f;
    for (int p = 0; p < 2; ++p)
    {
        g_swordIdleAnim[p].playing = false;
        g_swordIdleAnim[p].acc = 0.0f;
        g_swordIdleAnim[p].curFrame = 0;

        g_swordAttackAnim[p].playing = false;
        g_swordAttackAnim[p].acc = 0.0f;
        g_swordAttackAnim[p].curFrame = 0;
    }
#pragma endregion
#pragma region スロットスケール初期化
    // 基本セットアップ
    g_cursorP1 = 1;
    g_cursorP2 = 1;
    g_isP1Ready = false;
    g_isP2Ready = false;
    g_selectData.player1 = WeaponTerrain::SWORD_WALL;
    g_selectData.player2 = WeaponTerrain::SWORD_WALL;
    // スロット配置の Y を計算（バックバッファ高さを参照）
    float screenHeight = (float)Direct3D_GetBackBufferHeight();
    g_slotPosY = screenHeight * 0.85f;

    // 初期カーソル位置をスロット位置に設定
    g_cursorState[0].posX = SlotIndexToX(g_cursorP1);
    g_cursorState[0].index = g_cursorP1;
    g_cursorState[1].posX = SlotIndexToX(g_cursorP2);
    g_cursorState[1].index = g_cursorP2;

    // スロットスケール初期化
    for (int i = 0; i < selectCount; ++i)
    {
        g_slotScale[i] = g_slotBaseScale;
        g_slotAnimating[i] = false;
        g_slotAnimTime[i] = 0.0f;
    }
    // 初回選択はカーソルインデックスに対し選択スケールをセット
    g_slotScale[g_cursorP1] = g_slotSelectedScale;
    g_slotScale[g_cursorP2] = g_slotSelectedScale;
#pragma endregion
    // UI テクスチャ読み込み（既存コードをそのまま）
#pragma region UIテクスチャ読み込み
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\SelectBg_1.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBG[0]);
        assert(g_TextureBG[0]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\SelectBg_2.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureBG[1]);
        assert(g_TextureBG[1]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\select_card.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Card[0]);
        assert(g_TextureUi_Card[0]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\select_card2.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Card[1]);
        assert(g_TextureUi_Card[1]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\select_weapon_Bg.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Card[2]);
        assert(g_TextureUi_Card[2]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\1p_cursor.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Cursor[0]);
        assert(g_TextureUi_Cursor[0]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\2p_cursor.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Cursor[1]);
        assert(g_TextureUi_Cursor[1]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\select_button.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Button[0]);
        assert(g_TextureUi_Button[0]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\cancel_button.PNG", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUi_Button[1]);
        assert(g_TextureUi_Button[1]);
    }

    TexMetadata metadata;
    ScratchImage srcImage;
    HRESULT hr = LoadFromWICFile(L"asset\\texture\\selectBG_3.png", WIC_FLAGS_FORCE_SRGB, &metadata, srcImage);
    // 左用: -90度 -> 270度回転（TEX_FR_ROTATE270）
    ScratchImage leftImg;
    // FlipRotate のシグネチャは DirectXTex のバージョンによって異なる可能性あり
    // ここでは典型的な FlipRotate( images, count, metadata, flag, dest ) を想定
    FlipRotate(srcImage.GetImages(), srcImage.GetImageCount(), metadata, TEX_FR_ROTATE270, leftImg);
    TexMetadata leftMeta = leftImg.GetMetadata();
    g_BG3_LeftWidth = leftMeta.width;
    g_BG3_LeftHeight = leftMeta.height;
    CreateShaderResourceView(pDevice, leftImg.GetImages(), leftImg.GetImageCount(), leftMeta, &g_TextureBG3_Left);

    // 右用: +90度回転
    ScratchImage rightImg;
    FlipRotate(srcImage.GetImages(), srcImage.GetImageCount(), metadata, TEX_FR_ROTATE90, rightImg);
    TexMetadata rightMeta = rightImg.GetMetadata();
    g_BG3_RightWidth = rightMeta.width;
    g_BG3_RightHeight = rightMeta.height;
    CreateShaderResourceView(pDevice, rightImg.GetImages(), rightImg.GetImageCount(), rightMeta, &g_TextureBG3_Right);
    g_bg3OffsetLeft = 0.0f;
    g_bg3OffsetRight = 0.0f;


    // 武器アイコン読み込み
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\sword.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[0]);
        assert(g_TextureUI[0]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\spear.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[1]);
        assert(g_TextureUI[1]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\bow.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[2]);
        assert(g_TextureUI[2]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\hammer.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[3]);
        assert(g_TextureUI[3]);
    }
    {
        TexMetadata		metadata;
        ScratchImage	image;
        LoadFromWICFile(L"asset\\texture\\shuriken.png", WIC_FLAGS_FORCE_SRGB, &metadata, image);
        CreateShaderResourceView(pDevice, image.GetImages(), image.GetImageCount(), metadata, &g_TextureUI[4]);
        assert(g_TextureUI[4]);
    }
#pragma endregion
	#pragma region スプライトシート読み込み
    // ====== スプライトシート読み込み（sword idle / attack） ======
    {
        TexMetadata metaIdle;
        ScratchImage imgIdle;
        // ファイル名は実際のものに合わせてください
        HRESULT r = LoadFromWICFile(L"asset\\texture\\sword_idle.png", WIC_FLAGS_FORCE_SRGB, &metaIdle, imgIdle);
        if (SUCCEEDED(r))
        {
            CreateShaderResourceView(pDevice, imgIdle.GetImages(), imgIdle.GetImageCount(), metaIdle, &g_TextureSwordIdle);
        }
    }
    {
        TexMetadata metaAtk;
        ScratchImage imgAtk;
        HRESULT r = LoadFromWICFile(L"asset\\texture\\sword_attack.png", WIC_FLAGS_FORCE_SRGB, &metaAtk, imgAtk);
        if (SUCCEEDED(r))
        {
            CreateShaderResourceView(pDevice, imgAtk.GetImages(), imgAtk.GetImageCount(), metaAtk, &g_TextureSwordAttack);
        }
    }
	// ====== GO ボタン関連テクスチャ読み込み ======
    {
       
        TexMetadata metaGo;
        ScratchImage imgGo;
        HRESULT rgo = LoadFromWICFile(L"asset\\texture\\go_bg.png", WIC_FLAGS_FORCE_SRGB, &metaGo, imgGo);
        if (SUCCEEDED(rgo))
        {
            CreateShaderResourceView(pDevice, imgGo.GetImages(), imgGo.GetImageCount(), metaGo, &g_TextureGoBg);
        }
       
        TexMetadata metaBtn;
        ScratchImage imgBtn;
        HRESULT rbtn = LoadFromWICFile(L"asset\\texture\\GO_BUTTON.PNG", WIC_FLAGS_FORCE_SRGB, &metaBtn, imgBtn);
        if (SUCCEEDED(rbtn))
        {
            CreateShaderResourceView(pDevice, imgBtn.GetImages(), imgBtn.GetImageCount(), metaBtn, &g_TextureGoBtn);
          
        }
    }

#pragma endregion
    // スプライトアニメの初期設定
    for (int p = 0; p < 2; ++p)
    {
        // idle
        g_swordIdleAnim[p].srv = g_TextureSwordIdle;
        g_swordIdleAnim[p].cols = SWORD_IDLE_WC;
        g_swordIdleAnim[p].rows = SWORD_IDLE_HC;
        g_swordIdleAnim[p].frameCount = SWORD_IDLE_FRAMES;
        g_swordIdleAnim[p].frameTime = SWORD_IDLE_FRAME_TIME;
        g_swordIdleAnim[p].PlayLoop(0);

        // attack
        g_swordAttackAnim[p].srv = g_TextureSwordAttack;
        g_swordAttackAnim[p].cols = SWORD_ATTACK_WC;
        g_swordAttackAnim[p].rows = SWORD_ATTACK_HC;
        g_swordAttackAnim[p].frameCount = SWORD_ATTACK_FRAMES;
        g_swordAttackAnim[p].frameTime = SWORD_ATTACK_FRAME_TIME;
        g_swordAttackAnim[p].Stop();
    }

    // 3Dプレビューモデルは不要なら読み込まないが念のため NULL に
    g_CenterSwordModel = nullptr;
    // フェードイン
    XMFLOAT4 color = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
    SetFade(60.0f, color, FADE_IN, SCENE_GAME);
}

// ------------------ 終了処理 ------------------
void selectWT_Finalize()
{
  
    ID3D11ShaderResourceView* nullSRV[16] = { nullptr };
    if (g_pContext) g_pContext->PSSetShaderResources(0, 16, nullSRV);
    for (int i = 0; i < 2; i++)
    {
        SAFE_RELEASE(g_TextureBG[i]);
        SAFE_RELEASE(g_TextureUi_Cursor[i]);
        SAFE_RELEASE(g_TextureUi_Button[i]);
    }
    for (int i = 0; i < 3; i++)
    {
        SAFE_RELEASE(g_TextureUi_Card[i]);
    }

    SAFE_RELEASE(g_TextureBG3_Left);
    SAFE_RELEASE(g_TextureBG3_Right);

    for (int i = 0; i < selectCount; i++)
    {
        SAFE_RELEASE(g_TextureUI[i]);
    }

    SAFE_RELEASE(g_TextureSwordIdle);
    SAFE_RELEASE(g_TextureSwordAttack);

    SAFE_RELEASE(g_TextureGoBg);
    SAFE_RELEASE(g_TextureGoBtn);

    if (g_CenterSwordModel)
    {
        ModelRelease(g_CenterSwordModel);
        g_CenterSwordModel = nullptr;
    }
    g_isP1Ready = false;
    g_isP2Ready = false;
    g_goState = GO_NONE;
    g_goAnimTime = 0.0f;
}

// ------------------ 更新処理 ------------------
void selectWT_Update()
{
#pragma region カーソル移動アニメ更新(1P)
    float dt = FRAME_DT;

    if (!g_p1AttackPlaying)
    {
        // If selected slot is sword and sword idle exists -> keep original sword idle/attack behavior
        if (g_cursorP1 == SWORD_SLOT_INDEX && g_swordIdleAnim[0].srv)
        {
            if (!g_swordIdleAnim[0].playing) g_swordIdleAnim[0].PlayLoop(0);

            if (Keyboard_IsKeyDownTrigger(KK_LEFTCONTROL) && !g_isP1Ready)
            {
                g_selectData.player1 = static_cast<WeaponTerrain>(g_cursorP1 + 1);
                // sword-specific attack playback
                if (!g_p1AttackPlaying)
                {
                    g_swordAttackAnim[0].PlayOnce(0);
                    g_p1AttackPlaying = true;
                    g_swordIdleAnim[0].Stop();

                    // cursor scale anim
                    g_cursorScaleAnim[0] = true;
                    g_cursorScaleTime[0] = 0.0f;
                }
            }
            if (Keyboard_IsKeyDownTrigger(KK_Z))
            {
                g_p1AttackPlaying = false;
                g_isP1Ready = false;
                g_swordAttackAnim[0].Stop();
                g_swordIdleAnim[0].Stop();
                g_swordIdleAnim[0].PlayLoop(0);

                g_cursorScaleAnim[0] = false;
                g_cursorScale[0] = 1.0f;
                g_cursorScaleTime[0] = 0.0f;
            }
        }
        else
        {
            // Non-sword slot: stop any sword idle to avoid conflicts
            if (g_swordIdleAnim[0].playing) g_swordIdleAnim[0].Stop();

            // Selection for any weapon: start cursor scale anim and mark ready
            if (Keyboard_IsKeyDownTrigger(KK_LEFTCONTROL)&& !g_isP1Ready)
            {
                // start cursor animation
                g_cursorScaleAnim[0] = true;
                g_cursorScaleTime[0] = 0.0f;

                // mark ready and save choice
                g_isP1Ready = true;
                g_selectData.player1 = static_cast<WeaponTerrain>(g_cursorP1+1);
            }

    
            if (Keyboard_IsKeyDownTrigger(KK_Z))
            {
                if (g_isP1Ready)
                {
                    g_isP1Ready = false;
                    // optional: reset choice -> uncomment if you want to clear selection
                    // g_selectData.player1 = WeaponTerrain::SWORD_WALL;

                    g_cursorScaleAnim[0] = false;
                    g_cursorScale[0] = 1.0f;
                    g_cursorScaleTime[0] = 0.0f;
                }
            }
        }
    }
#pragma endregion
#pragma region カーソル移動アニメ更新(2P)
    // --- Player2 selection / cancel (generalized for any slot) ---
    if (!g_p2AttackPlaying)
    {
        if (g_cursorP2 == SWORD_SLOT_INDEX && g_swordIdleAnim[1].srv)
        {
            if (!g_swordIdleAnim[1].playing) g_swordIdleAnim[1].PlayLoop(0);

            if (Keyboard_IsKeyDownTrigger(KK_D5) && !g_isP2Ready)
            {
                if (!g_p2AttackPlaying)
                {
                    g_swordAttackAnim[1].PlayOnce(0);
                    g_p2AttackPlaying = true;
                    g_swordIdleAnim[1].Stop();

                    g_cursorScaleAnim[1] = true;
                    g_cursorScaleTime[1] = 0.0f;
                }
            }

            if (Keyboard_IsKeyDownTrigger(KK_D6))
            {
                g_p2AttackPlaying = false;
                g_isP2Ready = false;
                g_swordAttackAnim[1].Stop();
                g_swordIdleAnim[1].Stop();
                g_swordIdleAnim[1].PlayLoop(0);

                g_cursorScaleAnim[1] = false;
                g_cursorScale[1] = 1.0f;
                g_cursorScaleTime[1] = 0.0f;
            }
        }
        else
        {
            // Non-sword slot for player2
            if (g_swordIdleAnim[1].playing) g_swordIdleAnim[1].Stop();

            if (Keyboard_IsKeyDownTrigger(KK_D5) && !g_isP2Ready)
            {
                // start cursor anim and mark ready for any weapon
                g_cursorScaleAnim[1] = true;
                g_cursorScaleTime[1] = 0.0f;

                g_isP2Ready = true;
                g_selectData.player2 = static_cast<WeaponTerrain>(g_cursorP2 +1);
            }

            // cancel for any weapon (D6 or DELETE)
            if (Keyboard_IsKeyDownTrigger(KK_D6) || Keyboard_IsKeyDownTrigger(KK_DELETE))
            {
                if (g_isP2Ready)
                {
                    g_isP2Ready = false;
                    // optional reset:
                    // g_selectData.player2 = WeaponTerrain::SWORD_WALL;

                    g_cursorScaleAnim[1] = false;
                    g_cursorScale[1] = 1.0f;
                    g_cursorScaleTime[1] = 0.0f;
                }
            }
        }
    }
#pragma endregion
#pragma region アニメ更新(1P)
    // アニメ更新（攻撃アニメ優先）
    // P1
    if (g_p1AttackPlaying)
    {
        g_swordAttackAnim[0].Update(dt);
        if (!g_swordAttackAnim[0].playing)
        {
            // 攻撃アニメ終了 -> Ready にして idle に戻す
            g_p1AttackPlaying = false;
            g_isP1Ready = true;
            g_selectData.player1 = static_cast<WeaponTerrain>(g_cursorP1+1);
            g_swordIdleAnim[0].PlayLoop(0);
        }
    }
    else
    {
        g_swordIdleAnim[0].Update(dt);
    }
#pragma endregion
#pragma region アニメ更新(2P)
    // P2
    if (g_p2AttackPlaying)
    {
        g_swordAttackAnim[1].Update(dt);
        if (!g_swordAttackAnim[1].playing)
        {
            g_p2AttackPlaying = false;
            g_isP2Ready = true;
            g_selectData.player2 = static_cast<WeaponTerrain>(g_cursorP2+1);
            g_swordIdleAnim[1].PlayLoop(0);
        }
    }
    else
    {
        g_swordIdleAnim[1].Update(dt);
    }
#pragma endregion
#pragma region スロットスケールアニメ更新
    // --- カーソルスケールアニメ ---
    for (int p = 0; p < 2; ++p)
    {
        if (g_cursorScaleAnim[p])
        {
            g_cursorScaleTime[p] += dt;
            float t = g_cursorScaleTime[p] / g_cursorScaleDuration;
            if (t >= 1.0f)
            {
                g_cursorScaleAnim[p] = false;
                g_cursorScale[p] = 1.0f;
                g_cursorScaleTime[p] = 0.0f;
            }
            else
            {
                // 앞 50%: 축소(1.0 -> min), 뒤 50%: 복구(min -> 1.0)
                if (t < 0.5f)
                {
                    float tt = t / 0.5f; // 0..1
                    float e = EaseOutCos(tt); // ease
                    g_cursorScale[p] = 1.0f + (g_cursorScaleMin - 1.0f) * e;
                }
                else
                {
                    float tt = (t - 0.5f) / 0.5f; // 0..1
                    float e = EaseOutCos(tt);
                    g_cursorScale[p] = g_cursorScaleMin + (1.0f - g_cursorScaleMin) * e;
                }
            }
        }
    }
#pragma endregion

#pragma region 背景スクロール
    // 左は下方向へスクロール（オフセットを減らすことで描画式側で下方向へ移動）
    g_bg3OffsetLeft -= g_bg3SpeedLeft * dt;
    if (g_BG3_LeftHeight > 0)
    {
        float h = (float)g_BG3_LeftHeight;
        // 範囲を [0, h) に収める
        while (g_bg3OffsetLeft >= h) g_bg3OffsetLeft -= h;
        while (g_bg3OffsetLeft < 0.0f) g_bg3OffsetLeft += h;
    }

    // 右は上方向へスクロール（オフセットを減らすことで描画式側で上方向へ移動）
    g_bg3OffsetRight -= g_bg3SpeedRight * dt;
    if (g_BG3_RightHeight > 0)
    {
        float h = (float)g_BG3_RightHeight;
        while (g_bg3OffsetRight >= h) g_bg3OffsetRight -= h;
        while (g_bg3OffsetRight < 0.0f) g_bg3OffsetRight += h;
    }
#pragma endregion
#pragma region 選択処理
    // P1 操作
    if (!g_isP1Ready)
    {

        if (Keyboard_IsKeyDownTrigger(KK_LEFT))
        {
            int oldIndex = g_cursorP1;
            g_cursorP1 = (g_cursorP1 + selectCount - 1) % selectCount;

            // カーソル移動アニメを開始（現在位置 -> 新位置）
            g_cursorState[0].startX = g_cursorState[0].posX;
            g_cursorState[0].targetX = SlotIndexToX(g_cursorP1);
            g_cursorState[0].moveTime = 0.0f;
            g_cursorState[0].moving = true;
            g_cursorState[0].index = g_cursorP1;

            // スケール: 古いスロットは縮小、新しいスロットは拡大アニメ開始
            StartSlotScaleAnim(oldIndex, false);
            StartSlotScaleAnim(g_cursorP1, true);
        }
        if (Keyboard_IsKeyDownTrigger(KK_RIGHT))
        {
            int oldIndex = g_cursorP1;
            g_cursorP1 = (g_cursorP1 + 1) % selectCount;

            g_cursorState[0].startX = g_cursorState[0].posX;
            g_cursorState[0].targetX = SlotIndexToX(g_cursorP1);
            g_cursorState[0].moveTime = 0.0f;
            g_cursorState[0].moving = true;
            g_cursorState[0].index = g_cursorP1;

            StartSlotScaleAnim(oldIndex, false);
            StartSlotScaleAnim(g_cursorP1, true);
        }

        if (Keyboard_IsKeyDownTrigger(KK_LEFTCONTROL))
        {
            g_isP1Ready = true;
            g_selectData.player1 = static_cast<WeaponTerrain>(g_cursorP1+1);
        }
        else
        {
            if (Keyboard_IsKeyDownTrigger(KK_F1)) g_isP1Ready = false;
        }
    }

    // P2 操作
    if (!g_isP2Ready)
    {
        if (Keyboard_IsKeyDownTrigger(KK_D3))
        {
            int oldIndex = g_cursorP2;
            g_cursorP2 = (g_cursorP2 + selectCount - 1) % selectCount;

            g_cursorState[1].startX = g_cursorState[1].posX;
            g_cursorState[1].targetX = SlotIndexToX(g_cursorP2);
            g_cursorState[1].moveTime = 0.0f;
            g_cursorState[1].moving = true;
            g_cursorState[1].index = g_cursorP2;

            StartSlotScaleAnim(oldIndex, false);
            StartSlotScaleAnim(g_cursorP2, true);
        }
        if (Keyboard_IsKeyDownTrigger(KK_D4))
        {
            int oldIndex = g_cursorP2;
            g_cursorP2 = (g_cursorP2 + 1) % selectCount;

            g_cursorState[1].startX = g_cursorState[1].posX;
            g_cursorState[1].targetX = SlotIndexToX(g_cursorP2);
            g_cursorState[1].moveTime = 0.0f;
            g_cursorState[1].moving = true;
            g_cursorState[1].index = g_cursorP2;

            StartSlotScaleAnim(oldIndex, false);
            StartSlotScaleAnim(g_cursorP2, true);
        }

        if (Keyboard_IsKeyDownTrigger(KK_D5))
        {
            g_isP2Ready = true;
            g_selectData.player2 = static_cast<WeaponTerrain>(g_cursorP2+1);
        }
        else
        {
            if (Keyboard_IsKeyDownTrigger(KK_DELETE)) g_isP2Ready = false;
        }
    }

	// 両者 Ready なら GO アニメ開始
    if (g_isP1Ready && g_isP2Ready)
    {
        if (g_goState == GO_NONE)
        {
          
            Manager_SetWTselect(g_selectData);

           
            g_goState = GO_ANIMATING;
            g_goAnimTime = 0.0f;

            float screenW = (float)Direct3D_GetBackBufferWidth();
            float screenH = (float)Direct3D_GetBackBufferHeight();

            
            g_goBtnTargetX = screenW * 0.5f;
            g_goBtnY = screenH * 0.5f;

        
            g_goBtnStartX = -(g_goBtnSize.x * 0.5f) - 50.0f;
            g_goBtnX = g_goBtnStartX;
        }

    }

    if (g_goState == GO_ANIMATING)
    {
        g_goAnimTime += FRAME_DT; 
        float t = g_goAnimTime / g_goAnimDuration;
        if (t >= 1.0f) t = 1.0f;

        g_goBtnX = g_goBtnStartX + (g_goBtnTargetX - g_goBtnStartX) * t;

        if (t >= 1.0f)
        {
      
            g_goState = GO_WAIT_FOR_A;
        }
    }
    else if (g_goState == GO_WAIT_FOR_A)
    {
   
        if (Keyboard_IsKeyDownTrigger(KK_A))
        {
           
            XMFLOAT4 fadeColor(0.0f, 0.0f, 0.0f, 1.0f);
            SetFade(40.0f, fadeColor, FADE_STATE::FADE_OUT, SCENE_GAME);

            g_goState = GO_NONE;
            g_goAnimTime = 0.0f;
            g_goBtnX = g_goBtnStartX;
        }
    }
#pragma endregion


    // カーソル移動進行
    for (int p = 0; p < 2; ++p)
    {
        CursorState& cs = g_cursorState[p];
        if (cs.moving)
        {
            cs.moveTime += dt;
            float t = cs.moveTime / cs.moveDuration;
            if (t >= 1.0f)
            {
                t = 1.0f;
                cs.moving = false;
                cs.posX = cs.targetX;
            }
            else
            {
                float e = EaseOutCos(t);
                cs.posX = cs.startX + (cs.targetX - cs.startX) * e;
            }
        }
    }

    // スロットスケールアニメーション更新
    for (int i = 0; i < selectCount; ++i)
    {
        if (g_slotAnimating[i])
        {
            g_slotAnimTime[i] += dt;
            float t = g_slotAnimTime[i] / g_slotAnimDuration;
            if (t >= 1.0f)
            {
                t = 1.0f;
                g_slotAnimating[i] = false;
            }
            float e = EaseOutCos(t);

            // 目標スケールを決定: 現在のインデックスが P1 または P2 の場合は selectedScale、そうでない場合は baseScale
            bool shouldBeSelected = (i == g_cursorP1) || (i == g_cursorP2);

            if (shouldBeSelected)
            {
                // 拡大 (base -> selected)
                g_slotScale[i] = g_slotBaseScale + (g_slotSelectedScale - g_slotBaseScale) * e;
            }
            else
            {
                // 縮小 (selected -> base)
                g_slotScale[i] = g_slotSelectedScale + (g_slotBaseScale - g_slotSelectedScale) * e;
            }
            // アニメ終了時に確定値をセット
            if (!g_slotAnimating[i])
            {
                g_slotScale[i] = shouldBeSelected ? g_slotSelectedScale : g_slotBaseScale;
                g_slotAnimTime[i] = 0.0f;
            }
        }
        else
        {
            // アニメしていないときは目標値を保証
            bool shouldBeSelected = (i == g_cursorP1) || (i == g_cursorP2);
            g_slotScale[i] = shouldBeSelected ? g_slotSelectedScale : g_slotBaseScale;
        }
    }
}

// ------------------ 描画処理 ------------------
void selectWT_Draw(int playerID)
{
    // 描画用の画面サイズ
    float screenWidth = (float)Direct3D_GetBackBufferWidth();
    float screenHeight = (float)Direct3D_GetBackBufferHeight();
    g_pContext->PSSetShaderResources(0, 1, &g_TextureBG[0]);
    DrawSprite(XMFLOAT2(screenWidth * 0.5f, screenHeight * 0.5f), XMFLOAT2(screenWidth, screenHeight), XMFLOAT4(1, 1, 1, 1));
    // 右側描画: テクスチャは回転済み（縦長）なので、高さ = g_BG3_LeftHeight を用いる
    if (g_TextureBG3_Left && g_BG3_LeftHeight > 0)
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureBG3_Left);

        float texH = (float)g_BG3_LeftHeight;
        float texW = (float)g_BG3_LeftWidth;

        // 画面左の X 座標（少し内側に）
        float drawX = texW * 0.5f; // DrawSprite は中心座標なので幅の半分だけ右へ
        // 必要に応じて drawX を調整（例: 50px の余白）
        drawX = screenWidth - 50;

        // 何枚並べれば画面全体をカバーできるか
        int count = (int)ceilf(screenHeight / texH) + 2;

        // オフセット位置（0..texH）
        float baseOff = g_bg3OffsetLeft;

        // 上方向へ流す、つまり描画の Y は (i * texH) - baseOff
        for (int i = -1; i < count; ++i)
        {
            float y = (i * texH) - baseOff + (texH * 0.5f); // DrawSprite は中心基準
            DrawSprite(XMFLOAT2(drawX, y), XMFLOAT2(texW, texH), XMFLOAT4(1, 1, 1, 1));
        }
    }

    // 左側描画: 画面右端に寄せる
    if (g_TextureBG3_Right && g_BG3_RightHeight > 0)
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureBG3_Right);

        float texH = (float)g_BG3_RightHeight;
        float texW = (float)g_BG3_RightWidth;

        // 画面右の X 座標
        float drawX = 50.0f;
        // 少し内側に寄せたいなら drawX = screenWidth - 50.0f;

        int count = (int)ceilf(screenHeight / texH) + 2;
        float baseOff = g_bg3OffsetRight;

        // 右は下方向へ流す（描画の Y は baseOff - i*texH のようにして下方向へ進む）
        // ここでは同様に (i * texH) + baseOff を使って下へ動く表現にする
        for (int i = -1; i < count; ++i)
        {
            float y = (i * texH) + baseOff + (texH * 0.5f);
            DrawSprite(XMFLOAT2(drawX, y), XMFLOAT2(texW, texH), XMFLOAT4(1, 1, 1, 1));
        }
    }
    // 背景

    g_pContext->PSSetShaderResources(0, 1, &g_TextureBG[1]);
    DrawSprite(XMFLOAT2(screenWidth * 0.5f, screenHeight * 0.5f), XMFLOAT2(screenWidth, screenHeight), XMFLOAT4(1, 1, 1, 1));

    // カード
    int CardposX = (int)(screenWidth / 2 - (screenWidth / 4));
    for (int i = 0; i < 2; i++)
    {
        g_pContext->PSSetShaderResources(0, 1, &g_TextureUi_Card[i]);
        DrawSprite(XMFLOAT2((float)CardposX, screenHeight / 2 - 50.0f), XMFLOAT2(827 * 0.8f, 1013 * 0.8f), XMFLOAT4(1, 1, 1, 1));
        CardposX += (int)(screenWidth / 2);
    }
    g_pContext->PSSetShaderResources(0, 1, &g_TextureUi_Card[2]);
    DrawSprite(XMFLOAT2(screenWidth * 0.5f, screenHeight * 0.85f), XMFLOAT2(3357 * 0.3f, 750 * 0.3f), XMFLOAT4(1, 1, 1, 1));
   
    // カーソル描画: 各プレイヤー用カーソルテクスチャを現在の posX で描画
    // P1 カーソル
    float baseCursorW = 202.0f * 0.75f;
    float baseCursorH = 271.0f * 0.75f;
    float p1W = baseCursorW * g_cursorScale[0];
    float p1H = baseCursorH * g_cursorScale[0];
    g_pContext->PSSetShaderResources(0, 1, &g_TextureUi_Cursor[0]);
    DrawSprite(XMFLOAT2(g_cursorState[0].posX, g_slotPosY + 25.0f), XMFLOAT2(p1W, p1H), XMFLOAT4(1, 1, 1, 1));

    // P2 カーソル
    float p2W = baseCursorW * g_cursorScale[1];
    float p2H = baseCursorH * g_cursorScale[1];
    g_pContext->PSSetShaderResources(0, 1, &g_TextureUi_Cursor[1]);
    DrawSprite(XMFLOAT2(g_cursorState[1].posX, g_slotPosY - 20.0f), XMFLOAT2(p2W, p2H), XMFLOAT4(1, 1, 1, 1));

    // スロットアイコン描画 (スケール反映)
    float startX = g_slotStartX;
    float spacing = g_slotSpacing;
    float posY = g_slotPosY;

    for (int i = 0; i < selectCount; ++i)
    {
        if (g_TextureUI[i] == NULL) continue;

        XMFLOAT2 pos = { startX + (i * spacing), posY };
        // base サイズを横縦 100 として、スケールを掛ける
        float baseW = 100.0f;
        float baseH = 100.0f;
        float scale = g_slotScale[i];
        XMFLOAT2 size = { baseW * scale, baseH * scale };

        // 色 (選択でハイライト)
        XMFLOAT4 color = { 0.5f, 0.5f, 0.5f, 1.0f };
        if (i == g_cursorP1)
        {
            color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }
        else if (i == g_cursorP2)
        {
            color = { 1.0f, 1.0f, 1.0f, 1.0f };
        }

        g_pContext->PSSetShaderResources(0, 1, &g_TextureUI[i]);
        DrawSprite(pos, size, color);
    }

    
    // ---------- スプライトアニメ描画（剣のプレビュー） ----------
    // 描画位置はカードの中心あたりに設定（必要に応じて微調整）
    XMFLOAT2 p1Pos = XMFLOAT2(screenWidth * 0.25f, screenHeight * 0.5f - 50.0f);
    XMFLOAT2 p2Pos = XMFLOAT2(screenWidth * 0.75f, screenHeight * 0.5f - 50.0f);
    XMFLOAT2 drawSize = XMFLOAT2(600.0f, 600.0f); // 描画サイズ（ピクセル）: 調整可
    XMFLOAT4 white = XMFLOAT4(1, 1, 1, 1);

    // P1: 攻撃再生中なら attack を、そうでなければ cursor が剣上なら idle を描画
    if (g_p1AttackPlaying)
    {
        if (g_swordAttackAnim[0].srv)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_swordAttackAnim[0].srv);
            int bno = g_swordAttackAnim[0].GetBno();
            DrawSpriteEx(p1Pos, XMFLOAT2(drawSize.x+50,drawSize.y), white, bno, g_swordAttackAnim[0].cols, g_swordAttackAnim[0].rows);
        }
    }
    else
    {
        if (g_cursorP1 == SWORD_SLOT_INDEX && g_swordIdleAnim[0].srv)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_swordIdleAnim[0].srv);
            int bno = g_swordIdleAnim[0].GetBno();
            DrawSpriteEx(p1Pos, drawSize, white, bno, g_swordIdleAnim[0].cols, g_swordIdleAnim[0].rows);
        }
    }

    // P2: 右側に左右反転して描画（flip は幅に負値を渡すことで実現）
    if (g_p2AttackPlaying)
    {
        if (g_swordAttackAnim[1].srv)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_swordAttackAnim[1].srv);
            int bno = g_swordAttackAnim[1].GetBno();
            XMFLOAT2 flipSize = XMFLOAT2(-drawSize.x, drawSize.y); // 左右反転
            DrawSpriteEx(p2Pos, flipSize, white, bno, g_swordAttackAnim[1].cols, g_swordAttackAnim[1].rows);
        }
    }
    else
    {
        if (g_cursorP2 == SWORD_SLOT_INDEX && g_swordIdleAnim[1].srv)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_swordIdleAnim[1].srv);
            int bno = g_swordIdleAnim[1].GetBno();
            XMFLOAT2 flipSize = XMFLOAT2(-drawSize.x, drawSize.y); // 左右反転
            DrawSpriteEx(p2Pos, flipSize, white, bno, g_swordIdleAnim[1].cols, g_swordIdleAnim[1].rows);
        }
    }

    if (g_goState != GO_NONE)
    {
        float screenW = (float)Direct3D_GetBackBufferWidth();
        float screenH = (float)Direct3D_GetBackBufferHeight();

        
        if (g_TextureGoBg)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_TextureGoBg);
            DrawSprite(XMFLOAT2(screenW * 0.5f, screenH * 0.5f), XMFLOAT2(screenW, screenH), XMFLOAT4(1, 1, 1, 1));
        }

     
        if (g_TextureGoBtn)
        {
            g_pContext->PSSetShaderResources(0, 1, &g_TextureGoBtn);
            DrawSprite(XMFLOAT2(g_goBtnX, g_goBtnY), g_goBtnSize, XMFLOAT4(1, 1, 1, 1));
        }

     
    }
}

// ------------------ Getter ------------------
bool selectWT_IsP1Ready()
{
    return g_isP1Ready;
}

bool selectWT_IsP2Ready()
{
    return g_isP2Ready;
}