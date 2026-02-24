#include "CameraIntroSequence.h"
#include "Camera.h"
#include "Player.h"
#include "Player2.h"
#include <cmath>

using namespace DirectX;

namespace
{
    enum INTRO_STATE
    {
        INTRO_NONE = 0,
        INTRO_ORBIT,
        INTRO_MOVE_TO_PLAYERS,
        INTRO_DONE
    };

    struct IntroCameraPose
    {
        XMFLOAT3 pos;
        XMFLOAT3 at;
    };

    static INTRO_STATE g_state = INTRO_NONE;
    static float g_timer = 0.0f;
    static bool g_hasPlayedCorpseIntro = false;
    static XMFLOAT3 g_arenaCenter = XMFLOAT3(0.0f, 0.0f, 0.0f);

    static float g_orbitRadius = 18.0f;
    static float g_orbitHeight = 10.0f;
    static float g_orbitDuration = 2.2f;
    static float g_moveDuration = 1.0f;

    // ORBIT終了時点のカメラ姿勢
    static IntroCameraPose g_orbitEndP1;
    static IntroCameraPose g_orbitEndP2;

    // プレイヤー寄りの最終カメラ姿勢
    static IntroCameraPose g_targetP1;
    static IntroCameraPose g_targetP2;

    static float Clamp01(float t)
    {
        if (t < 0.0f) return 0.0f;
        if (t > 1.0f) return 1.0f;
        return t;
    }

    static float EaseInOutCubic(float t)
    {
        t = Clamp01(t);
        return (t < 0.5f)
            ? (4.0f * t * t * t)
            : (1.0f - std::pow(-2.0f * t + 2.0f, 3.0f) / 2.0f);
    }

    static float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    static XMFLOAT3 Lerp3(const XMFLOAT3& a, const XMFLOAT3& b, float t)
    {
        return XMFLOAT3(
            Lerp(a.x, b.x, t),
            Lerp(a.y, b.y, t),
            Lerp(a.z, b.z, t));
    }

    static void ApplyP1Camera(const IntroCameraPose& p)
    {
        SetCameraPosition(p.pos);
        SetCameraAtPosition(p.at);
        SetCameraUpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
    }

    static void ApplyP2Camera(const IntroCameraPose& p)
    {
        SetCamera2Position(p.pos);
        SetCamera2AtPosition(p.at);
        SetCamera2UpVector(XMFLOAT3(0.0f, 1.0f, 0.0f));
    }

    static IntroCameraPose MakeOrbitPose(
        const XMFLOAT3& center,
        float radius,
        float height,
        float rad)
    {
        IntroCameraPose p{};
        p.at = XMFLOAT3(center.x, center.y + 1.5f, center.z);
        p.pos = XMFLOAT3(
            center.x + std::cos(rad) * radius,
            center.y + height,
            center.z + std::sin(rad) * radius);
        return p;
    }

    // 今のCamera_Updateの通常追従感に近い「プレイヤー後方カメラ」っぽい姿勢を作る
    static IntroCameraPose MakePlayerFocusPoseP1()
    {
        XMFLOAT3 pl = GetPlayerPosition();

        IntroCameraPose p{};
        p.at = XMFLOAT3(pl.x, pl.y + 1.0f, pl.z);
        p.pos = XMFLOAT3(pl.x + 0.0f, pl.y + 2.3f, pl.z - 6.0f);
        return p;
    }

    static IntroCameraPose MakePlayerFocusPoseP2()
    {
        XMFLOAT3 pl = GetPlayer2Position();

        IntroCameraPose p{};
        p.at = XMFLOAT3(pl.x, pl.y + 1.0f, pl.z);
        p.pos = XMFLOAT3(pl.x + 0.0f, pl.y + 2.3f, pl.z - 6.0f);
        return p;
    }

    static void CacheMoveTargets()
    {
        g_targetP1 = MakePlayerFocusPoseP1();
        g_targetP2 = MakePlayerFocusPoseP2();
    }

    static void UpdateOrbit(float t01)
    {
        // 0 -> 360度
        const float rad = XMConvertToRadians(45.0f) * t01;

        // 2画面で少し違う見え方にする（180度ずらす）
        IntroCameraPose p1 = MakeOrbitPose(g_arenaCenter, g_orbitRadius, g_orbitHeight, rad);
        IntroCameraPose p2 = MakeOrbitPose(g_arenaCenter, g_orbitRadius, g_orbitHeight, rad + XM_PI);

        ApplyP1Camera(p1);
        ApplyP2Camera(p2);

        g_orbitEndP1 = p1;
        g_orbitEndP2 = p2;
    }

    static void UpdateMoveToPlayers(float t01)
    {
        const float e = EaseInOutCubic(t01);

        // 移動開始時点でのプレイヤー位置を少しだけ追従したい場合は都度再計算してもOK
        CacheMoveTargets();

        IntroCameraPose p1{};
        p1.pos = Lerp3(g_orbitEndP1.pos, g_targetP1.pos, e);
        p1.at = Lerp3(g_orbitEndP1.at, g_targetP1.at, e);

        IntroCameraPose p2{};
        p2.pos = Lerp3(g_orbitEndP2.pos, g_targetP2.pos, e);
        p2.at = Lerp3(g_orbitEndP2.at, g_targetP2.at, e);

        ApplyP1Camera(p1);
        ApplyP2Camera(p2);
    }
}

void CameraIntroSequence_Initialize()
{
    g_state = INTRO_NONE;
    g_timer = 0.0f;
}

void CameraIntroSequence_Finalize()
{
    g_state = INTRO_NONE;
    g_timer = 0.0f;
}

void CameraIntroSequence_Start(
    const XMFLOAT3& arenaCenter,
    float orbitRadius,
    float orbitHeight,
    float orbitDurationSec,
    float moveDurationSec)
{
    g_arenaCenter = arenaCenter;
    g_orbitRadius = orbitRadius;
    g_orbitHeight = orbitHeight;
    g_orbitDuration = (orbitDurationSec <= 0.01f) ? 0.01f : orbitDurationSec;
    g_moveDuration = (moveDurationSec <= 0.01f) ? 0.01f : moveDurationSec;

    g_timer = 0.0f;
    g_state = INTRO_ORBIT;

    // 開始直後に1フレーム目を反映
    UpdateOrbit(0.0f);
}

void CameraIntroSequence_Update(float dt)
{
    if (g_state == INTRO_NONE || g_state == INTRO_DONE) return;

    g_timer += dt;

    if (g_state == INTRO_ORBIT)
    {
        float t = Clamp01(g_timer / g_orbitDuration);
        UpdateOrbit(t);

        if (t >= 1.0f)
        {
            g_state = INTRO_MOVE_TO_PLAYERS;
            g_timer = 0.0f;

            // 移動先を確定
            CacheMoveTargets();
        }
        return;
    }

    if (g_state == INTRO_MOVE_TO_PLAYERS)
    {
        float t = Clamp01(g_timer / g_moveDuration);
        UpdateMoveToPlayers(t);

        if (t >= 1.0f)
        {
            // 最終姿勢を明示的に適用して終わる
            ApplyP1Camera(g_targetP1);
            ApplyP2Camera(g_targetP2);

            g_state = INTRO_DONE;
            g_timer = 0.0f;
        }
        return;
    }
}

bool CameraIntroSequence_IsActive()
{
    return (g_state == INTRO_ORBIT || g_state == INTRO_MOVE_TO_PLAYERS);
}

bool CameraIntroSequence_IsFinished()
{
    return (g_state == INTRO_DONE);
}

void CameraIntroSequence_ForceFinish()
{
    CacheMoveTargets();
    ApplyP1Camera(g_targetP1);
    ApplyP2Camera(g_targetP2);
    g_state = INTRO_DONE;
    g_timer = 0.0f;
}
bool GetCameraIntroPlayed()
{
	return g_hasPlayedCorpseIntro;
}
void SetCameraIntroPlayed(bool played)
{
    g_hasPlayedCorpseIntro = played;
}
bool CameraIntroSequence_IsOrbitPhase()
{
    return (g_state == INTRO_ORBIT);
}