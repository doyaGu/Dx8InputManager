#ifndef DX8INPUTMANAGER_H
#define DX8INPUTMANAGER_H

#include "CKInputManager.h"

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>

#define KEYBOARD_BUFFER_SIZE 256
#define MOUSE_BUFFER_SIZE 256

// Joystick axis enumeration for configuration methods
enum CK_JOYSTICK_AXIS
{
    CK_AXIS_X = 0,
    CK_AXIS_Y = 1,
    CK_AXIS_Z = 2,
    CK_AXIS_RX = 3,
    CK_AXIS_RY = 4,
    CK_AXIS_RZ = 5,
    CK_AXIS_SLIDER0 = 6,
    CK_AXIS_SLIDER1 = 7
};

// Joystick capabilities structure
struct CK_JOYSTICK_CAPS
{
    CKBOOL hasX;
    CKBOOL hasY;
    CKBOOL hasZ;
    CKBOOL hasRx;
    CKBOOL hasRy;
    CKBOOL hasRz;
    CKBOOL hasSlider0;
    CKBOOL hasSlider1;
    CKBOOL hasPOV;
    int buttonCount;
};

class DX8InputManager : public CKInputManager
{
public:
    class CKMouse
    {
        friend DX8InputManager;

    public:
        CKMouse();
        void Init(HWND hWnd);
        void Release();
        void Clear();
        void Poll(CKBOOL pause);

    private:
        LPDIRECTINPUTDEVICE8 m_Device;
        Vx2DVector m_Position;
        DIMOUSESTATE m_State;
        CKBYTE m_LastButtons[4];
        DIDEVICEOBJECTDATA m_Buffer[MOUSE_BUFFER_SIZE];
        int m_NumberOfBuffer;
    };

    class CKJoystick
    {
        friend DX8InputManager;

    public:
        CKJoystick();
        void Init(HWND hWnd);
        void Release();
        void Poll();
        void GetInfo();
        CKBOOL IsAttached();

    private:
        void ResetState();

        // Axis capability flags
        struct AxisCapabilities
        {
            CKBOOL hasX, hasY, hasZ;
            CKBOOL hasRx, hasRy, hasRz;
            CKBOOL hasSlider0, hasSlider1;

            AxisCapabilities()
            {
                hasX = hasY = hasZ = FALSE;
                hasRx = hasRy = hasRz = FALSE;
                hasSlider0 = hasSlider1 = FALSE;
            }
        };

        static BOOL CALLBACK EnumAxesCallback(LPCDIDEVICEOBJECTINSTANCE lpddoi, LPVOID pvRef);

        LPDIRECTINPUTDEVICE8 m_Device;
        GUID m_DeviceGUID;           // Device instance GUID
        char m_DeviceName[MAX_PATH]; // Device product name
        AxisCapabilities m_AxisCaps; // Track which axes are available on this device
        float m_DeadzoneRadius;      // Deadzone radius (0.0 to 1.0, default 0.01)
        float m_Gain;                // Sensitivity gain multiplier (0.0 to 2.0, default 1.0)
        int m_ButtonCount;           // Number of buttons on this device
        CKDWORD m_Polled;
        VxVector m_Position;
        VxVector m_Rotation;
        Vx2DVector m_Sliders;
        CKDWORD m_PointOfViewAngle;
        CKDWORD m_Buttons;
        CKDWORD m_Xmin;  // Minimum X-coordinate
        CKDWORD m_Xmax;  // Maximum X-coordinate
        CKDWORD m_Ymin;  // Minimum Y-coordinate
        CKDWORD m_Ymax;  // Maximum Y-coordinate
        CKDWORD m_Zmin;  // Minimum Z-coordinate
        CKDWORD m_Zmax;  // Maximum Z-coordinate
        CKDWORD m_XRmin; // Minimum X-rotation
        CKDWORD m_XRmax; // Maximum X-rotation
        CKDWORD m_YRmin; // Minimum Y-rotation
        CKDWORD m_YRmax; // Maximum Y-rotation
        CKDWORD m_ZRmin; // Minimum Z-rotation
        CKDWORD m_ZRmax; // Maximum Z-rotation
        CKDWORD m_Umin;  // Minimum u-coordinate (fifth axis)
        CKDWORD m_Vmin;  // Minimum v-coordinate (sixth axis)
        CKDWORD m_Umax;  // Maximum u-coordinate (fifth axis)
        CKDWORD m_Vmax;  // Maximum v-coordinate (sixth axis)
    };

    virtual void EnableKeyboardRepetition(CKBOOL iEnable = TRUE);
    virtual CKBOOL IsKeyboardRepetitionEnabled();

    virtual CKBOOL IsKeyDown(CKDWORD iKey, CKDWORD *oStamp = NULL);
    virtual CKBOOL IsKeyUp(CKDWORD iKey);
    virtual CKBOOL IsKeyToggled(CKDWORD iKey, CKDWORD *oStamp = NULL);

    virtual int GetKeyName(CKDWORD iKey, CKSTRING oKeyName);
    virtual CKDWORD GetKeyFromName(CKSTRING iKeyName);
    virtual unsigned char *GetKeyboardState();

    virtual CKBOOL IsKeyboardAttached();

    virtual int GetNumberOfKeyInBuffer();
    virtual int GetKeyFromBuffer(int i, CKDWORD &oKey, CKDWORD *oTimeStamp = NULL);

    virtual CKBOOL IsMouseButtonDown(CK_MOUSEBUTTON iButton);
    virtual CKBOOL IsMouseClicked(CK_MOUSEBUTTON iButton);
    virtual CKBOOL IsMouseToggled(CK_MOUSEBUTTON iButton);

    virtual void GetMouseButtonsState(CKBYTE oStates[4]);
    virtual void GetMousePosition(Vx2DVector &oPosition, CKBOOL iAbsolute = TRUE);
    virtual void GetMouseRelativePosition(VxVector &oPosition);

    virtual CKBOOL IsMouseAttached();

    virtual CKBOOL IsJoystickAttached(int iJoystick);

    virtual void GetJoystickPosition(int iJoystick, VxVector *oPosition);
    virtual void GetJoystickRotation(int iJoystick, VxVector *oRotation);
    virtual void GetJoystickSliders(int iJoystick, Vx2DVector *oPosition);
    virtual void GetJoystickPointOfViewAngle(int iJoystick, float *oAngle);
    virtual CKDWORD GetJoystickButtonsState(int iJoystick);
    virtual CKBOOL IsJoystickButtonDown(int iJoystick, int iButton);

    virtual void Pause(CKBOOL pause);

    virtual void ShowCursor(CKBOOL iShow);
    virtual CKBOOL GetCursorVisibility();
    virtual VXCURSOR_POINTER GetSystemCursor();
    virtual void SetSystemCursor(VXCURSOR_POINTER cursor);

    virtual int GetJoystickCount();
    virtual IDirectInputDevice8 *GetJoystickDxInterface(int iJoystick);

    virtual int GetMaxJoysticks();                  // Get current maximum joystick limit
    virtual void SetMaxJoysticks(int maxJoysticks); // Set maximum number of joysticks (must call before initialization)

    virtual const char *GetJoystickName(int iJoystick);
    virtual CKBOOL GetJoystickCapabilities(int iJoystick, CK_JOYSTICK_CAPS *caps); // Query device capabilities

    // Joystick configuration methods
    virtual float GetJoystickDeadzone(int iJoystick);              // Get current deadzone radius
    virtual void SetJoystickDeadzone(int iJoystick, float radius); // Set deadzone radius (0.0 to 1.0)

    virtual float GetJoystickGain(int iJoystick);            // Get current sensitivity gain
    virtual void SetJoystickGain(int iJoystick, float gain); // Set sensitivity gain (0.0 to 2.0, default 1.0)

    virtual CKBOOL GetJoystickAxisRange(int iJoystick, CK_JOYSTICK_AXIS axis, LONG *min, LONG *max); // Query axis range
    virtual CKBOOL SetJoystickAxisRange(int iJoystick, CK_JOYSTICK_AXIS axis, LONG min, LONG max);   // Set custom axis range
    virtual CKBOOL ResetJoystickAxisRanges(int iJoystick);                                           // Reset all axes to device defaults

    // Keyboard repeat configuration methods
    virtual CKDWORD GetKeyboardRepeatDelay();
    virtual void SetKeyboardRepeatDelay(CKDWORD delay);
    virtual CKDWORD GetKeyboardRepeatInterval();
    virtual void SetKeyboardRepeatInterval(CKDWORD interval);

    // Mouse wheel methods
    virtual int GetMouseWheelDelta();
    virtual int GetMouseWheelPosition();

    // State setting methods
    virtual void SetKeyDown(CKDWORD iKey);
    virtual void SetKeyUp(CKDWORD iKey);

    virtual void SetMouseButtonDown(CK_MOUSEBUTTON iButton);
    virtual void SetMouseButtonUp(CK_MOUSEBUTTON iButton);
    virtual void SetMousePosition(const Vx2DVector &position);
    virtual void SetMouseWheel(int wheelDelta);
    virtual void SetMouseWheelPosition(int position);

    virtual void SetJoystickButtonDown(int iJoystick, int iButton);
    virtual void SetJoystickButtonUp(int iJoystick, int iButton);
    virtual void SetJoystickPosition(int iJoystick, const VxVector &position);

    // Advanced state setting methods
    virtual void SetKeyboardState(const CKBYTE *states, const int *stamps);
    virtual void SetMouseState(const Vx2DVector &pos, const CKBYTE *buttons, const VxVector &delta);
    virtual void SetJoystickState(int iJoystick, const VxVector &pos, const VxVector &rot, const Vx2DVector &sliders, CKDWORD buttons, CKDWORD pov);
    virtual void SetMultipleKeys(const CKDWORD *keys, int count, CKBOOL pressed);
    virtual void ClearAllInputState();

    // Internal functions

    virtual CKERROR OnCKInit();
    virtual CKERROR OnCKEnd();

    virtual CKERROR OnCKReset();
    virtual CKERROR OnCKPause();
    virtual CKERROR OnCKPlay();

    virtual CKERROR PreProcess();
    virtual CKERROR PostProcess();

    virtual CKDWORD GetValidFunctionsMask()
    {
        return CKMANAGER_FUNC_OnCKInit |
               CKMANAGER_FUNC_OnCKEnd |
               CKMANAGER_FUNC_OnCKReset |
               CKMANAGER_FUNC_OnCKPause |
               CKMANAGER_FUNC_OnCKPlay |
               CKMANAGER_FUNC_PreProcess |
               CKMANAGER_FUNC_PostProcess;
    }

    virtual ~DX8InputManager();

    DX8InputManager(CKContext *context);

    void Initialize(HWND hWnd);
    void Uninitialize();
    void ClearBuffers();

    static BOOL CALLBACK JoystickEnum(const DIDEVICEINSTANCE *pdidInstance, void *pContext);

protected:
    LPDIRECTINPUT8 m_DirectInput;
    LPDIRECTINPUTDEVICE8 m_Keyboard;
    VXCURSOR_POINTER m_Cursor;
    CKMouse m_Mouse;
    CKJoystick *m_Joysticks;
    int m_JoystickCount;
    int m_MaxJoysticks;
    CKBYTE m_KeyboardState[KEYBOARD_BUFFER_SIZE];
    int m_KeyboardStamps[KEYBOARD_BUFFER_SIZE];
    DIDEVICEOBJECTDATA m_KeyInBuffer[KEYBOARD_BUFFER_SIZE];
    int m_NumberOfKeyInBuffer;
    CKBOOL m_Paused;
    CKBOOL m_WasPaused;
    CKBOOL m_EnableKeyboardRepetition;
    CKDWORD m_KeyboardRepeatDelay;
    CKDWORD m_KeyboardRepeatInterval;
    CKBOOL m_ShowCursor;
    int m_MouseWheelPosition;

private:
    void EnsureCursorVisible(CKBOOL iShow);
};

#endif // DX8INPUTMANAGER_H
