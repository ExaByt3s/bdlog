#include "stdafx.h"

#include <bdlog.h>
#include "logcontroller.h"

#ifdef BDLOG_BUILD_STATIC_LIB

// ��static lib������£���Ϊȫ�ֱ����ĳ�ʼ��˳���޷���֤������Ҳ���޷���֤LogController���㹻���ʱ���ʼ����
// ������static lib������£�LogController��ʵ��Ϊ���ڵ�һ�λ�ȡ��ʱ���ʼ��
// ���ⲻ���̰߳�ȫ�ģ�������ȷ�������߳�����֮ǰ��ʼ��LogController

BDLOGAPI ILogController* GetLogController()
{
#pragma warning(push)
#pragma warning(disable: 4640)
	static CLogController s_controller;  // ��ȷ�����������߳�����֮ǰ����GetLogController
#pragma warning(pop)

	return &s_controller;
}

#else // no BDLOG_BUILD_STATIC_LIB

static char Room_For_LogControler[sizeof(CLogController)];

BDLOGAPI ILogController* GetLogController()
{
	return (CLogController*)Room_For_LogControler;
}

BOOL APIENTRY DllMain( HMODULE /*hModule*/, DWORD ul_reason_for_call, LPVOID /*lpReserved*/)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		new(Room_For_LogControler) CLogController;
	}

	return TRUE;
}

#endif
