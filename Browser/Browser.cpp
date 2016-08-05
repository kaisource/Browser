#include "stdafx.h"
#include "Browser.h"
#include "BrowserUI.h"
#include "MiniDumper.h"

CMiniDumper g_miniDumper(true);
int CBrowserDlg::m_sCount=0;

int APIENTRY _tWinMain(HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPTSTR    lpCmdLine,
	int       nCmdShow)
{
	HRESULT Hr = ::CoInitialize(NULL);
	if(FAILED(Hr)) return 0;

	CPaintManagerUI::SetInstance(hInstance);
	CPaintManagerUI::SetResourceType(UILIB_ZIPRESOURCE);
	CPaintManagerUI::SetResourcePath(CPaintManagerUI::GetInstancePath());
	CPaintManagerUI::SetResourceZip(MAKEINTRESOURCE(IDR_ZIPRES), _T("ZIPRES"));

	CefMainArgs main_args(hInstance);
	CefRefPtr<CefApp> app;
	CefRefPtr<CefCommandLine> command_line = CefCommandLine::CreateCommandLine();
	command_line->InitFromString(::GetCommandLineW());
	const std::string& process_type = command_line->GetSwitchValue("type");
	if (process_type == "renderer"){
		app = new CRendererApp();
	}else{
		app = new CBrowserApp();
	}
	int exit_code = CefExecuteProcess(main_args, app.get(), NULL);
	if (exit_code >= 0) {
		// The sub-process has completed so return here.
		return exit_code;
	}

	CefSettings settings;// Specify CEF global settings here.
	settings.multi_threaded_message_loop=true;	//ʹ����������Ϣѭ��
	settings.ignore_certificate_errors = true;	//���Ե�ssl֤����֤����
    //settings.command_line_args_disabled = true;
	settings.no_sandbox = true;
	CefString(&settings.locale).FromASCII("zh-CN");

	// Initialize CEF.
	CefInitialize(main_args, settings, app.get(), NULL);

	CBrowserDlg* pFrame = new CBrowserDlg();
	if(pFrame == NULL) return 0;
	pFrame->Create(NULL,_T("Browser"),UI_WNDSTYLE_FRAME,WS_EX_APPWINDOW,0,0,800,600);
	pFrame->CenterWindow();

	CPaintManagerUI::MessageLoop();
	CPaintManagerUI::Term();

	// Shut down CEF.
	CefShutdown();

	::CoUninitialize();
	return 0;
}

CBrowserDlg::CBrowserDlg(BOOL bPopup,LPCTSTR sUrl)
{
	labTitle = NULL;
	uiToolbar = NULL;
	editUrl = NULL;
	btnHome = NULL;
	btnConfig = NULL;
	m_bPopup = bPopup;
	sHomepage = _T("https://www.baidu.com/");
	if(sUrl==NULL)
		m_sUrl = sHomepage;
	else
		m_sUrl = sUrl;
	m_sCount++;
}

CBrowserDlg::~CBrowserDlg()
{
	if(m_sCount--==1){
		PostQuitMessage(0);
	}
}

LPCTSTR CBrowserDlg::GetWindowClassName() const
{
	return _T("CBrowserDlg");
}

void CBrowserDlg::InitWindow()
{
	SetIcon(IDR_MAINFRAME);
	labTitle = static_cast<CLabelUI*>(m_pm.FindControl(_T("labTitle")));
	uiToolbar = static_cast<CControlUI*>(m_pm.FindControl(_T("uiToolbar")));
	editUrl = static_cast<CEditUI*>(m_pm.FindControl(_T("editUrl")));
	btnHome = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnHome")));
	btnConfig = static_cast<CButtonUI*>(m_pm.FindControl(_T("btnConfig")));
	mBrowser = static_cast<CBrowserUI*>(m_pm.FindControl(_T("mBrowser")));
	if (labTitle == NULL || uiToolbar == NULL || editUrl == NULL || btnHome == NULL || btnConfig == NULL || mBrowser == NULL)
	{
		MessageBox(NULL,_T("������Դ�ļ�ʧ��"),_T("Browser"),MB_OK|MB_ICONERROR);
		return;
	}
	if(m_bPopup)
		uiToolbar->SetVisible(false);
}

void CBrowserDlg::OnFinalMessage(HWND hWnd)
{
	WindowImplBase::OnFinalMessage(hWnd);
	delete this;
}

CDuiString CBrowserDlg::GetSkinFile()
{
	return _T("BrowserDlg.xml");
}

LRESULT CBrowserDlg::ResponseDefaultKeyEvent(WPARAM wParam)
{
	if (wParam == VK_RETURN)
	{
		return FALSE;
	}
	else if (wParam == VK_ESCAPE)
	{
		return TRUE;
	}
	return FALSE;
}

CControlUI* CBrowserDlg::CreateControl(LPCTSTR pstrClass)
{
	CControlUI* pUI = NULL;
	if (_tcsicmp(pstrClass, _T("BrowserUI")) == 0)
	{
		pUI = mBrowser = new CBrowserUI();
	}
	return pUI;
}

void CBrowserDlg::Notify(TNotifyUI& msg)
{
	CDuiString sCtrlName = msg.pSender->GetName();

	if (_tcsicmp(msg.sType,DUI_MSGTYPE_WINDOWINIT) == 0)
	{
		if(mBrowser){
			editUrl->SetText(m_sUrl);
			mBrowser->CreateBrowser(m_sUrl.GetData(),this);
		}
	}
	else if (_tcsicmp(msg.sType,_T("click")) == 0)
	{
		if (_tcsicmp(sCtrlName,_T("btnHome")) == 0){
			if(mBrowser){
				editUrl->SetText(sHomepage);
				mBrowser->LoadURL(sHomepage.GetData());
			}
			return;
		}else if (_tcsicmp(sCtrlName,_T("btnConfig")) == 0){
			if(mBrowser){
				mBrowser->LoadURL(_T("about:config"));
			}
			return;
		}
	}
	else if (_tcsicmp(msg.sType,DUI_MSGTYPE_RETURN) == 0)
	{
		if (_tcsicmp(sCtrlName,_T("editUrl")) == 0){
			CDuiString sAddr;
			CDuiString sUrl = editUrl->GetText();
			if(sUrl.Find(_T("http://")) >= 0 || sUrl.Find(_T("https://")) >= 0){
				sAddr = sUrl;
			}else{
				sAddr.Format(_T("http://%s"),sUrl.GetData());
			}
			editUrl->SetText(sAddr);
			if(mBrowser)
				mBrowser->LoadURL(sAddr.GetData());
			return;
		}
	}
	return WindowImplBase::Notify(msg);
}

void CBrowserDlg::SetAddress(LPCTSTR pstrAddress)
{
	editUrl->SetText(pstrAddress);
}

void CBrowserDlg::SetTitle(LPCTSTR pstrTitle)
{
	labTitle->SetText(pstrTitle);
	SetWindowText(m_hWnd, pstrTitle);
}

void CBrowserDlg::Popup(LPCTSTR pstrUrl)
{
	m_sPopup = pstrUrl;
	SendMessage(WM_POPUP_WINDOW);
}

void CBrowserDlg::OnPopup()
{
	CBrowserDlg* pPopup = new CBrowserDlg(TRUE,m_sPopup.GetData());
	if(pPopup == NULL) return;
	pPopup->Create(NULL,_T("Browser"),UI_WNDSTYLE_FRAME,WS_EX_APPWINDOW,0,0,800,600);
	pPopup->CenterWindow();
}


LRESULT CBrowserDlg::HandleCustomMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	switch (uMsg)
	{
	case WM_POPUP_WINDOW:
		bHandled = TRUE;
		OnPopup();
		break;
	default:
		bHandled = FALSE;
		break;
	}
	return 0;
}
LRESULT CBrowserDlg::OnClose(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	if(mBrowser)
		mBrowser->CloseAllBrowsers();
	bHandled = FALSE;
	return 0;
}

LRESULT CBrowserDlg::OnDestroy(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
	bHandled = FALSE;
	return 0;
}