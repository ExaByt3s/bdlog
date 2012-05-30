#pragma once

#include <string>
#include <set>

#include "service.h"
#include "serviceid.h"
#include "logcenter.h"

class CLogPropertyDB : public Service<SID_LogPropertyDB>, public CLogCenterListener
{
public:
	typedef std::set<std::wstring> TagSet;
	typedef std::set<UINT> LevelSet;

public:
	void Init()
	{
		m_logCenter->AddListener(this);
	}

	void AddLevel(UINT32 lvl)
	{
		if (m_runtimelvlset.insert(lvl).second)
		{
			m_lvlVersion++;
		}
	}

	void AddTag(const std::wstring& tag)
	{
		for (size_t i = 0, j = 0; ; j++)
		{
			if (tag[j] == L';' || tag[j] == L'\0')
			{
				if (i < j)
				{
					if (m_tagset.insert(tag.substr(i, j-i)).second)
					{
						m_tagVersion++;
					}
				}
				if (tag[j] == L'\0') break;
				i = j+1;
			}
		}
	}

	void Clear()
	{
		m_tagset.clear();
		m_tagVersion++;

		m_runtimelvlset.clear();
		m_lvlVersion++;
	}

	const TagSet& GetTags(int* ver) const
	{
		*ver = m_tagVersion;
		return m_tagset;
	}

	const LevelSet& GetRuntimeLevels(int* ver) const
	{
		*ver = m_lvlVersion;
		return m_runtimelvlset;
	}

	const LevelSet& GetDefaultLevels() const
	{
		return m_defaultlvlset;
	}

	static const wchar_t* GetLevelDesc(UINT level)
	{
		switch (level)
		{
		case 0x05: return L"���";
		case 0x10: return L"����";
		case 0x20: return L"��Ϣ";
		case 0x25: return L"�¼�";
		case 0x30: return L"����";
		case 0x40: return L"����";
		default: break;
		}
		return NULL;
	}

	CLogPropertyDB() : m_tagVersion(1), m_lvlVersion(1)
	{
		m_defaultlvlset.insert(0x05);
		m_defaultlvlset.insert(0x10);
		m_defaultlvlset.insert(0x20);
		m_defaultlvlset.insert(0x25);
		m_defaultlvlset.insert(0x30);
		m_defaultlvlset.insert(0x40);

		m_logCenter = ServiceMgr::Instance().GetService<CLogCenter>();
	}

	~CLogPropertyDB()
	{
		m_logCenter->RemoveListener(this);
	}

	static CLogPropertyDB& Instance()
	{
		return *ServiceMgr::GetService<CLogPropertyDB>().operator->();
	}

	virtual void OnNewLog(const LogRange& range)
	{
		for (UINT64 i = range.idmin; i < range.idmax; i++)
		{
			const LogInfo* li = m_logCenter->GetLog(i);
			AddTag(li->item->log_tags);
			AddLevel(li->item->log_class);
		}
	}

	virtual void OnConnect()
	{
		Clear();
	}
	
	virtual void OnDisconnect()
	{
	}

private:
	TagSet m_tagset;
	int m_tagVersion;

	LevelSet m_defaultlvlset;
	LevelSet m_runtimelvlset;
	int m_lvlVersion;

	CServicePtr<CLogCenter> m_logCenter;
};

DEFINE_SERVICE(CLogPropertyDB, L"��־���Թ�����");
