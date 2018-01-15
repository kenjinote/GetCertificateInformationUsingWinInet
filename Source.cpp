#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma comment(lib, "winInet")

#include <windows.h>
#include <stdio.h>
#include <iostream>
#include <tchar.h>
#include <wininet.h>

TCHAR szClassName[] = TEXT("Window");

LRESULT CALLBACK WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static HWND hEdit1;
	static HWND hButton;
	static HWND hEdit2;
	switch (msg)
	{
	case WM_CREATE:
		hEdit1 = CreateWindowEx(WS_EX_CLIENTEDGE,TEXT("EDIT"), TEXT("https://www.microsoft.com"), WS_VISIBLE | WS_CHILD | WS_TABSTOP | ES_AUTOHSCROLL, 10, 10, 1024, 32, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hButton = CreateWindow(TEXT("BUTTON"), TEXT("取得"), WS_VISIBLE | WS_CHILD | WS_TABSTOP, 0, 0, 0, 0, hWnd, (HMENU)IDOK, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		hEdit2 = CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("EDIT"), 0, WS_VISIBLE | WS_CHILD | WS_TABSTOP | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE | ES_READONLY, 0, 0, 0, 0, hWnd, 0, ((LPCREATESTRUCT)lParam)->hInstance, 0);
		break;
	case WM_SIZE:
		MoveWindow(hEdit1, 10, 10, LOWORD(lParam) - 20, 32, TRUE);
		MoveWindow(hButton, 10, 50, 256, 32, TRUE);
		MoveWindow(hEdit2, 10, 90, LOWORD(lParam) - 20, HIWORD(lParam) - 100, TRUE);
		break;
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK)
		{
			SetWindowText(hEdit2, 0);
			URL_COMPONENTS urlComp;
			ZeroMemory((void *)&urlComp, sizeof(URL_COMPONENTS));
			urlComp.dwSchemeLength = -1;
			urlComp.dwHostNameLength = -1;
			urlComp.dwUrlPathLength = -1;
			urlComp.dwStructSize = sizeof(URL_COMPONENTS);
			TCHAR szUrl[1024];
			GetWindowText(hEdit1, szUrl, _countof(szUrl));
			if (!InternetCrackUrl(szUrl, lstrlen(szUrl), 0, &urlComp))
			{
				SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)TEXT("InternetCrackUrl failed"));
			}
			else
			{
				if (urlComp.nScheme != INTERNET_SCHEME_HTTPS)
				{
					SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)TEXT("Please specify and HTTPS address to query the certificate information for: https://www.someserver.com"));
				}
				else
				{
					HINTERNET hInternet = InternetOpen(_T(""), INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0);
					if (hInternet)
					{
						HINTERNET hConnect = InternetConnect(hInternet, urlComp.lpszHostName, INTERNET_DEFAULT_HTTPS_PORT, NULL, NULL, INTERNET_SERVICE_HTTP, INTERNET_FLAG_SECURE, NULL);
						if (hConnect)
						{
							HINTERNET hRequest = HttpOpenRequest(hConnect, _T("HEAD"), NULL, NULL, NULL, NULL, INTERNET_FLAG_SECURE, NULL);
							if (NULL != hRequest)
							{
								BOOL fRet = HttpSendRequest(hRequest, _T(""), 0, NULL, 0);
								if (fRet)
								{
									CHAR szCertificateInfoStr[2048];
									szCertificateInfoStr[0] = '\0';
									DWORD certInfoLength = _countof(szCertificateInfoStr);
									if (TRUE == InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE, &szCertificateInfoStr, &certInfoLength) && szCertificateInfoStr[0])
									{
										SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)szCertificateInfoStr);
									}
									INTERNET_CERTIFICATE_INFO certificateInfo;
									certInfoLength = sizeof(INTERNET_CERTIFICATE_INFO);
									if (TRUE == InternetQueryOption(hRequest, INTERNET_OPTION_SECURITY_CERTIFICATE_STRUCT, &certificateInfo, &certInfoLength))
									{
										if (certificateInfo.lpszEncryptionAlgName)
										{
											SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)certificateInfo.lpszEncryptionAlgName);
											LocalFree(certificateInfo.lpszEncryptionAlgName);
										}

										if (certificateInfo.lpszIssuerInfo)
										{
											SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)certificateInfo.lpszIssuerInfo);
											LocalFree(certificateInfo.lpszIssuerInfo);
										}

										if (certificateInfo.lpszProtocolName)
										{
											SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)certificateInfo.lpszProtocolName);
											LocalFree(certificateInfo.lpszProtocolName);
										}

										if (certificateInfo.lpszSignatureAlgName)
										{
											SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)certificateInfo.lpszSignatureAlgName);
											LocalFree(certificateInfo.lpszSignatureAlgName);
										}

										if (certificateInfo.lpszSubjectInfo)
										{
											SendMessageA(hEdit2, EM_REPLACESEL, 0, (LPARAM)certificateInfo.lpszSubjectInfo);
											LocalFree(certificateInfo.lpszSubjectInfo);
										}
									}
								}
								InternetCloseHandle(hRequest);
							}
							else
							{
								SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)TEXT("HttpOpenRequest failed"));
							}
							InternetCloseHandle(hConnect);
						}
						else
						{
							SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)TEXT("InternetConnect failed"));
						}
						InternetCloseHandle(hInternet);
					}
					else
					{
						SendMessage(hEdit2, EM_REPLACESEL, 0, (LPARAM)TEXT("InternetOpen failed"));
					}
				}
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefDlgProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPreInst, LPSTR pCmdLine, int nCmdShow)
{
	MSG msg;
	WNDCLASS wndclass = {
		0,
		WndProc,
		0,
		DLGWINDOWEXTRA,
		hInstance,
		0,
		LoadCursor(0,IDC_ARROW),
		0,
		0,
		szClassName
	};
	RegisterClass(&wndclass);
	HWND hWnd = CreateWindow(
		szClassName,
		TEXT("入力されたURLの証明書情報を取得"),
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		0,
		CW_USEDEFAULT,
		0,
		0,
		0,
		hInstance,
		0
	);
	ShowWindow(hWnd, SW_SHOWDEFAULT);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, 0, 0, 0))
	{
		if (!IsDialogMessage(hWnd, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return (int)msg.wParam;
}
