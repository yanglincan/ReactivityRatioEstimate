#include "pch.h"
#include "ReactivityRatioEstimate.h"
#include "CParas.h"
#include "afxdialogex.h"

IMPLEMENT_DYNAMIC(CParas, CDialogEx)

CParas::CParas(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_PARA_DLG, pParent)
	, m_rA_LeftRange(0)
	, m_rA_RightRange(1)
	, m_rB_LeftRange(0)
	, m_rB_RightRange(1)
	, m_MinStepLengthRA(0)
	, m_MinStepLengthRB(0)
	, m_FilePathName(_T(""))
{
}

CParas::~CParas()
{
}


void CParas::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_RA_LEDIT, m_rA_LeftRange);
	DDX_Text(pDX, IDC_RA_REDIT, m_rA_RightRange);
	DDX_Text(pDX, IDC_RB_LEDIT, m_rB_LeftRange);
	DDX_Text(pDX, IDC_RB_REDIT, m_rB_RightRange);
	DDX_Text(pDX, IDC_MIN_EDIT_RA, m_MinStepLengthRA);
	DDX_Text(pDX, IDC_PATH_EDIT, m_FilePathName);
	DDX_Text(pDX, IDC_MIN_EDIT_RB, m_MinStepLengthRB);
}


BEGIN_MESSAGE_MAP(CParas, CDialogEx)
	ON_BN_CLICKED(IDC_FILE_BTN, &CParas::OnBnClickedFileBtn)
	ON_BN_CLICKED(IDOK, &CParas::OnBnClickedOk)
END_MESSAGE_MAP()


// CParas 消息处理程序
void CParas::OnBnClickedFileBtn()
{
	if (FALSE == UpdateData(TRUE))	//data is being retrieved (TRUE).
		return;
	CFileDialog fd(TRUE);
	if (IDCANCEL == fd.DoModal())
		return;
	m_FilePathName = fd.GetPathName();
	UpdateData(FALSE);	//dialog box is being initialized (FALSE)
}

void CParas::OnBnClickedOk()
{
	if (FALSE == UpdateData(TRUE))	//data is being retrieved (TRUE).
		return;
	if (m_rA_LeftRange >= m_rA_RightRange || m_rB_LeftRange >= m_rB_RightRange){
		MessageBox(_T("Wrong range!"), _T("💀💀💀"));
		return;
	}
	if (m_MinStepLengthRA < 0 || m_MinStepLengthRB < 0) {
		MessageBox(_T("Wrong step-length (🏃‍)!"), _T("💀💀💀"));
		return;
	}
	if (m_FilePathName.IsEmpty()) {
		MessageBox(_T("No file is open!"), _T("💀💀💀"));
		return;
	}
	CDialogEx::OnOK();
}

