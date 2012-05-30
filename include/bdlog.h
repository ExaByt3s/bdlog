#ifndef BDCLIENT_LOG_H
#define BDCLIENT_LOG_H

/** @file
 *  ��־�ӿ�
 *  - printf����﷨
 *  - ��־�������־��ǩ(ÿ����־�ɹ��������ǩ��������ڲ鿴�ͷ���)
 *  - ���������õ���־����豸(����̨���ļ����ܵ��������ڴ��)����������־����豸֧��
 *  - ����ʱͨ��INI�ļ�������־���ú���־������
 *
 *  ����bdlogʹ�÷�ʽ�ĺ꣨��ʹ��bdlog�Ĺ����ﶨ�壩
 *  - BDLOG_USE_AS_STATIC_LIB         ��Ϊ��̬��ʹ��
 *  - BDLOG_USE_AS_DLL_DYNAMIC_LOAD   ��ΪDLLʹ�ã���̬����
 *  - BDLOG_USE_AS_DLL                ��ΪDLLʹ�ã���̬���ء����δ��������������֮һ����ȱʡ����˿���
 */

#include <windows.h>
#include <stdio.h>

#ifdef BDLOG_SELF_BUILD
#	ifdef BDLOG_STATIC_LIB
#		define BDLOGAPI extern "C"
#	else
#		define BDLOGAPI extern "C" __declspec(dllexport)
#	endif
#else
#	if !defined(BDLOG_USE_AS_STATIC_LIB) && !defined(BDLOG_USE_AS_DLL_DYNAMIC_LOAD)
#		define BDLOG_USE_AS_DLL
#	endif
#	if defined(BDLOG_USE_AS_DLL)
#		define BDLOGAPI extern "C" __declspec(dllimport)
#		pragma comment(lib, "bdlog.lib")
#	elif defined(BDLOG_USE_AS_STATIC_LIB)
#		define BDLOGAPI extern "C"
#	elif defined(BDLOG_USE_AS_DLL_DYNAMIC_LOAD)
#		define BDLOGAPI extern "C"
#	endif
#endif


/// ��־����
enum LogLevel
{
	LL_DIAGNOSE = 0x05,
	LL_DEBUG    = 0x10,
	LL_EVENT    = 0x25,
	LL_ERROR    = 0x40,
};

#define FACILITY_BDLOG 0x10D
#define BDLOG_E_DEVICE_NOT_READY                  MAKE_HRESULT(1, FACILITY_BDLOG, 1)
#define BDLOG_E_NO_REPORTER                       MAKE_HRESULT(1, FACILITY_BDLOG, 2)
#define BDLOG_E_NOT_INITED                        MAKE_HRESULT(1, FACILITY_BDLOG, 3)
#define BDLOG_E_DESTRUCTED                        MAKE_HRESULT(1, FACILITY_BDLOG, 4)
#define BDLOG_E_ALREADY_INITED                    MAKE_HRESULT(1, FACILITY_BDLOG, 5)
#define BDLOG_E_OUTPUT_DEVICE_ALREADY_EXIST       MAKE_HRESULT(1, FACILITY_BDLOG, 6)
#define BDLOG_E_OUTPUT_DEVICE_NOT_FOUND           MAKE_HRESULT(1, FACILITY_BDLOG, 7)
#define BDLOG_E_OUTPUT_DEVICE_FULL                MAKE_HRESULT(1, FACILITY_BDLOG, 8)
#define BDLOG_E_BEFORE_CONSTRUCT                  MAKE_HRESULT(1, FACILITY_BDLOG, 9)
#define BDLOG_E_FUNCTION_UNAVAILBLE               MAKE_HRESULT(1, FACILITY_BDLOG, 10)

struct LogItem
{
	unsigned __int64 id;          // ÿ����־����ʱ���䣬������Ψһ

	__int64 unixTime;             // ��־����ʱ��ʱ�䣨�룩������ͬtime()�����ķ���ֵ
	int microSecond;              // ��־����ʱ��ʱ�䣺΢��
	LogLevel level;               // ��־����
	const wchar_t* tag;           // ��־��ǩ��һ����־���Թ��������ǩ���ֺŷָ���
	const wchar_t* content;       // ��־����

	unsigned int tid;             // ��־����ʱ���߳�ID
	unsigned int pid;             // ��־����ʱ�Ľ���ID
	int depth;                    // ��־��ȣ�ÿ���̵߳���־����Ƕ����ġ�
	const wchar_t* userContext;   // �û�ָ���������ģ�ÿ����־����ͬ
};

struct ILogOption
{
	virtual const wchar_t* GetOption(const wchar_t* key, const wchar_t* defaultValue) = 0;
	virtual int GetOptionAsInt(const wchar_t* key, int defaultValue) = 0;
	virtual bool GetOptionAsBool(const wchar_t* key, bool defaultValue) = 0;
};

struct ILogOutputDevice
{
	/** ��һ����־�豸
	 * 
	 *  @param config һ���ַ�����ʽ�����������Ϣ��
	 *                ������Ϣ�����ո�ֿ��Ķ��������������ʽ�� KEY:VAL
	 *                KEY=enable��KEY=filter������������־ϵͳ�����������������ɾ������־����豸����ͽ���
	 */
	virtual HRESULT Open(ILogOption* opt) = 0;

	/// �ر���־�豸
	virtual HRESULT Close() = 0;

	/// д����־
	virtual HRESULT Write(const LogItem* item) = 0;

	/// ˢ�»���
	virtual HRESULT Flush() = 0;

	/** ��Ӧ���ñ仯
	 *  ��Щ����£������û��ֶ��޸�����־�����ļ���ʹ��������������ö�̬��Ч
	 */
	virtual HRESULT OnConfigChange(ILogOption* opt) = 0;

	virtual void IncreaseRefCounter() = 0;
	virtual void DecreaseRefCounter() = 0;

	virtual ~ILogOutputDevice(){}
};

enum LogOutputDeviceType
{
	LODT_NULL,
	LODT_CONSOLE,
	LODT_DEBUGOUTPUT,
	LODT_FILE,
	LODT_PIPE,
	LODT_SHARED_MEMORY
};

struct ILogController
{
	/** ��ʼ����־������
	 *  
	 *  @param configname [opt] logcontroller����HKCU\Software\Baidu\BDLOG\configname�²���������Ϣ
	 *         configname���ܳ���32���ַ�
	 *         confignameΪNULL��ʾ��־�����������ע����ȡconfig��Ϣ
	 */
	virtual HRESULT Init(const wchar_t* configname) = 0;
	virtual HRESULT UnInit() = 0;

	/** �����־
	 *  ��־�����������Ŀǰ���õ���־�豸�������������������־�豸���ε���Write����
	 *  ��־ID��ʱ�䡢�̺߳ŵ���Ϣ����־�������Զ�����
	 */
	virtual BOOL NeedLog(LogLevel level, const wchar_t* tag) = 0;
	virtual HRESULT Log(LogLevel level, const wchar_t* tag, const wchar_t* content) = 0;

	/** ��־����豸����
	 *
	 *  @param name ��־�豸���֣��Ժ�Ψһ��ʶ����豸��Ҫ�󳤶�С��32���ַ�
	 *  @param type ��־�豸������
	 *  @param config ��־�豸�����ô�
	 *  @note ͬһ�����͵���־�豸�����в�ͬ���ֵĶ��ʵ��
	 */
	virtual HRESULT AddOutputDevice(const wchar_t* name, LogOutputDeviceType type, const wchar_t* config) = 0;
	virtual HRESULT AddCustomOutputDevice(const wchar_t* name, ILogOutputDevice* device, const wchar_t* config) = 0;
	virtual HRESULT RemoveOutputDevice(const wchar_t* name) = 0;
	virtual HRESULT ChangeOutputDeviceConfig(const wchar_t* name, const wchar_t* config) = 0;

	/// �ı���־��� 
	virtual HRESULT IncreaseCallDepth() = 0;
	virtual HRESULT DecreaseCallDepth() = 0;
	virtual HRESULT SetLogUserContext(const wchar_t* prefix) = 0;

	/** ������־���� 
	 *
	 *  ��־������������һ���µ��̣߳�����ע�����ĸĶ�
	 */
	virtual HRESULT MonitorConfigChange() = 0;
};

// ��־����ӿ�
BDLOGAPI ILogController* GetLogController();

#ifdef BDLOG_USE_AS_DLL_DYNAMIC_LOAD
namespace bdlog
{
	class DLLLoader
	{
		typedef ILogController* (*PFUNC_GetLogController)();

		HMODULE m_hDll;
		ILogController* m_ctrl;

		template <typename T>
		struct GlobalData
		{
			static DLLLoader s_loader;
		};

		DLLLoader() : m_hDll(NULL)
		{
			m_hDll = ::LoadLibraryW(L"bdlog.dll");
			if (m_hDll)
			{
				PFUNC_GetLogController pFunc;
#pragma warning(push)
#pragma warning(disable: 4191)
				pFunc = reinterpret_cast<PFUNC_GetLogController>(::GetProcAddress(m_hDll, "GetLogController"));
#pragma warning(pop)
				m_ctrl = pFunc();
			}
		}
		~DLLLoader()
		{
			if (m_hDll)
			{
				::FreeLibrary(m_hDll);
				m_hDll = NULL;
			}
			m_ctrl = NULL;
		}

	public:
		static DLLLoader& Instance()
		{
			return GlobalData<int>::s_loader;
		}
		ILogController* GetLogController() const
		{
			return m_ctrl;
		}
	};

	DLLLoader DLLLoader::GlobalData<int>::s_loader;
};
BDLOGAPI inline ILogController* GetLogController()
{
	return bdlog::DLLLoader::Instance().GetLogController();
}
#endif

// ��־��ǩ
struct LogTag
{
	explicit LogTag(const wchar_t* t): tag(t) {}
	const wchar_t* tag;
};

#ifndef LOGTAG_MODULE
#define LOGTAG_MODULE L""
#endif

#ifndef LOGTAG_FILE
#define LOGTAG_FILE L""
#endif

#define LOGTAG_EXTRA LOGTAG_MODULE LOGTAG_FILE

#define TAG_DEFAULT                LogTag(LOGTAG_EXTRA)
#define TAG(t)                     LogTag(t  L";" LOGTAG_EXTRA)
#define TAG2(t1, t2)               LogTag(t1 L";" t2 L";" LOGTAG_EXTRA)
#define TAG3(t1, t2, t3)           LogTag(t1 L";" t2 L";" t3 L";" LOGTAG_EXTRA)
#define TAG4(t1, t2, t3, t4)       LogTag(t1 L";" t2 L";" t3 L";" t4 L";" LOGTAG_EXTRA)

#define NOTAG TAG_DEFAULT

inline void LogV(LogLevel level, const wchar_t* tag, const wchar_t* fmt, va_list args) 
{
	ILogController* ctrl = GetLogController();
	if (!ctrl)
	{
		return;
	}
	if (!ctrl->NeedLog(level, tag))
	{
		return;
	}

	wchar_t buf[2048];
	_vsnwprintf_s(buf, _TRUNCATE, fmt, args);
	ctrl->Log(level, tag, buf);
}

#ifdef BDLOG_DISABLE_ALL
#define BDLOG_ARGV(l, t, f)
#else
#define BDLOG_ARGV(l, t, f) va_list args;va_start(args, f); LogV(l, t.tag, f, args); va_end(args);
#endif

#if (_MSC_VER >= 1500)
inline void Log(LogLevel level, LogTag tag, __in_z _Printf_format_string_ const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#else
#ifdef STATIC_CODE_ANALYSIS
// VS2005��֧��_Printf_format_string_,������CodeAnalysis����printf��ʽ��ʽ���Ĵ���(��������������Ͳ�ƥ���),
// �����ھ�̬�������ʱ��ʹ�ú����ʽ����wprintf��CodeAnalysis������
inline void LogF(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#define Log(level, tag, ...) LogF(level, tag, __VA_ARGS__); wprintf(__VA_ARGS__);
#else
inline void Log(LogLevel level, LogTag tag, __in_z const wchar_t* fmt, ...)
{
	BDLOG_ARGV(level, tag, fmt);
}
#endif // STATIC_CODE_ANALYSIS
#endif // _MSC_VER



#endif // BDCLIENT_LOG_H
