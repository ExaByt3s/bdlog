#pragma once

/************************************************************************
�򵥵ĸ��²��ԣ��Զ���顢�Զ����ء��Զ���װ��
��ϸ���̣�
1. ������
2. ���и��£����°汾���ص�bdlogview.exe.new
3. ��bdlogview.exe ����Ϊ bdlogview.exe.old
4. ��bdlogview.exe.new ����Ϊ bdlogview.exe
5. �´�����ʱ��Ч
6. ����ʱ��������bdlogview.exe.old����ɾ��֮����չʾ������ʾ����ʾ�°汾�Ĺ���
************************************************************************/

#include <vector>

struct VersionDetail
{
	UINT64 ver;
	CStringW description;
};

typedef std::vector<VersionDetail> Versions;

struct Updater
{
	static void CheckAndUpdate(BOOL bBackend);

	/// ����ʱ���ã�չʾ�°汾��ϸ��Ϣ
	static void NotifyNewVersion();

	static CStringW GetVersionInfoFilePath();

private:
	static Versions ParseVersionInfoFile();
	static CStringW GetOldExePath();
	static unsigned int __stdcall CheckAndUpdateWorker(void* param);
};

