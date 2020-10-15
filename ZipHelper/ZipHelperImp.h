#pragma once
#include "ZipHelper/IZipHelper.h"

namespace ZipHelper
{
	class CZipHelperImp : public IZipHelper
	{
	public:
		CZipHelperImp();
		~CZipHelperImp();
		BEGIN_INTERFACE_360MAP(CZipHelperImp, IZipHelper)
		END_INTERFACE_360MAP()
	protected:
		unsigned long m_nKey;
		std::string m_strLastInfo;
	public:
		STDMETHOD(Init)();
		STDMETHOD(UnInit)();
		STDMETHOD_(ZipHelperRet, LoadFileBuff)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN byte * pBuff, OUT unsigned long & nBufSize);
		STDMETHOD_(ZipHelperRet, LoadFileList)(LPCTSTR IN strZipPath, IN char * pJson, OUT unsigned long & nJsonSize, OUT unsigned long & nKey);
		STDMETHOD_(ZipHelperRet, CompressedFile)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName);
	};


};