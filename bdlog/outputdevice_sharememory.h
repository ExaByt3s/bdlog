#pragma once

#include "outputdevice_common.h"
#include <bdsharemem.h>

#pragma pack(push, 1)
struct MemLogQuene
{
	struct MemLogRecord
	{
		INT64 unixTime;
		INT32 milliSecond;
		UINT32 tid;
		WCHAR content[120];
	};

	UINT32 totalLength;                      //�����ڴ�Ĵ�С
	UINT32 recordCount;                 //��־���д�С
	UINT32 recordLength;                      //������־��С
	UINT32 head;                         //����ͷ
	UINT32 rear;                         //����β
	MemLogRecord record[1000]; //��������
};
#pragma pack(pop)

class CLOD_ShareMemory : public CLogOutputDeviceBase
{
public:
	virtual HRESULT Open(ILogOption* opt);
	virtual HRESULT Close();
	virtual HRESULT Write(const LogItem* item);
	virtual HRESULT Flush();
	virtual HRESULT OnConfigChange(ILogOption* opt);

	CLOD_ShareMemory();

private:
	static void GetShareMemoryName(ILogOption* opt, wchar_t* name, size_t len);
	CSharingMemory m_shareMemory;
	wchar_t m_smName[256];
};

