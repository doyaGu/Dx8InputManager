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
        *oAngle = (float)(joystick->m_PointOfViewAngle * 3.1415927 * 0.000055555556);
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
        ::OutputDebugString(TEXT("InPutManager Paused"));
        ClearBuffers();
        m_Paused = pause;
        return;
    }
    else if (m_Paused)
    {
        ::OutputDebugString(TEXT("InPutManager Un-Paused"));
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

IDirectInputDevice2 *DX8InputManager::GetJoystickDxInterface(int iJoystick)
{
    if (iJoystick < 0 || iJoystick >= m_JoystickCount)
        return NULL;
    else
        return m_Joysticks[iJoystick].m_Device;
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
            if (hr == DI_OK)
            {
                CKDWORD iKey;
                for (int i = 0; i < m_NumberOfKeyInBuffer; i++)
                {
                    iKey = m_KeyInBuffer[i].dwOfs;
                    if (iKey < KEYBOARD_BUFFER_SIZE)
                    {
                        if ((m_KeyInBuffer[i].dwData & 0x80) != 0)
                        {
							m_KeyboardState[iKey] |= KS_PRESSED;
                            m_KeyboardStamps[iKey] = m_KeyInBuffer[i].dwTimeStamp;
                        }
                        else
                        {
							m_KeyboardState[iKey] |= KS_RELEASED;
                            m_KeyboardStamps[iKey] = m_KeyInBuffer[i].dwTimeStamp - m_KeyboardStamps[iKey];
                        }
                    }
                }
            }

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
                                }
                            }
                        }
                    }
                }
            }
        }
        else
        {
            memset(m_KeyInBuffer, 0, sizeof(m_KeyInBuffer));
            memset(m_KeyboardStamps, 0, sizeof(m_KeyboardStamps));
            memset(m_KeyboardState, 0, sizeof(m_KeyboardState));
            m_NumberOfKeyInBuffer = 0;
        }
    }

    m_Mouse.Poll(m_Paused);

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

DX8InputManager::~DX8InputManager() {}

DX8InputManager::DX8InputManager(CKContext *context) : CKInputManager(context, "DirectX Input Manager")
{
    m_DirectInput = NULL;
    m_Keyboard = NULL;
    m_JoystickCount = 0;

    int keyboardDelay;
    DWORD keyboardSpeed;
    ::SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &keyboardDelay, 0);
    ::SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &keyboardSpeed, 0);
    m_KeyboardRepeatDelay = 50 * (5 * keyboardDelay + 5);
    m_KeyboardRepeatInterval = (CKDWORD)(1000.0 / (keyboardSpeed + 2.5));
    m_Paused = FALSE;
    m_EnableKeyboardRepetition = FALSE;

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
    // Register with the DirectInput subsystem and get a pointer
    // to a IDirectInput interface we can use.
    // Create a DInput object
    DirectInput8Create(::GetModuleHandle(TEXT("CK2.dll")), DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&m_DirectInput, NULL);
    if (!m_DirectInput)
    {
        ::OutputDebugString(TEXT("Cannot create, DirectInput Version 8"));
        ::MessageBox(hWnd, TEXT("Initialization Error"), TEXT("Cannot Initialize Input Manager"), MB_OK);
    }

    // Obtain an interface to the system keyboard device.
    m_DirectInput->CreateDevice(GUID_SysKeyboard, &m_Keyboard, NULL);

    // Obtain an interface to the system mouse device.
    m_DirectInput->CreateDevice(GUID_SysMouse, &m_Mouse.m_Device, NULL);

    // Look for some joysticks we can use.
    m_DirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, JoystickEnum, this, DIEDFL_ATTACHEDONLY);

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

    for (int i = 0; i < sizeof(m_Joysticks) / sizeof(CKJoystick); i++)
        m_Joysticks[i].Release();

    if (m_DirectInput)
    {
        m_DirectInput->Release();
        m_DirectInput = NULL;
    }
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
