#ifndef DX8INPUTMANAGER_H
#define DX8INPUTMANAGER_H

#include "CKInputManager.h"

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>
#include <xinput.h>

// XInput dynamic loading (no static linking)
typedef DWORD (WINAPI *PFN_XInputGetState)(DWORD, XINPUT_STATE*);
typedef DWORD (WINAPI *PFN_XInputSetState)(DWORD, XINPUT_VIBRATION*);
typedef DWORD (WINAPI *PFN_XInputGetCapabilities)(DWORD, DWORD, XINPUT_CAPABILITIES*);

#define KEYBOARD_BUFFER_SIZE 256
#define MOUSE_BUFFER_SIZE 256

struct InputFilter
{
    CKBOOL keyboardEnabled;
    CKBOOL mouseEnabled;
    CKBOOL joystickEnabled;
    CKDWORD *allowedKeys;
    int allowedKeyCount;
};

typedef void (*InputEventCallback)(CKDWORD eventType, CKDWORD param1, CKDWORD param2, void *userData);

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
        void TransferFrom(CKJoystick &source);

        // XInput dynamic loading (shared across all joystick instances)
        static void LoadXInput();
        static void UnloadXInput();
        static CKBOOL IsXInputAvailable() { return s_XInputGetState != NULL; }

    private:
        // XInput dynamic loading (static members shared by all instances)
        static HMODULE s_XInputDLL;
        static PFN_XInputGetState s_XInputGetState;
        static PFN_XInputSetState s_XInputSetState;
        static PFN_XInputGetCapabilities s_XInputGetCapabilities;

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
        GUID m_DeviceGUID;               // Device instance GUID for hot-plug detection
        DWORD m_XInputUserIndex;         // XInput user index (0-3), or -1 if not an XInput device
        AxisCapabilities m_AxisCaps;     // Track which axes are available on this device
        float m_DeadzoneRadius;          // Deadzone radius (0.0 to 1.0, default 0.01)
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

    // Joystick configuration methods
    virtual void SetJoystickDeadzone(int iJoystick, float radius);  // Set deadzone radius (0.0 to 1.0)
    virtual float GetJoystickDeadzone(int iJoystick);  // Get current deadzone radius
    virtual void SetMaxJoysticks(int maxJoysticks);  // Set maximum number of joysticks (must call before initialization)
    virtual int GetMaxJoysticks();  // Get current maximum joystick limit

    // State setting methods
    virtual void SetKeyDown(CKDWORD iKey);
    virtual void SetKeyUp(CKDWORD iKey);
    virtual void SetMouseButtonDown(CK_MOUSEBUTTON iButton);
    virtual void SetMouseButtonUp(CK_MOUSEBUTTON iButton);
    virtual void SetMousePosition(const Vx2DVector &position);
    virtual void SetJoystickButtonDown(int iJoystick, int iButton);
    virtual void SetJoystickButtonUp(int iJoystick, int iButton);
    virtual void SetJoystickPosition(int iJoystick, const VxVector &position);

    // Advanced state setting methods
    virtual void SetKeyboardState(const CKBYTE *states, const int *stamps);
    virtual void SetMouseState(const Vx2DVector &pos, const CKBYTE *buttons, const VxVector &delta);
    virtual void SetJoystickCompleteState(int iJoystick, const VxVector &pos, const VxVector &rot, const Vx2DVector &sliders, CKDWORD buttons, CKDWORD pov);
    virtual void SetMultipleKeys(const CKDWORD *keys, int count, CKBOOL pressed);
    virtual void ClearAllInputState();

    // Mouse wheel enhancements
    virtual int GetMouseWheelDelta();
    virtual int GetMouseWheelPosition();
    virtual void SetMouseWheel(int wheelDelta);
    virtual void SetMouseWheelPosition(int position);

    // Input filtering methods
    virtual void SetInputFilter(CKBOOL keyboard, CKBOOL mouse, CKBOOL joystick);
    virtual void SetKeyFilter(const CKDWORD *allowedKeys, int count);
    virtual void ClearInputFilters();
    virtual CKBOOL IsInputFiltered(CKDWORD key);

    // Event callback methods
    virtual void RegisterEventCallback(InputEventCallback callback, void *userData);
    virtual void UnregisterEventCallback(InputEventCallback callback);
    virtual void TriggerEvent(CKDWORD eventType, CKDWORD param1, CKDWORD param2);

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

    InputFilter m_Filter;
    InputEventCallback m_EventCallbacks[8];
    void *m_CallbackUserData[8];
    int m_CallbackCount;
    int m_MouseWheelPosition;

    // Hot-plug detection variables
    DWORD m_LastDeviceCheckTime;
    DWORD m_DeviceCheckInterval;  // Milliseconds between device checks (default 1000ms)
    GUID m_DeviceGUIDs[16];  // Track device GUIDs to detect changes
    DWORD m_XInputStates[XUSER_MAX_COUNT];  // Track XInput connection states (0 = disconnected, 1 = connected)

private:
    void EnsureCursorVisible(CKBOOL iShow);
    CKBOOL IsKeyAllowed(CKDWORD key);
    void CheckForDeviceChanges();
};

#endif // DX8INPUTMANAGER_H
