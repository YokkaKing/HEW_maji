#pragma once
#include <windows.gaming.input.h>
#include <wrl.h>
#include <wrl/wrappers/corewrappers.h> // HStringReferenceのために必須
#include <Xinput.h> // DWORD定義などのため

// 必要な名前空間を使いやすく定義
using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Gaming::Input;
using namespace ABI::Windows::Foundation;

namespace ControllerButton
{
    // WGI(Windows.Gaming.Input)用のボタン定数
    // プレステコンでもXboxコンでも同じ位置のボタンが反応します
    enum Button
    {
        NONE = 0,
        DPAD_UP = (unsigned int)GamepadButtons::GamepadButtons_DPadUp,    // 1
        DPAD_DOWN = (unsigned int)GamepadButtons::GamepadButtons_DPadDown,  // 2
        DPAD_LEFT = (unsigned int)GamepadButtons::GamepadButtons_DPadLeft,  // 4
        DPAD_RIGHT = (unsigned int)GamepadButtons::GamepadButtons_DPadRight, // 8
        A_BUTTON = (unsigned int)GamepadButtons::GamepadButtons_A,         // 16
        B_BUTTON = (unsigned int)GamepadButtons::GamepadButtons_B,         // 32
        X_BUTTON = (unsigned int)GamepadButtons::GamepadButtons_X,         // 64
        Y_BUTTON = (unsigned int)GamepadButtons::GamepadButtons_Y,         // 128
        L_SHOULDER = (unsigned int)GamepadButtons::GamepadButtons_LeftShoulder, // 256
        R_SHOULDER = (unsigned int)GamepadButtons::GamepadButtons_RightShoulder,// 512
        L_THUMB = (unsigned int)GamepadButtons::GamepadButtons_LeftThumbstick,
        R_THUMB = (unsigned int)GamepadButtons::GamepadButtons_RightThumbstick,
        START = (unsigned int)GamepadButtons::GamepadButtons_Menu,
        BACK = (unsigned int)GamepadButtons::GamepadButtons_View,
    };
}

class Controller
{
private:
    ComPtr<IGamepad> m_gamepad;
    GamepadReading m_currentState;
    GamepadReading m_prevState;
    bool m_isConnected;
    DWORD m_id;
public:
    // Player.cppからの呼び出し(g_Controller(0))に合わせるため引数を残す
    Controller(DWORD id = 0);

    // 毎フレーム呼び出す更新関数
    void Update();

    // ボタンの状態取得
    bool IsButtonDown(ControllerButton::Button button) const;
    bool IsButtonPushed(ControllerButton::Button button) const;
    bool IsButtonReleased(ControllerButton::Button button) const;

    // スティックの状態取得 (-1.0f ～ 1.0f)
    float GetLeftStickX() const;
    float GetLeftStickY() const;
    float GetRightStickX() const;
    float GetRightStickY() const;

    // トリガーの状態取得 (0.0f ～ 1.0f)
    float GetLeftTrigger() const;
    float GetRightTrigger() const;

    // 接続状態
    bool IsConnected() const { return m_isConnected; }

    // バイブレーション (0.0f ～ 1.0f)
    void SetVibration(float leftMotor, float rightMotor);
};