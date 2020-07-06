#pragma once

/*
解压缩辅助工具
*/

#ifndef __INTERFACE_ZIP_RESOURCE_MANAGER_H__
#define __INTERFACE_ZIP_RESOURCE_MANAGER_H__

namespace ZipHelper
{
	enum class ZipHelperRet;

	//interface __declspec(uuid("{6414CD6E-FFD8-4DA4-B2BE-54CC83CA43F3}"))
	DEFINE_INTERFACE(IZipResourceManager, "{6414CD6E-FFD8-4DA4-B2BE-54CC83CA43F3}")
		IZipResourceManager : public IUnknown
	{
		STDMETHOD(Init)() = 0;
		STDMETHOD(UnInit)() = 0;
		//加载zip的文件结构到内存
		STDMETHOD_(ZipHelperRet, PreloadZipFile)(LPCTSTR IN strZipPath) = 0;
		//读取文件buf，pBuff传空返回size，非空则判定size够不够，够了直接返回数据，bSaveCache指示是否将数据加载入缓存
		STDMETHOD_(ZipHelperRet, LoadFileBuf)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN bool bSaveCache, IN byte * pBuff, OUT unsigned long & nBufSize) = 0;
		STDMETHOD_(ZipHelperRet, QueryFileBufSize)(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, OUT unsigned long & nBufSize) = 0;
	};

	typedef ATL::CComPtr<IZipResourceManager> _ZipResourceManagerPtr;
};

#endif