/*
This is an example code how to use the Suzume Library.
*/

#define _USE_MATH_DEFINES
#define _WTL_NO_CSTRING
#include <math.h>
#include <stdlib.h>
#include <atlbase.h>
#include <atlstr.h>
#include <locale.h>
#include <atlapp.h>
#include <atlwin.h>
#include <atlcrack.h>
#include <atlmisc.h>
#include <vector>
#include "suzume.h"


//command line parameter parser
int UtilGetCommandLineParams(std::vector<CString> &rParams)
{
	int nArgc=0;
	LPWSTR *lplpArgs=CommandLineToArgvW(GetCommandLine(), &nArgc);
	rParams.resize(nArgc);
	for(int i=0;i<nArgc;i++){
		rParams[i]=lplpArgs[i];
	}
	LocalFree(lplpArgs);
	return nArgc;
}



CAppModule _Module;

class CMyWindow : public CWindowImpl<CMyWindow>
{
public:
	CMyWindow(CSVGImage &_image):image(_image){}
public:
	DECLARE_WND_CLASS(_T("SVG_GDIPLUS"));
	CSVGImage &image;
private:
	BEGIN_MSG_MAP_EX(CMyWindow)
		MSG_WM_PAINT(OnPaint)
		MSG_WM_CREATE(OnCreate)
		MSG_WM_DESTROY(OnDestroy)
	END_MSG_MAP()

	void OnPaint(HDC /*hDC*/){
		Gdiplus::Graphics graphics(m_hWnd);
		Gdiplus::SolidBrush brush(Gdiplus::Color(255,255,255));
		RECT rc;
		GetClientRect(&rc);
		graphics.FillRectangle(&brush,Gdiplus::RectF((float)rc.left,(float)rc.top,(float)(rc.right-rc.left),(float)(rc.bottom-rc.top)));

		//render
		//NOTE: you can specify more complicated transforms by calling graphics.SetTransform()
		image.render(graphics,0.0f,0.0f);
		ValidateRect(NULL);
	}

	LRESULT OnCreate(LPCREATESTRUCT){
		CString strTitle;
		strTitle.Format(_T("The Suzume SVG example - %.2f x %.2f"),image.getCanvasWidth(),image.getCanvasHeight());
		SetWindowText(strTitle);

		return 0;
	}

	void OnDestroy(){
		PostQuitMessage(0);
	}
};

int WINAPI _tWinMain(HINSTANCE hInstance, HINSTANCE, LPTSTR lpCmdLine, int nCmdShow)
{
#if defined(_DEBUG)
	// set flags to detect memory leaks
	_CrtSetDbgFlag(
		_CRTDBG_ALLOC_MEM_DF
		| _CRTDBG_LEAK_CHECK_DF
		|_CRTDBG_CHECK_ALWAYS_DF
		);
#endif
	std::vector<CString> args;
	UtilGetCommandLineParams(args);
	if(args.size()<=1){
		MessageBox(NULL,_T("Specify a SVG file"),_T("Usage"),MB_OK|MB_ICONINFORMATION);
		return 1;
	}

	_tsetlocale(LC_ALL,_T(""));	//set locale for UNICODE programs
	_Module.Init(NULL, hInstance);

	//Initialize COM : Required by the Suzume Library
	if(FAILED(CoInitialize(0))){
		return 1;
	}

	CMessageLoop theLoop;
	_Module.AddMessageLoop(&theLoop);
	// Initialize GDI+ : Required by the Suzume Library
	ULONG_PTR _nToken;
	Gdiplus::GdiplusStartupInput _sGdiplusStartupInput;
	Gdiplus::GdiplusStartup(&_nToken,&_sGdiplusStartupInput,NULL);


	if(args.size()>1){
		CSVGImage image;
		if(SUCCEEDED(image.load(args[1]))){
			// display a window
			CMyWindow wnd(image);
			wnd.Create(NULL, CWindow::rcDefault,_T("SVG_WINDOW_TITLE"), WS_OVERLAPPEDWINDOW | WS_VISIBLE);

			theLoop.Run();
		}else{
			MessageBox(NULL,image.getLastError(),NULL,MB_OK|MB_ICONEXCLAMATION);
		}
	}

	_Module.RemoveMessageLoop();

	//Terminate GDI+ and COM
	Gdiplus::GdiplusShutdown(_nToken);
	_Module.Term();

	CoUninitialize();
	return 0;
}
