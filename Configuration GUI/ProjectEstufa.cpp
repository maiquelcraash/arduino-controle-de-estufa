// ProjectEstufa.cpp : Defines the entry point for the application.
//
#include <stdio.h> 
#include <windows.h> 
#include <winbase.h>
#include <iostream>
#include <tchar.h>
#include "stdafx.h"
#include "ProjectEstufa.h"

#define _CRT_SECURE_NO_DEPRECATE
#define MAX_LOADSTRING 100

// Global Variables:

LPCWSTR porta = L"COM5";
struct serialData {
	bool status;
	char access;
	char type;
	int size;
	unsigned char data[30];
};
DWORD lpNumberOfBytesRead;
HANDLE hfile;
DCB dcbSerialParams = { 0 };					// Initializing DCB structure
COMMTIMEOUTS timeouts;							// timeouts

HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
HWND janela_baixo;
char lpBuffer[20] = "";






// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Principal_Proc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Historico_Proc(HWND, UINT, WPARAM, LPARAM);
bool				writeSerial(CHAR *, DWORD, HWND);
serialData			readSerial(HWND);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
					   _In_opt_ HINSTANCE hPrevInstance,
					   _In_ LPTSTR    lpCmdLine,
					   _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_PROJECTESTUFA, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_PROJECTESTUFA));

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int) msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_PROJECTESTUFA));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_PROJECTESTUFA);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	dcbSerialParams.DCBlength = sizeof(dcbSerialParams);
	dcbSerialParams.BaudRate = CBR_9600;  // Setting BaudRate = 9600
	dcbSerialParams.ByteSize = 8;         // Setting ByteSize = 8
	dcbSerialParams.StopBits = ONESTOPBIT;//ONESTOPBIT;// Setting StopBits = 1
	dcbSerialParams.Parity   = NOPARITY;  // Setting Parity = None


	timeouts.ReadIntervalTimeout = 1000;
	timeouts.ReadTotalTimeoutMultiplier = 0;
	timeouts.ReadTotalTimeoutConstant = 0;
	timeouts.WriteTotalTimeoutMultiplier = 0;
	timeouts.WriteTotalTimeoutConstant = 0;

	HWND hWnd;

	hInst = hInstance; // Store instance handle in our global variable
	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		janela_baixo = hWnd; // salvando tela que criamos na varivel janela_baixo

		DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, Principal_Proc);
		break;

	case WM_COMMAND:		

		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code here...
		EndPaint(hWnd, &ps);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		} 
		break;
	}
	return (INT_PTR)FALSE;
}

// criando a rotina de tratamento de evento
INT_PTR CALLBACK Principal_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int tp_evento = HIWORD(wParam);
	int id_evento = LOWORD(wParam);	
	static DWORD	quantidade;
	static char	toWrite[50] = "";
	//static LPTSTR	esp_new = new TCHAR[20];
	switch(message)
	{
	case WM_INITDIALOG:
		{
		}
		break;
	case WM_COMMAND:



		if (tp_evento == BN_CLICKED)
		{
			switch(id_evento)
			{
			case ID_UPDATE:
				{	
					char toWrite[6];
					toWrite[0] = 72;
					toWrite[2] = 0;
					toWrite[3] = 0;
					toWrite[4] = 0;
					toWrite[5] = 0;
					serialData result;

					//############ OBTEM TEMPERATURA #############
					toWrite[1] = 1;
					if (writeSerial(toWrite, 6, hDlg)){
						result = readSerial(hDlg);
						SetDlgItemInt(hDlg, ID_TEMP_ATUAL, result.data[0], true);
					}
					else {
						break;
					}


					//############ OBTEM LUMINOSIDADE #############
					toWrite[1] = 2;
					if (writeSerial(toWrite, 6, hDlg)){
						result = readSerial(hDlg);
						SetDlgItemInt(hDlg, ID_LUM_ATUAL, (result.data[0]*1000), true);
					}
					else {
						break;
					}

					//############ OBTEM ESPECIE #############
					toWrite[1] = 4;
					if (writeSerial(toWrite, 6, hDlg)){
						result = readSerial(hDlg);
						SetDlgItemTextA(hDlg, ID_ESP_ATUAL, (LPCSTR)result.data);						
					}
					else {
						break;
					}
				}
				break;

			case ID_ADD_NEW:
				{
					static char esp_new[40] = "";
					char	temp_min_new[10] = "";
					char	temp_max_new[10] = "";
					char	lum_min_new[20] = "";
					char	lum_max_new[20] = "";
					//char	hr_ini_new[10] = "";
					//char	hr_fim_new[10] = "";	

					//Pega valores dos campos de cadastro
					GetDlgItemTextA(hDlg, ID_ESP_NEW, (LPSTR)esp_new, 41);
					//temp_min_new = GetDlgItemInt(hDlg, ID_TEMP_MIN_NEW, NULL, true);
					//temp_max_new = GetDlgItemInt(hDlg, ID_TEMP_MAX_NEW, NULL, true);
					//lum_min_new = GetDlgItemInt(hDlg, ID_LUM_MIN_NEW, NULL, true);
					//lum_max_new = GetDlgItemInt(hDlg, ID_LUM_MAX_NEW, NULL, true);
					GetDlgItemText(hDlg, ID_TEMP_MIN_NEW, (LPWSTR)temp_min_new, 3);
					GetDlgItemText(hDlg, ID_TEMP_MAX_NEW, (LPWSTR)temp_max_new, 3);
					GetDlgItemText(hDlg, ID_LUM_MIN_NEW, (LPWSTR)lum_min_new, 6);
					GetDlgItemText(hDlg, ID_LUM_MAX_NEW, (LPWSTR)lum_max_new, 6);

					//Pega os campos de cada lista e seta os valores
					HWND hwndList = GetDlgItem(hDlg, ID_LIST_ESP);
					SendMessageA(hwndList, LB_INSERTSTRING, 0, (LPARAM)esp_new);

					HWND hwndList2 = GetDlgItem(hDlg, ID_LIST_TEMP_MIN);
					SendMessage(hwndList2, LB_INSERTSTRING, 0, (LPARAM)temp_min_new);
					HWND hwndList3 = GetDlgItem(hDlg, ID_LIST_TEMP_MAX);
					SendMessage(hwndList3, LB_INSERTSTRING, 0, (LPARAM)temp_max_new);

					HWND hwndList4 = GetDlgItem(hDlg, ID_LIST_LUM_MIN);
					SendMessage(hwndList4, LB_INSERTSTRING, 0, (LPARAM)lum_min_new);
					HWND hwndList5 = GetDlgItem(hDlg, ID_LIST_LUM_MAX);
					SendMessage(hwndList5, LB_INSERTSTRING, 0, (LPARAM)lum_max_new);


					//############ ENVIA NOVO PERFIL AO ARDUINO #############
					char toWrite[24];
					serialData result;

					//############ ENVIA ESPECIE #############
					toWrite[0] = 27;
					toWrite[1] = 4;
					toWrite[2] = 20;
					toWrite[3] = 0;

					int i;
					for (i = 0; i < 20; i++){
						toWrite[i + 4] = esp_new[i];
					}

					if (writeSerial(toWrite, 24, hDlg)){
						result = readSerial(hDlg);

						if (result.data[0] != 1){ break;}
					}
					else {
						break;
					}

					//############ ENVIA Temperaturas #############
					toWrite[0] = 27;
					toWrite[1] = 1;
					toWrite[2] = 6;
					toWrite[3] = 0;
					toWrite[4] = (BYTE)temp_max_new;
					toWrite[5] = (BYTE)temp_min_new;

					if (writeSerial(toWrite, 6, hDlg)){
						result = readSerial(hDlg);

						if (result.data[0] != 1){ break;}
					}
					else {
						break;
					}

					//############ ENVIA Luminosidade #############
					toWrite[0] = 27;
					toWrite[1] = 2;
					toWrite[2] = 6;
					toWrite[3] = 0;
					toWrite[4] = (BYTE)lum_max_new;
					toWrite[5] = (BYTE)lum_min_new;

					if (writeSerial(toWrite, 6, hDlg)){
						result = readSerial(hDlg);

						if (result.data[0] != 1){ break;}
					}
					else {
						break;
					}


					//Limpa os campos de cadastro
					SetDlgItemText(hDlg, ID_ESP_NEW, L"");
					SetDlgItemText(hDlg, ID_TEMP_MIN_NEW, L"");
					SetDlgItemText(hDlg, ID_TEMP_MAX_NEW, L"");
					SetDlgItemText(hDlg, ID_LUM_MIN_NEW, L"");
					SetDlgItemText(hDlg, ID_LUM_MAX_NEW, L"");
					SetDlgItemText(hDlg, ID_HR_INI_NEW, L"");
					SetDlgItemText(hDlg, ID_HR_FIM_NEW, L"");
				}
				break;

			case IDCANCEL:
				EndDialog(hDlg,  LOWORD(wParam));
				DestroyWindow(janela_baixo);
				break;

			case ID_BT_HIST:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG2), hDlg, Historico_Proc);
				break;

			default:
				return FALSE;
			}
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg,  LOWORD(wParam));
		DestroyWindow(janela_baixo);
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

INT_PTR CALLBACK Historico_Proc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	int tp_evento = HIWORD(wParam);
	int id_evento = LOWORD(wParam);

	switch (message)
	{
	case WM_INITDIALOG:
		{

						  
						  /*
			// Add items to list. 
			HWND hwndListHist = GetDlgItem(hDlg, ID_LIST_ESP_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);

			hwndListHist = GetDlgItem(hDlg, ID_LIST_TEMP_MIN_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);
			hwndListHist = GetDlgItem(hDlg, ID_LIST_TEMP_MAX_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);

			hwndListHist = GetDlgItem(hDlg, ID_LIST_LUM_MIN_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);
			hwndListHist = GetDlgItem(hDlg, ID_LIST_LUM_MAX_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);

			hwndListHist = GetDlgItem(hDlg, ID_LIST_HR_HIST);
			SendMessage(hwndListHist, LB_INSERTSTRING, 0, (LPARAM)lpBuffer);
			*/
		}
		break;
	case WM_COMMAND:

		if (tp_evento == BN_CLICKED)
		{
			switch (id_evento)
			{								   
			case IDOK_HIST:
				EndDialog(hDlg, LOWORD(wParam));				
				break;			
			default:
				return FALSE;
			}
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg, LOWORD(wParam));		
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

bool writeSerial(CHAR *toWrite, DWORD bytesToWrite, HWND hDlg){
	DWORD dNoOFBytestoWrite;					// No of bytes to write into the port
	DWORD dNoOfBytesWritten = 0;				// No of bytes written to the port
	dNoOFBytestoWrite = bytesToWrite;

	hfile = CreateFile(porta, GENERIC_READ | GENERIC_WRITE, 0,	NULL, OPEN_EXISTING, 0,	NULL);

	if(hfile == INVALID_HANDLE_VALUE){
		MessageBox(hDlg, L"Nao foi possivel localizar a porta. Verifique as configuracoes",L"Erro",MB_OK);
		CloseHandle(hfile);
		return false;
	}
	else{
		SetCommState(hfile, &dcbSerialParams);
		SetCommTimeouts(hfile, &timeouts);
		WriteFile(hfile, toWrite, dNoOFBytestoWrite, &dNoOfBytesWritten, NULL);
		CloseHandle(hfile);
		return true;
	}
}

serialData readSerial(HWND hDlg){
	serialData readedData;
	readedData.status = false;
	hfile = CreateFile(porta, GENERIC_READ | GENERIC_WRITE, 0,	NULL, OPEN_EXISTING, 0,	NULL);

	if(hfile == INVALID_HANDLE_VALUE){
		MessageBox(hDlg, L"Nao foi possivel localizar a porta. Verifique as configuracoes",L"Erro",MB_OK);
		CloseHandle(hfile);
		return readedData;
	}
	else {
		SetCommState(hfile, &dcbSerialParams);
		SetCommTimeouts(hfile, &timeouts);

		unsigned char SerialBuffer[30];						//Buffer for storing Rxed Data
		memset(SerialBuffer, 0, 30);						//clean the buffer
		memset(readedData.data, 0, 30);
		DWORD NoBytesRead;
		int i = 0;

		ReadFile(hfile, &SerialBuffer, 30, &NoBytesRead, NULL);

		if(SerialBuffer[0] == 27 || SerialBuffer[0] == 72){
			//ReadFile(hfile, &SerialBuffer, SerialBuffer[3], &NoBytesRead, NULL);
			CloseHandle(hfile);

			readedData.access = SerialBuffer[0];
			readedData.type = SerialBuffer[1];
			readedData.size = SerialBuffer[2]; //SerialBuffer[2]*255 + SerialBuffer[3];

			for(int i = 0; i < readedData.size; i++){
				readedData.data[i] = SerialBuffer[i+4]; 
			}

			return readedData;
		}
		else{
			CloseHandle(hfile);
			return readedData;
		}
	}
}
