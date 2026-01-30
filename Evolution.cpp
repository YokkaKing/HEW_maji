/*
* ファイル名	Evolution.cpp
* タイトル	進化
* 作成者		イトイ
* 作成日		12月02日
* 更新日		12月02日
*/

//================================================================
//	インクルード
//================================================================
#include"Evolution.h"
#include"keyboard.h"
#include"Controller.h"
#include<cstdio>
#include<cstdlib>

//================================================================
//	グローバル変数
//================================================================
extern PLAYER g_Player;
extern PLAYER2 g_Player2;
extern Controller g_Controller[2];
const char* INITIAL_MODEL_PATH;

const char* INITIAL_MODEL_PATH_P1 = nullptr;
const char* INITIAL_MODEL_PATH_P2 = nullptr;

const int EVOLUTION_LIMIT_FRAME = 20 * 60;

void EvolutionInitialize(WeaponTerrain selectP1, WeaponTerrain selectP2)
{
    g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;
    g_Player.EvolutionTimer = 0;
    g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE;
    g_Player2.EvolutionTimer = 0;
    g_Player.m_moveSpeed = 0;
    g_Player.m_jumpForce = 0;
    // P1の初期モデル設定
    switch (selectP1) {
    case WeaponTerrain::SWORD_WALL: INITIAL_MODEL_PATH_P1 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::SPEAR_HILL: INITIAL_MODEL_PATH_P1 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::BOW_HILL:   INITIAL_MODEL_PATH_P1 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::HAMMER_:    INITIAL_MODEL_PATH_P1 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::SHURIKEN_:  INITIAL_MODEL_PATH_P1 = "asset\\model\\char_shuriken.fbx"; break;
    default:                        INITIAL_MODEL_PATH_P1 = "asset\\model\\default.fbx"; break;
    }

    // P2の初期モデル設定
    switch (selectP2) {
    case WeaponTerrain::SWORD_WALL: INITIAL_MODEL_PATH_P2 = "asset\\model\\char_default_sword_motion.fbx"; break;
    case WeaponTerrain::SPEAR_HILL: INITIAL_MODEL_PATH_P2 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::BOW_HILL:   INITIAL_MODEL_PATH_P2 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::HAMMER_:    INITIAL_MODEL_PATH_P2 = "asset\\model\\char_shuriken.fbx"; break;
    case WeaponTerrain::SHURIKEN_:  INITIAL_MODEL_PATH_P2 = "asset\\model\\char_shuriken.fbx"; break;
    default:                        INITIAL_MODEL_PATH_P2 = "asset\\model\\default.fbx"; break;
    }
}

void EvolutionFinalize()
{
    // 終了処理
}

void EvolvePlayer()
{
    const char* newModelPath = nullptr;

    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;
        if (Keyboard_IsKeyDownTrigger(KK_D1)||g_Controller[0].IsButtonPushed(ControllerButton::L_THUMB))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx";
            evolved = true;
        }
        else if (Keyboard_IsKeyDownTrigger(KK_D2)||g_Controller[0].IsButtonPushed(ControllerButton::R_THUMB))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_B;
            newModelPath = "asset\\model\\tree.fbx";
            evolved = true;
        }

        if (evolved)
        {
            if (g_Player.m_model != nullptr) ModelRelease(g_Player.m_model);
            g_Player.m_model = ModelLoad(newModelPath);
            g_Player.EvolutionTimer = EVOLUTION_LIMIT_FRAME;
        }
    }
    else // 進化中
    {
        bool unevolve = false;

        // タイマーカウントダウン
        if (g_Player.EvolutionTimer > 0)
        {
            g_Player.EvolutionTimer--;
            if (g_Player.EvolutionTimer <= 0) unevolve = true;
        }

        // 手動解除判定
        if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_A)
        {
            if (g_Controller[0].GetLeftTrigger() >= 0.9f) unevolve = true;
        }
        else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
        {
            if (g_Controller[0].GetRightTrigger() >= 0.9f) unevolve = true;
        }

        if (unevolve)
        {
            if (g_Player.m_model != nullptr) ModelRelease(g_Player.m_model);
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;
            g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH_P1);
            g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
            g_Player.EvolutionTimer = 0;
        }
    }
}


void ApplyEvolutionEffect()
{
    // 現在の武器情報を取得
    WeaponTerrain weapon = GetSetWTP1();

    // 進化していないとき
    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
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
            g_Player.m_moveSpeed =0.2f; g_Player.m_jumpForce = 0.1f; break;
        }
    }
    else
    {
        switch (weapon) {
        case WeaponTerrain::SWORD_WALL:
            g_Player.m_moveSpeed = 0.12f; g_Player.m_jumpForce = 0.20f; break;
        case WeaponTerrain::HAMMER_:
            g_Player.m_moveSpeed = 0.10f; g_Player.m_jumpForce = 0.18f; break;
        default:
            g_Player.m_moveSpeed = 0.12f; g_Player.m_jumpForce = 0.20f; break;
        }
    }
}



void EvolvePlayer2()
{
    const char* newModelPath = nullptr;
    if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE) {
        bool evolved = false;
        if (Keyboard_IsKeyDownTrigger(KK_D8)|| g_Controller[1].IsButtonPushed(ControllerButton::L_THUMB)) {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx";
            evolved = true;
        }
        else if (Keyboard_IsKeyDownTrigger(KK_D9)|| g_Controller[1].IsButtonPushed(ControllerButton::R_THUMB)) {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_B;
            newModelPath = "asset\\model\\tree.fbx";
            evolved = true;
        }
        if (evolved) {
            if (g_Player2.m_model != nullptr) ModelRelease(g_Player2.m_model);
            g_Player2.m_model = ModelLoad(newModelPath);
            g_Player2.EvolutionTimer = EVOLUTION_LIMIT_FRAME;
        }
    }
    else {
        bool unevolve = false;
        if (g_Player2.EvolutionTimer > 0) {
            g_Player2.EvolutionTimer--;
            if (g_Player2.EvolutionTimer <= 0) unevolve = true;
        }
        if (Keyboard_IsKeyDownTrigger(KK_D0)) unevolve = true;
        if (unevolve) {
            if (g_Player2.m_model != nullptr) ModelRelease(g_Player2.m_model);
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE;
            g_Player2.m_model = ModelLoad(INITIAL_MODEL_PATH_P2); // P2用を参照
            g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
            g_Player2.EvolutionTimer = 0;
        }
    }
}
void ApplyEvolutionEffect2()
{
    // Player 2用の進化効果
    if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE)
    {
        // 初期状態の基本パラメータ
        g_Player2.m_acceleration.x = 0.0f;
        g_Player2.m_acceleration.z = 0.0f;
        g_Player2.FrictionRate = 0.98f;
        g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    else if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_A)
    {
        // 進化先 A (機動力特化)
        g_Player2.m_acceleration.x = 0.005f;
        g_Player2.m_acceleration.z = 0.005f;
        g_Player2.FrictionRate = 0.99f; // 減速しにくくする (滑りやすい)
        // g_Player2.m_scale = XMFLOAT3(1.5f, 1.5f, 1.5f); // 必要に応じてスケール変更を適用
    }
    else if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_B)
    {
        // 進化先 B (制動・防御特化)
        g_Player2.m_acceleration.x = 0.0f;
        g_Player2.m_acceleration.z = 0.0f;
        g_Player2.FrictionRate = 0.95f; // 減速しやすくする (止まりやすい)
        // g_Player2.m_scale = XMFLOAT3(1.2f, 1.2f, 1.2f); // 必要に応じてスケール変更を適用
    }
}


