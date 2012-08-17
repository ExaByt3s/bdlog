#include "stdafx.h"
#include "outputdevice_file.h"
#include "logutil.h"
#include <time.h>

static BOOL SyncWriteFile(HANDLE file, const void* buffer, size_t len)
{
	DWORD bytesWrite;
	if (!::WriteFile(file, buffer, len, &bytesWrite, NULL))
	{
		LOGWINERR(L"д�ļ�ʧ��");
		return FALSE;
	}

	return TRUE;
}

VOID CALLBACK FileIOCompletionRoutine(
									  __in     DWORD dwErrorCode,
									  __in     DWORD /*dwNumberOfBytesTransfered*/,
									  __inout  LPOVERLAPPED /*lpOverlapped*/
									  )
{
	LOG(tostr(dwErrorCode));
}


OVERLAPPED overlap;
static BOOL AsyncWriteFile(HANDLE file, const void* buffer, size_t len)
{
	overlap.Offset = 0xFFFFFFFF;
	overlap.OffsetHigh = 0xFFFFFFFF;
	::WriteFileEx(file, buffer, len, &overlap, &FileIOCompletionRoutine);
	return FALSE;
}

CLOD_File::CLOD_File() 
	: m_file(NULL), m_buffer(NULL), m_bufferLen(0), m_pos(0), m_syncMode(true), m_lastTime(-1)
{
	m_path[0] = L'\0';
}

CLOD_File::~CLOD_File()
{
	Close();
}

HRESULT CLOD_File::Open(ILogOption* opt)
{
	wcsncpy_s(m_path, opt->GetOption(L"path", L""), _TRUNCATE);
	DWORD shareMode = opt->GetOptionAsBool(L"share_read", true) ? FILE_SHARE_READ : 0U;

	// �Զ�������ͻ���������չ
	WCHAR path[MAX_PATH];
	helper::ExpandVariable(m_path, path, _countof(path));
	::ExpandEnvironmentStringsW(path, m_path, _countof(m_path));

	helper::MakeRequiredDirectory(m_path);
	m_file = ::CreateFileW(m_path, GENERIC_WRITE, shareMode, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	if (m_file == INVALID_HANDLE_VALUE)
	{
		return LastError_HRESULT();
	}

	m_bufferLen = static_cast<size_t>(opt->GetOptionAsInt(L"buffer_size", 1000000));
	m_pos = 0;
	if (m_bufferLen > 0)
	{
		m_buffer = new char[m_bufferLen];
	}

	m_syncMode = !opt->GetOptionAsBool(L"async", false);

	LONG posL = 0;
	LONG posH = 0;
	posL = (LONG)::SetFilePointer(m_file, posL, &posH, FILE_END);
	if (posL == 0 && posH == 0)
	{
		Write("\xFF\xFE", 2);
	}

	return S_OK;
}

HRESULT CLOD_File::Close()
{
	if (m_file != INVALID_HANDLE_VALUE)
	{
		Flush();
		::CloseHandle(m_file);
		m_file = INVALID_HANDLE_VALUE;
	}
	return S_OK;
}

HRESULT CLOD_File::Write(const LogItem* item)
{
	size_t len = 0;

	// ���������־ʱ����ͬ����һ����־�Ϳ���ֱ��ʹ��ǰһ�θ�ʽ������ʱ�䴮��û��Ҫ�ٸ�ʽ��һ��
	if (m_lastTime != item->unixTime)
	{
		m_lastTime = item->unixTime;
		helper::UnixTimeToString(item->unixTime, L"Y-m-d H:M:S", m_cvtbuf, _countof(m_cvtbuf));
	}
	
	len += 19;
	m_cvtbuf[len++] = L'.';
	helper::IntToStr_Padding0(m_cvtbuf + len, _countof(m_cvtbuf) - len, 6, item->microSecond);
	len += 6;

	m_cvtbuf[len++] = L' ';
	helper::IntToStr_Padding0(m_cvtbuf + len, _countof(m_cvtbuf) - len, 2, item->level);
	len += 2;

	m_cvtbuf[len++] = L' ';
	m_cvtbuf[len++] = L'[';
	_itow_s(static_cast<int>(item->pid), m_cvtbuf + len, _countof(m_cvtbuf) - len, 16);
	len += wcslen(m_cvtbuf + len);
	m_cvtbuf[len++] = L':';
	_itow_s(static_cast<int>(item->tid), m_cvtbuf + len, _countof(m_cvtbuf) - len, 16);
	len += wcslen(m_cvtbuf + len);
	m_cvtbuf[len++] = L']';

	m_cvtbuf[len++] = L' ';
	m_cvtbuf[len++] = L'{';
	wcsncpy_s(m_cvtbuf + len, _countof(m_cvtbuf) - len - 2, item->tag, _TRUNCATE);
	wcsncat_s(m_cvtbuf + len, _countof(m_cvtbuf) - len, L"} ", _TRUNCATE);
	len += wcslen(m_cvtbuf + len);

	size_t contentLen = wcslen(item->content);
	size_t copyLen = min(contentLen, _countof(m_cvtbuf) - len - 2);

	wcsncpy_s(m_cvtbuf + len, _countof(m_cvtbuf) - len, item->content, copyLen);
	len += copyLen;

	if (m_cvtbuf[len-1] != L'\n')
	{
		m_cvtbuf[len++] = L'\n';
	}

	Write(m_cvtbuf, len * 2);

	return S_OK;
}

HRESULT CLOD_File::Flush()
{
	if (m_pos > 0)
	{
		if (m_syncMode)
		{
			SyncWriteFile(m_file, m_buffer, m_pos);
		}
		else
		{
			AsyncWriteFile(m_file, m_buffer, m_pos);
		}
		m_pos = 0;
	}

	return S_OK;
}

HRESULT CLOD_File::OnConfigChange(ILogOption* opt)
{
	Close();
	return Open(opt);
}

void CLOD_File::Write(const void* data, size_t len)
{
	if (m_pos + len >= m_bufferLen)
	{
		Flush();
		m_pos = 0;
	}

	if (len >= m_bufferLen)
	{
		if (m_syncMode)
		{
			SyncWriteFile(m_file, data, len);
		}
		else
		{
			AsyncWriteFile(m_file, data, len);
		}
		m_pos = 0;
	}
	else
	{
		memcpy(m_buffer + m_pos, data, len);
		m_pos += len;
	}
}
