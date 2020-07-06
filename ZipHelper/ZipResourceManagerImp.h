#pragma once
#include <ZipHelper\IZipResourceManager.h>
#include <ZipHelper\ZipHelperTool.hpp>
#include "ZipHelperImp.h"

using namespace ZipHelper;

class ZipResourceManagerImp: public IZipResourceManager
{
private:
	CZipHelperImp m_zipHelper;
	std::map<std::string, std::map<std::string, FileBufInfo>> m_mapZipFileBufCache;
public:
	ZipResourceManagerImp();
	~ZipResourceManagerImp();
	BEGIN_INTERFACE_360MAP(ZipResourceManagerImp, IZipResourceManager)
	END_INTERFACE_360MAP()
protected:
	FileBufInfo QueryCache(const std::string & strZipPath, const std::string & strFileName);
public:
	STDMETHOD(Init)();
	STDMETHOD(UnInit)();
	STDMETHOD_(ZipHelperRet, PreloadZipFile)(LPCTSTR IN strZipPath);
	STDMETHOD_(ZipHelperRet, LoadFileBuf)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN bool bSaveCache, IN byte * pBuff, OUT unsigned long & nBufSize);
	STDMETHOD_(ZipHelperRet, QueryFileBufSize)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, OUT unsigned long & nBufSize);
};

