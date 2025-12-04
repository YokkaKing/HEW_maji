#pragma once
#include <Windows.h>
#include <Xinput.h>

#pragma comment(lib, "Xinput.lib")

//コントローラーボタンの定数定義
namespace ControllerButton
{
    enum Button
    {
        // ダーティなポインタを避けるために0番目を使用しない慣習があります
        NONE = 0x0000,

        // 十字キー (XINPUT_GAMEPAD_DPAD_...)
        DPAD_UP = XINPUT_GAMEPAD_DPAD_UP,
        DPAD_DOWN = XINPUT_GAMEPAD_DPAD_DOWN,
        DPAD_LEFT = XINPUT_GAMEPAD_DPAD_LEFT,
        DPAD_RIGHT = XINPUT_GAMEPAD_DPAD_RIGHT,

        // スタート/バックボタン
        START = XINPUT_GAMEPAD_START,
        BACK = XINPUT_GAMEPAD_BACK,

        // L/R スティック押し込み
        L_THUMB = XINPUT_GAMEPAD_LEFT_THUMB,
        R_THUMB = XINPUT_GAMEPAD_RIGHT_THUMB,

        // L/R ショルダーボタン
        L_SHOULDER = XINPUT_GAMEPAD_LEFT_SHOULDER,
        R_SHOULDER = XINPUT_GAMEPAD_RIGHT_SHOULDER,

        // アクションボタン (A, B, X, Y)
        A_BUTTON = XINPUT_GAMEPAD_A,
        B_BUTTON = XINPUT_GAMEPAD_B,
        X_BUTTON = XINPUT_GAMEPAD_X,
        Y_BUTTON = XINPUT_GAMEPAD_Y
    };

    // --- スティック/トリガーの閾値 (必要な場合は調整してください) ---
    const float THUMB_THRESHOLD = 0.5f; // スティックの傾きを判定する閾値 (0.0～1.0)
    const int TRIGGER_THRESHOLD = 30;   // トリガーの押し込みを判定する閾値 (0～255)
}

// 

// --- コントローラー管理クラス ---
class Controller
{
private:
    DWORD m_controllerId; // プレイヤーID (0～3)
    XINPUT_STATE m_currentState;
    XINPUT_STATE m_prevState;

public:
    Controller(DWORD controllerId = 0);

    // 毎フレーム呼び出す更新関数
    void Update();

    //ボタンの状態取得関数

    // 現在押されているか
    bool IsButtonDown(ControllerButton::Button button) const;

    // 押された瞬間か
    bool IsButtonPushed(ControllerButton::Button button) const;

    // 離された瞬間か
    bool IsButtonReleased(ControllerButton::Button button) const;

    //スティックの状態取得関数

    // 左スティックのX軸の傾き (-1.0f～1.0f)
    float GetLeftStickX() const;

    // 左スティックのY軸の傾き (-1.0f～1.0f)
    float GetLeftStickY() const;

    // 右スティックのX軸の傾き (-1.0f～1.0f)
    float GetRightStickX() const;

    // 右スティックのY軸の傾き (-1.0f～1.0f)
    float GetRightStickY() const;

    // --- トリガーの状態取得関数 ---

    // 左トリガーの押し込み量 (0.0f～1.0f)
    float GetLeftTrigger() const;

    // 右トリガーの押し込み量 (0.0f～1.0f)
    float GetRightTrigger() const;

    // --- 接続状態チェック ---
    bool IsConnected() const;

    // --- バイブレーション機能 ---
    void SetVibration(float leftMotor, float rightMotor);
};