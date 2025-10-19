#include "DX8InputManager.h"

// Initialize static XInput members
HMODULE DX8InputManager::CKJoystick::s_XInputDLL = NULL;
PFN_XInputGetState DX8InputManager::CKJoystick::s_XInputGetState = NULL;
PFN_XInputSetState DX8InputManager::CKJoystick::s_XInputSetState = NULL;
PFN_XInputGetCapabilities DX8InputManager::CKJoystick::s_XInputGetCapabilities = NULL;

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

void DX8InputManager::CKJoystick::LoadXInput()
{
    if (s_XInputDLL)
        return;  // Already loaded

    // Dynamically load XInput library (try multiple versions for compatibility)
    const char* xinput_dll_names[] = {
        "xinput1_4.dll",   // Windows 8+
        "xinput1_3.dll",   // DirectX SDK / Windows 7
        "xinput9_1_0.dll", // Windows Vista, Windows 7
        "xinput1_2.dll",   // DirectX SDK (older)
        "xinput1_1.dll"    // DirectX SDK (oldest)
    };
    for (int n = 0; n < 5; n++)
    {
        HMODULE dll = ::LoadLibraryA(xinput_dll_names[n]);
        if (dll)
        {
            s_XInputDLL = dll;
            s_XInputGetState = (PFN_XInputGetState)::GetProcAddress(dll, "XInputGetState");
            s_XInputSetState = (PFN_XInputSetState)::GetProcAddress(dll, "XInputSetState");
            s_XInputGetCapabilities = (PFN_XInputGetCapabilities)::GetProcAddress(dll, "XInputGetCapabilities");

            if (s_XInputGetState)
            {
                TCHAR msg[256];
                snprintf(msg, 256, TEXT("CKJoystick: Loaded %s successfully"), xinput_dll_names[n]);
                ::OutputDebugString(msg);
                break;
            }
            else
            {
                ::FreeLibrary(dll);
                s_XInputDLL = NULL;
            }
        }
    }

    if (!s_XInputDLL)
    {
        ::OutputDebugString(TEXT("CKJoystick: XInput not available (optional, DirectInput devices will still work)"));
    }
}

void DX8InputManager::CKJoystick::UnloadXInput()
{
    if (s_XInputDLL)
    {
        ::FreeLibrary(s_XInputDLL);
        s_XInputDLL = NULL;
        s_XInputGetState = NULL;
        s_XInputSetState = NULL;
        s_XInputGetCapabilities = NULL;
    }
}

DX8InputManager::CKJoystick::CKJoystick()
{
    m_Device = NULL;
    memset(&m_DeviceGUID, 0, sizeof(GUID));  // Initialize to empty GUID
    m_XInputUserIndex = (DWORD)-1;  // Not an XInput device by default
    m_DeadzoneRadius = 0.01f;       // Default 1% deadzone
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

void DX8InputManager::CKJoystick::Poll()
{
    if (m_Polled) return;

    if (m_Device)
    {
        HRESULT hr = m_Device->Poll();
        if (FAILED(hr))
        {
            hr = m_Device->Acquire();
            if (FAILED(hr))
                return;
            hr = m_Device->Poll();
            if (FAILED(hr))
                return;
        }

        DIJOYSTATE2 state;
        memset(&state, 0, sizeof(DIJOYSTATE2));
        hr = m_Device->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED)
        {
            hr = m_Device->Acquire();
            if (FAILED(hr))
                return;
            hr = m_Device->GetDeviceState(sizeof(DIJOYSTATE2), &state);
        }
        if (FAILED(hr))
            return;

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

        m_Position.Set((float)x, (float)y, (float)z);

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

        m_Rotation.Set((float)xr, (float)yr, (float)zr);

        double slider0 = m_AxisCaps.hasSlider0 ? NormalizeAxis(state.rglSlider[0], m_Umin, m_Umax) : 0.0;
        double slider1 = m_AxisCaps.hasSlider1 ? NormalizeAxis(state.rglSlider[1], m_Vmin, m_Vmax) : 0.0;
        if (fabs(slider0) < 0.01)
            slider0 = 0.0;
        if (fabs(slider1) < 0.01)
            slider1 = 0.0;
        m_Sliders.Set((float)slider0, (float)slider1);

        m_PointOfViewAngle = (state.rgdwPOV[0] != 0xFFFF) ? static_cast<CKDWORD>(state.rgdwPOV[0]) : -1;

        m_Buttons = 0;
        for (int i = 0; i < 32; i++)
        {
            if ((state.rgbButtons[i] & 0x80) != 0)
                m_Buttons |= (1 << i);
        }

        m_Polled = TRUE;
    }
    else if (s_XInputGetState && m_XInputUserIndex != (DWORD)-1)
    {
        // Poll XInput device (only if XInput is loaded)
        XINPUT_STATE xstate;
        DWORD result = s_XInputGetState(m_XInputUserIndex, &xstate);

        if (result == ERROR_SUCCESS)
        {
            // Map Xbox controller to joystick interface
            // Left stick -> Position X,Y
            // Right stick -> Rotation X,Y
            // Triggers -> Position Z (combined)

            // Normalize stick values from [-32768, 32767] to [-1.0, 1.0]
            float leftX = xstate.Gamepad.sThumbLX / 32768.0f;
            float leftY = xstate.Gamepad.sThumbLY / 32768.0f;
            float rightX = xstate.Gamepad.sThumbRX / 32768.0f;
            float rightY = xstate.Gamepad.sThumbRY / 32768.0f;

            // Apply deadzones (Xbox default is ~7849, which is about 0.24)
            if (fabs(leftX) < 0.24f) leftX = 0.0f;
            if (fabs(leftY) < 0.24f) leftY = 0.0f;
            if (fabs(rightX) < 0.24f) rightX = 0.0f;
            if (fabs(rightY) < 0.24f) rightY = 0.0f;

            // Combine triggers into Z axis (normalized from [0, 255] to [-1.0, 1.0])
            float triggerZ = (xstate.Gamepad.bRightTrigger - xstate.Gamepad.bLeftTrigger) / 255.0f;

            m_Position.Set(leftX, leftY, triggerZ);
            m_Rotation.Set(rightX, rightY, 0.0f);

            // No sliders on Xbox controller
            m_Sliders.Set(0.0f, 0.0f);

            // Map DPad to POV (0 = up, 9000 = right, 18000 = down, 27000 = left)
            WORD dpad = xstate.Gamepad.wButtons & 0x000F;  // DPad is first 4 bits
            if (dpad == 0)
                m_PointOfViewAngle = -1;  // Centered
            else if (dpad == XINPUT_GAMEPAD_DPAD_UP)
                m_PointOfViewAngle = 0;
            else if ((dpad & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT)) == (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_RIGHT))
                m_PointOfViewAngle = 4500;
            else if (dpad == XINPUT_GAMEPAD_DPAD_RIGHT)
                m_PointOfViewAngle = 9000;
            else if ((dpad & (XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT)) == (XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_RIGHT))
                m_PointOfViewAngle = 13500;
            else if (dpad == XINPUT_GAMEPAD_DPAD_DOWN)
                m_PointOfViewAngle = 18000;
            else if ((dpad & (XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT)) == (XINPUT_GAMEPAD_DPAD_DOWN | XINPUT_GAMEPAD_DPAD_LEFT))
                m_PointOfViewAngle = 22500;
            else if (dpad == XINPUT_GAMEPAD_DPAD_LEFT)
                m_PointOfViewAngle = 27000;
            else if ((dpad & (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT)) == (XINPUT_GAMEPAD_DPAD_UP | XINPUT_GAMEPAD_DPAD_LEFT))
                m_PointOfViewAngle = 31500;

            // Map Xbox buttons to generic joystick buttons (first 16 buttons)
            m_Buttons = xstate.Gamepad.wButtons & 0xFFFF;

            m_Polled = TRUE;
        }
    }
}

// EnumAxesCallback: Enumerate device axes and query their properties
BOOL CALLBACK DX8InputManager::CKJoystick::EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef)
{
    CKJoystick* joystick = (CKJoystick*)pvRef;

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
    return m_Device != NULL || m_XInputUserIndex != (DWORD)-1;
}

void DX8InputManager::CKJoystick::TransferFrom(CKJoystick &source)
{
    // Transfer COM interface pointer without AddRef (move semantics)
    m_Device = source.m_Device;
    source.m_Device = NULL;  // Null out source to prevent double-release

    // Copy all other members (XInput members are static, so no need to copy)
    m_DeviceGUID = source.m_DeviceGUID;
    m_XInputUserIndex = source.m_XInputUserIndex;
    m_AxisCaps = source.m_AxisCaps;
    m_DeadzoneRadius = source.m_DeadzoneRadius;
    m_Polled = source.m_Polled;
    m_Position = source.m_Position;
    m_Rotation = source.m_Rotation;
    m_Sliders = source.m_Sliders;
    m_PointOfViewAngle = source.m_PointOfViewAngle;
    m_Buttons = source.m_Buttons;
    m_Xmin = source.m_Xmin;
    m_Xmax = source.m_Xmax;
    m_Ymin = source.m_Ymin;
    m_Ymax = source.m_Ymax;
    m_Zmin = source.m_Zmin;
    m_Zmax = source.m_Zmax;
    m_XRmin = source.m_XRmin;
    m_XRmax = source.m_XRmax;
    m_YRmin = source.m_YRmin;
    m_YRmax = source.m_YRmax;
    m_ZRmin = source.m_ZRmin;
    m_ZRmax = source.m_ZRmax;
    m_Umin = source.m_Umin;
    m_Umax = source.m_Umax;
    m_Vmin = source.m_Vmin;
    m_Vmax = source.m_Vmax;
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
            im->m_Joysticks[im->m_JoystickCount].m_Device = pJoystick;
            // Store the device GUID for hot-plug detection
            im->m_Joysticks[im->m_JoystickCount].m_DeviceGUID = pdidInstance->guidInstance;
            ++im->m_JoystickCount;
            return DIENUM_CONTINUE;
        }
    }

    return DIENUM_STOP;
}
