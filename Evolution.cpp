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
    //共通の進化条件: 未進化 (EVOLUTION_TYPE_NONE) の場合のみ

    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;
        // Eキーが押されたらタイプAに変身
        if (g_Controller.IsButtonPushed(ControllerButton::L_SHOULDER))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx"; // A用モデルパス
            evolved = true;
        }
        // Rキーが押されたらタイプBに変身
        else if (g_Controller.IsButtonPushed(ControllerButton::R_SHOULDER))
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
        bool unevolve = false;

        if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_A)
        {
            // タイプA の時、LT (左トリガー) が完全に押されたら解除
            if (g_Controller.GetLeftTrigger() >= 0.9f)
            {
                unevolve = true;
            }
        }
        else if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
        {
            // タイプB の時、RT (右トリガー) が完全に押されたら解除
            if (g_Controller.GetRightTrigger() >= 0.9f)
            {
                unevolve = true;
            }
        }
        if (unevolve) // 解除条件が満たされたらリセット処理を実行
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




//ここから下はプレイヤー２をコントローラーにしたら削除

void EvolvePlayer2()
{
    const char* newModelPath = nullptr;
    //共通の進化条件: 未進化 (EVOLUTION_TYPE_NONE) の場合のみ
    if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;

        //タイプAに変身
        if (Keyboard_IsKeyDownTrigger(KK_Y))
        {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx"; // A用モデルパス
            evolved = true;
        }
        //タイプBに変身
        else if (Keyboard_IsKeyDownTrigger(KK_I))
        {
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_B;
            newModelPath = "asset\\model\\tree.fbx"; // B用モデルパス
            evolved = true;
        }

        // 変身が完了したら共通のスケール変更を適用
        if (evolved)
        {
            if (g_Player2.m_model != nullptr)
            {
                ModelRelease(g_Player2.m_model);
            }

            //新しいモデルをロード
            g_Player2.m_model = ModelLoad(newModelPath);

            // 進化後の共通処理：スケール変更
            // g_Player2.scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
        }
    }

    else if (g_Player2.EvolutionType != EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE)
    {
        bool unevolve = false;
        if (g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_A ||
            g_Player2.EvolutionType == EVOLUTION_TYPE2::EVOLUTION_TYPE_B)
        {
            //キーボード KK_F が押されたら解除 (タイプA, B共通)
            if (Keyboard_IsKeyDownTrigger(KK_O))
            {
                unevolve = true;
            }
        }

        if (unevolve) // 解除条件が満たされたらリセット処理を実行
        {
            // 古いモデル（進化後のモデル）を解放
            if (g_Player2.m_model != nullptr)
            {
                ModelRelease(g_Player2.m_model);
            }
            // 進化タイプをリセット
            g_Player2.EvolutionType = EVOLUTION_TYPE2::EVOLUTION_TYPE_NONE;

            // 初期モデルをロード
            g_Player2.m_model = ModelLoad(INITIAL_MODEL_PATH);

            // スケールを初期値に戻す (1.0倍)
            g_Player2.m_scale = XMFLOAT3(1.0f, 1.0f, 1.0f);
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
    //共通の進化条件: 未進化 (EVOLUTION_TYPE_NONE) の場合のみ
    if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool evolved = false;

        //タイプAに変身
        if (Keyboard_IsKeyDownTrigger(KK_T))
        {
            g_Player.EvolutionType = EVOLUTION_TYPE::EVOLUTION_TYPE_A;
            newModelPath = "asset\\model\\ball.fbx"; // A用モデルパス
            evolved = true;
        }
        //タイプBに変身
        else if (Keyboard_IsKeyDownTrigger(KK_R))
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
            // g_Player2.scale = XMFLOAT3(1.5f, 1.5f, 1.5f);
        }
    }

    else if (g_Player.EvolutionType != EVOLUTION_TYPE::EVOLUTION_TYPE_NONE)
    {
        bool unevolve = false;
        if (g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_A ||
            g_Player.EvolutionType == EVOLUTION_TYPE::EVOLUTION_TYPE_B)
        {
            //キーボード KK_F が押されたら解除 (タイプA, B共通)
            if (Keyboard_IsKeyDownTrigger(KK_F))
            {
                unevolve = true;
            }
        }

        if (unevolve) // 解除条件が満たされたらリセット処理を実行
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
