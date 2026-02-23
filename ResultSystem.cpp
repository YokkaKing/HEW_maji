#include "ResultSystem.h"

#include <cassert>
#include <cmath>

#include "direct3d.h"
#include "keyboard.h"
#include "Controller.h"
#include "fade.h"
#include "Manager.h"
#include "shader.h"
#include "sprite.h"
#include "Camera.h"
#include "Stage.h"
#include "model.h"
#include "Player.h"
#include "Player2.h"
#include "Result_Ui.h"
#include "Result_Ui2.h"
#include "Entry.h"


using namespace DirectX;
extern Controller g_Controller[2];

// Result_Ui 側
bool ResultUi_IsResultAttackReady();
bool ResultUi_IsChallengeAttackReady();
bool ResultUi_IsScoreAttackReady();
bool ResultUi_IsAnyUiEnding();

// ---------------- utility ----------------
static float Clamp01(float t)
{
    if (t < 0.0f) return 0.0f;
    if (t > 1.0f) return 1.0f;
    return t;
}
static float Lerp(float a, float b, float t) { return a + (b - a) * t; }
static float SmoothStep(float t)
{
    t = Clamp01(t);
    return t * t * (3.0f - 2.0f * t);
}

// ---------------- match info ----------------
static RESULT_MATCH_INFO s_info;

// ---------------- state ----------------
static int  s_step = 0;                 // 0=result, 1=challenge, 2=score
static bool s_attackStarted[3] = { false,false,false };

// ---------------- Light / Stage ----------------
static LIGHTOBJECT s_Light;
static STAGE s_Stage;

// ---------------- Model utils ----------------
static const char* GetBaseModelPath(WeaponTerrain wt)
{
    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL: return "asset\\model\\default_sword.fbx";
    case WeaponTerrain::SPEAR_HILL: return "asset\\model\\default_spear.fbx";
    case WeaponTerrain::BOW_HILL:   return "asset\\model\\default_bow.fbx";
    case WeaponTerrain::HAMMER_:    return "asset\\model\\default_hammer.fbx";
    case WeaponTerrain::SHURIKEN_:  return "asset\\model\\default_shuriken.fbx";
    default:                        return "asset\\model\\block.fbx";
    }
}
static const char* GetTransformModelPath(WeaponTerrain wt)
{
    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL: return "asset\\model\\sword.fbx";
    case WeaponTerrain::SPEAR_HILL: return "asset\\model\\spear.fbx";
    case WeaponTerrain::BOW_HILL:   return "asset\\model\\bow.fbx";
    case WeaponTerrain::HAMMER_:    return "asset\\model\\hammer.fbx";
    case WeaponTerrain::SHURIKEN_:  return "asset\\model\\shuriken.fbx";
    default:                        return "asset\\model\\block.fbx";
    }
}
static void SafeReleaseModel(MODEL*& m)
{
    if (m) { ModelRelease(m); m = nullptr; }
}
static void InitBonesToIdentity(MODEL* m)
{
    if (!m) return;
    for (auto& b : m->Bones) b.finalTransform = XMMatrixIdentity();
}

// ---------------- Attack clips ----------------
struct ClipRange { int start = 0; int end = 0; float fps = 60.0f; };

static ClipRange GetAttackRange_Base(WeaponTerrain wt)
{
    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL: return { 167,227,60.0f };
    case WeaponTerrain::SHURIKEN_:  return { 151,210,60.0f };
    case WeaponTerrain::SPEAR_HILL: return { 420, 479,60.0f };
    case WeaponTerrain::BOW_HILL:   return { 301,420,60.0f };
    case WeaponTerrain::HAMMER_:    return { 241,419,60.0f };
    default:                        return { 80,150,60.0f };
    }
}
static ClipRange GetAttackRange_Transform(WeaponTerrain wt)
{
    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL: return { 167,227,60.0f };
    case WeaponTerrain::SHURIKEN_:  return { 151,210,60.0f };
    case WeaponTerrain::SPEAR_HILL: return { 420, 479,60.0f };
    case WeaponTerrain::BOW_HILL:   return { 301,420,60.0f };
    case WeaponTerrain::HAMMER_:    return { 241,419,60.0f };
    default:                        return { 80,150,60.0f };
    }
}
static void PlayAttackOnceHold(MODEL* m, const ClipRange& r)
{
    if (!m) return;
    ModelPlayClip(m, r.start, r.end, r.fps, false); // 最後で止まる
}

// ---------------- ResultDisplay ----------------
struct ResultDisplay
{
    MODEL* base = nullptr;
    MODEL* a = nullptr;
    MODEL* b = nullptr;
    WeaponTerrain baseWT = WeaponTerrain::NONE;
    WeaponTerrain t0 = WeaponTerrain::NONE;
    WeaponTerrain t1 = WeaponTerrain::NONE;
    bool started = false;
    int drawMask = 0x7;
    XMFLOAT3 center = { 0,0,0 };
    float attackTimerBase = 0.0f;
    float attackTimerA = 0.0f;
    float attackTimerB = 0.0f;
    void Initialize(const RESULT_LOADOUT& lo)
    {
        SafeReleaseModel(base);
        SafeReleaseModel(a);
        SafeReleaseModel(b);
        baseWT = lo.baseWT;
        t0 = lo.t0;
        t1 = lo.t1;

        base = ModelLoad(GetBaseModelPath(lo.baseWT));
        a = ModelLoad(GetTransformModelPath(lo.t0));
        b = ModelLoad(GetTransformModelPath(lo.t1));

        InitBonesToIdentity(base);
        InitBonesToIdentity(a);
        InitBonesToIdentity(b);

        started = false;
        if (base) ModelUpdateAnimation(base, 0.0f);
        if (a)    ModelUpdateAnimation(a, 0.0f);
        if (b)    ModelUpdateAnimation(b, 0.0f);
    }

    void Finalize()
    {
        SafeReleaseModel(base);
        SafeReleaseModel(a);
        SafeReleaseModel(b);
    }
    void PlayIdleBase()
    {
        if (!base) return;
        switch (baseWT)
        {
        case WeaponTerrain::SWORD_WALL: ModelPlayClip(base, 0, 60, 60.0f, true); break;
        case WeaponTerrain::SPEAR_HILL: ModelPlayClip(base, 0, 120, 60.0f, true); break;
        case WeaponTerrain::BOW_HILL:   ModelPlayClip(base, 0, 60, 60.0f, true); break;
        case WeaponTerrain::HAMMER_:    ModelPlayClip(base, 0, 120, 60.0f, true); break;
        case WeaponTerrain::SHURIKEN_:  ModelPlayClip(base, 0, 60, 60.0f, true); break;
        }
    }
    void PlayIdleA()
    {
        if (!a) return;
        switch (t0)
        {
        case WeaponTerrain::SWORD_WALL: ModelPlayClip(a, 0, 60, 60.0f, true); break;
        case WeaponTerrain::SPEAR_HILL: ModelPlayClip(a, 0, 120, 60.0f, true); break;
        case WeaponTerrain::BOW_HILL:   ModelPlayClip(a, 0, 60, 60.0f, true); break;
        case WeaponTerrain::HAMMER_:    ModelPlayClip(a, 0, 120, 60.0f, true); break;
        case WeaponTerrain::SHURIKEN_:  ModelPlayClip(a, 0, 60, 60.0f, true); break;
        }
    }
    void PlayIdleB()
    {
        if (!b) return;
        switch (t1)
        {
        case WeaponTerrain::SWORD_WALL: ModelPlayClip(b, 0, 60, 60.0f, true); break;
        case WeaponTerrain::SPEAR_HILL: ModelPlayClip(b, 0, 120, 60.0f, true); break;
        case WeaponTerrain::BOW_HILL:   ModelPlayClip(b, 0, 60, 60.0f, true); break;
        case WeaponTerrain::HAMMER_:    ModelPlayClip(b, 0, 120, 60.0f, true); break;
        case WeaponTerrain::SHURIKEN_:  ModelPlayClip(b, 0, 60, 60.0f, true); break;
        }
    }
    void Update()
    {
        const float dt = 1.0f / 60.0f;

        // --- 攻撃終了チェック → 待機に戻す ---
        if (attackTimerBase > 0.0f)
        {
            attackTimerBase -= dt;
            if (attackTimerBase <= 0.0f) PlayIdleBase();
        }
        if (attackTimerA > 0.0f)
        {
            attackTimerA -= dt;
            if (attackTimerA <= 0.0f) PlayIdleA();
        }
        if (attackTimerB > 0.0f)
        {
            attackTimerB -= dt;
            if (attackTimerB <= 0.0f) PlayIdleB();
        }

        // 初回だけ待機を設定
        if (!started)
        {
            PlayIdleBase();
            PlayIdleA();
            PlayIdleB();
            started = true;
        }

        // ★毎フレーム進める（これが無いと止まる）
        if (base) ModelUpdateAnimation(base, dt);
        if (a)    ModelUpdateAnimation(a, dt);
        if (b)    ModelUpdateAnimation(b, dt);
    }
    static float GetClipDurationSec(const ClipRange& r)
    {
        if (r.fps <= 0.0f) return 0.0f;
        const int frames = (r.end > r.start) ? (r.end - r.start) : 0;
        return frames / r.fps;
    }
    void PlayAttackForStep(int step)
    {
        if (step == 0)
        {
            const ClipRange r = GetAttackRange_Base(baseWT);
            PlayAttackOnceHold(base, r);
            attackTimerBase = GetClipDurationSec(r);
        }
        if (step == 1)
        {
            const ClipRange r = GetAttackRange_Transform(t0);
            PlayAttackOnceHold(a, r);
            attackTimerA = GetClipDurationSec(r);
        }
        if (step == 2)
        {
            const ClipRange r = GetAttackRange_Transform(t1);
            PlayAttackOnceHold(b, r);
            attackTimerB = GetClipDurationSec(r);
        }
    }

    void Draw()
    {
        if (!base && !a && !b) return;

        const float y = 0.5f;
        const float z0 = 1.0f;
        const float r = 2.0f;

        const XMFLOAT3 basePos(0.0f, y, z0 - r);
        const XMFLOAT3 aPos(-r, y, z0 + r * 0.577f);
        const XMFLOAT3 bPos(r, y, z0 + r * 0.577f);

        center = XMFLOAT3(
            (basePos.x + aPos.x + bPos.x) / 3.0f,
            y,
            (basePos.z + aPos.z + bPos.z) / 3.0f
        );

        auto calcYawOutward = [&](const XMFLOAT3& pos) -> float
            {
                float dx = pos.x - center.x;
                float dz = pos.z - center.z;
                float len = std::sqrt(dx * dx + dz * dz);
                if (len < 0.0001f) return 0.0f;
                dx /= len; dz /= len;
                float yaw = std::atan2(dx, dz);
                yaw += XM_PI;
                return yaw;
            };

        auto drawOne = [&](MODEL* m, const XMFLOAT3& pos, bool isBase)
            {
                if (!m) return;

                float yaw = calcYawOutward(pos);

                float yOffset = -0.55f;
                

                XMMATRIX S = XMMatrixScaling(0.01f, 0.01f, 0.01f);
                XMMATRIX R = XMMatrixRotationRollPitchYaw(0.0f, yaw, 0.0f);
                XMMATRIX T = XMMatrixTranslation(pos.x, pos.y + yOffset, pos.z);

                XMMATRIX W = S * R * T;

                Shader_SetWorldMatrix(W);
                if (!m->Bones.empty()) Shader_SetBones(m);
                ModelDraw(m);
            };

        if (drawMask & 0x1) drawOne(base, basePos, true);
        if (drawMask & 0x2) drawOne(a, aPos, false);
        if (drawMask & 0x4) drawOne(b, bPos, false);
    }


    // ★追加：一時的にマスクを差し替えて描画（描画順制御用）
    void DrawWithMask(int mask)
    {
        const int old = drawMask;
        drawMask = mask;
        Draw();
        drawMask = old;
    }
};

static ResultDisplay s_dispP1;
static ResultDisplay s_dispP2;

// ---------------- continuous camera ----------------
struct SlowBand { float a0; float a1; };

struct ResultCam
{
    float yawNow = 0.0f;
    float pitchDeg = -15.0f;
    float distance = 4.0f;

    XMFLOAT3 pos = { 0,0,0 };
    XMFLOAT3 at = { 0,0,0 };

    bool continuous = false;
    float endYaw = 0.0f;

    float fastDegPerSec = 180.0f;
    float slowDegPerSec = 30.0f;
    float blendDeg = 10.0f;

    SlowBand slowBands[8];
    int slowBandCount = 0;

    float slowWNow = 0.0f; // 0..1

    void Initialize(float initialYawDeg, float initialPitchDeg, float initialDistance)
    {
        yawNow = initialYawDeg;
        pitchDeg = initialPitchDeg;
        distance = initialDistance;
    }

    void StartContinuous(float endYawDeg, float fastSpeed, float slowSpeed, float blendAngleDeg,
        const SlowBand* bands, int bandCount)
    {
        continuous = true;
        endYaw = endYawDeg;

        fastDegPerSec = fastSpeed;
        slowDegPerSec = slowSpeed;
        blendDeg = (blendAngleDeg <= 0.0f) ? 0.0001f : blendAngleDeg;

        slowBandCount = (bandCount > 8) ? 8 : bandCount;
        for (int i = 0; i < slowBandCount; ++i) slowBands[i] = bands[i];
    }

    float CalcSlowWeight(float yaw) const
    {
        float wMax = 0.0f;

        for (int i = 0; i < slowBandCount; ++i)
        {
            const float a0 = slowBands[i].a0;
            const float a1 = slowBands[i].a1;

            float w = 0.0f;

            if (yaw < a0 - blendDeg) w = 0.0f;
            else if (yaw < a0)       w = SmoothStep((yaw - (a0 - blendDeg)) / blendDeg);
            else if (yaw <= a1)      w = 1.0f;
            else if (yaw < a1 + blendDeg) w = 1.0f - SmoothStep((yaw - a1) / blendDeg);
            else w = 0.0f;

            if (w > wMax) wMax = w;
        }
        return wMax;
    }

    void Update(const XMFLOAT3& center)
    {
        const float dt = 1.0f / 60.0f;

        if (continuous)
        {
            slowWNow = CalcSlowWeight(yawNow);
            const float omega = Lerp(fastDegPerSec, slowDegPerSec, slowWNow);
            yawNow += omega * dt;

            if (yawNow >= endYaw)
            {
                yawNow = endYaw;
                continuous = false;
                slowWNow = 1.0f;
            }
        }
        else
        {
            slowWNow = 1.0f;
        }

        XMFLOAT3 c = center;
        c.y += 0.6f;
        at = c;

        float yawRad = XMConvertToRadians(yawNow);
        float pitchRad = XMConvertToRadians(pitchDeg);

        pos.x = c.x + std::cos(pitchRad) * std::sinf(yawRad) * distance;
        pos.y = c.y + std::sinf(pitchRad) * distance;
        pos.z = c.z + std::cos(pitchRad) * std::cosf(yawRad) * distance;
    }
};

static ResultCam s_camP1;
static ResultCam s_camP2;

// ---------------- rotation pages ----------------
struct RotPage
{
    float driftFromDeg;
    float driftToDeg;
    float driftSec;
    float quickDeltaDeg;
    float quickSec;
};

static RotPage kRotPages[3] =
{
    { 195.0f, 200.0f, 4.0f, 115.0f, 0.6f },
    { 315.0f, 320.0f, 4.0f, 115.0f, 0.6f },
    { 435.0f, 440.0f, 4.0f,   0.0f, 0.0f },
};

// UI表示のチラつき止め（ヒステリシス）
static bool s_isQuickRotating = false; // true=fast(3体&UI消す)
static bool s_uiVisible = true;


// ★追加：今のstepの「前面(最後に描く)モデル」を返す
static int GetFrontMaskByStep(int step)
{
    // step: 0=base, 1=a, 2=b
    if (step == 0) return 0x1;
    if (step == 1) return 0x2;
    return 0x4;
}

// ★追加：前面以外（先に描く2体）
static int GetBackMaskByStep(int step)
{
    const int front = GetFrontMaskByStep(step);
    return 0x7 & (~front);
}

static void ResetAttackFlags()
{
    s_attackStarted[0] = s_attackStarted[1] = s_attackStarted[2] = false;
}

void ResultSystem_SetMatchInfo(const RESULT_MATCH_INFO& info)
{
    s_info = info;
}

void ResultSystem_Initialize(ID3D11Device* dev, ID3D11DeviceContext* ctx)
{
    s_Stage.Initialize(dev, ctx);
    Result_Ui_Initialize(dev, ctx);
    Result2_Ui_Initialize(dev, ctx);


    // Light
    XMFLOAT4 para;
    para = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
    s_Light.SetAmbient(para);
    para = XMFLOAT4(0.6f, 0.6f, 0.6f, 1.0f);
    s_Light.SetDiffuse(para);

    para = XMFLOAT4(0.5f, -1.0f, 0.0f, 1.0f);
    float len = std::sqrt(para.x * para.x + para.y * para.y + para.z * para.z);
    if (len > 0.0f) { para.x /= len; para.y /= len; para.z /= len; }
    s_Light.SetDirection(para);

    // models
    s_dispP1.Initialize(s_info.p1);
    s_dispP2.Initialize(s_info.p2);

    // slow bands
    SlowBand bands[3] =
    {
        { kRotPages[0].driftFromDeg, kRotPages[0].driftToDeg },
        { kRotPages[1].driftFromDeg, kRotPages[1].driftToDeg },
        { kRotPages[2].driftFromDeg, kRotPages[2].driftToDeg },
    };

    const float slowSpeed = (kRotPages[0].driftToDeg - kRotPages[0].driftFromDeg) / kRotPages[0].driftSec;
    const float fastSpeed = kRotPages[0].quickDeltaDeg / kRotPages[0].quickSec;
    const float blendDeg = 10.0f;

    s_camP1.Initialize(kRotPages[0].driftFromDeg, -15.0f, 4.0f);
    s_camP2.Initialize(kRotPages[0].driftFromDeg, -15.0f, 4.0f);
    s_camP1.StartContinuous(kRotPages[2].driftToDeg, fastSpeed, slowSpeed, blendDeg, bands, 3);
    s_camP2.StartContinuous(kRotPages[2].driftToDeg, fastSpeed, slowSpeed, blendDeg, bands, 3);

    // step init
    s_step = 0;
    ResetAttackFlags();

    // UI start (winnerはP1が勝ったら1 / 負けなら2)
    int winnerForUi = (s_info.winnerId == 1) ? 1 : 2;
    if (winnerForUi == 1)
    {
        SetupResultUi(true, winnerForUi);
        Result2_SetupResultUi(true, 2);
    }
    else
    {
        SetupResultUi(true, winnerForUi);
		Result2_SetupResultUi(true, 1);
    }
    
    


    s_dispP1.drawMask = 0x1;
    s_dispP2.drawMask = 0x1;

    s_uiVisible = true;
    s_isQuickRotating = false;
}

void ResultSystem_Finalize()
{
    s_dispP1.Finalize();
    s_dispP2.Finalize();
    Result_Ui_Finalize();
    Result2_Ui_Finalize();

    ResetWinCount();
    s_Stage.Finalize();
}

// UI進行に合わせて攻撃を1回だけ開始
static void UpdateAttackStartByUiReady()
{
    if (s_step == 0 && !s_attackStarted[0] && ResultUi_IsResultAttackReady())
    {
        s_dispP1.PlayAttackForStep(0);
        s_dispP2.PlayAttackForStep(0);
        s_attackStarted[0] = true;
    }
    else if (s_step == 1 && !s_attackStarted[1] && ResultUi_IsChallengeAttackReady())
    {
        s_dispP1.PlayAttackForStep(1);
        s_dispP2.PlayAttackForStep(1);
        s_attackStarted[1] = true;
    }
    else if (s_step == 2 && !s_attackStarted[2] && ResultUi_IsScoreAttackReady())
    {
        s_dispP1.PlayAttackForStep(2);
        s_dispP2.PlayAttackForStep(2);
        s_attackStarted[2] = true;
    }
}

void ResultSystem_Update()
{
    s_dispP1.Update();
    s_dispP2.Update();
    Result_Ui_Update();
    Result2_Ui_Update();


    // camera
    s_camP1.Update(s_dispP1.center);
    s_camP2.Update(s_dispP2.center);

    // ---- UI表示のチラつき止め（slowWNowでヒステリシス）----
    // slowWNow: 1に近いほど「ゆっくり」
    const float w = s_camP1.slowWNow;
    const float TH_ON = 0.75f; // これ以上でUI ON
    const float TH_OFF = 0.35f; // これ以下でUI OFF

    if (s_uiVisible)
    {
        if (w < TH_OFF) s_uiVisible = false;
    }
    else
    {
        if (w > TH_ON) s_uiVisible = true;
    }
    s_isQuickRotating = !s_uiVisible;

    // ---- step / UI切替（角度を跨いだ瞬間で判定）----
    static float prevYaw = 0.0f;
    const float nowYaw = s_camP1.yawNow;

    // Result fadeout 開始：200 到達
    if (prevYaw < kRotPages[0].driftToDeg && nowYaw >= kRotPages[0].driftToDeg)
    {
        SetResultIsEnd(true);
        Result2_SetResultIsEnd(true);

    }

    // Challenge 開始：315 到達
    if (prevYaw < kRotPages[1].driftFromDeg && nowYaw >= kRotPages[1].driftFromDeg)
    {
        s_step = 1;
        SetChallengeUi(true);
        Result2_SetChallengeUi(true);
        // step変わったので attackの「そのstep」を1回だけにしたい → ここだけリセット
        s_attackStarted[1] = false;
    }

    // Challenge fadeout：320 到達
    if (prevYaw < kRotPages[1].driftToDeg && nowYaw >= kRotPages[1].driftToDeg)
    {
        SetChallengeIsEnd(true);
        Result2_SetChallengeIsEnd(true);
    }

    // Score 開始：435 到達
    if (prevYaw < kRotPages[2].driftFromDeg && nowYaw >= kRotPages[2].driftFromDeg)
    {
        s_step = 2;
        SetScoreUi(true);
        Result2_SetScoreUi(true);
        s_attackStarted[2] = false;
    }

    prevYaw = nowYaw;

    // ---- model draw mask ----
    if (s_isQuickRotating)
    {
        s_dispP1.drawMask = 0x7;
        s_dispP2.drawMask = 0x7;
    }
    else
    {
        if (s_step == 0) { s_dispP1.drawMask = 0x1; s_dispP2.drawMask = 0x1; }
        if (s_step == 1) { s_dispP1.drawMask = 0x2; s_dispP2.drawMask = 0x2; }
        if (s_step == 2) { s_dispP1.drawMask = 0x4; s_dispP2.drawMask = 0x4; }
    }

    // attack start
    UpdateAttackStartByUiReady();

    // ---- 最後（440到達後）Enterで Title ----
    const bool finished = (!s_camP1.continuous && s_camP1.yawNow >= kRotPages[2].driftToDeg);
    if (finished && GetFadeState() == FADE_NONE)
    {
        bool isNextTriggered = Keyboard_IsKeyDownTrigger(KK_ENTER);
        for (int i = 0; i < 2; i++)
        {
            int ctrlIdx = GetControllerIndexFromPlayerNo(i);
            if (ctrlIdx != -1)
            {
                if (g_Controller[ctrlIdx].IsButtonPushed(ControllerButton::A_BUTTON)) {
                    isNextTriggered = true;
                }
            }
            else
            {
                if (g_Controller[i].IsButtonPushed(ControllerButton::A_BUTTON)) {
                    isNextTriggered = true;
                }
            }
        }
        if (isNextTriggered)
        {
            XMFLOAT4 color(0, 0, 0, 1);
            Player_AllCountReset();
            SetFade(40.0f, color, FADE_OUT, SCENE_TITLE);
        }
    }

    SetCameraPosition(s_camP1.pos);
    SetCameraAtPosition(s_camP1.at);
    SetCamera2Position(s_camP2.pos);
    SetCamera2AtPosition(s_camP2.at);
}

// ---------------- draw ----------------
static void DrawCommon3D_P1()
{
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);

    Camera_Draw();
    Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());
    s_Stage.Draw();
}
static void DrawCommon3D_P2()
{
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);

    Camera2_Draw();
    Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());
    s_Stage.Draw();
}

static void DrawModels_P1()
{
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);

    Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());
    s_dispP1.Draw();
}
static void DrawModels_P2()
{
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);

    Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());
    s_dispP2.Draw();
}

void ResultSystem_Draw_Player1()
{
    DrawCommon3D_P1();

    // --- 3D(背面2体) → UI → 3D(前面1体) の順で描く ---
    // これで「回転の遅くなるタイミングでモデルが消える」「UIの前後関係が不自然」
    // が起きにくくなる。

    const int backMask = GetBackMaskByStep(s_step);
    const int frontMask = GetFrontMaskByStep(s_step);

    // 背面（2体） or 全部
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);
    Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());

    if (s_uiVisible)
    {
        s_dispP1.DrawWithMask(backMask);
    }
    else
    {
        // UIが出ない（高速回転/フェード中など）は全部まとめて描画
        s_dispP1.DrawWithMask(0x7);
    }

    // UI
    s_Light.SetEnable(FALSE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(FALSE);

    if (s_uiVisible)
    {
        Result_Ui_Draw();
    }

    // 前面（1体）…UIの上に必ず来る
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);
    Shader_SetMatrix(GetViewMatrix() * GetProjectionMatrix());

    if (s_uiVisible)
    {
        s_dispP1.DrawWithMask(frontMask);
    }

    // 後片付け
    s_Light.SetEnable(FALSE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(FALSE);
}

void ResultSystem_Draw_Player2()
{
    DrawCommon3D_P2();

    const int backMask = GetBackMaskByStep(s_step);
    const int frontMask = GetFrontMaskByStep(s_step);

    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);
    Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());
    if (s_uiVisible)
    {
        s_dispP2.DrawWithMask(backMask);
    }
    else
    {
        // UIが出ない（高速回転/フェード中など）は全部まとめて描画
        s_dispP2.DrawWithMask(0x7);
    }
    s_Light.SetEnable(FALSE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(FALSE);

    if (s_uiVisible)
    {
        Result2_Ui_Draw();
    }
    s_Light.SetEnable(TRUE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(TRUE);
    Shader_SetMatrix(GetViewMatrix2() * GetProjectionMatrix2());

    if (s_uiVisible)
    {
        s_dispP2.DrawWithMask(frontMask);
    }

    s_Light.SetEnable(FALSE);
    Shader_SetLight(s_Light.Light);
    SetDepthTest(FALSE);
}
