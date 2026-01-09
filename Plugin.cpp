#include "CKAll.h"

#include "DX8InputManager.h"

#ifdef CK_LIB
#define CreateNewManager				CreateNewInputManager
#define RemoveManager					RemoveInputManager
#define CKGetPluginInfoCount			CKGet_InputManager_PluginInfoCount
#define CKGetPluginInfo					CKGet_InputManager_PluginInfo
#define g_PluginInfo					g_InputManager_PluginInfo
#else
#define CreateNewManager                CreateNewManager
#define RemoveManager                   RemoveManager
#define CKGetPluginInfoCount            CKGetPluginInfoCount
#define CKGetPluginInfo                 CKGetPluginInfo
#define g_PluginInfo                    g_PluginInfo
#endif

CKPluginInfo g_InputManager_PluginInfo;

#define DX8_INPUTMANAGER_GUID CKGUID(0xF787C904, 0)

void CKInitializeParameterTypes(CKContext *context);
void CKInitializeOperationTypes(CKContext *context);
void CKInitializeOperationFunctions(CKContext *context);
void CKUnInitializeParameterTypes(CKContext *context);
void CKUnInitializeOperationTypes(CKContext *context);

CKERROR CreateNewManager(CKContext *context)
{
    CKInitializeParameterTypes(context);
    CKInitializeOperationTypes(context);
    CKInitializeOperationFunctions(context);

    new DX8InputManager(context);

    return CK_OK;
}

CKERROR RemoveManager(CKContext *context)
{
    DX8InputManager *man = (DX8InputManager *)context->GetManagerByName("DirectX Input Manager");
    if (man) delete man;

    CKUnInitializeParameterTypes(context);
    CKUnInitializeOperationTypes(context);

    return CK_OK;
}

PLUGIN_EXPORT CKPluginInfo *CKGetPluginInfo(int Index)
{
    g_InputManager_PluginInfo.m_Author = "Virtools";
    g_InputManager_PluginInfo.m_Description = "DirectX Keyboard/Mouse/Joystick Manager";
    g_InputManager_PluginInfo.m_Extension = "";
    g_InputManager_PluginInfo.m_Type = CKPLUGIN_MANAGER_DLL;
    g_InputManager_PluginInfo.m_Version = 0x000002;
    g_InputManager_PluginInfo.m_InitInstanceFct = CreateNewManager;
    g_InputManager_PluginInfo.m_ExitInstanceFct = RemoveManager;
    g_InputManager_PluginInfo.m_GUID = DX8_INPUTMANAGER_GUID;
    g_InputManager_PluginInfo.m_Summary = "DirectX Input Manager";
    return &g_InputManager_PluginInfo;
}