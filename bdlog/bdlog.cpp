#include "stdafx.h"

#include <bdlog.h>
#include "logcontroller.h"

#define BDLOG_ENV_NAME_CONTROLLER_OJBECT_ADDR L"BDLOG_ENV_NAME_CONTROLLER_OBJECT_ADDR"

CLogController g_logController;

#ifdef BDLOG_USE_AS_LIB
// ���뱣֤���������ģ�����õ���ͬһ��log controller����
// ����DLL��ʽ����û������ģ�������LIB��ʽ�¾Ͳ����ˣ���Ϊ���������ģ����ܶ���̬������bdlog lib

CLogController* g_logControllerPtr = NULL;
BDLOGAPI ILogController* GetLogController()
{
	if (g_logControllerPtr == NULL)
	{
		wchar_t buf[256] = L"0";
		if (::GetEnvironmentVariableW(BDLOG_ENV_NAME_CONTROLLER_OJBECT_ADDR, buf, _countof(buf)) > 0)
		{
			swscanf_s(buf, L"%p", &g_logControllerPtr);
		}
		if (g_logControllerPtr == NULL)
		{
			static CLogController s_ctrl;
			g_logControllerPtr = &s_ctrl;
			_snwprintf_s(buf, _TRUNCATE, L"%p", g_logControllerPtr);
			::SetEnvironmentVariableW(BDLOG_ENV_NAME_CONTROLLER_OJBECT_ADDR, buf);
		}
	}

	return g_logControllerPtr;
}

#else // no BDLOG_USE_AS_LIB
BDLOGAPI ILogController* GetLogController()
{
	return &g_logController;
}
#endif // BDLOG_USE_AS_LIB


