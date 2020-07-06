// ZipHelper.cpp : 定义 DLL 应用程序的导出函数。
//
#include "stdafx.h"
#include "ZipHelper.h"
#include "ZipHelperImp.h"
#include "ZipResourceManagerImp.h"
using namespace ZipHelper;

HRESULT STDMETHODCALLTYPE CreateInterface(REFIID riid, void **ppvObject)
{
	ATLASSERT(ppvObject);
	if (!ppvObject) return E_INVALIDARG;

	HRESULT hRet = E_NOTIMPL;

	{
		REFIID id = __uuidof(IZipHelper);
		if (IsEqualGUID(riid, id))
		{
			_ZipHelperPtr pZipHelper = new(std::nothrow) CZipHelperImp();
			ATLASSERT(NULL != pZipHelper);
			if (NULL == pZipHelper)
			{
				hRet = E_OUTOFMEMORY;
			}
			else
			{
				hRet = pZipHelper->QueryInterface(riid, ppvObject);
			}
		}
	}

	{
		REFIID id = __uuidof(IZipResourceManager);
		if (IsEqualGUID(riid, id))
		{
			_ZipResourceManagerPtr pZipResourceManager = new(std::nothrow) ZipResourceManagerImp();
			ATLASSERT(NULL != pZipResourceManager);
			if (NULL == pZipResourceManager)
			{
				hRet = E_OUTOFMEMORY;
			}
			else
			{
				hRet = pZipResourceManager->QueryInterface(riid, ppvObject);
			}
		}
	}

	return hRet;
}

