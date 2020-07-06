#include "stdafx.h"
#include "ZipResourceManagerImp.h"
using namespace ZipHelper;

ZipResourceManagerImp::ZipResourceManagerImp()
{
}


ZipResourceManagerImp::~ZipResourceManagerImp()
{
	for (auto itZip : m_mapZipFileBufCache)
	{
		for (auto itFile : itZip.second)
		{
			itFile.second.Release();
		}
	}
}

STDMETHODIMP ZipResourceManagerImp::Init()
{
	return S_OK;
}

STDMETHODIMP ZipResourceManagerImp::UnInit()
{
	return S_OK;
}

STDMETHODIMP_(ZipHelperRet) ZipResourceManagerImp::PreloadZipFile(LPCTSTR IN strZipPath)
{
	std::string strZipPathA = CW2A(strZipPath);
	unsigned long nBufSize = 0;
	unsigned long nKey = 0;
	ZipHelper::ZipHelperRet ret = m_zipHelper.LoadFileList(strZipPath, NULL, nBufSize, nKey);
	if (ZipHelper::ZipHelperRet::Success == ret)
	{
		char * pBuffer = new(std::nothrow) char[nBufSize];
		if (pBuffer)
		{
			memset(pBuffer, 0, nBufSize);
			ret = m_zipHelper.LoadFileList(strZipPath, pBuffer, nBufSize, nKey);
			if (ZipHelper::ZipHelperRet::Success == ret)
			{
				std::map<std::string, FileBufInfo> mapFileInfo = ZipHelperTool::DeserializeJsonToMapStruct(pBuffer);
				m_mapZipFileBufCache[strZipPathA] = mapFileInfo;
			}
			delete pBuffer;
			return ret;
		}
		else
		{
			return ret;
		}
	}
	else
	{
		return ret;
	}
	return ZipHelperRet::Success;
}

STDMETHODIMP_(ZipHelperRet) ZipResourceManagerImp::LoadFileBuf(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN bool bSaveCache, IN byte * pBuff, OUT unsigned long & nBufSize)
{
	std::string strZipPathA = CW2A(strZipPath, CP_UTF8);
	std::string strFileNameA = CW2A(strFileName, CP_UTF8);
	//先查找有没有，有的话直接读
	FileBufInfo stuFileBufInfo = QueryCache(strZipPathA, strFileNameA);
	if (stuFileBufInfo.pBuf && stuFileBufInfo.nBufSize > 0)
	{
		if (stuFileBufInfo.nBufSize > nBufSize)
		{
			//返回真正需要的bufsize
			nBufSize = stuFileBufInfo.nBufSize;
			return ZipHelperRet::NotEnoughBufSize;
		}
		else
		{
			memcpy(pBuff, stuFileBufInfo.pBuf, stuFileBufInfo.nBufSize);
			nBufSize = stuFileBufInfo.nBufSize;
			return ZipHelperRet::Success;
		}
	}
	else
	{
		ZipHelperRet ret = m_zipHelper.LoadFileBuff(strZipPath, strFileName, pBuff, nBufSize);
		//如果需要cache，则创建并保存cache
		if (ZipHelperRet::Success == ret && bSaveCache && pBuff && nBufSize > 0)
		{
			stuFileBufInfo.strFileName = strFileNameA;
			stuFileBufInfo.CopyBuf(pBuff, nBufSize);
		}
		return ret;
	}
	return ZipHelperRet::Success;
}

STDMETHODIMP_(ZipHelperRet) ZipResourceManagerImp::QueryFileBufSize(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, OUT unsigned long & nBufSize)
{
	std::string strZipPathA = CW2A(strZipPath, CP_UTF8);
	std::string strFileNameA = CW2A(strFileName, CP_UTF8);
	//先查找有没有，有的话直接读
	FileBufInfo stuFileBufInfo = QueryCache(strZipPathA, strFileNameA);
	if (stuFileBufInfo.nBufSize > 0)
	{
		nBufSize = stuFileBufInfo.nBufSize;
		return ZipHelperRet::Success;
	}
	else
	{
		//没有则拿个空指针查询一下
		return m_zipHelper.LoadFileBuff(strZipPath, strFileName, NULL, nBufSize);
	}
	return ZipHelperRet::Success;
}

FileBufInfo ZipResourceManagerImp::QueryCache(const std::string & strZipPath, const std::string & strFileName)
{
	FileBufInfo stuFileBufInfo;
	auto itZipFile = m_mapZipFileBufCache.find(strZipPath);
	if (itZipFile != m_mapZipFileBufCache.end())
	{
		auto itFile = itZipFile->second.find(strFileName);
		if (itFile != itZipFile->second.end())
		{
			return itFile->second;
		}
	}
	return stuFileBufInfo;
}