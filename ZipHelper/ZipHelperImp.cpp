#include "stdafx.h"
#include "ZipHelperImp.h"
#include "minizip\unzip.h"
#include "minizip\zip.h"
#include <atlconv.h> 
#include <fstream>   
#include <codecvt>
#include <io.h>
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
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
		converter(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));
	std::string strZipPathA = converter.to_bytes(strZipPath);
	std::string strFileNameA = converter.to_bytes(strFileName);
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

// void EnumDirFiles(const std::string& dirPrefix, const std::string& dirName, vector<string>& vFiles)
// {
// 	if (dirPrefix.empty() || dirName.empty())
// 		return;
// 	std::string dirNameTmp = dirName;
// 	std::string dirPre = dirPrefix;
// 
// 	if (dirNameTmp.find_last_of("/") != dirNameTmp.length() - 1)
// 		dirNameTmp += "/";
// 	if (dirNameTmp[0] == '/')
// 		dirNameTmp = dirNameTmp.substr(1);
// 	if (dirPre.find_last_of("/") != dirPre.length() - 1)
// 		dirPre += "/";
// 
// 	std::string path;
// 
// 	path = dirPre + dirNameTmp;
// 
// 
// 	struct stat fileStat;
// 	DIR* pDir = opendir(path.c_str());
// 	if (!pDir) return;
// 
// 	struct dirent* pDirEnt = NULL;
// 	while ((pDirEnt = readdir(pDir)) != NULL)
// 	{
// 		if (strcmp(pDirEnt->d_name, ".") == 0 || strcmp(pDirEnt->d_name, "..") == 0)
// 			continue;
// 
// 		string tmpDir = dirPre + dirNameTmp + pDirEnt->d_name;
// 		if (stat(tmpDir.c_str(), &fileStat) != 0)
// 			continue;
// 
// 		string innerDir = dirNameTmp + pDirEnt->d_name;
// 		if (fileStat.st_mode & S_IFDIR == S_IFDIR)
// 		{
// 			EnumDirFiles(dirPrefix, innerDir, vFiles);
// 			continue;
// 		}
// 
// 		vFiles.push_back(innerDir);
// 	}
// 
// 	if (pDir)
// 		closedir(pDir);
// }



void AddFileToZip(zipFile zf, const char* fileNameInZip, const char* srcFile)
{
	FILE* srcfp = NULL;

	//初始化写入zip的文件信息  
	zip_fileinfo zi;
	zi.tmz_date.tm_sec = zi.tmz_date.tm_min = zi.tmz_date.tm_hour =
		zi.tmz_date.tm_mday = zi.tmz_date.tm_mon = zi.tmz_date.tm_year = 0;
	zi.dosDate = 0;
	zi.internal_fa = 0;
	zi.external_fa = 0;

	//如果srcFile为空，加入空目录  
	char new_file_name[MAX_PATH];
	memset(new_file_name, 0, sizeof(new_file_name));
	strcat(new_file_name, fileNameInZip);
	if (srcFile == NULL)
	{
		strcat(new_file_name, "/");
	}

	//在zip文件中创建新文件  
	zipOpenNewFileInZip(zf, new_file_name, &zi, NULL, 0, NULL, 0, NULL, Z_DEFLATED, Z_DEFAULT_COMPRESSION);

	if (srcFile != NULL)
	{
		//打开源文件  
		srcfp = fopen(srcFile, "rb");
		if (srcfp == NULL)
		{
			zipCloseFileInZip(zf); //关闭zip文件  
			return;
		}

		//读入源文件并写入zip文件  
		char buf[100 * 1024]; //buffer  
		int numBytes = 0;
		while (!feof(srcfp))
		{
			numBytes = fread(buf, 1, sizeof(buf), srcfp);
			zipWriteInFileInZip(zf, buf, numBytes);
			if (ferror(srcfp))
				break;
		}

		//关闭源文件  
		fclose(srcfp);
	}

	//关闭zip文件  
	zipCloseFileInZip(zf);
}

void CollectFilesInDirToZip(zipFile zf, const CString& strPath, const CString& parentDir)
{
	USES_CONVERSION; //for W2CA 
	CString strRelativePath;
	WIN32_FIND_DATA next;
	TCHAR pathname[MAX_PATH];
	swprintf_s(pathname, L"%s\\*.*", strPath);
	TCHAR buff[MAX_PATH];
	TCHAR buffA[MAX_PATH];
	HANDLE handle = FindFirstFile(pathname, &next);
	if (handle == INVALID_HANDLE_VALUE)
	{
		return;
	}

	while (1)
	{
		if (FindNextFile(handle, &next) == FALSE)
			return;
		else
		{
			if (next.cFileName[0] == '.')  continue;//用FindNextFile刚开始有.或者..,我们要避免这种情况
													/*memset(buff, 0, MAX_PATH);*/
			if (parentDir == _T(""))
			strRelativePath = next.cFileName;
			else
			strRelativePath = parentDir + _T("\\") + next.cFileName; //生成在zip文件中的相对路径  
			swprintf_s(buff, L"%s\\%s", strPath, next.cFileName);//文件的绝对路径
			if (next.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
				AddFileToZip(zf, CW2A(strRelativePath, CP_ACP), NULL);
				CollectFilesInDirToZip(zf, buff, strRelativePath); //递归收集子目录文件 
				continue;
			}
			AddFileToZip(zf, CW2A(strRelativePath, CP_ACP), W2CA(buff, CP_ACP)); //将文件添加到zip文件中
		}
	}

// 	BOOL bWorking = finder.FindFile(strPath + _T("//*.*"));
// 	while (bWorking)
// 	{
// 		bWorking = finder.FindNextFile();
// 		if (finder.IsDots())
// 			continue;
// 
// 		if (parentDir == _T(""))
// 			strRelativePath = finder.GetFileName();
// 		else
// 			strRelativePath = parentDir + _T("//") + finder.GetFileName(); //生成在zip文件中的相对路径  
// 
// 		if (finder.IsDirectory())
// 		{
// 			AddFileToZip(zf, W2CA(strRelativePath), NULL); //在zip文件中生成目录结构  
// 			CollectFilesInDirToZip(zf, finder.GetFilePath(), strRelativePath); //递归收集子目录文件  
// 			continue;
// 		}
// 
// 		AddFileToZip(zf, W2CA(strRelativePath), W2CA(finder.GetFilePath())); //将文件添加到zip文件中  
// 	}
}

STDMETHODIMP_(ZipHelperRet) ZipHelper::CZipHelperImp::CompressedFile(LPCTSTR IN strZipPath, LPCTSTR IN strFileName)
{
	std::wstring_convert<std::codecvt<wchar_t, char, std::mbstate_t>>
		converter(new std::codecvt<wchar_t, char, std::mbstate_t>("CHS"));
	std::string strZipPathA = converter.to_bytes(strZipPath);
	std::string strFileNameA = converter.to_bytes(strFileName);
	USES_CONVERSION;
	zipFile newZipFile = zipOpen(strZipPathA.c_str(), APPEND_STATUS_CREATE); //创建zip文件 
	if (newZipFile == NULL)
	{
		return ZipHelperRet::BuildZipFailed;
	}

	CollectFilesInDirToZip(newZipFile, strFileName, _T(""));

	zipClose(newZipFile, NULL); //关闭zip文件  

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
