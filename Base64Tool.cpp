// Base64Tool.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Base64Tool.h"
extern "C"
{
#include "cencode.h"
#include "cdecode.h"
}

#include <CommCtrl.h>
#include <commdlg.h>

#pragma comment(linker, \
	"\"/manifestdependency:type='Win32' "\
	"name='Microsoft.Windows.Common-Controls' "\
	"version='6.0.0.0' "\
	"processorArchitecture='*' "\
	"publicKeyToken='6595b64144ccf1df' "\
	"language='*'\"")

#pragma comment(lib, "ComCtl32.lib")
// Global Variables:
HINSTANCE hInst;								// current instance


BOOL CenterWindow(HWND hwndWindow)
{
	HWND hwndParent;
	RECT rectWindow, rectParent;

	int nScreenWidth = GetSystemMetrics(SM_CXSCREEN);
	int nScreenHeight = GetSystemMetrics(SM_CYSCREEN);

	GetWindowRect(hwndWindow, &rectWindow);

	int nWidth = rectWindow.right - rectWindow.left;
	int nHeight = rectWindow.bottom - rectWindow.top;

	int nX, nY;


	// make the window relative to its parent
	if ((hwndParent = GetParent(hwndWindow)) != NULL)
	{
		GetWindowRect(hwndParent, &rectParent);

		nX = ((rectParent.right - rectParent.left) - nWidth) / 2 + rectParent.left;
		nY = ((rectParent.bottom - rectParent.top) - nHeight) / 2 + rectParent.top;

		// make sure that the dialog box never moves outside of the screen
		if (nX < 0) nX = 0;
		if (nY < 0) nY = 0;
		if (nX + nWidth > nScreenWidth) nX = nScreenWidth - nWidth;
		if (nY + nHeight > nScreenHeight) nY = nScreenHeight - nHeight;
	}
	else
	{
		nX = (nScreenWidth - nWidth) / 2;
		nY = (nScreenHeight - nHeight) / 2;
	}
	MoveWindow(hwndWindow, nX, nY, nWidth, nHeight, FALSE);

	return TRUE;
}


INT_PTR CALLBACK DialogProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEditBox;
	static HWND hCheckBox;
	static HWND hMaxLineEdit;
	OPENFILENAME ofn;
	char char_per_line_buf[10];
	int src_len = 0;
	int dst_len = 0;
	char* src_content = NULL;
	char* dst_content = NULL;
	HFONT hFont;

	/* keep track of our encoded position */
	char* c = dst_content;
	/* store the number of bytes encoded by a single call */
	int cnt = 0;
	/* we need an encoder state */
	base64_encodestate s;
	/* we need a decoder state */
	base64_decodestate ds;

	TCHAR szFilter[] = TEXT ("All Files (*.*)\0*.*\0\0") ;
	TCHAR FilePath[MAX_PATH] = {0};
	HANDLE file;
	DWORD file_len;

	switch(uMsg)
	{
	case WM_INITDIALOG:
		CenterWindow(hDlg);
		hEditBox = GetDlgItem(hDlg, IDC_TEXT);
		hFont=CreateFont(15,0,0,0,0,0,0,0,0,0,0,0,0,TEXT("Courier New"));
		SendMessage(hEditBox,WM_SETFONT,(WPARAM)hFont,0);

		hCheckBox = GetDlgItem(hDlg, IDC_CHAR_PER_LINE);
		hMaxLineEdit = GetDlgItem(hDlg, IDC_MAX_CHAR);
		EnableWindow(hMaxLineEdit, FALSE);
		SetWindowText(hMaxLineEdit, "76");

		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDCANCEL:
			SendMessage(hDlg, WM_CLOSE, 0, 0);
			return TRUE;
		case IDC_CHAR_PER_LINE:
			EnableWindow(hMaxLineEdit, SendMessage(hCheckBox,BM_GETCHECK,0,0));
			return TRUE;
// 		case IDC_BTN1:
// 			//ret = DialogBox(g_hinst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, DialogProc2);
// 			return TRUE;
		case IDC_ENCODE:
			//加密
			src_len = GetWindowTextLength(hEditBox);
			//计算加密后长度
			dst_len = 4 * ((src_len + 2) / 3);
			//获取行长度
			if (SendMessage(hCheckBox,BM_GETCHECK,0,0))
			{
				GetWindowText(hMaxLineEdit, char_per_line_buf, 10);
				CHARS_PER_LINE = atoi(char_per_line_buf);
			dst_len += ((dst_len + CHARS_PER_LINE - 1) / CHARS_PER_LINE) * 2;
			}
			else
			{
				CHARS_PER_LINE = 0;
			}
			src_content = (char*)malloc(src_len + 1);
			dst_content = (char*)malloc(dst_len + 1);
			GetWindowText(hEditBox, src_content, src_len + 1);
			c = dst_content;
			/*---------- START ENCODING ----------*/
			/* initialise the encoder state */
			base64_init_encodestate(&s);
			/* gather data from the input and send it to the output */
			cnt = base64_encode_block(src_content, strlen(src_content), c, &s);
			c += cnt;
			/* since we have encoded the entire input string, we know that 
			   there is no more input data; finalise the encoding */
			cnt = base64_encode_blockend(c, &s);
			c += cnt;
			/*---------- STOP ENCODING  ----------*/
	
			/* we want to print the encoded data, so null-terminate it: */
			*c = 0;

			SetWindowText(hEditBox, dst_content);
			free(src_content);
			free(dst_content);

			return TRUE;
		case IDC_DECODE:
			//解密
			src_len = GetWindowTextLength(hEditBox);
			dst_len = src_len / 4 * 3;
			src_content = (char*)malloc(src_len + 1);
			dst_content = (char*)malloc(dst_len + 1);

			GetWindowText(hEditBox, src_content, src_len + 1);
			c = dst_content;

			/*---------- START DECODING ----------*/
			/* initialise the decoder state */
			base64_init_decodestate(&ds);
			/* decode the input data */
			cnt = base64_decode_block(src_content, strlen(src_content), c, &ds);
			c += cnt;
			/* note: there is no base64_decode_blockend! */
			/*---------- STOP DECODING  ----------*/

			/* we want to print the decoded data, so null-terminate it: */
			*c = 0;
			SetWindowText(hEditBox, dst_content);
			free(src_content);
			free(dst_content);

			return TRUE;
		case IDC_ENCODE_FROM_FILE:
			//Open File
	
			ofn.lStructSize       = sizeof (OPENFILENAME) ;
			ofn.hwndOwner         = hDlg ;
			ofn.hInstance         = NULL ;
			ofn.lpstrFilter       = szFilter ;
			ofn.lpstrCustomFilter = NULL ;
			ofn.nMaxCustFilter    = 0 ;
			ofn.nFilterIndex      = 0 ;
			ofn.nMaxFile          = MAX_PATH ;
			ofn.nMaxFileTitle     = MAX_PATH ;
			ofn.lpstrInitialDir   = NULL ;
			ofn.lpstrTitle        = NULL ;
			ofn.Flags             = 0 ;             // Set in Open and Close functions
			ofn.nFileOffset       = 0 ;
			ofn.nFileExtension    = 0 ;
			ofn.lpstrDefExt       = TEXT ("bmp") ;
			ofn.lCustData         = 0 ;
			ofn.lpfnHook          = NULL ;
			ofn.lpTemplateName    = NULL ;
			ofn.lpstrFile         = FilePath;
			ofn.lpstrFileTitle    = NULL ;
			ofn.Flags             = 0 ;

			if (GetOpenFileName(&ofn) == FALSE)
			{
				return TRUE;
			}
			//读取文件内容并加密

			file = CreateFile(FilePath, GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			file_len = GetFileSize(file, NULL);
			if (file_len > (1 * 1024 * 1024))
			{
				MessageBox(hDlg, "文件过大", "错误", MB_OK);
				return TRUE;
			}
			src_len = file_len;
			//计算加密后长度
			dst_len = src_len * 2;
			//获取行长度
			if (SendMessage(hCheckBox,BM_GETCHECK,0,0))
			{
				GetWindowText(hMaxLineEdit, char_per_line_buf, 10);
				CHARS_PER_LINE = atoi(char_per_line_buf);
			dst_len += ((dst_len + CHARS_PER_LINE - 1) / CHARS_PER_LINE) * 2;
			}
			else
			{
				CHARS_PER_LINE = 0;
			}
			src_content = (char*)malloc(src_len + 1);
			dst_content = (char*)malloc(dst_len + 1);
			//GetWindowText(hEditBox, src_content, src_len + 1);
			DWORD read_len;
			if (ReadFile(file, src_content, file_len, &read_len, NULL) == FALSE)
			{
				return TRUE;
			}
			CloseHandle(file);

			c = dst_content;
			/*---------- START ENCODING ----------*/
			/* initialise the encoder state */
			base64_init_encodestate(&s);
			/* gather data from the input and send it to the output */
			cnt = base64_encode_block(src_content, read_len, c, &s);
			c += cnt;
			/* since we have encoded the entire input string, we know that 
			   there is no more input data; finalise the encoding */
			cnt = base64_encode_blockend(c, &s);
			c += cnt;
			/*---------- STOP ENCODING  ----------*/
	
			/* we want to print the encoded data, so null-terminate it: */
			*c = 0;

			SetWindowText(hEditBox, dst_content);
			free(src_content);
			free(dst_content);
			return TRUE;
		case IDC_DECODE_TO_FILE:
			//Open File
			ofn.lStructSize       = sizeof (OPENFILENAME) ;
			ofn.hwndOwner         = hDlg ;
			ofn.hInstance         = NULL ;
			ofn.lpstrFilter       = szFilter ;
			ofn.lpstrCustomFilter = NULL ;
			ofn.nMaxCustFilter    = 0 ;
			ofn.nFilterIndex      = 0 ;
			ofn.nMaxFile          = MAX_PATH ;
			ofn.nMaxFileTitle     = MAX_PATH ;
			ofn.lpstrInitialDir   = NULL ;
			ofn.lpstrTitle        = NULL ;
			ofn.Flags             = 0 ;             // Set in Open and Close functions
			ofn.nFileOffset       = 0 ;
			ofn.nFileExtension    = 0 ;
			ofn.lpstrDefExt       = TEXT ("bin") ;
			ofn.lCustData         = 0 ;
			ofn.lpfnHook          = NULL ;
			ofn.lpTemplateName    = NULL ;
			ofn.lpstrFile         = FilePath;
			ofn.lpstrFileTitle    = NULL ;
			ofn.Flags             = 0 ;

			if (GetSaveFileName(&ofn) == FALSE)
			{
				return TRUE;
			}

			//解密
			src_len = GetWindowTextLength(hEditBox);
			dst_len = src_len / 4 * 3;
			src_content = (char*)malloc(src_len + 1);
			dst_content = (char*)malloc(dst_len + 1);

			GetWindowText(hEditBox, src_content, src_len + 1);
			c = dst_content;

			/*---------- START DECODING ----------*/
			/* initialise the decoder state */
			base64_init_decodestate(&ds);
			/* decode the input data */
			cnt = base64_decode_block(src_content, src_len, c, &ds);
			c += cnt;
			/* note: there is no base64_decode_blockend! */
			/*---------- STOP DECODING  ----------*/

			/* we want to print the decoded data, so null-terminate it: */
			*c = 0;
			//写入文件
			file = CreateFile(FilePath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
			WriteFile(file, dst_content, c - dst_content, &read_len, NULL);
			CloseHandle(file);

			free(src_content);
			free(dst_content);

			return TRUE;
		}
		break;
	case WM_CTLCOLORSTATIC:
		// Set the colour of the text for our URL
		if ((HWND)lParam == GetDlgItem(hDlg, IDC_MY_EMAIL)) 
		{
			// we're about to draw the static
			// set the text colour in (HDC)lParam
			SetBkMode((HDC)wParam,TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(255,0,0));
			return (BOOL)CreateSolidBrush (GetSysColor(COLOR_MENU));
		}
		break;
	case WM_CLOSE:
// 		if(MessageBox(hDlg, TEXT("Close the program?"), TEXT("Close"),
// 			MB_ICONQUESTION | MB_YESNO) == IDYES)
// 		{
			DestroyWindow(hDlg);
// 		}
		return TRUE;

	case WM_DESTROY:
		PostQuitMessage(0);
		return TRUE;
	}

	return FALSE;
}

int APIENTRY _tWinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPTSTR    lpCmdLine,
                     int       nCmdShow)
{
	HWND hDlg;
	MSG msg;
	BOOL ret;

	hInst = hInstance;

	InitCommonControls();
	hDlg = CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_DIALOG1), 0, DialogProc, 0);
	ShowWindow(hDlg, nCmdShow);

	while((ret = GetMessage(&msg, 0, 0, 0)) != 0) {
		if(ret == -1)
			return -1;

		if(!IsDialogMessage(hDlg, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return 0;
}
