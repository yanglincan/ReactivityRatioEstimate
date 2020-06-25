#pragma once
class CParas : public CDialogEx
{
	DECLARE_DYNAMIC(CParas)
public:
	CParas(CWnd* pParent = nullptr);   // 标准构造函数
	virtual ~CParas();
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_PARA_DLG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持
	DECLARE_MESSAGE_MAP()
public:
	double m_rA_LeftRange;	//rA 范围的左边界
	double m_rA_RightRange;	//rA 范围的右边界
	double m_rB_LeftRange;	//rB 范围的左边界
	double m_rB_RightRange;	//rB 范围的右边界
	double m_MinStepLengthRA;	//rA最小步长
	double m_MinStepLengthRB;	//rB最小步长
	CString m_FilePathName;	//输入文件的名称，用于确定输出文件的名称
public:
	afx_msg void OnBnClickedFileBtn();
	afx_msg void OnBnClickedOk();
};
