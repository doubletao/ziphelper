#pragma once

/*
解压缩辅助工具
*/

#ifndef __INTERFACE_ZIP_HELPER_H__
#define __INTERFACE_ZIP_HELPER_H__
#pragma once

namespace ZipHelper
{
	//返回值
	enum class ZipHelperRet
	{
		Success, //执行成功
		NotFound,
		InfoGetFailed,
		OpenFailed,
		ReadFailed,
		NotEnoughBufSize, //bufsize不足
	};

	//interface __declspec(uuid("{E327EE17-6D96-4F96-8323-B59C0C886947}"))
	DEFINE_INTERFACE(IZipHelper, "{E327EE17-6D96-4F96-8323-B59C0C886947}")
		IZipHelper : public IUnknown
	{
		STDMETHOD(Init)() = 0;
		STDMETHOD(UnInit)() = 0;
		STDMETHOD_(ZipHelperRet, LoadFileBuff)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN byte * pBuff, OUT unsigned long & nBufSize) = 0;
		STDMETHOD_(ZipHelperRet, LoadFileList)(LPCTSTR IN strZipPath, IN char * pJson, OUT unsigned long & nJsonSize, OUT unsigned long & nKey) = 0;
	};

	typedef ATL::CComPtr<IZipHelper> _ZipHelperPtr;
};

#endif
