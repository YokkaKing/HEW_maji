// Evolution.cpp

#include "Evolution.h"
#include "Player.h"
#include "keyboard.h"
#include <cstdio>
#include <cstdlib>

extern PLAYER g_Player;

const char* INITIAL_MODEL_PATH = "asset\\model\\test.fbx";

void EvolutionInitialize()
{
    // 初期化処理
}

void EvolutionFinalize()
{
    // 終了処理
}

void EvolvePlayer()
{
    const char* newModelPath = nullptr;
    //共通の進化条件: 未進化 (EVOLUTION_TYPE_NONE) の場合のみ

    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;

        // Eキーが押されたらタイプAに変身
        if (Keyboard_IsKeyDownTrigger(KK_R))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx"; // A用モデルパス
            evolved = true;
        }
        // Rキーが押されたらタイプBに変身
        else if (Keyboard_IsKeyDownTrigger(KK_T))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_B;
            newModelPath = "asset\\model\\tree.fbx"; // B用モデルパス
            evolved = true;
        }

        // 変身が完了したら共通のスケール変更を適用
        if (evolved)
        {
            if (g_Player.m_model != nullptr)
            {
                ModelRelease(g_Player.m_model);
            }

            //新しいモデルをロード
            g_Player.m_model = ModelLoad(newModelPath);

            // 進化後の共通処理：スケール変更
           // g_Player.scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
        }
    }
    else if (g_Player.EvolutionType != EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        if (Keyboard_IsKeyDownTrigger(KK_F)) //Fキーが押されたかチェック
        {
            // 古いモデル（進化後のモデル）を解放
            if (g_Player.m_model != nullptr)
            {
                ModelRelease(g_Player.m_model);
            }
            // 進化タイプをリセット
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_NONE;

            // 初期モデルをロード
            g_Player.m_model = ModelLoad(INITIAL_MODEL_PATH);

            // スケールを初期値に戻す (1.0倍)
            g_Player.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
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