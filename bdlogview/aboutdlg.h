#pragma  once
#include "resource.h"
#include "whitebkdlg.h"

#define DLL_DOWNLOAD_URL L"ftp://tmp:tmp@win.baidu.com/incoming/timepp/bdlogview.exe"

const LPCWSTR g_tips[] =
{
	L"������ctrl+f�ڹ��˽���н�������Ŷ",
	L"��־��Ϣ�Ի������ѡ���Զ�����",
	L"�������ø������򣬰ѷ�����������־ͻ����ʾ",
};

class CAboutDlg
	: public CDialogImpl<CAboutDlg>
	, public CColoredDlgImpl
{
public:
	enum {IDD = IDD_ABOUT};

	BEGIN_MSG_MAP(CAboutDlg)
		MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		COMMAND_ID_HANDLER(IDOK, OnOK)
		COMMAND_ID_HANDLER(IDCANCEL, OnCancel)
		COMMAND_ID_HANDLER(IDC_BTN_DOWNLOAD, OnDownload)

		CHAIN_MSG_MAP(CColoredDlgImpl)
	END_MSG_MAP()

private:

	CHyperLink m_link;

	LRESULT OnInitDialog(HWND /*hwndFocus*/, LPARAM /*lp*/)
	{
		CenterWindow(GetParent());

		CStringW strConfPath = CConfig::Instance()->GetConfigFilePath();
		m_link.SubclassWindow(GetDlgItem(IDC_STATIC_CONF_PATH));
		m_link.SetLabel(strConfPath);
		m_link.SetHyperLink(strConfPath);

		GetDlgItem(IDC_STATIC_PRODUCT_NAME).SetWindowText(CConfig::Instance()->GetConfig().product_name.c_str());
		GetDlgItem(IDC_STATIC_VERSION).SetWindowText(helper::GetVersion());

		SetStaticTextColor(IDC_STATIC_INFO, RGB(0, 0, 255));
		CConfig::Instance()->Save();

		return 0;
	}
	void OnClose()
	{
		EndDialog(IDOK);
	}
	LRESULT OnOK(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnCancel(WORD , WORD , HWND , BOOL& )
	{
		OnClose();
		return 0;
	}
	LRESULT OnDownload(WORD , WORD , HWND , BOOL& )
	{
		CStringW strVersion = helper::GetLatestVersion();
		CStringW strInfo;
		if (!strVersion.IsEmpty() && strVersion != helper::GetVersion())
		{
			CStringW strDllPath = helper::GetModuleFilePath();

			CStringW strErr = helper::UpdateSelf();
			if (strErr.IsEmpty())
			{
				strInfo.Format(L"���������°汾: %s, \n�����������Ч��", (LPCWSTR)strVersion);
				MessageBox(strInfo, L"��Ϣ", MB_ICONINFORMATION|MB_OK);
			}
			else
			{
				strInfo.Format(L"����ʧ��: %s", (LPCWSTR)strErr);
				MessageBox(strInfo, L"��Ϣ", MB_ICONWARNING|MB_OK);
			}
		}
		else
		{
			strInfo.Format(L"���°汾Ϊ: %s, ���������°汾.", (LPCWSTR)strVersion);
			MessageBox(strInfo, L"��Ϣ", MB_ICONINFORMATION|MB_OK);
		}

		return 0;
	}
};