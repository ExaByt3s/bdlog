#include "stdafx.h"
#include "logcenter.h"
#include "helper.h"
#include <algorithm>
#include "resource.h"
#include "servicehelper.h"

class CAutoCriticalSection
{
	CCriticalSection& m_cs;
public:
	CAutoCriticalSection(CCriticalSection& cs) : m_cs(cs)
	{
		m_cs.Enter();
	}
	~CAutoCriticalSection()
	{
		m_cs.Leave();
	}
};

CLogCenter::CLogCenter()
{
	m_logID = 1;
	m_timerID = 0;
	m_buffer.reserve(1000);
	m_logDB.reserve(100000);
	m_autoEnablePipeDeviceFile = false;
	m_autoEnablePipeDeviceReg = false;
}

CLogCenter::~CLogCenter()
{

}

void CLogCenter::Init()
{

}

void CLogCenter::ConnectPipe()
{
	Disconnect();

	m_autoEnablePipeDeviceFile = EnablePipeDeviceFile(true);
	m_autoEnablePipeDeviceReg = EnablePipeDeviceReg(true);
	m_logPipeReader.set_listener(this);
	m_logPipeReader.start();

	{
		CAutoCriticalSection cs(m_csListener);
		for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
		{
			(*it)->OnConnect();
		}
	}

	m_timerID = ::SetTimer(NULL, 0, 500, &CLogCenter::TimerProc);
}

void CLogCenter::ConnectFile(LPCWSTR pszPath)
{
	Disconnect();

	m_logFileReader.set_listener(this);
	m_logFileReader.setpath(pszPath);
	m_logFileReader.start();

	{
			CAutoCriticalSection cs(m_csListener);
			for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
			{
				(*it)->OnConnect();
			}
	}

	m_timerID = ::SetTimer(NULL, 0, 500, &CLogCenter::TimerProc);
}

void CLogCenter::Disconnect()
{
	m_logFileReader.stop();
	m_logFileReader.set_listener(NULL);
	m_logPipeReader.stop();
	m_logPipeReader.set_listener(NULL);

	{
			CAutoCriticalSection cs(m_csListener);
			for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
			{
				(*it)->OnDisconnect();
			}
	}

	if (m_autoEnablePipeDeviceFile)
	{
		EnablePipeDeviceFile(false);
	}
	m_autoEnablePipeDeviceFile = false;

	if (m_autoEnablePipeDeviceReg)
	{
		EnablePipeDeviceReg(false);
	}
	m_autoEnablePipeDeviceReg = false;

	::KillTimer(NULL, m_timerID);
}

void CLogCenter::AddListener(CLogCenterListener* pListener)
{
	CAutoCriticalSection cs(m_csListener);

	ListenerList::const_iterator it = std::find(m_listeners.begin(), m_listeners.end(), pListener);
	if (it == m_listeners.end())
	{
		m_listeners.push_back(pListener);
	}
}

void CLogCenter::RemoveListener(CLogCenterListener* pListener)
{
	CAutoCriticalSection cs(m_csListener);

	ListenerList::iterator it = std::find(m_listeners.begin(), m_listeners.end(), pListener);
	if (it != m_listeners.end())
	{
		m_listeners.erase(it);
	}
}

void CLogCenter::on_new_log(bdlog::logitem* li)
{
	CAutoCriticalSection cs(m_csLogBuffer);

	m_buffer.push_back(li);
}


void CLogCenter::on_notify(int notifyid, HRESULT hr)
{

}

void CLogCenter::MergeBuffer()
{
	if (m_csLogDB.TryEnter())
	{
		ON_LEAVE_1(m_csLogDB.Leave(), CCriticalSection&, m_csLogDB);

		LogRange newrange;
		newrange.idmin = m_logID;

		{
			CAutoCriticalSection cs(m_csLogBuffer);
			for (LogBuffer::const_iterator it = m_buffer.begin(); it != m_buffer.end(); ++it)
			{
				LogInfo* li = new LogInfo;
				li->logid = m_logID++;
				li->item = *it;
				m_logDB.push_back(li);
			}
			m_buffer.clear();
		}

		newrange.idmax = m_logID;
		
		if (newrange.idmin < newrange.idmax)
		{
			CAutoCriticalSection cs(m_csListener);
			for (ListenerList::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
			{
				(*it)->OnNewLog(newrange);
			}
		}
	}
}

VOID CLogCenter::TimerProc( __in HWND hwnd, __in UINT uMsg, __in UINT_PTR idEvent, __in DWORD dwTime )
{
	ServiceHelper::GetLogCenter()->MergeBuffer();
}

LogRange CLogCenter::GetLogRange()
{
	CAutoCriticalSection cs(m_csLogDB);

	LogRange range;
	if (!m_logDB.empty())
	{
		range.idmin = m_logDB.front()->logid;
		range.idmax = m_logDB.back()->logid + 1;
	}
	else
	{
		range.idmax = range.idmin = 0;
	}

	return range;
}

LogInfo* CLogCenter::GetLog(UINT64 logid)
{
	CAutoCriticalSection cs(m_csLogDB);

	if (m_logDB.empty())
	{
		return NULL;
	}

	UINT64 firstid = m_logDB.front()->logid;
	if (firstid > logid)
	{
		return NULL;
	}

	size_t index = static_cast<size_t>(logid - firstid);
	return m_logDB[index];
}

void CLogCenter::LockLog()
{
	m_csLogDB.Enter();
}

void CLogCenter::UnlockLog()
{
	m_csLogDB.Leave();
}

bool CLogCenter::EnablePipeDeviceFile(bool bEnable)
{
	CStringW strXlogPath = helper::ExpandPath(BSP_BDXLOG_INI);
	helper::WriteFileIfNotExist(strXlogPath, MAKEINTRESOURCEW(IDR_BDXLOG_INI), L"PDATA");

	CStringW desiredEnableString = bEnable? L"enable:true" : L"enable:false";
	WCHAR buffer[4096];
	::GetPrivateProfileStringW(L"LOG_CONFIG", L"ld_pipe", L"", buffer, _countof(buffer), strXlogPath);
	CStringW bufR = buffer;
	CStringW bufW = buffer;
	bufR.MakeLower();
	int pos = bufR.Find(L"enable:");
	if (pos == -1)
	{
		bufW = desiredEnableString + L" " + bufW;
	}
	else
	{
		int pos2 = bufR.Find(L' ', pos);
		if (pos2 == -1) pos2 = bufR.GetLength();
		bufW = bufW.Mid(0, pos) + desiredEnableString + bufW.Mid(pos2);
	}

	if (bufW.Compare(buffer) != 0)
	{
		::WritePrivateProfileStringW(L"LOG_CONFIG", L"ld_pipe", bufW, strXlogPath);
		return true;
	}

	return false;
}

bool CLogCenter::EnablePipeDeviceReg(bool bEnable)
{
	CStringW desiredEnableString = bEnable? L"enable:true" : L"enable:false";

	HKEY hKey = NULL;
	CStringW path = CStringW(L"Software\\Baidu\\BDLOG\\") + helper::GetProductName();
	::RegCreateKeyExW(HKEY_CURRENT_USER, path, 0, NULL, 0, KEY_QUERY_VALUE|KEY_SET_VALUE, NULL, &hKey, NULL);
	if (!hKey)
	{
		helper::GUIReportError(L"��ע����޸�pipe�豸������ʧ��");
		return false;
	}

	WCHAR buffer[4096] = {0};
	DWORD buflen = sizeof(buffer);
	::RegQueryValueExW(hKey, L"ld_pipe", NULL, NULL, (LPBYTE)buffer, &buflen);

	CStringW bufR = buffer;
	CStringW bufW = buffer;
	bufR.MakeLower();
	int pos = bufR.Find(L"enable:");
	if (pos == -1)
	{
		bufW = desiredEnableString + L" " + bufW;
	}
	else
	{
		int pos2 = bufR.Find(L' ', pos);
		if (pos2 == -1) pos2 = bufR.GetLength();
		bufW = bufW.Mid(0, pos) + desiredEnableString + bufW.Mid(pos2);
	}

	if (bufW.Compare(buffer) != 0)
	{
		DWORD cbData = static_cast<DWORD>(bufW.GetLength() * 2 + 2);
		::RegSetValueExW(hKey, L"ld_pipe", 0, REG_SZ, (const BYTE*)(LPCWSTR)bufW, cbData);
		::RegCloseKey(hKey);
		return true;
	}

	::RegCloseKey(hKey);
	return false;
}

bool CLogCenter::MonitoringPipe() const
{
	return m_logPipeReader.working();
}

size_t CLogCenter::source_count() const
{
	if (m_logPipeReader.working())
	{
		return m_logPipeReader.source_count();
	}
	else
	{
		return m_logFileReader.source_count();
	}
}

bdlog::lsi_vec_t CLogCenter::get_sources() const
{
	if (m_logPipeReader.working())
	{
		return m_logPipeReader.get_sources();
	}
	else
	{
		return m_logFileReader.get_sources();
	}
}

void CLogCenter::ClearAllLog()
{
	ClearOldLog((UINT64)-1);
}

void CLogCenter::ClearOldLog(UINT64 logid)
{
	CAutoCriticalSection cs(m_csLogDB);

	LogDB::iterator it;
	for (it = m_logDB.begin(); it != m_logDB.end() && (*it)->logid < logid; ++it)
	{
		delete (*it)->item;
		delete *it;
	}
	m_logDB.erase(m_logDB.begin(), it);
}
