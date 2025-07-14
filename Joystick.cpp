#include "DX8InputManager.h"

DX8InputManager::CKJoystick::CKJoystick()
{
    m_Device = NULL;
    m_JoyID = -1;
    m_Polled = FALSE;
}

void DX8InputManager::CKJoystick::Init(HWND hWnd)
{
    if (m_Device)
    {
        m_Device->SetDataFormat(&c_dfDIJoystick);
        m_Device->SetCooperativeLevel(hWnd, DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);
        m_Device->Acquire();
    }
    GetInfo();
}

void DX8InputManager::CKJoystick::Release()
{
    if (m_Device)
    {
        // Unacquire the device.
        m_Device->Unacquire();
        m_Device->Release();
        m_Device = NULL;
    }
}

void DX8InputManager::CKJoystick::Poll()
{
    if (m_Polled) return;

    if (m_Device)
    {
        m_Device->Poll();

        DIJOYSTATE state;
        memset(&state, 0, sizeof(DIJOYSTATE));
        if (m_Device->GetDeviceState(sizeof(DIJOYSTATE), &state) == DIERR_INPUTLOST)
        {
            m_Device->Acquire();
            m_Device->GetDeviceState(sizeof(DIJOYSTATE), &state);
        }

        double x = (((state.lX - m_Xmin) * 2.0) / (double)(m_Xmax - m_Xmin)) - 1.0;
        double y = (((state.lY - m_Ymin) * 2.0) / (double)(m_Ymax - m_Ymin)) - 1.0;
        double z = (((state.lZ - m_Zmin) * 2.0) / (double)(m_Zmax - m_Zmin)) - 1.0;
        if (fabs(x) < 0.01) x = 0.0;
        if (fabs(y) < 0.01) y = 0.0;
        if (fabs(z) < 0.01) z = 0.0;
        m_Position.Set((float)x, (float)y, (float)z);

        double xr = (((state.lRx - m_XRmin) * 2.0) / (double)(m_XRmax - m_XRmin)) - 1.0;
        double yr = (((state.lRy - m_YRmin) * 2.0) / (double)(m_YRmax - m_YRmin)) - 1.0;
        double zr = (((state.lRz - m_ZRmin) * 2.0) / (double)(m_ZRmax - m_ZRmin)) - 1.0;
        if (fabs(xr) < 0.01) xr = 0.0;
        if (fabs(yr) < 0.01) yr = 0.0;
        if (fabs(zr) < 0.01) zr = 0.0;
        m_Rotation.Set((float)xr, (float)yr, (float)zr);

        double slider[2] = {
            (((state.rglSlider[0] - m_Umin) * 2.0) / (double)(m_Umax - m_Umin)) - 1.0,
            (((state.rglSlider[1] - m_Vmin) * 2.0) / (double)(m_Vmax - m_Vmin)) - 1.0,
        };
        if (fabs(slider[0]) < 0.01) slider[0] = 0.0;
        if (fabs(slider[1]) < 0.01) slider[1] = 0.0;
        m_Sliders.Set((float)slider[0], (float)slider[1]);

        m_PointOfViewAngle = (state.rgdwPOV[0] != 0xFFFF) ? state.rgdwPOV[0] : -1;

        m_Buttons = 0;
        for (int i = 0; i < 32; i++)
        {
            if ((state.rgbButtons[i] & 0x80) != 0)
                m_Buttons |= (1 << i);
        }

        m_Polled = TRUE;
    }
    else if (m_JoyID != -1)
    {
        JOYINFOEX ji;
        ji.dwSize = sizeof(JOYINFOEX);
        ji.dwFlags = JOY_RETURNALL;
        joyGetPosEx(m_JoyID, &ji);

        m_Position.x = (float)((((ji.dwXpos - m_Xmin) * 2.0) / (double)(m_Xmax - m_Xmin)) - 1.0);
        m_Position.y = (float)((((ji.dwYpos - m_Ymin) * 2.0) / (double)(m_Ymax - m_Ymin)) - 1.0);
        m_Position.z = (float)((((ji.dwZpos - m_Zmin) * 2.0) / (double)(m_Zmax - m_Zmin)) - 1.0);

        m_Rotation.x = (float)((ji.dwRpos - m_XRmin) / (double)(m_XRmax - m_XRmin));
        m_Rotation.y = 0;
        m_Rotation.z = 0;

        m_Sliders.x = (float)((ji.dwUpos - m_Umin) / (double)(m_Umax - m_Umin));
        m_Sliders.y = (float)((ji.dwVpos - m_Vmin) / (double)(m_Vmax - m_Vmin));

        m_PointOfViewAngle = ji.dwPOV;

        m_Buttons = ji.dwButtons;

        m_Polled = TRUE;
    }
}

void DX8InputManager::CKJoystick::GetInfo()
{
    if (m_Device)
    {
        DIPROPRANGE range;
        range.diph.dwSize = sizeof(DIPROPRANGE);
        range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
        range.diph.dwHow = DIPH_BYOFFSET;

        range.diph.dwObj = 0;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_Xmin = range.lMin;
        m_Xmax = range.lMax;

        range.diph.dwObj = 4;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_Ymin = range.lMin;
        m_Ymax = range.lMax;

        range.diph.dwObj = 8;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_Zmin = range.lMin;
        m_Zmax = range.lMax;

        range.diph.dwObj = 12;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_XRmin = range.lMin;
        m_XRmax = range.lMax;

        range.diph.dwObj = 16;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_YRmin = range.lMin;
        m_YRmax = range.lMax;

        range.diph.dwObj = 20;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_ZRmin = range.lMin;
        m_ZRmax = range.lMax;

        range.diph.dwObj = 24;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_Umin = range.lMin;
        m_Umax = range.lMax;

        range.diph.dwObj = 28;
        m_Device->GetProperty(DIPROP_RANGE, &range.diph);
        m_Vmin = range.lMin;
        m_Vmax = range.lMax;
    }
    else
    {
        JOYCAPS jc;
        if (!joyGetDevCaps(m_JoyID, &jc, sizeof(JOYCAPS)))
        {
            m_Xmin = jc.wXmin;
            m_Xmax = jc.wXmax;
            m_Ymin = jc.wYmin;
            m_Ymax = jc.wYmax;
            m_Zmin = jc.wZmin;
            m_Zmax = jc.wZmax;
            m_XRmin = jc.wRmin;
            m_XRmax = jc.wRmax;
            m_YRmin = 0;
            m_YRmax = 0;
            m_ZRmin = 0;
            m_ZRmax = 0;
            m_Umin = jc.wUmin;
            m_Umax = jc.wUmax;
            m_Vmin = jc.wVmin;
            m_Vmax = jc.wVmax;
        }
    }
}

CKBOOL DX8InputManager::CKJoystick::IsAttached()
{
    return m_Device != NULL || m_JoyID != -1;
}

BOOL DX8InputManager::JoystickEnum(const DIDEVICEINSTANCE *pdidInstance, void *pContext)
{
    DX8InputManager *im = (DX8InputManager *)pContext;

    if (im->m_JoystickCount < 4)
    {
        LPDIRECTINPUTDEVICE8 pJoystick = NULL;
        im->m_DirectInput->CreateDevice(pdidInstance->guidInstance, &pJoystick, NULL);
        if (pJoystick)
        {
            pJoystick->QueryInterface(IID_IDirectInputDevice2, (void **)&im->m_Joysticks[im->m_JoystickCount].m_Device);
            pJoystick->Release();
            if (im->m_Joysticks[im->m_JoystickCount].m_Device)
            {
                ++im->m_JoystickCount;
                return TRUE;
            }

            JOYINFO ji;
            if (im->m_JoystickCount == 0 && !joyGetPos(JOYSTICKID1, &ji))
            {
                im->m_Joysticks[im->m_JoystickCount].m_JoyID = JOYSTICKID1;
                ++im->m_JoystickCount;
                return TRUE;
            }
            if (im->m_JoystickCount == 1 && !joyGetPos(JOYSTICKID2, &ji))
            {
                im->m_Joysticks[im->m_JoystickCount].m_JoyID = JOYSTICKID2;
                ++im->m_JoystickCount;
                return TRUE;
            }
        }
    }

    return FALSE;
}
