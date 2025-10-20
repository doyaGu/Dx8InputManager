#include "DX8InputManager.h"

static double NormalizeAxis(LONG value, LONG minValue, LONG maxValue)
{
    if (maxValue == minValue)
        return 0.0;

    double dMin = static_cast<double>(minValue);
    double range = static_cast<double>(maxValue) - dMin;
    if (range == 0.0)
        return 0.0;

    double normalized = (static_cast<double>(value) - dMin) * 2.0 / range - 1.0;
    if (normalized > 1.0)
        normalized = 1.0;
    else if (normalized < -1.0)
        normalized = -1.0;

    return normalized;
}

static float ClampAxis(float value)
{
    if (value > 1.0f)
        return 1.0f;
    if (value < -1.0f)
        return -1.0f;
    return value;
}

DX8InputManager::CKJoystick::CKJoystick()
{
    m_Device = NULL;
    memset(&m_DeviceGUID, 0, sizeof(GUID));        // Initialize to empty GUID
    memset(m_DeviceName, 0, sizeof(m_DeviceName)); // Initialize device name to empty
    // Default 1% deadzone for all devices
    m_DeadzoneRadius = 0.01f;
    m_Gain = 1.0f;     // Default gain (no scaling)
    m_ButtonCount = 0; // Will be set during initialization
    m_Polled = FALSE;
    m_Position.Set(0.0f, 0.0f, 0.0f);
    m_Rotation.Set(0.0f, 0.0f, 0.0f);
    m_Sliders.Set(0.0f, 0.0f);
    m_PointOfViewAngle = -1;
    m_Buttons = 0;
    m_AxisCaps = AxisCapabilities();
    m_Xmin = m_Ymin = m_Zmin = -1000;
    m_Xmax = m_Ymax = m_Zmax = 1000;
    m_XRmin = m_YRmin = m_ZRmin = -1000;
    m_XRmax = m_YRmax = m_ZRmax = 1000;
    m_Umin = m_Vmin = -1000;
    m_Umax = m_Vmax = 1000;
}

void DX8InputManager::CKJoystick::Init(HWND hWnd)
{
    if (m_Device)
    {
        m_Device->SetDataFormat(&c_dfDIJoystick2);
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

void DX8InputManager::CKJoystick::ResetState()
{
    m_Position.Set(0.0f, 0.0f, 0.0f);
    m_Rotation.Set(0.0f, 0.0f, 0.0f);
    m_Sliders.Set(0.0f, 0.0f);
    m_PointOfViewAngle = -1;
    m_Buttons = 0;
    m_Polled = TRUE;
}

void DX8InputManager::CKJoystick::Poll()
{
    if (m_Polled)
        return;

    if (m_Device)
    {
        HRESULT hr = m_Device->Poll();
        if (FAILED(hr))
        {
            hr = m_Device->Acquire();
            if (FAILED(hr))
            {
                ResetState();
                return;
            }
            hr = m_Device->Poll();
            if (FAILED(hr))
            {
                ResetState();
                return;
            }
        }

        DIJOYSTATE2 state;
        memset(&state, 0, sizeof(DIJOYSTATE2));
        hr = m_Device->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            hr = m_Device->Acquire();
            if (FAILED(hr))
            {
                ResetState();
                return;
            }
            hr = m_Device->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        }
        if (FAILED(hr))
        {
            ResetState();
            return;
        }

        double x = m_AxisCaps.hasX ? NormalizeAxis(state.lX, m_Xmin, m_Xmax) : 0.0;
        double y = m_AxisCaps.hasY ? NormalizeAxis(state.lY, m_Ymin, m_Ymax) : 0.0;
        double z = m_AxisCaps.hasZ ? NormalizeAxis(state.lZ, m_Zmin, m_Zmax) : 0.0;

        const double deadzone = static_cast<double>(m_DeadzoneRadius);
        double xyMagnitude = sqrt(x * x + y * y);
        if (xyMagnitude < deadzone || deadzone >= 1.0)
        {
            x = y = 0.0;
        }
        else if (xyMagnitude > 0.0)
        {
            double scale = (xyMagnitude - deadzone) / (1.0 - deadzone);
            x = (x / xyMagnitude) * scale;
            y = (y / xyMagnitude) * scale;
        }

        if (fabs(z) < deadzone)
            z = 0.0;

        // Apply gain/sensitivity multiplier and clamp to [-1.0, 1.0]
        const double gain = static_cast<double>(m_Gain);
        m_Position.Set(ClampAxis((float)(x * gain)), ClampAxis((float)(y * gain)), ClampAxis((float)(z * gain)));

        double xr = m_AxisCaps.hasRx ? NormalizeAxis(state.lRx, m_XRmin, m_XRmax) : 0.0;
        double yr = m_AxisCaps.hasRy ? NormalizeAxis(state.lRy, m_YRmin, m_YRmax) : 0.0;
        double zr = m_AxisCaps.hasRz ? NormalizeAxis(state.lRz, m_ZRmin, m_ZRmax) : 0.0;

        double rxryMagnitude = sqrt(xr * xr + yr * yr);
        if (rxryMagnitude < deadzone || deadzone >= 1.0)
        {
            xr = yr = 0.0;
        }
        else if (rxryMagnitude > 0.0)
        {
            double scale = (rxryMagnitude - deadzone) / (1.0 - deadzone);
            xr = (xr / rxryMagnitude) * scale;
            yr = (yr / rxryMagnitude) * scale;
        }

        if (fabs(zr) < deadzone)
            zr = 0.0;

        // Apply gain/sensitivity multiplier and clamp to [-1.0, 1.0]
        m_Rotation.Set(ClampAxis((float)(xr * gain)), ClampAxis((float)(yr * gain)), ClampAxis((float)(zr * gain)));

        double slider0 = m_AxisCaps.hasSlider0 ? NormalizeAxis(state.rglSlider[0], m_Umin, m_Umax) : 0.0;
        double slider1 = m_AxisCaps.hasSlider1 ? NormalizeAxis(state.rglSlider[1], m_Vmin, m_Vmax) : 0.0;
        if (fabs(slider0) < deadzone)
            slider0 = 0.0;
        if (fabs(slider1) < deadzone)
            slider1 = 0.0;
        m_Sliders.Set(ClampAxis((float)(slider0 * gain)), ClampAxis((float)(slider1 * gain)));

        m_PointOfViewAngle = (state.rgdwPOV[0] != 0xFFFF) ? static_cast<CKDWORD>(state.rgdwPOV[0]) : -1;

        m_Buttons = 0;
        for (int i = 0; i < 32; i++)
        {
            if ((state.rgbButtons[i] & 0x80) != 0)
                m_Buttons |= (1 << i);
        }

        m_Polled = TRUE;
    }
    else
    {
        ResetState();
    }
}

// EnumAxesCallback: Enumerate device axes and query their properties
BOOL CALLBACK DX8InputManager::CKJoystick::EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    CKJoystick *joystick = (CKJoystick *)pvRef;

    if (!joystick || !joystick->m_Device)
        return DIENUM_STOP;

    // Set up the property range structure
    DIPROPRANGE range;
    memset(&range, 0, sizeof(DIPROPRANGE));
    range.diph.dwSize = sizeof(DIPROPRANGE);
    range.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    range.diph.dwHow = DIPH_BYID;
    range.diph.dwObj = lpddoi->dwType & ~DIDFT_COLLECTION;

    HRESULT hr = joystick->m_Device->GetProperty(DIPROP_RANGE, &range.diph);
    if (FAILED(hr) || range.lMax == range.lMin)
    {
        // Fall back to a symmetric range to keep normalization stable
        range.lMin = -1000;
        range.lMax = 1000;
        if (FAILED(joystick->m_Device->SetProperty(DIPROP_RANGE, &range.diph)))
            return DIENUM_CONTINUE;
    }

    // Determine which axis this is and store its range
    GUID guidType = lpddoi->guidType;

    if (guidType == GUID_XAxis)
    {
        joystick->m_AxisCaps.hasX = TRUE;
        joystick->m_Xmin = range.lMin;
        joystick->m_Xmax = range.lMax;
    }
    else if (guidType == GUID_YAxis)
    {
        joystick->m_AxisCaps.hasY = TRUE;
        joystick->m_Ymin = range.lMin;
        joystick->m_Ymax = range.lMax;
    }
    else if (guidType == GUID_ZAxis)
    {
        joystick->m_AxisCaps.hasZ = TRUE;
        joystick->m_Zmin = range.lMin;
        joystick->m_Zmax = range.lMax;
    }
    else if (guidType == GUID_RxAxis)
    {
        joystick->m_AxisCaps.hasRx = TRUE;
        joystick->m_XRmin = range.lMin;
        joystick->m_XRmax = range.lMax;
    }
    else if (guidType == GUID_RyAxis)
    {
        joystick->m_AxisCaps.hasRy = TRUE;
        joystick->m_YRmin = range.lMin;
        joystick->m_YRmax = range.lMax;
    }
    else if (guidType == GUID_RzAxis)
    {
        joystick->m_AxisCaps.hasRz = TRUE;
        joystick->m_ZRmin = range.lMin;
        joystick->m_ZRmax = range.lMax;
    }
    else if (guidType == GUID_Slider)
    {
        // Sliders are numbered sequentially
        if (!joystick->m_AxisCaps.hasSlider0)
        {
            joystick->m_AxisCaps.hasSlider0 = TRUE;
            joystick->m_Umin = range.lMin;
            joystick->m_Umax = range.lMax;
        }
        else if (!joystick->m_AxisCaps.hasSlider1)
        {
            joystick->m_AxisCaps.hasSlider1 = TRUE;
            joystick->m_Vmin = range.lMin;
            joystick->m_Vmax = range.lMax;
        }
    }

    return DIENUM_CONTINUE;
}

void DX8InputManager::CKJoystick::GetInfo()
{
    if (m_Device)
    {
        m_AxisCaps = AxisCapabilities();
        // Initialize default ranges (in case device has no axes)
        m_Xmin = m_Ymin = m_Zmin = -1000;
        m_Xmax = m_Ymax = m_Zmax = 1000;
        m_XRmin = m_YRmin = m_ZRmin = -1000;
        m_XRmax = m_YRmax = m_ZRmax = 1000;
        m_Umin = m_Vmin = -1000;
        m_Umax = m_Vmax = 1000;

        // Enumerate all axes on the device to determine capabilities
        m_Device->EnumObjects(EnumAxesCallback, (LPVOID)this, DIDFT_AXIS);
    }
}

CKBOOL DX8InputManager::CKJoystick::IsAttached()
{
    return m_Device != NULL;
}

BOOL DX8InputManager::JoystickEnum(const DIDEVICEINSTANCE *pdidInstance, void *pContext)
{
    DX8InputManager *im = (DX8InputManager *)pContext;

    if (im->m_JoystickCount < im->m_MaxJoysticks)
    {
        LPDIRECTINPUTDEVICE8 pJoystick = NULL;
        HRESULT hr = im->m_DirectInput->CreateDevice(pdidInstance->guidInstance, &pJoystick, NULL);
        if (SUCCEEDED(hr) && pJoystick)
        {
            CKJoystick &joystick = im->m_Joysticks[im->m_JoystickCount];
            joystick.m_Device = pJoystick;

            // Store the device GUID
            joystick.m_DeviceGUID = pdidInstance->guidInstance;

            // Store the device product name (convert from TCHAR to char if needed)
#ifdef UNICODE
            WideCharToMultiByte(CP_UTF8, 0, pdidInstance->tszProductName, -1,
                                joystick.m_DeviceName, MAX_PATH, NULL, NULL);
#else
            strncpy(joystick.m_DeviceName, pdidInstance->tszProductName, MAX_PATH - 1);
            joystick.m_DeviceName[MAX_PATH - 1] = '\0';
#endif

            // Query button count from device capabilities
            DIDEVCAPS caps;
            memset(&caps, 0, sizeof(DIDEVCAPS));
            caps.dwSize = sizeof(DIDEVCAPS);
            if (SUCCEEDED(pJoystick->GetCapabilities(&caps)))
            {
                joystick.m_ButtonCount = (int)caps.dwButtons;
            }
            else
            {
                joystick.m_ButtonCount = 32; // Default to maximum
            }

            ++im->m_JoystickCount;
            return DIENUM_CONTINUE;
        }
    }

    return DIENUM_STOP;
}
