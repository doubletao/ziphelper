#include "stdafx.h"
#include "ZipHelperImp.h"
#include "minizip\unzip.h"
using namespace ZipHelper;

CZipHelperImp::CZipHelperImp()
	: m_nKey(0)
{
}


CZipHelperImp::~CZipHelperImp()
{
}

int do_list(unzFile uf);

STDMETHODIMP CZipHelperImp::Init()
{
	return S_OK;
}

STDMETHODIMP CZipHelperImp::UnInit()
{
	return S_OK;
}

STDMETHODIMP_(ZipHelperRet) CZipHelperImp::LoadFileBuff(LPCTSTR IN strZipPath, LPCTSTR IN strFileName, IN byte * pBuff, OUT unsigned long & nBufSize)
{
	std::string strZipPathA = CW2A(strZipPath, CP_UTF8);
	std::string strFileNameA = CW2A(strFileName, CP_UTF8);
	//打开压缩文件
	unzFile uf = unzOpen64(strZipPathA.c_str());
	if (uf)
	{
		//定位到指定文件
		int nErr = unzLocateFile(uf, strFileNameA.c_str(), 0);
		if (UNZ_OK != nErr)
		{
			ErrorLog("CZipHelperImp::LoadFileBuff:unzLocateFile ", strFileNameA.c_str(), " in ", strZipPathA.c_str(), "[", nErr, "]");
			return ZipHelperRet::NotFound;
		}

		unz_file_info file_info;

		char szFileName[512] = {0};
		strcpy(szFileName, strFileNameA.c_str());
		//获取当前选择的内部压缩文件的信息
		nErr = unzGetCurrentFileInfo(uf, &file_info, szFileName, sizeof(szFileName), NULL, 0, NULL, 0);
		if (UNZ_OK != nErr)
		{
			ErrorLog("CZipHelperImp::LoadFileBuff:unzGetCurrentFileInfo ", strFileNameA.c_str(), " in ", strZipPathA.c_str(), "[", nErr, "]");
			return ZipHelperRet::InfoGetFailed;
		}

		//取到size
		unsigned long len = file_info.uncompressed_size;

		//分两种情况，如果pbuf是null，则返回size回去，否则读取内容并返回size
		if (NULL == pBuff)
		{
			nBufSize = len;
			//关闭压缩文件
			unzClose(uf);
			return ZipHelperRet::Success;
		}
		else
		{
			//选择打开定位到的文件
			nErr = unzOpenCurrentFile(uf);
			if (nErr != UNZ_OK)
			{
				ErrorLog("CZipHelperImp::LoadFileBuff:unzOpenCurrentFile ", strFileNameA.c_str(), " in ", strZipPathA.c_str(), "[", nErr, "]");
				//关闭压缩文件
				unzClose(uf);
				return ZipHelperRet::OpenFailed;
			}
			//读取内容
			if (len > nBufSize)
			{
				//关闭压缩文件
				unzClose(uf);
				//返回真正需要的bufsize
				nBufSize = len;
				return ZipHelperRet::NotEnoughBufSize;
			}
			else
			{
				nErr = unzReadCurrentFile(uf, pBuff, file_info.uncompressed_size);
				if (nErr < 0)
				{
					ErrorLog("CZipHelperImp::LoadFileBuff:unzReadCurrentFile ", strFileNameA.c_str(), " in ", strZipPathA.c_str(), "[", nErr, "]");
					//关闭压缩文件
					unzClose(uf);
					return ZipHelperRet::ReadFailed;
				}
				else
				{
					nBufSize = len;
					//关闭压缩文件
					unzClose(uf);
					return ZipHelperRet::Success;
				}
			}
		}
		unzClose(uf);
	}
	else
	{
		ErrorLog("CZipHelperImp::LoadFileBuff:open ", strZipPathA.c_str(), " failed");
		return ZipHelperRet::OpenFailed;
	}
	return ZipHelperRet::Success;
}

STDMETHODIMP_(ZipHelperRet) CZipHelperImp::LoadFileList(LPCTSTR IN strZipPath, IN char * pJson, OUT unsigned long & nJsonSize, OUT unsigned long & nKey)
{
	//如果是上一次缓存的key，直接返回
	if (m_nKey > 0 && m_nKey == nKey)
	{
		//传的是非空，则检测buf是否够大
		if (m_strLastInfo.size() <= nJsonSize)
		{
			m_nKey++;
			nKey = m_nKey;
			nJsonSize = m_strLastInfo.size();
			memcpy(pJson, m_strLastInfo.c_str(), nJsonSize);
			return ZipHelperRet::Success;
		}
		else
		{
			return ZipHelperRet::NotEnoughBufSize;
		}

		return ZipHelperRet::Success;
	}

	std::string strZipPathA = CW2A(strZipPath, CP_UTF8);
	//打开压缩文件
	unzFile uf = unzOpen64(strZipPathA.c_str());
	if (uf)
	{
		unz_global_info64 gi;
		int nErr;
		//拿出文件总数
		nErr = unzGetGlobalInfo64(uf, &gi);
		if (nErr != UNZ_OK)
		{
			ErrorLog("CZipHelperImp::LoadFileList:unzGetGlobalInfo64 ", " in ", strZipPathA.c_str(), "[", nErr, "]");
			unzClose(uf);
			return ZipHelperRet::InfoGetFailed;
		}

		Json::Value rootNode;
		for (ZPOS64_T i = 0; i < gi.number_entry; i++)
		{
			char filename_inzip[256];
			unz_file_info64 file_info;
			uLong ratio = 0;
			char charCrypt = ' ';
			//读当前文件信息
			nErr = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
			if (nErr != UNZ_OK)
			{
				ErrorLog("CZipHelperImp::LoadFileList:unzGetCurrentFileInfo64 ", " in ", strZipPathA.c_str(), "[", nErr, "]");
				break;
			}

			//信息记入json
			Json::Value fileInfoNode;
			fileInfoNode["n"] = filename_inzip;
			fileInfoNode["s"] = file_info.uncompressed_size;
			rootNode.append(fileInfoNode);
			
			//如果没结束，则继续移向下一个文件
			if ((i + 1) < gi.number_entry)
			{
				nErr = unzGoToNextFile(uf);
				if (nErr != UNZ_OK)
				{
					ErrorLog("CZipHelperImp::LoadFileList:unzGoToNextFile ", " in ", strZipPathA.c_str(), "[", nErr, "]");
					break;
				}
			}
		}
		if (rootNode.size() > 0)
		{
			if (NULL == pJson)
			{
				//传空进来，则暂存起来，然后返回key和size
				Json::FastWriter writer;
				m_strLastInfo = writer.write(rootNode);
				m_nKey++;
				nKey = m_nKey;
				nJsonSize = m_strLastInfo.size();
				unzClose(uf);
				return ZipHelperRet::Success;
			}
			else
			{
				//传的是非空，则检测buf是否够大
				if (m_strLastInfo.size() <= nJsonSize)
				{
					Json::FastWriter writer;
					m_strLastInfo = writer.write(rootNode);
					m_nKey++;
					nKey = m_nKey;
					nJsonSize = m_strLastInfo.size();
					memcpy(pJson, m_strLastInfo.c_str(), nJsonSize);
					unzClose(uf);
					return ZipHelperRet::Success;
				}
				else
				{
					unzClose(uf);
					return ZipHelperRet::NotEnoughBufSize;
				}
			}
		}
		else
		{
			ErrorLog("CZipHelperImp::LoadFileList:file empty ", " in ", strZipPathA.c_str(), "[", nErr, "]");
			unzClose(uf);
			return ZipHelperRet::NotFound;
		}
		unzClose(uf);
	}
	else
	{
		ErrorLog("CZipHelperImp::LoadFileBuff:open ", strZipPathA.c_str(), " failed");
		return ZipHelperRet::OpenFailed;
	}
	return ZipHelperRet::Success;
}

void OutputDebugFormat(const char * lpszFormat, ...)
{
	char pszDest[1024] = {0};
	va_list argList;
	va_start(argList, lpszFormat); //一定要“...”之前的那个参数
	_vsnprintf(pszDest, 1024, lpszFormat, argList);
	OutputDebugStringA(pszDest);
	va_end(argList);
}

void Display64BitsSize(ZPOS64_T n, int size_char)
{
	/* to avoid compatibility problem , we do here the conversion */
	char number[21];
	int offset = 19;
	int pos_string = 19;
	number[20] = 0;
	for (;;) {
		number[offset] = (char)((n % 10) + '0');
		if (number[offset] != '0')
			pos_string = offset;
		n /= 10;
		if (offset == 0)
			break;
		offset--;
	}
	{
		int size_display_string = 19 - pos_string;
		while (size_char > size_display_string)
		{
			size_char--;
			OutputDebugFormat(" ");
		}
	}

	OutputDebugFormat("%s", &number[pos_string]);
}

int do_list(unzFile uf)
{
	uLong i;
	unz_global_info64 gi;
	int err;

	err = unzGetGlobalInfo64(uf, &gi);
	if (err != UNZ_OK)
		OutputDebugFormat("error %d with zipfile in unzGetGlobalInfo \n", err);
	OutputDebugFormat("  Length  Method     Size Ratio   Date    Time   CRC-32     Name\n");
	OutputDebugFormat("  ------  ------     ---- -----   ----    ----   ------     ----\n");
	for (i = 0; i<gi.number_entry; i++)
	{
		char filename_inzip[256];
		unz_file_info64 file_info;
		uLong ratio = 0;
		const char *string_method;
		char charCrypt = ' ';
		err = unzGetCurrentFileInfo64(uf, &file_info, filename_inzip, sizeof(filename_inzip), NULL, 0, NULL, 0);
		if (err != UNZ_OK)
		{
			OutputDebugFormat("error %d with zipfile in unzGetCurrentFileInfo\n", err);
			break;
		}
		if (file_info.uncompressed_size>0)
			ratio = (uLong)((file_info.compressed_size * 100) / file_info.uncompressed_size);

		/* display a '*' if the file is crypted */
		if ((file_info.flag & 1) != 0)
			charCrypt = '*';

		if (file_info.compression_method == 0)
			string_method = "Stored";
		else
			if (file_info.compression_method == Z_DEFLATED)
			{
				uInt iLevel = (uInt)((file_info.flag & 0x6) / 2);
				if (iLevel == 0)
					string_method = "Defl:N";
				else if (iLevel == 1)
					string_method = "Defl:X";
				else if ((iLevel == 2) || (iLevel == 3))
					string_method = "Defl:F"; /* 2:fast , 3 : extra fast*/
			}
			else
				if (file_info.compression_method == Z_BZIP2ED)
				{
					string_method = "BZip2 ";
				}
				else
					string_method = "Unkn. ";

		Display64BitsSize(file_info.uncompressed_size, 7);
		OutputDebugFormat("  %6s%c", string_method, charCrypt);
		Display64BitsSize(file_info.compressed_size, 7);
		OutputDebugFormat(" %3lu%%  %2.2lu-%2.2lu-%2.2lu  %2.2lu:%2.2lu  %8.8lx   %s\n",
			ratio,
			(uLong)file_info.tmu_date.tm_mon + 1,
			(uLong)file_info.tmu_date.tm_mday,
			(uLong)file_info.tmu_date.tm_year % 100,
			(uLong)file_info.tmu_date.tm_hour, (uLong)file_info.tmu_date.tm_min,
			(uLong)file_info.crc, filename_inzip);
		if ((i + 1)<gi.number_entry)
		{
			err = unzGoToNextFile(uf);
			if (err != UNZ_OK)
			{
				OutputDebugFormat("error %d with zipfile in unzGoToNextFile\n", err);
				break;
			}
		}
	}

	return 0;
}
