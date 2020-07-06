// stdafx.h : 标准系统包含文件的包含文件，
// 或是经常使用但不常更改的
// 特定于项目的包含文件
//

#pragma once

#pragma warning(disable:4819)			//warning C4819: 该文件包含不能在当前代码页(936)中表示的字符。请将该文件保存为 Unicode 格式以防止数据丢失
#pragma warning(disable:4099)			//warning LNK4099: 未找到正在链接对象的pdb，如同没有调试信息一样

#include "targetver.h"

#define _ATL_CUSTOM_THROW
#define AtlThrow

#define _WTL_NO_CSTRING //屏蔽WTL中的wstring，使用ATL的wstring
#define _CSTRING_NS ATL


#define WIN32_LEAN_AND_MEAN             // 从 Windows 头中排除极少使用的资料
// Windows 头文件: 
#include <windows.h>
#include <atlbase.h>
#include <atlstr.h>
#include <atlsync.h>
#include <atlenc.h>
#include <atltime.h>
#include <atlutil.h>
#include <vector>
#include <map>
#include <base/base.h>
#include <base/defs/UnknownImpl.hpp>
#include <json/value.h>
#include <json/json.h>


#ifdef _DEBUG
#pragma comment(lib,"Zlib/1.2.8/debug/zlibstat.lib")
#pragma comment(lib, "base/windows/debug/LxBase.lib")
#pragma comment(lib,"json/1.7.7/debug/jsoncpp.lib")
#else
#ifdef WIN32
#pragma comment(lib,"Zlib/1.2.8/release/x86/zlibstat.lib")
#pragma comment(lib, "base/windows/release/LxBase.lib")
#pragma comment(lib,"json/1.7.7/release/jsoncpp.lib")
#endif
#endif


// TODO:  在此处引用程序需要的其他头文件
#include <Log/Logger.h>