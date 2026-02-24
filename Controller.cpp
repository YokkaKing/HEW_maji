
#pragma comment(lib, "runtimeobject.lib")

#include "Controller.h"
#include <algorithm>

Controller g_Controller[2] = { Controller(0), Controller(1) };

Controller::Controller(DWORD id) : m_id(id), m_isConnected(false)
{
    m_currentState = {};
    m_prevState = {};
}

void Controller::Update()
{
    m_prevState = m_currentState;

    ComPtr<IGamepadStatics> gamepadStatics;
    HRESULT hr = ABI::Windows::Foundation::GetActivationFactory(
        HStringReference(RuntimeClass_Windows_Gaming_Input_Gamepad).Get(),
        &gamepadStatics);

    if (FAILED(hr)) return;

    ComPtr<ABI::Windows::Foundation::Collections::IVectorView<Gamepad*>> gamepads;
    if (FAILED(gamepadStatics->get_Gamepads(&gamepads))) return;

    unsigned int count = 0;
    gamepads->get_Size(&count);

    if (count > m_id)
    {
        // 毎回 GetAt で最新のインスタンスを取得し直す
        ComPtr<IGamepad> currentGamepad;
        if (SUCCEEDED(gamepads->GetAt(m_id, &currentGamepad)))
        {
            m_gamepad = currentGamepad;
            m_gamepad->GetCurrentReading(&m_currentState);
            m_isConnected = true;
        }
    }
    else
    {
        m_isConnected = false;
        m_gamepad = nullptr;
    }

    // デバッグ用出力：これで Buttons: の後の数字が変わるか確認してください
    if (m_isConnected) {
        char buf[128];
        sprintf_s(buf, "ID:%d Buttons:%u\n", m_id, (unsigned int)m_currentState.Buttons);
        OutputDebugStringA(buf);
    }
}
// ボタン押下判定（ビット演算）
bool Controller::IsButtonDown(ControllerButton::Button button) const
{
    if (!m_isConnected) return false;
    return (static_cast<unsigned int>(m_currentState.Buttons) & button);
}

// 押した瞬間
bool Controller::IsButtonPushed(ControllerButton::Button button) const
{
    if (!m_isConnected) return false;
    unsigned int btn = (unsigned int)button;
    unsigned int current = (unsigned int)m_currentState.Buttons;
    unsigned int prev = (unsigned int)m_prevState.Buttons;

    // 今回押されていて、前回押されていなければ「押した瞬間」
    return ((current & btn) != 0) && ((prev & btn) == 0);
}
// 離した瞬間
bool Controller::IsButtonReleased(ControllerButton::Button button) const
{
    if (!m_isConnected) return false;
    return !(static_cast<unsigned int>(m_currentState.Buttons) & button) &&
        (static_cast<unsigned int>(m_prevState.Buttons) & button);
}

// スティックの値取得（デッドゾーン処理を追加）
float Controller::GetLeftStickX() const {
    if (!m_isConnected) return 0.0f;
    float val = (float)m_currentState.LeftThumbstickX;
    return (fabs(val) < 0.1f) ? 0.0f : val; // 0.1(10%)未満の傾きは0にする
}

float Controller::GetLeftStickY() const {
    if (!m_isConnected) return 0.0f;
    float val = (float)m_currentState.LeftThumbstickY;
    return (fabs(val) < 0.1f) ? 0.0f : val;
}

float Controller::GetRightStickX() const {
    if (!m_isConnected) return 0.0f;
    float val = (float)m_currentState.RightThumbstickX;
    return (fabs(val) < 0.1f) ? 0.0f : val;
}

float Controller::GetRightStickY() const {
    if (!m_isConnected) return 0.0f;
    float val = (float)m_currentState.RightThumbstickY;
    return (fabs(val) < 0.1f) ? 0.0f : val;
}
// トリガーの値取得（WGIは 0.0～1.0）
float Controller::GetLeftTrigger() const { return m_isConnected ? (float)m_currentState.LeftTrigger : 0.0f; }
float Controller::GetRightTrigger() const { return m_isConnected ? (float)m_currentState.RightTrigger : 0.0f; }

// 振動の設定
void Controller::SetVibration(float leftMotor, float rightMotor)
{
    if (!m_isConnected || !m_gamepad) return;

    GamepadVibration vibration;
    vibration.LeftMotor = (double)leftMotor;
    vibration.RightMotor = (double)rightMotor;
    vibration.LeftTrigger = 0.0;
    vibration.RightTrigger = 0.0;

    m_gamepad->put_Vibration(vibration);
}