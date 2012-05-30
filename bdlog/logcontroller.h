#pragma once

#include <bdlog.h>
#include <accutime.h>
#include "logutil.h"
#include "logfilter.h"
#include "logoption.h"

class CLogController : public ILogController
{
public:
	CLogController();
	virtual ~CLogController();

public:
	virtual HRESULT Init(const wchar_t* configname);
	virtual HRESULT UnInit();
	virtual HRESULT MonitorConfigChange();
	virtual BOOL NeedLog(LogLevel level, const wchar_t* tag);
	virtual HRESULT Log(LogLevel level, const wchar_t* tag, const wchar_t* content);
	virtual HRESULT AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config);
	virtual HRESULT AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config);
	virtual HRESULT RemoveOutputDevice(const wchar_t* name);
	virtual HRESULT ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config);
	virtual HRESULT IncreaseCallDepth();
	virtual HRESULT DecreaseCallDepth();
	virtual HRESULT SetLogUserContext(const wchar_t* prefix);

	/** logcontroller��״̬
	 *  ��Ϊlogcontroller��ȫ�ֶ��������ڶ���δ����ʱ��Щ������ֵ����false��ǡ���ܹ�����Ӧ����˼
	 */
	bool m_inited;      // logcontroller�����Ƿ��ѳ�ʼ��(����ʹ���䷽��)
	bool m_constructed; // logcontroller�����Ƿ��ѹ���
	bool m_destructed;	// logcontroller�����Ƿ�������

private: // types

	struct OutputDevice
	{
		ILogOutputDevice* pDevice;
		wchar_t name[32];
		CLogOption defaultOption;
		wchar_t overlayConfig[1024];
		bool enabled;                             // �Ƿ�����
		CLogFilter* filter;                       // ������
	};
	
private:

	CRITICAL_SECTION m_csLog;
	HANDLE m_monitorThreadQuitEvent;
	HANDLE m_monitorThread;

	OutputDevice* m_ods[32];
	size_t m_odsLen;

	wchar_t m_configpath[64];
	HKEY m_hConfigKey;

	unsigned __int64 m_logID;
	unsigned int m_pid;
	CLogAccurateTime m_accurateTime;
	DWORD m_calldepthTlsIndex;
	wchar_t m_userContext[32];

private:
	HRESULT GetLogOutputDeviceOverlayConfig(const wchar_t* name, wchar_t* buffer, size_t len);
	HRESULT ChangeOutputDeviceOverlayConfig(OutputDevice* device, const wchar_t* config);
	HRESULT OnConfigFileChange();

	// �����߳�
	static unsigned int __stdcall MonitorThread(void* pParam);

};
