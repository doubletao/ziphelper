
// testZipHelperDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "testZipHelper.h"
#include "testZipHelperDlg.h"
#include "afxdialogex.h"
#include <string>
#include <ZipHelper/ZipHelperTool.hpp>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CtestZipHelperDlg 对话框



CtestZipHelperDlg::CtestZipHelperDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_TESTZIPHELPER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CtestZipHelperDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CtestZipHelperDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
END_MESSAGE_MAP()


// CtestZipHelperDlg 消息处理程序

BOOL CtestZipHelperDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	InitDll();
	if (m_pZipHelper)
	{
		m_pZipHelper->Init();
		{
			unsigned long nBufSize = 0;
			ZipHelper::ZipHelperRet ret = m_pZipHelper->LoadFileBuff(L"D:\\lanxin_code\\ZipHelper\\Debug\\debug.zip", L"debug/themes/LanXin/Lanxin_theme/emoji/1f1ec-1f1eb.png", NULL, nBufSize);
			if (ZipHelper::ZipHelperRet::Success == ret)
			{
				byte * pBuffer = new byte[nBufSize];
				memset(pBuffer, 0, nBufSize);
				ret = m_pZipHelper->LoadFileBuff(L"D:\\lanxin_code\\ZipHelper\\Debug\\debug.zip", L"debug/themes/LanXin/Lanxin_theme/emoji/1f1ec-1f1eb.png", pBuffer, nBufSize);
				if (ZipHelper::ZipHelperRet::Success == ret)
				{
					CFile file;
					if (file.Open(L"test.png", CFile::modeCreate | CFile::modeWrite))
					{
						file.Write(pBuffer, nBufSize);
						file.Close();
					}
				}

				delete pBuffer;
			}
		}
		m_pZipHelper->UnInit();
	}

	if (m_pZipResourceManager)
	{
		m_pZipResourceManager->Init();
		{
			unsigned long nBufSize = 0;
			unsigned long nKey = 0;
			ZipHelper::ZipHelperRet ret = m_pZipResourceManager->PreloadZipFile(L"D:\\lanxin_code\\ZipHelper\\Debug\\debug.zip");
			if (ZipHelper::ZipHelperRet::Success == ret)
			{
				ZipHelper::ZipHelperRet ret = m_pZipResourceManager->QueryFileBufSize(L"D:\\lanxin_code\\ZipHelper\\Debug\\debug.zip", L"debug/themes/LanXin/Lanxin_theme/emoji/1f1ec-1f1eb.png", nBufSize);
				if (ZipHelper::ZipHelperRet::Success == ret)
				{
					byte * pBuffer = new byte[nBufSize];
					memset(pBuffer, 0, nBufSize);
					ret = m_pZipHelper->LoadFileBuff(L"D:\\lanxin_code\\ZipHelper\\Debug\\debug.zip", L"debug/themes/LanXin/Lanxin_theme/emoji/1f1ec-1f1eb.png", pBuffer, nBufSize);
					if (ZipHelper::ZipHelperRet::Success == ret)
					{
						CFile file;
						if (file.Open(L"test01.png", CFile::modeCreate | CFile::modeWrite))
						{
							file.Write(pBuffer, nBufSize);
							file.Close();
						}
					}
					delete pBuffer;
				}
			}
		}
		m_pZipResourceManager->UnInit();
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CtestZipHelperDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CtestZipHelperDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CtestZipHelperDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CtestZipHelperDlg::InitDll()
{
	std::wstring strDllPath = GetExePath() + _T("\\ZipHelper.dll");
	HMODULE handle = LoadLibrary(strDllPath.c_str());
	//DYHANDLE handle = base::dyLoad(CW2A(strDllPath.c_str(), CP_UTF8), 1);
	if (!handle) {
		MessageBox(L"load ZipHelper.dll failure.");
		return;
	}

	typedef HRESULT(STDMETHODCALLTYPE *fnCreateInterface)(REFIID riid, void **ppvObject);

	{
		bool result = false;
		m_pZipHelper = NULL;
		auto uuid = __uuidof(ZipHelper::IZipHelper);
		if (S_OK == base::callSym<HRESULT __stdcall(REFIID, void**)>(handle, "CreateInterface", uuid, (void**)&m_pZipHelper)) {
			if (!m_pZipHelper) {
				MessageBox(L"init IZipHelper failure ");
			}
		}
	}

	{
		bool result = false;
		m_pZipResourceManager = NULL;
		auto uuid = __uuidof(ZipHelper::IZipResourceManager);
		if (S_OK == base::callSym<HRESULT __stdcall(REFIID, void**)>(handle, "CreateInterface", uuid, (void**)&m_pZipResourceManager)) {
			if (!m_pZipHelper) {
				MessageBox(L"init IZipResourceManager failure ");
			}
		}
	}
}

std::wstring CtestZipHelperDlg::GetExePath()
{
	static TCHAR szInstPath[1024] = { 0 };
	if (szInstPath[0] == _T('\0'))
	{
		::GetModuleFileName(AfxGetApp()->m_hInstance, szInstPath, 1024);
		::PathRemoveFileSpec(szInstPath);
	}
	return szInstPath;
}

