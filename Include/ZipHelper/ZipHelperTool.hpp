#pragma once
#include <string>
#include <map>
#include <json/value.h>
#include <json/json.h>

#ifndef __INTERFACE_ZIP_HELPER_TOOL_H__
#define __INTERFACE_ZIP_HELPER_TOOL_H__

namespace ZipHelper
{
	struct FileBufInfo
	{
		std::string strFileName;
		unsigned long nBufSize;
		unsigned char * pBuf;
		FileBufInfo()
		{
			Reset();
		}
		void Reset()
		{
			strFileName.clear();
			nBufSize = 0;
			pBuf = NULL;
		}
		void Release()
		{
			strFileName.clear();
			nBufSize = 0;
			if (pBuf)
			{
				delete pBuf;
			}
			pBuf = NULL;
		}
		bool CopyBuf(unsigned char * pBufSrc, unsigned long nBufSizeSrc)
		{
			Release();
			if (pBufSrc && nBufSizeSrc > 0)
			{
				pBuf = new(std::nothrow) unsigned char[nBufSizeSrc];
				if (pBuf)
				{
					nBufSize = nBufSizeSrc;
					memcpy(pBuf, pBufSrc, nBufSizeSrc);
					return true;
				}
			}
			return false;
		}
	};

	class ZipHelperTool
	{
	public:
		//将ziphelper读出来的文件列表信息json解析成结构体map
		static std::map<std::string, FileBufInfo> DeserializeJsonToMapStruct(const std::string & strJson)
		{
			std::map<std::string, FileBufInfo> mapFileInfo;
			if (!strJson.empty())
			{
				Json::Value root;
				Json::Reader reader;
				if (reader.parse(strJson.c_str(), root) && root.isArray())
				{
					for (auto it : root)
					{
						FileBufInfo stuFileBufInfo;
						const Json::Value & nameNode = it["n"];
						if (nameNode.isString())
						{
							stuFileBufInfo.strFileName = nameNode.asString();
						}
						const Json::Value & sizeNode = it["s"];
						if (sizeNode.isNumeric())
						{
							stuFileBufInfo.nBufSize = (unsigned long)sizeNode.asUInt64();
						}
						mapFileInfo.insert(std::make_pair(stuFileBufInfo.strFileName, stuFileBufInfo));
					}
				}
			}
			return mapFileInfo;
		}
	};
};

#endif