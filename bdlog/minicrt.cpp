#include "stdafx.h"
#include "minicrt.h"
#include "logutil.h"

#ifndef BDLOG_BUILD_STATIC_LIB

#define WIDE_STR_(x) L##x
#define WIDE_STR(x) WIDE_STR_(x)

// ʹ��symview�������Ƿ��С����ñ����ӡ������ķ��ų��֣����У�˵����С����������Ӧ�ĺ�����Ӧ�ü�鲢�����
#define SHOULD_NOT_LINK(reason)  LOG(L"���ñ�����(" reason L"): " WIDE_STR(__FUNCTION__))


/// ȱʡʵ�ֵ�new/delete�Ƚϸ��ӣ�
/// bdlog��û�м���new/delete���ò�����ô���ӵĹ��ܣ�ֱ��ʹ��HeapAlloc/HeapFree����
void * operator new(unsigned int size)
{
	return ::HeapAlloc(::GetProcessHeap(), 0, size);
}
void * operator new (unsigned int /*size*/, void * p)
{ 
	return p ; 
}
void operator delete(void* p)
{
	::HeapFree(::GetProcessHeap(), 0, p);
}

int _purecall()
{
	return 0;
}


/// ȱʡ��wcschr�Ƚϸ���(ռ��170����ֽڣ���wcschr.c)
/// bdlog�еĳ���������һ���Ƚ϶̵��ַ����н��в��ң��Լ�дһ��wcschr�͹���
const wchar_t* wcschr(const wchar_t* src, wchar_t ch)
{
	while (*src)
	{
		if (*src == ch) return src;
		src++;
	}
	return NULL;
}


/// �����ǲ�Ӧ�ñ����ӽ����ĺ���
#ifndef _DEBUG
int _itow_s(int /*val*/, wchar_t* /*buf*/, size_t /*buflen*/, int /*redix*/)
{
	SHOULD_NOT_LINK(L"ȱʡʵ��̫��");
	return 0;
}

int wcsncpy_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"ȱʡʵ��̫��");
	return 0;
}

// error LNK2005: __wtoi already defined in LIBCMT.lib(wtox.obj)
int wcsncat_s(wchar_t* /*dest*/, size_t /*destlen*/, const wchar_t* /*src*/, size_t /*copylen*/)
{
	SHOULD_NOT_LINK(L"ȱʡʵ��̫��");
	return 0;
}

int _wtoi(const wchar_t* /*str*/)
{
	SHOULD_NOT_LINK(L"ȱʡʵ��̫��");
	return 0;
}

int atexit ( void ( * /*func*/ ) (void) )
{
	SHOULD_NOT_LINK(L"û�����п�ĳ�ʼ������֧��atexit");
	return 0;
}


#endif

#endif