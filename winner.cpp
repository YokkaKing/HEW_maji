/*
* ファイル名    winner.cpp
* タイトル      Winner（Result用 表示専用）
*
* 目的:
*  - Result画面で「勝者の初期武器モデル + 変身先モデル2つ」を同時に描画する。
*  - 入力・当たり判定・攻撃などの処理は一切行わない。
*/

#include "Winner.h"
#include "shader.h"   // Shader_SetWorldMatrix
#include "Result.h"
static ID3D11Device* s_pDevice = nullptr;
static ID3D11DeviceContext* s_pContext = nullptr;

static WINNER_RESULT_INFO s_resultInfo;

static MODEL* s_modelBase = nullptr;
static MODEL* s_modelA = nullptr;
static MODEL* s_modelB = nullptr;
static int s_drawMask = 0x7; // 初期は全部描画
void Winner_SetDrawMask(int mask)
{
    s_drawMask = mask;
}
static XMFLOAT3 center;
static const char* GetBaseModelPath(WeaponTerrain wt)
{
    // 「初期武器」は default_ 系
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
    // 「変身先」は変身後モデル（既存で使ってるモデル名に合わせる）
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
    if (m)
    {
        ModelRelease(m);
        m = nullptr;
    }
}

void Winner_SetResultInfo(int winnerPlayerId, int decidedRound,
    WeaponTerrain baseWT, WeaponTerrain transformA, WeaponTerrain transformB)
{
    s_resultInfo.winnerPlayerId = winnerPlayerId;
    s_resultInfo.decidedRound = decidedRound;
    s_resultInfo.baseWT = baseWT;
    s_resultInfo.transformWT[0] = transformA;
    s_resultInfo.transformWT[1] = transformB;
}
static bool s_started = false;

static void InitBonesToIdentity(MODEL* m)
{
    if (!m) return;
    for (auto& b : m->Bones)
        b.finalTransform = XMMatrixIdentity();
}

WINNER_RESULT_INFO Winner_GetResultInfo()
{
    return s_resultInfo;
}

void WinnerInitialize(ID3D11Device* pDevice, ID3D11DeviceContext* pContext)
{
    s_pDevice = pDevice;
    s_pContext = pContext;

    SafeReleaseModel(s_modelBase);
    SafeReleaseModel(s_modelA);
    SafeReleaseModel(s_modelB);

    s_modelBase = ModelLoad(GetBaseModelPath(s_resultInfo.baseWT));
    s_modelA = ModelLoad(GetTransformModelPath(s_resultInfo.transformWT[0]));
    s_modelB = ModelLoad(GetTransformModelPath(s_resultInfo.transformWT[1]));
    InitBonesToIdentity(s_modelBase);
    InitBonesToIdentity(s_modelA);
    InitBonesToIdentity(s_modelB);

    // ★PlayClipは「初回だけ」やる
    s_started = false;
	center = XMFLOAT3(0.0f, 0.0f, 0.0f);
    // ★初期フレームの骨を一度計算しておく（dt=0でOK）
    if (s_modelBase) ModelUpdateAnimation(s_modelBase, 0.0f);
    if (s_modelA)    ModelUpdateAnimation(s_modelA, 0.0f);
    if (s_modelB)    ModelUpdateAnimation(s_modelB, 0.0f);
}

void WinnerFinalize()
{
    SafeReleaseModel(s_modelBase);
    SafeReleaseModel(s_modelA);
    SafeReleaseModel(s_modelB);

    s_pDevice = nullptr;
    s_pContext = nullptr;
}

void WinnerUpdate()
{
    // ★PlayClipは1回だけ（毎フレ呼ぶと毎回時間0に戻って破綻しやすい）
    if (!s_started)
    {
        if (s_modelBase && GetResultPressCount() == 0)
        {
            switch (s_resultInfo.baseWT)
            {
            case WeaponTerrain::SWORD_WALL:
                ModelPlayClip(s_modelBase, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SPEAR_HILL:
                ModelPlayClip(s_modelBase, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::BOW_HILL:
                ModelPlayClip(s_modelBase, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::HAMMER_:
                ModelPlayClip(s_modelBase, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SHURIKEN_:
                ModelPlayClip(s_modelBase, 0, 60, 60.0f, true, 1.0f);
                break;

            }
        }
        if (s_modelA && GetResultPressCount() == 1)
        {
            switch (s_resultInfo.baseWT)
            {
            case WeaponTerrain::SWORD_WALL:
                ModelPlayClip(s_modelA, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SPEAR_HILL:
                ModelPlayClip(s_modelA, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::BOW_HILL:
                ModelPlayClip(s_modelA, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::HAMMER_:
                ModelPlayClip(s_modelA, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SHURIKEN_:
                ModelPlayClip(s_modelA, 0, 60, 60.0f, true, 1.0f);
                break;
            }
        }
        if (s_modelB && GetResultPressCount() == 2)
        {
            switch (s_resultInfo.baseWT)
            {
            case WeaponTerrain::SWORD_WALL:
                ModelPlayClip(s_modelB, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SPEAR_HILL:
                ModelPlayClip(s_modelB, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::BOW_HILL:
                ModelPlayClip(s_modelB, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::HAMMER_:
                ModelPlayClip(s_modelB, 0, 60, 60.0f, true, 1.0f);
                break;
            case WeaponTerrain::SHURIKEN_:
                ModelPlayClip(s_modelB, 0, 60, 60.0f, true, 1.0f);
                break;
            }
        }
        s_started = true;

    }
    // ★骨行列を更新（ここがないとShader_SetBonesで崩れる）
    const float dt = 1.0f / 60.0f;
    if (s_modelBase) ModelUpdateAnimation(s_modelBase, dt);
    if (s_modelA)    ModelUpdateAnimation(s_modelA, dt);
    if (s_modelB)    ModelUpdateAnimation(s_modelB, dt);
}

void WinnerDraw()
{
    if (!s_modelBase && !s_modelA && !s_modelB) return;

    // ===== 三角配置（Y同じ、XZで三角）=====
    const float y = 0.0f;
    const float z0 = 1.0f;
    const float r = 2.0f;

     XMFLOAT3 basePos(0.0f, y, z0 - r);              // 奥（頂点）
    const XMFLOAT3 aPos(-r, y, z0 + r * 0.577f);      // 手前左
    const XMFLOAT3 bPos(r, y, z0 + r * 0.577f);      // 手前右
    if (s_resultInfo.baseWT == WeaponTerrain::SPEAR_HILL)
    {
        basePos = XMFLOAT3(0.0f, y+0.5f, z0 - r);              // 奥（頂点）
    }

    // 三角の中心（XZだけ使う）
    center=XMFLOAT3(
        (basePos.x + aPos.x + bPos.x) / 3.0f,
        y+0.1f,
        (basePos.z + aPos.z + bPos.z) / 3.0f
    );

    auto calcYawOutward = [&](const XMFLOAT3& pos) -> float
        {
            // center -> pos（外向き）
            float dx = pos.x - center.x;
            float dz = pos.z - center.z;

            // 方向ベクトルの正規化（なくても atan2 ではOKだけど、ゼロ除け）
            float len = std::sqrt(dx * dx + dz * dz);
            if (len < 0.0001f) return 0.0f;
            dx /= len;
            dz /= len;

            // yaw（Y軸回転）
            // ※「モデルの前方向」が +Z を向いている前提の式
            //    forward(+Z) を dir(dx,dz) に向ける => yaw = atan2(dx, dz)
            float yaw = std::atan2(dx, dz);

            // もし逆向きだったらここを有効に（180度回す）
             yaw += XM_PI;

            return yaw;
        };

    auto drawOne = [&](MODEL* model, const XMFLOAT3& pos)
        {
            if (!model) return;

            const float yaw = calcYawOutward(pos);

            XMMATRIX S = XMMatrixScaling(0.013f, 0.013f, 0.013f);
            XMMATRIX R = XMMatrixRotationRollPitchYaw(0.0f, yaw, 0.0f);
            XMMATRIX T = XMMatrixTranslation(pos.x, pos.y, pos.z);
            XMMATRIX W = S * R * T;

            Shader_SetWorldMatrix(W);

            if (!model->Bones.empty())
                Shader_SetBones(model);

            ModelDraw(model);
        };

    if (s_drawMask & 0x1) drawOne(s_modelBase, basePos);
    if (s_drawMask & 0x2) drawOne(s_modelA, aPos);
    if (s_drawMask & 0x4) drawOne(s_modelB, bPos);
}
XMFLOAT3 GetWinnerCenterposition()
{
    return center;
}

void SetStartedFlag(bool flag)
{
	s_started = flag;
}

