#include <Windows.h>

#include "CKAll.h"

#define CKOGUID_GETMOUSEPOSITION CKGUID(0x6ea0201, 0x680e3a62)
#define CKOGUID_GETMOUSEX CKGUID(0x53c51abe, 0xeba68de)
#define CKOGUID_GETMOUSEY CKGUID(0x27af3c9f, 0xdbc4eb3)

int CKKeyStringFunc(CKParameter *param, CKSTRING ValueString, CKBOOL ReadFromString)
{
    if (!param) return 0;

    CKInputManager *im = (CKInputManager *)param->m_Context->GetManagerByGuid(INPUT_MANAGER_GUID);
    if (ReadFromString)
    {
        if (!ValueString) return 0;
        CKSTRING name = NULL;
        if (ValueString[0] != '\0')
            name = (CKSTRING)im->GetKeyFromName(ValueString);
        param->SetValue(&name);
    }
    else
    {
        CKDWORD key = NULL;
        param->GetValue(&key, FALSE);
        int len = im->GetKeyName(key, ValueString);
        if (len > 1) return len;
    }
    return 0;
}

void CK2dVectorGetMousePos(CKContext *context, CKParameterOut *res, CKParameterIn *p1, CKParameterIn *p2)
{
    CKInputManager *im = (CKInputManager *)context->GetManagerByGuid(INPUT_MANAGER_GUID);
    if (im)
    {
        Vx2DVector pos;
        im->GetMousePosition(pos, TRUE);

        CKParameter *param = p1->GetRealSource();
        if (!param)
        {
            *(Vx2DVector *)res->GetWriteDataPtr() = pos;
            return;
        }

        CKBOOL absolute = FALSE;
        param->GetValue(&absolute);
        if (absolute)
        {
            CKRenderContext *rc = context->GetPlayerRenderContext();
            if (rc)
            {
                HWND hWnd = (HWND)rc->GetWindowHandle();
                POINT pt;
                pt.x = (LONG)pos.x;
                pt.y = (LONG)pos.y;
                ::ScreenToClient(hWnd, &pt);
                if (pt.x >= 0)
                {
                    int width = rc->GetWidth();
                    if (pt.x >= width)
                        pt.x = width - 1;
                }
                else
                {
                    pt.x = 0;
                }
                if (pt.y >= 0)
                {
                    int height = rc->GetHeight();
                    if (pt.y >= height)
                        pt.y = height - 1;
                }
                else
                {
                    pt.y = 0;
                }
                pos.x = (float)pt.x;
                pos.y = (float)pt.y;
            }
        }
        *(Vx2DVector *)res->GetWriteDataPtr() = pos;
    }
}

void CKIntGetMouseX(CKContext *context, CKParameterOut *res, CKParameterIn *p1, CKParameterIn *p2)
{
    CKInputManager *im = (CKInputManager *)context->GetManagerByGuid(INPUT_MANAGER_GUID);
    if (im)
    {
        Vx2DVector pos;
        im->GetMousePosition(pos, TRUE);
        *(int *)res->GetWriteDataPtr() = (int)pos.x;
    }
}

void CKIntGetMouseY(CKContext *context, CKParameterOut *res, CKParameterIn *p1, CKParameterIn *p2)
{
    CKInputManager *im = (CKInputManager *)context->GetManagerByGuid(INPUT_MANAGER_GUID);
    if (im)
    {
        Vx2DVector pos;
        im->GetMousePosition(pos, TRUE);
        *(int *)res->GetWriteDataPtr() = (int)pos.y;
    }
}

void CKInitializeParameterTypes(CKContext *context)
{
    CKParameterTypeDesc desc;
    desc.TypeName = "Keyboard Key";
    desc.Guid = CKPGUID_KEY;
    desc.DerivedFrom = CKPGUID_INT;
    desc.Valid = TRUE;
    desc.DefaultSize = 4;
    desc.CreateDefaultFunction = NULL;
    desc.DeleteFunction = NULL;
    desc.SaveLoadFunction = NULL;
    desc.StringFunction = CKKeyStringFunc;
    desc.UICreatorFunction = NULL;
    desc.dwParam = 0;
    desc.dwFlags = 0;
    desc.Cid = 0;

    CKParameterManager *pm = context->GetParameterManager();
    pm->RegisterParameterType(&desc);
}

void CKInitializeOperationTypes(CKContext *context)
{
    CKParameterManager *pm = context->GetParameterManager();
    pm->RegisterOperationType(CKOGUID_GETMOUSEPOSITION, "Get Mouse Position");
    pm->RegisterOperationType(CKOGUID_GETMOUSEX, "Get Mouse X");
    pm->RegisterOperationType(CKOGUID_GETMOUSEY, "Get Mouse Y");
}

void CKInitializeOperationFunctions(CKContext *context)
{
    CKParameterManager *pm = context->GetParameterManager();
    pm->RegisterOperationFunction(CKOGUID_GETMOUSEX, CKPGUID_INT, CKPGUID_NONE, CKPGUID_NONE, CKIntGetMouseX);
    pm->RegisterOperationFunction(CKOGUID_GETMOUSEY, CKPGUID_INT, CKPGUID_NONE, CKPGUID_NONE, CKIntGetMouseY);
    pm->RegisterOperationFunction(CKOGUID_GETMOUSEPOSITION, CKPGUID_2DVECTOR, CKPGUID_NONE, CKPGUID_NONE, CK2dVectorGetMousePos);
    pm->RegisterOperationFunction(CKOGUID_GETMOUSEPOSITION, CKPGUID_2DVECTOR, CKPGUID_BOOL, CKPGUID_NONE, CK2dVectorGetMousePos);
}

void CKUnInitializeParameterTypes(CKContext *context)
{
    CKParameterManager *pm = context->GetParameterManager();
    pm->UnRegisterParameterType(CKPGUID_KEY);
}

void CKUnInitializeOperationTypes(CKContext *context)
{
    CKParameterManager *pm = context->GetParameterManager();
    pm->UnRegisterOperationType(CKOGUID_GETMOUSEPOSITION);
    pm->UnRegisterOperationType(CKOGUID_GETMOUSEX);
    pm->UnRegisterOperationType(CKOGUID_GETMOUSEY);
}