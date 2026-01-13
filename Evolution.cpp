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
extern Controller g_Controller;
const char* INITIAL_MODEL_PATH;
const int EVOLUTION_LIMIT_FRAME = 2 * 60;

void EvolutionInitialize()
{
    // 初期化処理
    INITIAL_MODEL_PATH = "asset\\model\\char_hammer.fbx";
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
        if (g_Controller.IsButtonPushed(ControllerButton::L_SHOULDER))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx";
            evolved = true;
        }
        else if (g_Controller.IsButtonPushed(ControllerButton::R_SHOULDER))
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
            if (g_Controller.GetLeftTrigger() >= 0.9f) unevolve = true;
        }
        else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
        {
            if (g_Controller.GetRightTrigger() >= 0.9f) unevolve = true;
        }

        if (unevolve)
        {
            if (g_Player.m_model != nullptr) ModelRelease(g_Player.m_model);
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;
            g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH);
            g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
            g_Player.EvolutionTimer = 0;
        }
    }
}


void ApplyEvolutionEffect()
{
    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        // 初期状態の基本パラメータ
        g_Player.m_acceleration.x = 0.0f;
        g_Player.m_acceleration.z = 0.0f;
        g_Player.FrictionRate = 0.98f;
        g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_A)
    {
        // 進化先 A 
        g_Player.m_acceleration.x = 0.005f;
        g_Player.m_acceleration.z = 0.005f;
        g_Player.FrictionRate = 0.99f; // 減速しにくくする (滑りやすい)
    }
    else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
    {
        // 進化先 B
        g_Player.m_acceleration.x = 0.0f;
        g_Player.m_acceleration.z = 0.0f;
        g_Player.FrictionRate = 0.95f; // 減速しやすくする (止まりやすい)
    }
}




//ここから下はプレイヤー２をコントローラーにしたら削除

void EvolvePlayer2()
{
    const char* newModelPath = nullptr;

    if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;
        if (Keyboard_IsKeyDownTrigger(KK_Y))
        {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx";
            evolved = true;
        }
        else if (Keyboard_IsKeyDownTrigger(KK_I))
        {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_B;
            newModelPath = "asset\\model\\tree.fbx";
            evolved = true;
        }

        if (evolved)
        {
            if (g_Player2.m_model != nullptr) ModelRelease(g_Player2.m_model);
            g_Player2.m_model = ModelLoad(newModelPath);
            g_Player2.EvolutionTimer = EVOLUTION_LIMIT_FRAME;
        }
    }
    else // 進化中
    {
        bool unevolve = false;

        if (g_Player2.EvolutionTimer > 0)
        {
            g_Player2.EvolutionTimer--;
            if (g_Player2.EvolutionTimer <= 0) unevolve = true;
        }

        if (Keyboard_IsKeyDownTrigger(KK_O)) unevolve = true;

        if (unevolve)
        {
            if (g_Player2.m_model != nullptr) ModelRelease(g_Player2.m_model);
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE;
            g_Player2.m_model = ModelLoad(INITIAL_MODEL_PATH);
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

void EvolvePlayer3()
{
    const char* newModelPath = nullptr;

    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;
        if (Keyboard_IsKeyDownTrigger(KK_T))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx";
            evolved = true;
        }
        else if (Keyboard_IsKeyDownTrigger(KK_R))
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

        if (g_Player.EvolutionTimer > 0)
        {
            g_Player.EvolutionTimer--;
            if (g_Player.EvolutionTimer <= 0) unevolve = true;
        }

        if (Keyboard_IsKeyDownTrigger(KK_F)) unevolve = true;

        if (unevolve)
        {
            if (g_Player.m_model != nullptr) ModelRelease(g_Player.m_model);
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;
            g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH);
            g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
            g_Player.EvolutionTimer = 0;
        }
    }
}
void ApplyEvolutionEffect3()
{
    // Player 2用の進化効果
    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        // 初期状態の基本パラメータ
        g_Player.m_acceleration.x = 0.0f;
        g_Player.m_acceleration.z = 0.0f;
        g_Player.FrictionRate = 0.98f;
        g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
    }
    else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_A)
    {
        // 進化先 A (機動力特化)
        g_Player.m_acceleration.x = 0.005f;
        g_Player.m_acceleration.z = 0.005f;
        g_Player.FrictionRate = 0.99f; // 減速しにくくする (滑りやすい)
        // g_Player2.m_scale = XMFLOAT3(1.5f, 1.5f, 1.5f); // 必要に応じてスケール変更を適用
    }
    else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
    {
        // 進化先 B (制動・防御特化)
        g_Player.m_acceleration.x = 0.0f;
        g_Player.m_acceleration.z = 0.0f;
        g_Player.FrictionRate = 0.95f; // 減速しやすくする (止まりやすい)
        // g_Player2.m_scale = XMFLOAT3(1.2f, 1.2f, 1.2f); // 必要に応じてスケール変更を適用
    }
}
