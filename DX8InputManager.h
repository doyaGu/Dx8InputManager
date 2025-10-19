#ifndef DX8INPUTMANAGER_H
#define DX8INPUTMANAGER_H

#include "CKInputManager.h"

#define DIRECTINPUT_VERSION 0x800
#include <dinput.h>

#define KEYBOARD_BUFFER_SIZE 256
#define MOUSE_BUFFER_SIZE 256

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
        LPDIRECTINPUTDEVICE2 m_Device;
        CKDWORD m_JoyID;
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
    virtual IDirectInputDevice2 *GetJoystickDxInterface(int iJoystick);

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
    CKJoystick m_Joysticks[4];
    int m_JoystickCount;
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

private:
    void EnsureCursorVisible(CKBOOL iShow);
};

#endif // DX8INPUTMANAGER_H
