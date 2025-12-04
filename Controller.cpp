#include "Controller.h"
#include <cstdlib>
#include <algorithm>

// コントローラーIDを渡して初期化
Controller::Controller(DWORD controllerId)
    : m_controllerId(controllerId)
{
    // 初期状態をクリア
    ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
    ZeroMemory(&m_prevState, sizeof(XINPUT_STATE));
}

// 毎フレーム呼び出すことで状態を更新
void Controller::Update()
{
    // 現在の状態を前の状態として保存
    m_prevState = m_currentState;

    // XInputGetStateで現在の状態を取得
    DWORD result = XInputGetState(m_controllerId, &m_currentState);

    // 接続が切れている場合は状態をクリア（デバッグ目的）
    if (result != ERROR_SUCCESS)
    {
        ZeroMemory(&m_currentState, sizeof(XINPUT_STATE));
    }
}

// --- ボタンの状態取得関数 ---

// 現在押されているか (Keyboard_IsKeyDownに相当)
bool Controller::IsButtonDown(ControllerButton::Button button) const
{
    // 接続されていて、現在の状態のボタンフラグが立っているか
    return IsConnected() && (m_currentState.Gamepad.wButtons & button);
}

// 押された瞬間か (Keyboard_IsKeyPushedに相当)
bool Controller::IsButtonPushed(ControllerButton::Button button) const
{
    // 接続されていて
    // 現在は押されていて、かつ、前フレームでは押されていなかった
    return IsConnected() &&
        (m_currentState.Gamepad.wButtons & button) &&
        !(m_prevState.Gamepad.wButtons & button);
}

// 離された瞬間か (Keyboard_IsKeyReleasedに相当)
bool Controller::IsButtonReleased(ControllerButton::Button button) const
{
    // 接続されていて
    // 現在は押されていなくて、かつ、前フレームでは押されていた
    return IsConnected() &&
        !(m_currentState.Gamepad.wButtons & button) &&
        (m_prevState.Gamepad.wButtons & button);
}

// --- スティックの状態取得関数 (正規化とデッドゾーン処理) ---

// スティックの生の値 (-32768～32767) を -1.0f～1.0f に正規化し、デッドゾーンを処理
static float NormalizeAndDeadZone(SHORT value, SHORT deadZone)
{
    if (std::abs(value) < deadZone)
    {
        return 0.0f;
    }

    // デッドゾーンを超えた部分を正規化
    if (value > 0)
    {
        return (float)(value - deadZone) / (32767.0f - deadZone);
    }
    else
    {
        return (float)(value + deadZone) / (32768.0f - deadZone);
    }
}

float Controller::GetLeftStickX() const
{
    return NormalizeAndDeadZone(m_currentState.Gamepad.sThumbLX, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
}

float Controller::GetLeftStickY() const
{
    return NormalizeAndDeadZone(m_currentState.Gamepad.sThumbLY, XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
}

float Controller::GetRightStickX() const
{
    return NormalizeAndDeadZone(m_currentState.Gamepad.sThumbRX, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}

float Controller::GetRightStickY() const
{
    return NormalizeAndDeadZone(m_currentState.Gamepad.sThumbRY, XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE);
}


// --- トリガーの状態取得関数 (正規化) ---

// トリガーの生の値 (0～255) を 0.0f～1.0f に正規化
static float NormalizeTrigger(BYTE value, BYTE deadZone)
{
    if (value < deadZone)
    {
        return 0.0f;
    }
    // デッドゾーンを超えた部分を正規化
    return (float)(value - deadZone) / (255.0f - deadZone);
}

float Controller::GetLeftTrigger() const
{
    return NormalizeTrigger(m_currentState.Gamepad.bLeftTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}

float Controller::GetRightTrigger() const
{
    return NormalizeTrigger(m_currentState.Gamepad.bRightTrigger, XINPUT_GAMEPAD_TRIGGER_THRESHOLD);
}


// --- 接続状態チェック ---
bool Controller::IsConnected() const
{
    // XInputGetStateを試行し、ERROR_SUCCESSが返るかチェック
    XINPUT_STATE state;
    return XInputGetState(m_controllerId, &state) == ERROR_SUCCESS;
}


// --- バイブレーション機能 ---
// leftMotor: 低周波モーター (0.0f～1.0f), rightMotor: 高周波モーター (0.0f～1.0f)
void Controller::SetVibration(float leftMotor, float rightMotor)
{
    XINPUT_VIBRATION vibration;
    vibration.wLeftMotorSpeed = (WORD)((std::min)(1.0f, (std::max)(0.0f, leftMotor)) * 65535.0f);
    vibration.wRightMotorSpeed = (WORD)((std::min)(1.0f, (std::max)(0.0f, rightMotor)) * 65535.0f);
    XInputSetState(m_controllerId, &vibration);
}
