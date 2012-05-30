#pragma once

#include <windows.h>
#include <time.h>
#include "bdsharemem.h"

// TODO ���ߺ��û�׼ʱ��+�������������ʱ������ڵ�ǰʱ��

#pragma pack(push, 1)
struct TimingInfo
{
	__int32 version;

	__int64 baseTime;                     // ��׼ʱ��(unix time)

	__int64 basePerformanceCounter;       // ��׼�߾��ȼ�ʱ������
	__int64 performanceFrequency;         // �߾��ȼ�ʱ��Ƶ��

	unsigned __int64 baseTickCount;       // ��׼tickcount�����߾��ȼ�ʱ��������ʱ��
};
#pragma pack(pop)


struct AccurateTime
{
	time_t unix_time;
	int micro_second;
};

class CLogAccurateTime
{
public:
	CLogAccurateTime()
		: m_timingInfo()
	{
	}

	void Init()
	{
		HRESULT hr = m_sm.Open(L"bdlog_timing_info", sizeof(TimingInfo), FALSE);
		if (FAILED(hr))
		{
			m_timingInfo = CreateTimingInfo();
			return;
		}

		m_sm.Lock();
		TimingInfo& info = m_sm.GetDataAs<TimingInfo>();
		{
			if (hr == S_OK)
			{
				info = CreateTimingInfo();
			}

			m_timingInfo = info;
		}
		m_sm.UnLock();
	}

	AccurateTime GetTime() const
	{
		AccurateTime t;
		if (m_timingInfo.performanceFrequency > 0)
		{
			LARGE_INTEGER li;
			::QueryPerformanceCounter(&li);
			__int64 counterDelta = li.QuadPart - m_timingInfo.basePerformanceCounter;
			__int64 counterPerSecond = m_timingInfo.performanceFrequency;
			t.unix_time = m_timingInfo.baseTime + counterDelta / counterPerSecond;
			t.micro_second = static_cast<int>(counterDelta % counterPerSecond * 1000000 / counterPerSecond);
		}
		else
		{
			__int64 tickCount = static_cast<__int64>(GetLowResolutionTickCount());
			__int64 maxDWORD = 0x100000000LL;
			time_t currTime = time(NULL);
			for (__int64 i = 0; ;i++)
			{
				t.unix_time = m_timingInfo.baseTime + (maxDWORD * i + tickCount - static_cast<__int64>(m_timingInfo.baseTickCount)) / 1000;
				if (t.unix_time > currTime - 10) break;
			}
			t.micro_second = static_cast<int>((tickCount - m_timingInfo.baseTickCount) % 1000 * 1000);
		}

		return t;
	}

private:
	TimingInfo m_timingInfo;
	CSharingMemory m_sm;

	static unsigned __int64 GetLowResolutionTickCount()
	{
		// NOTE �ⲻ��һ��BUG
		// GetTickCount64��XPϵͳ�ϲ�����
		// ��Ȼʹ���˿��ܲ������Ƶ�32λGetTickCount������accutime��Ի��Ƶ���������˴���
		__pragma(warning(push));
		__pragma(warning(disable:28159));
		return ::GetTickCount();
		__pragma(warning(pop));
	}

	static TimingInfo CreateTimingInfo()
	{
		TimingInfo info;
		info.version = 1;

		LARGE_INTEGER freq;
		LARGE_INTEGER counter;
		if (::QueryPerformanceFrequency(&freq) && ::QueryPerformanceCounter(&counter))
		{
			info.basePerformanceCounter = counter.QuadPart;
			info.performanceFrequency = freq.QuadPart;
			info.baseTime = time(NULL);
			info.baseTickCount = 0;
		}
		else
		{
			info.basePerformanceCounter = 0;
			info.performanceFrequency = 0;
			info.baseTime = time(NULL);
			info.baseTickCount = GetLowResolutionTickCount();
		}

		return info;
	}
};
