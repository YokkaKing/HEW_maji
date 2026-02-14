/*
* ファイル名	Transform.cpp
* タイトル	進化
* 作成者		イトイ
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Transform.h"
#include"keyboard.h"
#include"Controller.h"
#include "terrain.h"
#include "generateWT.h"
#include<cstdio>
#include<cstdlib>

#include "sword.h"
#include "spear.h"
#include "hammer.h"
#include "arrow.h" 
#include "syuriken.h"
#include <memory>

//================================================================
//	グローバル変数
//================================================================
extern PLAYER g_Player;
extern PLAYER2 g_Player2;
extern Controller g_Controller[2];
const char* INITIAL_MODEL_PATH;

const char* INITIAL_MODEL_PATH_P1 = nullptr;
const char* INITIAL_MODEL_PATH_P2 = nullptr;

const int TRANSFORM_LIMIT_FRAME = 20 * 60;

WeaponTerrain g_TransformA_P1 = WeaponTerrain::NONE;
WeaponTerrain g_TransformB_P1 = WeaponTerrain::NONE;
WeaponTerrain g_TransformA_P2 = WeaponTerrain::NONE;
WeaponTerrain g_TransformB_P2 = WeaponTerrain::NONE;

bool g_IsUsedA_P1 = false;
bool g_IsUsedB_P1 = false;
bool g_IsUsedA_P2 = false;
bool g_IsUsedB_P2 = false;


void TransformInitialize(WeaponTerrain selectP1_A, WeaponTerrain selectP1_B, WeaponTerrain selectP2_A, WeaponTerrain selectP2_B) {

    g_Player.TransformType = TRANSFORM_TYPE::TRANSFORM_TYPE_NONE;
    g_Player.TransformTimer = 0;
    g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE;
    g_Player2.TransformTimer = 0;
    g_Player.m_moveSpeed = 0;
    g_Player.m_jumpForce = 0;

    g_TransformA_P1 = selectP1_A;
    g_TransformB_P1 = selectP1_B;
    g_TransformA_P2 = selectP2_A;
    g_TransformB_P2 = selectP2_B;

    // P1の初期モデル設定
    switch (g_Player.m_baseWT) {
    case WeaponTerrain::SWORD_WALL: INITIAL_MODEL_PATH_P1 = "asset\\model\\default_sword.fbx"; break;
    case WeaponTerrain::SPEAR_HILL: INITIAL_MODEL_PATH_P1 = "asset\\model\\default_spear.fbx"; break;
    case WeaponTerrain::BOW_HILL:   INITIAL_MODEL_PATH_P1 = "asset\\model\\default_bow.fbx"; break;
    case WeaponTerrain::HAMMER_:    INITIAL_MODEL_PATH_P1 = "asset\\model\\default_hammer.fbx"; break;
    case WeaponTerrain::SHURIKEN_:  INITIAL_MODEL_PATH_P1 = "asset\\model\\default_shuriken.fbx"; break;
    default:                        INITIAL_MODEL_PATH_P1 = "asset\\model\\default.fbx"; break;
    }

    // P2の初期モデル設定
    switch (g_Player2.m_baseWT) {
    case WeaponTerrain::SWORD_WALL: INITIAL_MODEL_PATH_P2 = "asset\\model\\default_sword.fbx"; break;
    case WeaponTerrain::SPEAR_HILL: INITIAL_MODEL_PATH_P2 = "asset\\model\\default_spear.fbx"; break;
    case WeaponTerrain::BOW_HILL:   INITIAL_MODEL_PATH_P2 = "asset\\model\\default_bow.fbx"; break;
    case WeaponTerrain::HAMMER_:    INITIAL_MODEL_PATH_P2 = "asset\\model\\default_hammer.fbx"; break;
    case WeaponTerrain::SHURIKEN_:  INITIAL_MODEL_PATH_P2 = "asset\\model\\default_shuriken.fbx"; break;
    default:                        INITIAL_MODEL_PATH_P2 = "asset\\model\\default.fbx"; break;
    }

    g_IsUsedA_P1 = false;
    g_IsUsedB_P1 = false;
    g_IsUsedA_P2 = false;
    g_IsUsedB_P2 = false;

}

void TransformFinalize()
{
    // 終了処理
}

void ApplyTransformationP1(PLAYER* p, WeaponTerrain wt, bool isTransform)
{
    if (wt == WeaponTerrain::NONE) return;

    //地形と武器の適用
    TerrainSet(wt, FALSE);
    p->SetCurrentWT(wt);

    switch (wt)
    {
    case WeaponTerrain::SWORD_WALL:
        p->EquipWeapon(std::make_unique<Sword>(p, FALSE));
        break;
    case WeaponTerrain::SPEAR_HILL:
        p->EquipWeapon(std::make_unique<Spear>(p, FALSE));
        break;
    case WeaponTerrain::BOW_HILL:
        p->EquipWeapon(std::make_unique<Arrow>(p, FALSE)); // 弓クラス
       
        break;
    case WeaponTerrain::HAMMER_:
        p->EquipWeapon(std::make_unique<Hammer>(p, FALSE));
        break;
    case WeaponTerrain::SHURIKEN_:
        p->EquipWeapon(std::make_unique<Shuriken>(p, FALSE)); // 手裏剣クラス
        break;
    }
    SetWTP1(wt);

    //モデルの決定
    const char* path = nullptr;
    if (!isTransform) {
        // 通常時（m_baseWTに戻す時など）は初期設定パスを使用
        path = INITIAL_MODEL_PATH_P1;
    }
    else {
        // 変身時
        switch (wt) {
        case WeaponTerrain::SWORD_WALL:   path = "asset\\model\\sword.fbx"; break;
        case WeaponTerrain::SPEAR_HILL:   path = "asset\\model\\spear.fbx"; break;
        case WeaponTerrain::BOW_HILL:     path = "asset\\model\\bow.fbx"; break;
        case WeaponTerrain::HAMMER_:      path = "asset\\model\\hammer.fbx"; break;
        case WeaponTerrain::SHURIKEN_:    path = "asset\\model\\shuriken.fbx"; break;
        default:                          path = "asset\\model\\ball.fbx"; break;
        }
    }

    // 3. モデルのリロード
    if (path) {
        MODEL* newModel = ModelLoad(path);
        if (newModel != nullptr) {
            // 新しいモデルのロードに成功した場合のみ、古いモデルを解放して入れ替える
            if (p->m_model != nullptr) {
                ModelRelease(p->m_model);
                p->m_model = nullptr;
            }
            p->m_model = newModel;
        }
        else {
            // ロード失敗時にログを出す
            printf("Failed to load model: %s\n", path);
        }
    }
}
void ApplyTransformationP2(PLAYER2* p, WeaponTerrain wt, bool isTransform)
{
    if (wt == WeaponTerrain::NONE) return;

    //地形と武器の適用
    TerrainSet(wt, TRUE);
    p->SetCurrentWT(wt);

    switch (wt) {
    case WeaponTerrain::SWORD_WALL:
        p->EquipWeapon(std::make_unique<Sword>(p, TRUE));
        break;
    case WeaponTerrain::SPEAR_HILL:
        p->EquipWeapon(std::make_unique<Spear>(p, TRUE));
        break;
    case WeaponTerrain::BOW_HILL:
        p->EquipWeapon(std::make_unique<Arrow>(p, TRUE)); // 弓クラス
        break;
    case WeaponTerrain::HAMMER_:
        p->EquipWeapon(std::make_unique<Hammer>(p, TRUE));
        break;
    case WeaponTerrain::SHURIKEN_:
        p->EquipWeapon(std::make_unique<Shuriken>(p, TRUE)); // 手裏剣クラス
        break;
    }
    SetWTP2(wt);

    //モデルの決定
    const char* path = nullptr;
    if (!isTransform) {
        // 通常時（m_baseWTに戻す時など）は初期設定パスを使用
        path = INITIAL_MODEL_PATH_P2;
    }
    else {
        // 変身時
        switch (wt) {
        case WeaponTerrain::SWORD_WALL:   path = "asset\\model\\sword.fbx"; break;
        case WeaponTerrain::SPEAR_HILL:   path = "asset\\model\\spear.fbx"; break;
        case WeaponTerrain::BOW_HILL:     path = "asset\\model\\bow.fbx"; break;
        case WeaponTerrain::HAMMER_:      path = "asset\\model\\hammer.fbx"; break;
        case WeaponTerrain::SHURIKEN_:    path = "asset\\model\\shuriken.fbx"; break;
        default:                          path = "asset\\model\\ball.fbx"; break;
        }
    }

    // 3. モデルのリロード
    if (path) {
        MODEL* newModel = ModelLoad(path);
        if (newModel != nullptr) {
            // 新しいモデルのロードに成功した場合のみ、古いモデルを解放して入れ替える
            if (p->m_model != nullptr) {
                ModelRelease(p->m_model);
                p->m_model = nullptr;
            }
            p->m_model = newModel;
        }
        else {
            // ロード失敗時にログを出す
            printf("Failed to load model: %s\n", path);
        }
    }
}

void TransformPlayer()
{
    const char* newModelPath = nullptr;
    TRANSFORM_TYPE nextType = TRANSFORM_TYPE::TRANSFORM_TYPE_NONE;
    if (g_Player.TransformType == TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
    {
        WeaponTerrain targetWT = WeaponTerrain::NONE;

        if ((Keyboard_IsKeyDownTrigger(KK_D1) || g_Controller[0].IsButtonPushed(ControllerButton::L_SHOULDER)) && !g_IsUsedA_P1)
        {
            g_Player.TransformType = TRANSFORM_TYPE::TRANSFORM_TYPE_A;
            targetWT = g_TransformA_P1;
            Player_PlusTransformCount();
            g_IsUsedA_P1 = true;
			SetPlayer_IsTransformed(true);
        }
        else if ((Keyboard_IsKeyDownTrigger(KK_D0) || g_Controller[0].IsButtonPushed(ControllerButton::R_SHOULDER)) && !g_IsUsedB_P1)
        {
            g_Player.TransformType = TRANSFORM_TYPE::TRANSFORM_TYPE_B;
            targetWT = g_TransformB_P1;
            Player_PlusTransformCount();

            g_IsUsedB_P1 = true;
            SetPlayer_IsTransformed(true);

        }
        if (targetWT != WeaponTerrain::NONE) {
            ApplyTransformationP1(&g_Player, targetWT, true);
            
            g_Player.TransformTimer = TRANSFORM_LIMIT_FRAME;
        }
    }
    else {
        // 解除判定
        bool unevolve = (g_Player.TransformTimer-- <= 0) || Keyboard_IsKeyDownTrigger(KK_D8);
        if (g_Player.TransformType == TRANSFORM_TYPE::TRANSFORM_TYPE_A && g_Controller[0].GetLeftTrigger() >= 0.9f) unevolve = true;
        if (g_Player.TransformType == TRANSFORM_TYPE::TRANSFORM_TYPE_B && g_Controller[0].GetRightTrigger() >= 0.9f) unevolve = true;

        if (unevolve) {
            g_Player.TransformType = TRANSFORM_TYPE::TRANSFORM_TYPE_NONE;
            SetPlayer_IsTransformed(false);
            ApplyTransformationP1(&g_Player, g_Player.m_baseWT, false); // 元の武器に戻す
            SetWTP1(g_Player.m_baseWT);
            g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
        }
    }
}


void ApplyTransformEffect()
{
    // 現在の武器情報を取得
    WeaponTerrain weapon = GetSetWTP1();

    // 進化していないとき
    if (g_Player.TransformType == TRANSFORM_TYPE::TRANSFORM_TYPE_NONE)
    {
        switch (weapon) {
        case WeaponTerrain::SWORD_WALL:
            g_Player.m_moveSpeed = 0.2f; g_Player.m_jumpForce = 0.1f; break;
        case WeaponTerrain::SPEAR_HILL:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.08f; break;
        case WeaponTerrain::BOW_HILL:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.11f; break;
        case WeaponTerrain::SHURIKEN_:
            g_Player.m_moveSpeed = 0.24f; g_Player.m_jumpForce = 0.12f; break;
        case WeaponTerrain::HAMMER_:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.08f; break;
        default:
            g_Player.m_moveSpeed = 0.2f; g_Player.m_jumpForce = 0.1f; break;
        }
    }
    else
    {
        switch (weapon) {
        case WeaponTerrain::SWORD_WALL:
            g_Player.m_moveSpeed = 0.2f; g_Player.m_jumpForce = 0.1f; break;
        case WeaponTerrain::SPEAR_HILL:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.08f; break;
        case WeaponTerrain::BOW_HILL:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.11f; break;
        case WeaponTerrain::SHURIKEN_:
            g_Player.m_moveSpeed = 0.24f; g_Player.m_jumpForce = 0.12f; break;
        case WeaponTerrain::HAMMER_:
            g_Player.m_moveSpeed = 0.18f; g_Player.m_jumpForce = 0.08f; break;
        default:
            g_Player.m_moveSpeed = 0.2f; g_Player.m_jumpForce = 0.1f; break;
        }
    }
}

void TransformPlayer2()
{
    const char* newModelPath = nullptr;
    TRANSFORM_TYPE2 nextType = TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE;
    if (g_Player2.TransformType == TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
    {
        WeaponTerrain targetWT = WeaponTerrain::NONE;

        if ((Keyboard_IsKeyDownTrigger(KK_D2) || g_Controller[1].IsButtonPushed(ControllerButton::L_SHOULDER)) && !g_IsUsedA_P2)
        {
            g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_A;
            targetWT = g_TransformA_P2;
            g_IsUsedA_P2 = true;

            Player2_PlusTransformCount();
            SetPlayer2_IsTransformed(true);
        }
        else if ((Keyboard_IsKeyDownTrigger(KK_D9) || g_Controller[1].IsButtonPushed(ControllerButton::R_SHOULDER)) && !g_IsUsedB_P2)
        {
            g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_B;
            targetWT = g_TransformB_P2;
            g_IsUsedB_P2 = true;
            Player2_PlusTransformCount();
            SetPlayer2_IsTransformed(true);

        }
        if (targetWT != WeaponTerrain::NONE) {
            ApplyTransformationP2(&g_Player2, targetWT, true);
            g_Player2.m_isTransformed = true;
            g_Player2.TransformTimer = TRANSFORM_LIMIT_FRAME;
            

        }
    }
    else {
        // 解除判定
        bool unevolve = (g_Player2.TransformTimer-- <= 0) || Keyboard_IsKeyDownTrigger(KK_D8);
        if (g_Player2.TransformType == TRANSFORM_TYPE2::TRANSFORM_TYPE_A && g_Controller[1].GetLeftTrigger() >= 0.9f) unevolve = true;
        if (g_Player2.TransformType == TRANSFORM_TYPE2::TRANSFORM_TYPE_B && g_Controller[1].GetRightTrigger() >= 0.9f) unevolve = true;

        if (unevolve) {
            g_Player2.TransformType = TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE;
            g_Player2.m_isTransformed = false;
            ApplyTransformationP2(&g_Player2, g_Player2.m_baseWT, false); // 元の武器に戻す
            SetWTP2(g_Player2.m_baseWT);
            SetPlayer2_IsTransformed(false);

            g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
        }
    }
}
void ApplyTransformEffect2()
{
    // 現在の武器情報を取得
    WeaponTerrain weapon = GetSetWTP2();

    // 進化していないとき
    if (g_Player2.TransformType == TRANSFORM_TYPE2::TRANSFORM_TYPE_NONE)
    {
        switch (weapon) {
        case WeaponTerrain::SWORD_WALL:
            g_Player2.m_moveSpeed = 0.2f; g_Player2.m_jumpForce = 0.1f; break;
        case WeaponTerrain::SPEAR_HILL:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.08f; break;
        case WeaponTerrain::BOW_HILL:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.11f; break;
        case WeaponTerrain::SHURIKEN_:
            g_Player2.m_moveSpeed = 0.24f; g_Player2.m_jumpForce = 0.12f; break;
        case WeaponTerrain::HAMMER_:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.08f; break;
        default:
            g_Player2.m_moveSpeed = 0.2f; g_Player2.m_jumpForce = 0.1f; break;
        }
    }
    else
    {
        switch (weapon) {
        case WeaponTerrain::SWORD_WALL:
            g_Player2.m_moveSpeed = 0.2f; g_Player2.m_jumpForce = 0.1f; break;
        case WeaponTerrain::SPEAR_HILL:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.08f; break;
        case WeaponTerrain::BOW_HILL:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.11f; break;
        case WeaponTerrain::SHURIKEN_:
            g_Player2.m_moveSpeed = 0.24f; g_Player2.m_jumpForce = 0.12f; break;
        case WeaponTerrain::HAMMER_:
            g_Player2.m_moveSpeed = 0.18f; g_Player2.m_jumpForce = 0.08f; break;
        default:
            g_Player2.m_moveSpeed = 0.2f; g_Player2.m_jumpForce = 0.1f; break;
        }
    }
}

bool GetIsUsedA_P1()
{
    return g_IsUsedA_P1;
}
bool GetIsUsedB_P1()
{
    return g_IsUsedB_P1;
}
bool GetIsUsedA_P2()
{
    return g_IsUsedA_P2;
}
bool GetIsUsedB_P2()
{
    return g_IsUsedB_P2;
}
void SetIsUsed_P1(int no, bool flag)
{
    if (no == 0)
    {
        g_IsUsedA_P1 = flag;
    }
    else
    {
        g_IsUsedB_P1 = flag;
    }
}
void SetIsUsed_P2(int no, bool flag)
{
    if (no == 0)
    {
        g_IsUsedA_P2 = flag;
    }
    else
    {
        g_IsUsedB_P2 = flag;
    }
}
WeaponTerrain GetTransform_P1(int num)
{
    if (num == 0)
    {
        return g_TransformA_P1;
    }
    else
    {
        return g_TransformB_P1;
	}
}
WeaponTerrain GetTransform_P2(int num)
{
    if (num == 0)
    {
        return g_TransformA_P2;
    }
    else
    {
        return g_TransformB_P2;
	}
}