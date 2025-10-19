#include "DX8InputManager.h"

#include "CKAll.h"

void DX8InputManager::EnableKeyboardRepetition(CKBOOL iEnable)
{
    m_EnableKeyboardRepetition = iEnable;
}

CKBOOL DX8InputManager::IsKeyboardRepetitionEnabled()
{
    return m_EnableKeyboardRepetition;
}

CKBOOL DX8InputManager::IsKeyDown(CKDWORD iKey, CKDWORD *oStamp)
{
    if (iKey >= KEYBOARD_BUFFER_SIZE)
        return FALSE;
    if ((m_KeyboardState[iKey] & KS_PRESSED) == 0)
        return FALSE;
    if (oStamp) *oStamp = m_KeyboardStamps[iKey];
    return TRUE;
}

CKBOOL DX8InputManager::IsKeyUp(CKDWORD iKey)
{
    return iKey < KEYBOARD_BUFFER_SIZE && m_KeyboardState[iKey] == KS_IDLE;
}

CKBOOL DX8InputManager::IsKeyToggled(CKDWORD iKey, CKDWORD *oStamp)
{
    if (iKey >= KEYBOARD_BUFFER_SIZE)
        return FALSE;
    if ((m_KeyboardState[iKey] & KS_RELEASED) == 0)
        return FALSE;
    if (oStamp) *oStamp = m_KeyboardStamps[iKey];
    return TRUE;
}

int DX8InputManager::GetKeyName(CKDWORD iKey, CKSTRING oKeyName)
{
    return VxScanCodeToName(iKey, oKeyName);
}

CKDWORD DX8InputManager::GetKeyFromName(CKSTRING iKeyName)
{
    char keyName[32];
    CKDWORD iKey;
    for (iKey = 0; iKey < KEYBOARD_BUFFER_SIZE; iKey++)
        if (GetKeyName(iKey, keyName) != 0 && stricmp(keyName, iKeyName) == 0)
            break;
    return iKey;
}

unsigned char *DX8InputManager::GetKeyboardState()
{
    return m_KeyboardState;
}

CKBOOL DX8InputManager::IsKeyboardAttached()
{
    return m_Keyboard != NULL;
}

int DX8InputManager::GetNumberOfKeyInBuffer()
{
    return m_NumberOfKeyInBuffer;
}

int DX8InputManager::GetKeyFromBuffer(int i, CKDWORD &oKey, CKDWORD *oTimeStamp)
{
    if (i >= m_NumberOfKeyInBuffer) return 0;
    oKey = m_KeyInBuffer[i].dwOfs;
    if (oTimeStamp) *oTimeStamp = m_KeyInBuffer[i].dwTimeStamp;
    return (m_KeyInBuffer[i].dwData & 0x80) ? KS_PRESSED : KS_RELEASED;
}

CKBOOL DX8InputManager::IsMouseButtonDown(CK_MOUSEBUTTON iButton)
{
    return m_Mouse.m_State.rgbButtons[iButton] & KS_PRESSED;
}

CKBOOL DX8InputManager::IsMouseClicked(CK_MOUSEBUTTON iButton)
{
    return (m_Mouse.m_State.rgbButtons[iButton] & KS_PRESSED) != 0 && (m_Mouse.m_LastButtons[iButton] & KS_PRESSED) == 0;
}

CKBOOL DX8InputManager::IsMouseToggled(CK_MOUSEBUTTON iButton)
{
    // Check if button was released (bit 1 set)
    // (state >> 1) & KS_PRESSED is equivalent to (state & KS_RELEASED)
    return (m_Mouse.m_State.rgbButtons[iButton] >> 1) & KS_PRESSED;
}

void DX8InputManager::GetMouseButtonsState(CKBYTE *oStates)
{
    *(CKDWORD *)oStates = *(CKDWORD *)(m_Mouse.m_State.rgbButtons);
}

void DX8InputManager::GetMousePosition(Vx2DVector &oPosition, CKBOOL iAbsolute)
{
    if (iAbsolute)
    {
        oPosition = m_Mouse.m_Position;
    }
    else
    {
        CKRenderContext *rc = m_Context->GetPlayerRenderContext();
        if (rc)
        {
            VxRect rect;
            rc->GetWindowRect(rect, TRUE);
            oPosition.Set(m_Mouse.m_Position.x - rect.left, m_Mouse.m_Position.y - rect.top);
        }
        else
        {
            oPosition = m_Mouse.m_Position;
        }
    }
}

void DX8InputManager::GetMouseRelativePosition(VxVector &oPosition)
{
    oPosition.Set((float)m_Mouse.m_State.lX, (float)m_Mouse.m_State.lY, (float)m_Mouse.m_State.lZ);
}

CKBOOL DX8InputManager::IsMouseAttached()
{
    return m_Mouse.m_Device != NULL;
}

CKBOOL DX8InputManager::IsJoystickAttached(int iJoystick)
{
    return 0 <= iJoystick && iJoystick < m_JoystickCount && m_Joysticks[iJoystick].IsAttached();
}

void DX8InputManager::GetJoystickPosition(int iJoystick, VxVector *oPosition)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return;

    if (oPosition)
    {
        CKJoystick *joystick = &m_Joysticks[iJoystick];
        joystick->Poll();
        *oPosition = joystick->m_Position;
    }
}

void DX8InputManager::GetJoystickRotation(int iJoystick, VxVector *oRotation)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return;

    if (oRotation)
    {
        CKJoystick *joystick = &m_Joysticks[iJoystick];
        joystick->Poll();
        *oRotation = joystick->m_Rotation;
    }
}

void DX8InputManager::GetJoystickSliders(int iJoystick, Vx2DVector *oPosition)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return;

    if (oPosition)
    {
        CKJoystick *joystick = &m_Joysticks[iJoystick];
        joystick->Poll();
        *oPosition = joystick->m_Sliders;
    }
}

void DX8InputManager::GetJoystickPointOfViewAngle(int iJoystick, float *oAngle)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return;

    if (oAngle)
    {
        CKJoystick *joystick = &m_Joysticks[iJoystick];
        joystick->Poll();
        *oAngle = (joystick->m_PointOfViewAngle == -1)
                      ? -1.0f
                      : (float)(joystick->m_PointOfViewAngle * PI * 0.000055555556);
    }
}

CKDWORD DX8InputManager::GetJoystickButtonsState(int iJoystick)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return 0;

    CKJoystick *joystick = &m_Joysticks[iJoystick];
    joystick->Poll();
    return joystick->m_Buttons;
}

CKBOOL DX8InputManager::IsJoystickButtonDown(int iJoystick, int iButton)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return FALSE;

    CKJoystick *joystick = &m_Joysticks[iJoystick];
    joystick->Poll();
    return (joystick->m_Buttons & (1 << iButton)) != 0;
}

void DX8InputManager::Pause(CKBOOL pause)
{
    if (pause && !m_Paused)
    {
        ::OutputDebugString(TEXT("InputManager Paused"));
        ClearBuffers();
        m_Paused = pause;
        m_WasPaused = TRUE;  // Set flag to prevent redundant clears in PreProcess
        return;
    }
    else if (m_Paused)
    {
        ::OutputDebugString(TEXT("InputManager Un-Paused"));
    }

    m_Paused = pause;
}

void DX8InputManager::ShowCursor(CKBOOL iShow)
{
    m_ShowCursor = iShow;
    EnsureCursorVisible(iShow);
}

void DX8InputManager::EnsureCursorVisible(CKBOOL iShow)
{
    if (iShow)
    {
        int dc = VxShowCursor(TRUE);
        if (dc > 0)
            dc = VxShowCursor(FALSE);
        if (dc < 0)
            while (VxShowCursor(TRUE) < 0)
                continue;
    }
    else
    {
        int dc = VxShowCursor(FALSE);
        if (dc < -1)
            dc = VxShowCursor(TRUE);
        if (dc >= 0)
            while (VxShowCursor(FALSE) >= 0)
                continue;
    }
}

CKBOOL DX8InputManager::GetCursorVisibility()
{
    return m_ShowCursor;
}

VXCURSOR_POINTER DX8InputManager::GetSystemCursor()
{
    return m_Cursor;
}

void DX8InputManager::SetSystemCursor(VXCURSOR_POINTER cursor)
{
    m_Cursor = cursor;
    VxSetCursor(cursor);
}

int DX8InputManager::GetJoystickCount()
{
    return m_JoystickCount;
}

IDirectInputDevice8 *DX8InputManager::GetJoystickDxInterface(int iJoystick)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return NULL;
    else
        return m_Joysticks[iJoystick].m_Device;  // Returns NULL for XInput devices, which is correct
}

void DX8InputManager::SetJoystickDeadzone(int iJoystick, float radius)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount)
    {
        // Clamp deadzone radius to valid range [0.0, 1.0]
        if (radius < 0.0f) radius = 0.0f;
        if (radius > 1.0f) radius = 1.0f;
        m_Joysticks[iJoystick].m_DeadzoneRadius = radius;
    }
}

float DX8InputManager::GetJoystickDeadzone(int iJoystick)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount)
        return m_Joysticks[iJoystick].m_DeadzoneRadius;
    return 0.01f;  // Default deadzone
}

void DX8InputManager::SetMaxJoysticks(int maxJoysticks)
{
    // Can only change before initialization
    if (m_Joysticks != NULL)
    {
        ::OutputDebugString(TEXT("DX8InputManager: Cannot change max joysticks after initialization"));
        return;
    }

    // Clamp to reasonable range (1-16)
    if (maxJoysticks < 1) maxJoysticks = 1;
    if (maxJoysticks > 16) maxJoysticks = 16;

    m_MaxJoysticks = maxJoysticks;
}

int DX8InputManager::GetMaxJoysticks()
{
    return m_MaxJoysticks;
}

void DX8InputManager::SetKeyDown(CKDWORD iKey)
{
    if (iKey < KEYBOARD_BUFFER_SIZE)
    {
        m_KeyboardState[iKey] |= KS_PRESSED;
        m_KeyboardStamps[iKey] = ::GetTickCount();
    }
}

void DX8InputManager::SetKeyUp(CKDWORD iKey)
{
    if (iKey < KEYBOARD_BUFFER_SIZE)
    {
        m_KeyboardState[iKey] |= KS_RELEASED;
        m_KeyboardStamps[iKey] = ::GetTickCount();
    }
}

void DX8InputManager::SetMouseButtonDown(CK_MOUSEBUTTON iButton)
{
    if (iButton < 4)
    {
        m_Mouse.m_State.rgbButtons[iButton] |= KS_PRESSED;
    }
}

void DX8InputManager::SetMouseButtonUp(CK_MOUSEBUTTON iButton)
{
    if (iButton < 4)
    {
        m_Mouse.m_State.rgbButtons[iButton] |= KS_RELEASED;
    }
}

void DX8InputManager::SetMousePosition(const Vx2DVector &position)
{
    m_Mouse.m_Position = position;
    ::SetCursorPos((int)position.x, (int)position.y);
}

void DX8InputManager::SetJoystickButtonDown(int iJoystick, int iButton)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount && iButton >= 0 && iButton < 32)
    {
        m_Joysticks[iJoystick].m_Buttons |= (1 << iButton);
        m_Joysticks[iJoystick].m_Polled = TRUE;
    }
}

void DX8InputManager::SetJoystickButtonUp(int iJoystick, int iButton)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount && iButton >= 0 && iButton < 32)
    {
        m_Joysticks[iJoystick].m_Buttons &= ~(1 << iButton);
        m_Joysticks[iJoystick].m_Polled = TRUE;
    }
}

void DX8InputManager::SetJoystickPosition(int iJoystick, const VxVector &position)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount)
    {
        m_Joysticks[iJoystick].m_Position = position;
        m_Joysticks[iJoystick].m_Polled = TRUE;
    }
}

void DX8InputManager::SetKeyboardState(const CKBYTE *states, const int *stamps)
{
    if (states)
    {
        memcpy(m_KeyboardState, states, sizeof(m_KeyboardState));
        if (stamps)
            memcpy(m_KeyboardStamps, stamps, sizeof(m_KeyboardStamps));
    }
}

void DX8InputManager::SetMouseState(const Vx2DVector &pos, const CKBYTE *buttons, const VxVector &delta)
{
    m_Mouse.m_Position = pos;
    if (buttons)
        memcpy(m_Mouse.m_State.rgbButtons, buttons, 4);
    m_Mouse.m_State.lX = (long)delta.x;
    m_Mouse.m_State.lY = (long)delta.y;
    m_Mouse.m_State.lZ = (long)delta.z;
}

void DX8InputManager::SetJoystickCompleteState(int iJoystick, const VxVector &pos, const VxVector &rot, const Vx2DVector &sliders, CKDWORD buttons, CKDWORD pov)
{
    if (iJoystick >= 0 && iJoystick < m_JoystickCount)
    {
        m_Joysticks[iJoystick].m_Position = pos;
        m_Joysticks[iJoystick].m_Rotation = rot;
        m_Joysticks[iJoystick].m_Sliders = sliders;
        m_Joysticks[iJoystick].m_Buttons = buttons;
        m_Joysticks[iJoystick].m_PointOfViewAngle = (pov == 0xFFFFFFFF) ? -1 : (LONG)pov;
        m_Joysticks[iJoystick].m_Polled = TRUE;
    }
}

void DX8InputManager::SetMultipleKeys(const CKDWORD *keys, int count, CKBOOL pressed)
{
    if (!keys) return;

    for (int i = 0; i < count; i++)
    {
        CKDWORD key = keys[i];
        if (key < KEYBOARD_BUFFER_SIZE)
        {
            if (pressed)
            {
                m_KeyboardState[key] |= KS_PRESSED;
                m_KeyboardStamps[key] = ::GetTickCount();
            }
            else
            {
                m_KeyboardState[key] |= KS_RELEASED;
                m_KeyboardStamps[key] = ::GetTickCount();
            }
        }
    }
}

void DX8InputManager::ClearAllInputState()
{
    memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
    memset(m_KeyboardStamps, 0, sizeof(m_KeyboardStamps));
    memset(m_Mouse.m_State.rgbButtons, 0, sizeof(m_Mouse.m_State.rgbButtons));
    memset(m_Mouse.m_LastButtons, 0, sizeof(m_Mouse.m_LastButtons));

    for (int i = 0; i < m_JoystickCount; i++)
    {
        m_Joysticks[i].m_Buttons = 0;
        m_Joysticks[i].m_Position.Set(0, 0, 0);
        m_Joysticks[i].m_Rotation.Set(0, 0, 0);
        m_Joysticks[i].m_Sliders.Set(0, 0);
        m_Joysticks[i].m_PointOfViewAngle = -1;
    }
}

CKBOOL DX8InputManager::IsKeyAllowed(CKDWORD key)
{
    if (!m_Filter.keyboardEnabled)
        return FALSE;

    // If allowlist is set, check if key is in the list
    if (m_Filter.allowedKeys && m_Filter.allowedKeyCount > 0)
    {
        for (int i = 0; i < m_Filter.allowedKeyCount; i++)
        {
            if (m_Filter.allowedKeys[i] == key)
                return TRUE;
        }
        return FALSE;
    }

    // No filter set - all keys allowed
    return TRUE;
}

// Mouse wheel enhancements
int DX8InputManager::GetMouseWheelDelta()
{
    return m_Mouse.m_State.lZ;
}

int DX8InputManager::GetMouseWheelPosition()
{
    return m_MouseWheelPosition;
}

void DX8InputManager::SetMouseWheel(int wheelDelta)
{
    m_Mouse.m_State.lZ = wheelDelta;
    m_MouseWheelPosition += wheelDelta;
    TriggerEvent(4, wheelDelta, 0);
}

void DX8InputManager::SetMouseWheelPosition(int position)
{
    int delta = position - m_MouseWheelPosition;
    m_Mouse.m_State.lZ = delta;
    m_MouseWheelPosition = position;
    TriggerEvent(4, delta, 0);
}

// Input filtering methods
void DX8InputManager::SetInputFilter(CKBOOL keyboard, CKBOOL mouse, CKBOOL joystick)
{
    m_Filter.keyboardEnabled = keyboard;
    m_Filter.mouseEnabled = mouse;
    m_Filter.joystickEnabled = joystick;
}

void DX8InputManager::SetKeyFilter(const CKDWORD *allowedKeys, int count)
{
    if (m_Filter.allowedKeys)
        delete [] m_Filter.allowedKeys;

    if (allowedKeys && count > 0)
    {
        m_Filter.allowedKeys = new CKDWORD[count];
        if (m_Filter.allowedKeys)
        {
            memcpy(m_Filter.allowedKeys, allowedKeys, count * sizeof(CKDWORD));
            m_Filter.allowedKeyCount = count;
        }
        else
        {
            // Memory allocation failed
            ::OutputDebugString(TEXT("DX8InputManager: Failed to allocate memory for key filter"));
            m_Filter.allowedKeyCount = 0;
        }
    }
    else
    {
        m_Filter.allowedKeys = NULL;
        m_Filter.allowedKeyCount = 0;
    }
}

void DX8InputManager::ClearInputFilters()
{
    m_Filter.keyboardEnabled = TRUE;
    m_Filter.mouseEnabled = TRUE;
    m_Filter.joystickEnabled = TRUE;

    if (m_Filter.allowedKeys)
    {
        delete [] m_Filter.allowedKeys;
        m_Filter.allowedKeys = NULL;
    }
    m_Filter.allowedKeyCount = 0;
}

CKBOOL DX8InputManager::IsInputFiltered(CKDWORD key)
{
    return IsKeyAllowed(key);
}

// Event callback methods
void DX8InputManager::RegisterEventCallback(InputEventCallback callback, void *userData)
{
    if (!callback)
        return;

    if (m_CallbackCount < 8)
    {
        // Check if callback already registered
        for (int i = 0; i < m_CallbackCount; i++)
        {
            if (m_EventCallbacks[i] == callback)
            {
                m_CallbackUserData[i] = userData;
                return;
            }
        }

        // Add new callback
        m_EventCallbacks[m_CallbackCount] = callback;
        m_CallbackUserData[m_CallbackCount] = userData;
        m_CallbackCount++;
    }
    else
    {
        // Warn that callback limit has been reached
        ::OutputDebugString(TEXT("DX8InputManager: Cannot register callback - maximum limit (8) reached"));
    }
}

void DX8InputManager::UnregisterEventCallback(InputEventCallback callback)
{
    for (int i = 0; i < m_CallbackCount; i++)
    {
        if (m_EventCallbacks[i] == callback)
        {
            // Shift remaining callbacks
            for (int j = i; j < m_CallbackCount - 1; j++)
            {
                m_EventCallbacks[j] = m_EventCallbacks[j + 1];
                m_CallbackUserData[j] = m_CallbackUserData[j + 1];
            }
            m_CallbackCount--;
            break;
        }
    }
}

void DX8InputManager::TriggerEvent(CKDWORD eventType, CKDWORD param1, CKDWORD param2)
{
    for (int i = 0; i < m_CallbackCount; i++)
    {
        if (m_EventCallbacks[i])
        {
            m_EventCallbacks[i](eventType, param1, param2, m_CallbackUserData[i]);
        }
    }
}

CKERROR DX8InputManager::OnCKInit()
{
    if (!m_Keyboard)
        Initialize((HWND)m_Context->GetMainWindow());
    return CK_OK;
}

CKERROR DX8InputManager::OnCKEnd()
{
    Uninitialize();
    return CK_OK;
}

CKERROR DX8InputManager::OnCKReset()
{
    m_ShowCursor = TRUE;
    ClearBuffers();
    return CK_OK;
}

CKERROR DX8InputManager::OnCKPause()
{
    if (!m_ShowCursor)
        EnsureCursorVisible(TRUE);
    return CK_OK;
}

CKERROR DX8InputManager::OnCKPlay()
{
    HWND hWnd = (HWND)m_Context->GetMainWindow();
    if (m_Keyboard)
    {
        m_Keyboard->Unacquire();
        m_Keyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        m_Keyboard->Acquire();
    }
    if (m_Mouse.m_Device)
    {
        m_Mouse.m_Device->Unacquire();
        m_Mouse.m_Device->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        m_Mouse.m_Device->Acquire();
    }
    for (int i = 0; i < m_JoystickCount; i++)
    {
        if (m_Joysticks[i].m_Device)
        {
            m_Joysticks[i].m_Device->Unacquire();
            m_Joysticks[i].m_Device->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
            m_Joysticks[i].m_Device->Acquire();
        }
    }

    m_Mouse.Poll(m_Paused);

    memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
    if (m_Keyboard)
    {
        m_NumberOfKeyInBuffer = KEYBOARD_BUFFER_SIZE;
        HRESULT hr;
        do
        {
            hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                m_Keyboard->Acquire();
                hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
            }
        } while (hr == DI_NOTATTACHED);
    }

    if (!m_ShowCursor)
        EnsureCursorVisible(FALSE);

    return CK_OK;
}

CKERROR DX8InputManager::PreProcess()
{
    if (m_Keyboard)
    {
        m_NumberOfKeyInBuffer = KEYBOARD_BUFFER_SIZE;
        HRESULT hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            m_Keyboard->Acquire();
            hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
        }

        if (!m_Paused)
        {
            if (hr <= DI_NOTATTACHED)
            {
                CKDWORD iKey;
                for (int i = 0; i < m_NumberOfKeyInBuffer; i++)
                {
                    iKey = m_KeyInBuffer[i].dwOfs;

                    // Apply key filtering
                    if (!IsKeyAllowed(iKey))
                        continue;

                    if (iKey < KEYBOARD_BUFFER_SIZE)
                    {
                        if ((m_KeyInBuffer[i].dwData & 0x80) != 0)
                        {
                            m_KeyboardState[iKey] |= KS_PRESSED;
                            m_KeyboardStamps[iKey] = m_KeyInBuffer[i].dwTimeStamp;
                            TriggerEvent(1, iKey, m_KeyboardStamps[iKey]); // Key press trigger
                        }
                        else
                        {
                            m_KeyboardState[iKey] |= KS_RELEASED;
                            m_KeyboardStamps[iKey] = m_KeyInBuffer[i].dwTimeStamp - m_KeyboardStamps[iKey];
                            TriggerEvent(5, iKey, m_KeyboardStamps[iKey]); // Key release trigger
                        }
                    }
                }
            }

            // Keyboard repetition: generates synthetic key events for held keys
            // Uses timestamp negation to track repetition state:
            // - Positive timestamp: Initial press, waiting for repeat delay
            // - Negative timestamp: Actively repeating at repeat interval
            if (m_EnableKeyboardRepetition)
            {
                for (int i = 0; i < KEYBOARD_BUFFER_SIZE; i++)
                {
                    if (m_KeyboardState[i] == KS_PRESSED)
                    {
                        if (m_KeyboardStamps[i] > 0 && ::GetTickCount() - m_KeyboardStamps[i] > m_KeyboardRepeatDelay)
                            m_KeyboardStamps[i] = -m_KeyboardStamps[i];
                        if (m_KeyboardStamps[i] < 0)
                        {
                            for (int t = m_KeyboardStamps[i] - m_KeyboardRepeatDelay + ::GetTickCount(); t > (int)m_KeyboardRepeatInterval;)
                            {
                                t -= m_KeyboardRepeatInterval;
                                m_KeyboardStamps[i] -= m_KeyboardRepeatInterval;
                                if (m_NumberOfKeyInBuffer < KEYBOARD_BUFFER_SIZE)
                                {
                                    m_KeyInBuffer[m_NumberOfKeyInBuffer].dwData = 0x80;
                                    m_KeyInBuffer[m_NumberOfKeyInBuffer].dwOfs = i;
                                    m_KeyInBuffer[m_NumberOfKeyInBuffer].dwTimeStamp = -m_KeyboardStamps[i];
                                    ++m_NumberOfKeyInBuffer;
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            // Only clear buffers when transitioning to pause state, not every frame
            if (!m_WasPaused)
            {
                memset(m_KeyInBuffer, 0, sizeof(m_KeyInBuffer));
                memset(m_KeyboardStamps, 0, sizeof(m_KeyboardStamps));
                memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
                m_NumberOfKeyInBuffer = 0;
                m_WasPaused = TRUE;
            }
        }
    }
    else if (m_WasPaused)
    {
        // Transitioning from paused to unpaused
        m_WasPaused = FALSE;
    }

    m_Mouse.Poll(m_Paused);

    // Check for hot-plugged controller changes (every ~1 second)
    CheckForDeviceChanges();

    for (int i = 0; i < m_JoystickCount; i++)
        m_Joysticks[i].m_Polled = FALSE;

    return CK_OK;
}

CKERROR DX8InputManager::PostProcess()
{
    for (int iKey = 0; iKey < KEYBOARD_BUFFER_SIZE; iKey++)
    {
        if ((m_KeyboardState[iKey] & KS_RELEASED) != 0)
            m_KeyboardState[iKey] = KS_IDLE;
    }

    for (int iButton = 0; iButton < 4; iButton++)
    {
        if ((m_Mouse.m_State.rgbButtons[iButton] & KS_RELEASED) != 0)
            m_Mouse.m_State.rgbButtons[iButton] = KS_IDLE;
    }

    return CK_OK;
}

DX8InputManager::~DX8InputManager()
{
    if (m_Filter.allowedKeys)
        delete [] m_Filter.allowedKeys;

    // Free dynamically allocated joystick array
    if (m_Joysticks)
        delete[] m_Joysticks;

    // Clear event callbacks
    memset(m_EventCallbacks, 0, sizeof(m_EventCallbacks));
    memset(m_CallbackUserData, 0, sizeof(m_CallbackUserData));
    m_CallbackCount = 0;
}

DX8InputManager::DX8InputManager(CKContext *context) : CKInputManager(context, "DirectX Input Manager")
{
    m_DirectInput = NULL;
    m_Keyboard = NULL;
    m_Joysticks = NULL;
    m_JoystickCount = 0;
    m_MaxJoysticks = 4;

    int keyboardDelay;
    DWORD keyboardSpeed;
    ::SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &keyboardDelay, 0);
    ::SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &keyboardSpeed, 0);
    m_KeyboardRepeatDelay = 50 * (5 * keyboardDelay + 5);
    m_KeyboardRepeatInterval = (CKDWORD)(1000.0 / (keyboardSpeed + 2.5));
    m_Paused = FALSE;
    m_WasPaused = FALSE;
    m_EnableKeyboardRepetition = FALSE;

    // Initialize input filter
    memset(&m_Filter, 0, sizeof(m_Filter));
    m_Filter.keyboardEnabled = TRUE;
    m_Filter.mouseEnabled = TRUE;
    m_Filter.joystickEnabled = TRUE;
    m_Filter.allowedKeys = NULL;
    m_Filter.allowedKeyCount = 0;

    // Initialize event system
    memset(m_EventCallbacks, 0, sizeof(m_EventCallbacks));
    memset(m_CallbackUserData, 0, sizeof(m_CallbackUserData));
    m_CallbackCount = 0;

    m_MouseWheelPosition = 0;

    // Initialize hot-plug detection
    m_LastDeviceCheckTime = ::GetTickCount();
    m_DeviceCheckInterval = 1000;  // Check for device changes every 1 second
    memset(m_DeviceGUIDs, 0, sizeof(m_DeviceGUIDs));
    memset(m_XInputStates, 0, sizeof(m_XInputStates));

    Initialize((HWND)m_Context->GetMainWindow());

    memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
    memset(m_KeyInBuffer, 0, sizeof(m_KeyInBuffer));
    m_NumberOfKeyInBuffer = KEYBOARD_BUFFER_SIZE;
    m_ShowCursor = TRUE;
    SetSystemCursor(VXCURSOR_NORMALSELECT);

    m_Context->RegisterNewManager(this);
}

void DX8InputManager::Initialize(HWND hWnd)
{
    // Allocate joystick array dynamically
    if (m_Joysticks == NULL)
    {
        m_Joysticks = new CKJoystick[m_MaxJoysticks];
        if (!m_Joysticks)
        {
            ::OutputDebugString(TEXT("DX8InputManager: Failed to allocate joystick array"));
            m_MaxJoysticks = 0;
        }
    }

    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    HRESULT hr = DirectInput8Create(::GetModuleHandle(TEXT("CK2.dll")), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_DirectInput, NULL);
    if (!m_DirectInput || FAILED(hr))
    {
        TCHAR msg[256];
        snprintf(msg, 256, TEXT("DX8InputManager: DirectInput8Create failed (HRESULT: 0x%08X)"), hr);
        ::OutputDebugString(msg);
        ::MessageBox(hWnd, TEXT("Cannot Initialize Input Manager"), TEXT("Initialization Error"), MB_OK);
        // Note: Continues execution to allow graceful degradation - methods check device availability
    }

    // Load XInput dynamically (optional, used for Xbox controllers)
    CKJoystick::LoadXInput();

    // Obtain an interface to the system keyboard device.
    if (m_DirectInput)
    {
        hr = m_DirectInput->CreateDevice(GUID_SysKeyboard, &m_Keyboard, NULL);
        if (FAILED(hr))
        {
            TCHAR msg[256];
            snprintf(msg, 256, TEXT("DX8InputManager: CreateDevice for keyboard failed (HRESULT: 0x%08X)"), hr);
            ::OutputDebugString(msg);
        }

        // Obtain an interface to the system mouse device.
        hr = m_DirectInput->CreateDevice(GUID_SysMouse, &m_Mouse.m_Device, NULL);
        if (FAILED(hr))
        {
            TCHAR msg[256];
            snprintf(msg, 256, TEXT("DX8InputManager: CreateDevice for mouse failed (HRESULT: 0x%08X)"), hr);
            ::OutputDebugString(msg);
        }

        // Look for some joysticks we can use.
        m_DirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, JoystickEnum, this, DIEDFL_ATTACHEDONLY);

        // After DirectInput enumeration, check for XInput devices (Xbox controllers)
        // XInput supports up to 4 controllers (indices 0-3)
        // Only enumerate XInput devices if the library was successfully loaded
        if (CKJoystick::IsXInputAvailable())
        {
            for (DWORD userIndex = 0; userIndex < XUSER_MAX_COUNT && m_JoystickCount < m_MaxJoysticks; userIndex++)
            {
                XINPUT_STATE state;
                if (CKJoystick::s_XInputGetState(userIndex, &state) == ERROR_SUCCESS)
                {
                    // XInput controller found, add it to the joystick list
                    m_Joysticks[m_JoystickCount].m_XInputUserIndex = userIndex;
                    m_XInputStates[userIndex] = 1;  // Mark as connected to prevent false connection event
                    m_JoystickCount++;
                    ::OutputDebugString(TEXT("DX8InputManager: XInput controller detected"));
                }
            }
        }
    }

    if (m_Keyboard)
    {
        // Set the data format to "keyboard format".
        // This tells DirectInput that we will be passing an array
        // of 256 bytes to IDirectInputDevice::GetDeviceState.
        m_Keyboard->SetDataFormat(&c_dfDIKeyboard);

        // Set the cooperative level to let DirectInput know how
        // this device should interact with the system and with
        // other DirectInput applications.
        m_Keyboard->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

        // Set the buffer size to 256 to let DirectInput uses
        // buffered I/O (buffer size = 256).
        DIPROPDWORD dipdw;
        dipdw.diph.dwSize = sizeof(DIPROPDWORD);
        dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        dipdw.diph.dwObj = 0;
        dipdw.diph.dwHow = DIPH_DEVICE;
        dipdw.dwData = KEYBOARD_BUFFER_SIZE;
        m_Keyboard->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph);

        // Acquire the newly created device.
        m_Keyboard->Acquire();
    }

    m_Mouse.Init(hWnd);

    for (int i = 0; i < m_JoystickCount; i++)
        m_Joysticks[i].Init(hWnd);
}

void DX8InputManager::Uninitialize()
{
    if (m_Keyboard)
    {
        // Unacquire the device.
        m_Keyboard->Unacquire();
        m_Keyboard->Release();
        m_Keyboard = NULL;
    }

    m_Mouse.Release();

    // Only release joysticks that were actually initialized
    for (int i = 0; i < m_JoystickCount; i++)
        m_Joysticks[i].Release();

    if (m_DirectInput)
    {
        m_DirectInput->Release();
        m_DirectInput = NULL;
    }

    // Unload XInput library (static, so only unload once)
    CKJoystick::UnloadXInput();
}

void DX8InputManager::ClearBuffers()
{
    if (m_Keyboard)
    {
        HRESULT hr;
        do
        {
            m_NumberOfKeyInBuffer = KEYBOARD_BUFFER_SIZE;
            hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
            if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
            {
                m_Keyboard->Acquire();
                hr = m_Keyboard->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_KeyInBuffer, (LPDWORD)&m_NumberOfKeyInBuffer, 0);
            }
        } while (hr == DI_NOTATTACHED);
    }
    m_Mouse.Clear();
    memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
}

void DX8InputManager::CheckForDeviceChanges()
{
    // Check if enough time has passed since last check
    DWORD currentTime = ::GetTickCount();
    if (currentTime - m_LastDeviceCheckTime < m_DeviceCheckInterval)
        return;

    m_LastDeviceCheckTime = currentTime;

    if (!m_DirectInput)
        return;

    // Save current device states for comparison
    GUID oldDeviceGUIDs[16];
    DWORD oldXInputStates[XUSER_MAX_COUNT];
    int oldJoystickCount = m_JoystickCount;

    memcpy(oldDeviceGUIDs, m_DeviceGUIDs, sizeof(oldDeviceGUIDs));
    memcpy(oldXInputStates, m_XInputStates, sizeof(oldXInputStates));

    // Build current device state snapshot
    memset(m_DeviceGUIDs, 0, sizeof(m_DeviceGUIDs));
    for (int i = 0; i < m_JoystickCount && i < 16; i++)
    {
        m_DeviceGUIDs[i] = m_Joysticks[i].m_DeviceGUID;
    }

    // Check XInput devices (only if XInput is available)
    if (CKJoystick::IsXInputAvailable())
    {
        for (DWORD userIndex = 0; userIndex < XUSER_MAX_COUNT; userIndex++)
        {
            XINPUT_STATE state;
            m_XInputStates[userIndex] = (CKJoystick::s_XInputGetState(userIndex, &state) == ERROR_SUCCESS) ? 1 : 0;
        }
    }
    else
    {
        // XInput not available, ensure all states are disconnected
        memset(m_XInputStates, 0, sizeof(m_XInputStates));
    }

    // Detect disconnected DirectInput devices
    for (int i = 0; i < oldJoystickCount; i++)
    {
        if (m_Joysticks[i].m_Device)
        {
            // Try to poll the device to check if it's still connected
            HRESULT hr = m_Joysticks[i].m_Device->Poll();
            if (FAILED(hr))
            {
                hr = m_Joysticks[i].m_Device->Acquire();
                if (FAILED(hr))
                {
                    // Device is no longer available
                    ::OutputDebugString(TEXT("DX8InputManager: DirectInput device disconnected"));
                    m_Joysticks[i].Release();

                    // Compact the joystick array to remove the gap using safe transfer
                    for (int j = i; j < m_JoystickCount - 1; j++)
                    {
                        m_Joysticks[j].TransferFrom(m_Joysticks[j + 1]);
                    }
                    m_JoystickCount--;

                    // Trigger event AFTER array compaction so index is correct
                    TriggerEvent(7, i, 0);  // Event type 7 = joystick disconnected

                    i--;  // Recheck this index since we shifted the array
                }
            }
        }
    }

    // Detect XInput device changes
    for (DWORD userIndex = 0; userIndex < XUSER_MAX_COUNT; userIndex++)
    {
        if (m_XInputStates[userIndex] != oldXInputStates[userIndex])
        {
            if (m_XInputStates[userIndex] == 1)
            {
                // XInput device connected
                if (m_JoystickCount < m_MaxJoysticks)
                {
                    // Find the joystick with this XInput index or add new one
                    CKBOOL found = FALSE;
                    for (int i = 0; i < m_JoystickCount; i++)
                    {
                        if (m_Joysticks[i].m_XInputUserIndex == userIndex)
                        {
                            found = TRUE;
                            break;
                        }
                    }

                    if (!found)
                    {
                        ::OutputDebugString(TEXT("DX8InputManager: XInput controller connected"));
                        m_Joysticks[m_JoystickCount].m_XInputUserIndex = userIndex;
                        HWND hWnd = (HWND)m_Context->GetMainWindow();
                        m_Joysticks[m_JoystickCount].Init(hWnd);
                        TriggerEvent(6, m_JoystickCount, userIndex);  // Event type 6 = joystick connected
                        m_JoystickCount++;
                    }
                }
            }
            else
            {
                // XInput device disconnected
                for (int i = 0; i < m_JoystickCount; i++)
                {
                    if (m_Joysticks[i].m_XInputUserIndex == userIndex)
                    {
                        ::OutputDebugString(TEXT("DX8InputManager: XInput controller disconnected"));
                        m_Joysticks[i].m_XInputUserIndex = (DWORD)-1;

                        // Compact the joystick array using safe transfer
                        for (int j = i; j < m_JoystickCount - 1; j++)
                        {
                            m_Joysticks[j].TransferFrom(m_Joysticks[j + 1]);
                        }
                        m_JoystickCount--;

                        // Trigger event AFTER array compaction so index is correct
                        TriggerEvent(7, i, userIndex);  // Event type 7 = joystick disconnected
                        break;
                    }
                }
            }
        }
    }

    // Check for newly attached DirectInput devices
    // Note: We don't re-enumerate all devices every time as it's expensive
    // Instead, we only check for disconnections above. Full re-enumeration
    // can be added if needed, but it requires tracking more state
}
