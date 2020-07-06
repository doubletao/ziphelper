
// testZipHelperDlg.h : 头文件
//

#pragma once
#include <base/base.h>
#include <ZipHelper\IZipHelper.h>
#include <ZipHelper\IZipResourceManager.h>

// CtestZipHelperDlg 对话框
class CtestZipHelperDlg : public CDialogEx
{
// 构造
public:
	CtestZipHelperDlg(CWnd* pParent = NULL);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_TESTZIPHELPER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

protected:
	void InitDll();
	std::wstring GetExePath();
private:
	ZipHelper::_ZipHelperPtr m_pZipHelper;
	ZipHelper::_ZipResourceManagerPtr m_pZipResourceManager;
};
