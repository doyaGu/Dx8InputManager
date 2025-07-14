#include "DX8InputManager.h"

DX8InputManager::CKMouse::CKMouse() : m_Device(NULL)
{
    memset(&m_State, 0, sizeof(m_State));
    memset(m_LastButtons, 0, sizeof(m_LastButtons));
    memset(m_Buffer, 0, sizeof(m_Buffer));
    m_NumberOfBuffer = 0;
}

void DX8InputManager::CKMouse::Init(HWND hWnd)
{
    if (!m_Device) return;

    if (FAILED(m_Device->SetDataFormat(&c_dfDIMouse)))
        ::OutputDebugString(TEXT("Input Manager =  Failed : SetDataFormat (Mouse)"));

    if (FAILED(m_Device->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND)))
        ::OutputDebugString(TEXT("Input Manager =  Failed : SetCooperativeLevel (Mouse)"));

    DIPROPDWORD dipdw;
    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = DIPROPAXISMODE_REL;
    if (FAILED(m_Device->SetProperty(DIPROP_AXISMODE, &dipdw.diph)))
        ::OutputDebugString(TEXT("Input Manager =  Failed : SetProperty (Mouse) Relative Coord"));

    dipdw.diph.dwSize = sizeof(DIPROPDWORD);
    dipdw.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipdw.diph.dwObj = 0;
    dipdw.diph.dwHow = DIPH_DEVICE;
    dipdw.dwData = 256;
    if (FAILED(m_Device->SetProperty(DIPROP_BUFFERSIZE, &dipdw.diph)))
        ::OutputDebugString(TEXT("Input Manager =  Failed : SetProperty (Mouse) Buffered Data"));
}

void DX8InputManager::CKMouse::Release()
{
    if (m_Device)
    {
        // Unacquire the device.
        m_Device->Unacquire();
        m_Device->Release();
        m_Device = NULL;
    }
}

void DX8InputManager::CKMouse::Clear()
{
    memset(m_LastButtons, 0, sizeof(m_LastButtons));
    memset(m_State.rgbButtons, 0, sizeof(m_State.rgbButtons));
}

void DX8InputManager::CKMouse::Poll(CKBOOL pause)
{
    if (!m_Device) return;

    HRESULT hr;

    *(CKDWORD *)m_LastButtons = *(CKDWORD *)m_State.rgbButtons;
    m_NumberOfBuffer = MOUSE_BUFFER_SIZE;
    hr = m_Device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_Buffer, (LPDWORD)&m_NumberOfBuffer, 0);
    if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
    {
        m_Device->Acquire();
        m_NumberOfBuffer = MOUSE_BUFFER_SIZE;
        hr = m_Device->GetDeviceData(sizeof(DIDEVICEOBJECTDATA), m_Buffer, (LPDWORD)&m_NumberOfBuffer, 0);
    }

    if (pause) m_NumberOfBuffer = 0;

    if (SUCCEEDED(hr))
    {
        for (int i = 0; i < m_NumberOfBuffer; i++)
        {
            switch (m_Buffer[i].dwOfs)
            {
            case DIMOFS_BUTTON0:
            case DIMOFS_BUTTON1:
            case DIMOFS_BUTTON2:
            case DIMOFS_BUTTON3:
                if ((m_Buffer[i].dwData & 0x80) != 0)
                    m_State.rgbButtons[m_Buffer[i].dwOfs - DIMOFS_BUTTON0] |= KS_PRESSED;
                else
                    m_State.rgbButtons[m_Buffer[i].dwOfs - DIMOFS_BUTTON0] |= KS_RELEASED;
                break;
            }
        }
    }

    if (!pause)
    {
        POINT pt;
        ::GetCursorPos(&pt);
        DIMOUSESTATE state;
        memset(&state, 0, sizeof(DIMOUSESTATE));
        m_Position.x = (float)pt.x;
        m_Position.y = (float)pt.y;
        hr = m_Device->GetDeviceState(sizeof(DIMOUSESTATE), &state);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            m_Device->Acquire();
            m_Device->GetDeviceState(sizeof(DIMOUSESTATE), &state);
        }
        m_State.lX = state.lX;
        m_State.lY = state.lY;
        m_State.lZ = state.lZ;
    }
}