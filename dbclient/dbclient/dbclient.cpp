// dbclient.cpp : Defines the entry point for the application.
//

#include <stdlib.h>
#include "network.h"
#include "stdafx.h"
#include "dbclient.h"
#include <windows.h>    
#include <commctrl.h>   
#include <stdio.h>
#include <string.h>
#include <TCHAR.H>




#define INPUT_BOX_SIZE 150
#define SEND_BUTTON_BOX_SIZE_XY 150
#define MAIN_WINDOW_WIDTH 900
#define MAIN_WINDOW_HEIGHT 700

#define MAX_LOADSTRING 100
#define MAX_COLUMNS 6
#define MAX_ITEMLENGTH 120
#define MAX_ROWS 11000

#define SERVER_ADDR "127.0.0.1"
#define SERVER_PORT 8888

//Struct used to store one row of data from the server.
typedef struct query{
	char date[MAX_ITEMLENGTH];
	INT64 externalid;
	char number[MAX_ITEMLENGTH];
	float constant;
	int digits;
	short int decimals;
} QUERY;

#pragma comment(lib, "comctl32.lib")

// Global Variables:
HINSTANCE hInst;								// current instance
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
QUERY *listofitem;
bool displaycolumns[6] = { false, false, false, false, false, false };


// Forward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	About(HWND, UINT, WPARAM, LPARAM);
HWND				CreateListView(HWND hWndParent);
HWND				CreateEditView(HWND hWndParent);
HWND				CreateSendButton(HWND hWndParent);
LRESULT				NotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
int					setup_connection();
HWND				populate_listview(HWND hWndList, QUERY *items, int numitems, int numcolumns);
int					copy_reply_value(char *dest, char *src, int *pos, int maxlen);
HWND				add_to_listview(HWND hWndList,  int index);
int					find_string(char *src, char *dest, int maxlen);
HWND				parse_show_result(HWND hWndListView);

//--------------------------------------------------------------------------------
int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPTSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	MSG msg;
	HACCEL hAccelTable;

	//Allocate array on heap to store values from queries
	listofitem = (QUERY *)malloc(sizeof(QUERY) * MAX_ROWS);
	memset(listofitem, 0, sizeof(QUERY) * MAX_ROWS);

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_DBCLIENT, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow))
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_DBCLIENT));



	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0) > 0)
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
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_DBCLIENT));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+2);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_DBCLIENT);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));




	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindowEx(
		0,
	   szWindowClass, 
	   szTitle, 
	   WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE,
      0,
	  0, 
	  MAIN_WINDOW_WIDTH + 15,
	  MAIN_WINDOW_HEIGHT + 60,
	  NULL, 
	  NULL, 
	  NULL, 
	  NULL);


   if (!hWnd)
   {
      return FALSE;
   }
   
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;
	TCHAR sendbuffer[255];
	static HWND hWndListView, hWndEditView, hWndSendButton;
	PAINTSTRUCT ps;
	HDC hdc;

	switch (message)
	{
	case WM_CREATE:
		hWndListView = CreateListView(hWnd);
		if (hWndListView == NULL)
			MessageBox(NULL, "Listview not created!", NULL, MB_OK);

		hWndEditView = CreateEditView(hWnd);
		if (hWndEditView == NULL)
			MessageBox(NULL, "EditView not created!", NULL, MB_OK);

		hWndSendButton = CreateSendButton(hWnd);
		if (hWndSendButton == NULL)
			MessageBox(NULL, "Send Button not created!", NULL, MB_OK);

		if (setup_connection() != 0){
			MessageBox(NULL, "Error connecting to host", NULL, MB_OK);
		}
		
		break;
	case WM_NOTIFY:
		return(NotifyHandler(hWnd, message, wParam, lParam));
		break;
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		// Parse the menu selections:
		switch (wmId)
		{
			case ID_SENDBUTTON:
			{

				//Remove previous result if any
				ListView_DeleteAllItems(hWndListView);
				memset(listofitem, 0, sizeof(QUERY) * MAX_ROWS);
				for (int i = 0; i < MAX_COLUMNS; i++) displaycolumns[i] = false;

				//Get query text.
				int length = GetWindowText(hWndEditView, sendbuffer, 255);
				if (length > 0){
					//Get the number of packets coming
					network_send_packet(sendbuffer, length);
					//get data from all packets and show them on screen
					hWndListView = parse_show_result(hWndListView);
				}

				
				break;
			}
			case IDM_EXIT:
				delete[] listofitem;
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

/****************************************************************************
*
*    FUNCTION: CreateEditView(HWND hWndParent)
*
*    PURPOSE:  Creates the editview.
*
****************************************************************************/
HWND CreateEditView(HWND hWndParent) {

	HWND hWndEdit;

	// Ensure that the common control DLL is loaded.
	InitCommonControls();


	hWndEdit = CreateWindowEx(0L,
		WC_EDIT,                
		"select * from data",     
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_EX_CLIENTEDGE,
		0, 
		MAIN_WINDOW_HEIGHT - INPUT_BOX_SIZE,
		MAIN_WINDOW_WIDTH - SEND_BUTTON_BOX_SIZE_XY,
		INPUT_BOX_SIZE,
		hWndParent,
		(HMENU)ID_EDITVIEW,
		hInst,
		NULL);

	if (hWndEdit == NULL)
		return NULL;



	return hWndEdit;
}
/****************************************************************************
*
*    FUNCTION: CreateSendButton(HWND hWndParent)
*
*    PURPOSE:  Creates the Send button.
*
****************************************************************************/
HWND CreateSendButton(HWND hWndParent){

	HWND hWndSend;

	// Ensure that the common control DLL is loaded.
	InitCommonControls();


	hWndSend = CreateWindowEx(
		0L,
		WC_BUTTON,
		"SEND            ",
		WS_VISIBLE | WS_CHILD | WS_BORDER | WS_EX_CLIENTEDGE | SS_CENTER,
		MAIN_WINDOW_WIDTH - SEND_BUTTON_BOX_SIZE_XY,
		MAIN_WINDOW_HEIGHT - SEND_BUTTON_BOX_SIZE_XY,
		SEND_BUTTON_BOX_SIZE_XY,
		SEND_BUTTON_BOX_SIZE_XY,
		hWndParent,
		(HMENU)ID_SENDBUTTON,
		hInst,
		NULL);

	if (hWndSend == NULL)
		return NULL;

	
	return hWndSend;

}


/****************************************************************************
* 
*    FUNCTION: CreateListView(HWND)
*
*    PURPOSE:  Creates the list view window and initializes it
*
****************************************************************************/
HWND CreateListView (HWND hWndParent)                                     
{
	HWND hWndList;      // Handle to the list view window
	LV_COLUMN lvC;      // List View Column structure
	char szText[MAX_PATH];    // Place to store some text

	// Ensure that the common control DLL is loaded.
	InitCommonControls();

	// Create the list view window that starts out in report view
    // and allows label editing.
	hWndList = CreateWindowEx( 0L,
		WC_LISTVIEW,                // list view class
		"Default",                         // no default text
		WS_VISIBLE | WS_CHILD | WS_BORDER | LVS_REPORT |
		    LVS_EDITLABELS | WS_EX_CLIENTEDGE,
		0, 
		0,
		MAIN_WINDOW_WIDTH, 
		MAIN_WINDOW_HEIGHT - INPUT_BOX_SIZE,
		hWndParent,
		(HMENU) ID_LISTVIEW,
		hInst,
		NULL );

	if (hWndList == NULL )
		return NULL;


	// Now initialize the columns we will need
	// Initialize the LV_COLUMN structure
	// the mask specifies that the .fmt, .ex, width, and .subitem members 
	// of the structure are valid,
	lvC.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
	lvC.fmt = LVCFMT_LEFT;  // left align the column
	lvC.cx = 75;            // width of the column, in pixels
	lvC.pszText = szText;

	// Add the columns.
	for (int index = 0; index < MAX_COLUMNS; index++)
	{
		lvC.iSubItem = index;
		LoadString(hInst,
			IDS_STRING129 + index,
			szText,
			sizeof(szText));
		if (ListView_InsertColumn(hWndList, index, &lvC) == -1)
			return NULL;
	}


	return (hWndList);
}
/****************************************************************************
*
*    FUNCTION: add_to_listview(HWND hWndList, int index)
*
*    PURPOSE:  Adds an element to the listview at index.
*
****************************************************************************/
HWND add_to_listview(HWND hWndList, int index){

	LV_ITEM lvI;        // List view item structure

	lvI.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
	lvI.state = 0;      //

	lvI.iItem = index;
	lvI.iSubItem = 0;

	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.cchTextMax = MAX_ITEMLENGTH;
	lvI.lParam = (LPARAM)&listofitem[index];
	

	if (ListView_InsertItem(hWndList, &lvI) == -1)
		return NULL;

	return hWndList;
}


/****************************************************************************
*
*    FUNCTION: NotifyHandler(HWND, UINT, UINT, LONG)
*
*    PURPOSE: This function is the handler for the WM_NOTIFY that is
*    sent to the parent of the list view window.
*
****************************************************************************/
LRESULT NotifyHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LV_DISPINFO *pLvdi = (LV_DISPINFO *)lParam;
	QUERY *pquery = (QUERY *)(pLvdi->item.lParam);
	static char szText[50];

	if (wParam != ID_LISTVIEW)
		return 0L;

	switch (pLvdi->hdr.code)
	{
		case LVN_GETDISPINFO:

			switch (pLvdi->item.iSubItem)
			{
			case 0:
				if (displaycolumns[0])
					pLvdi->item.pszText = pquery->date;
				break;
			case 1:
				if (displaycolumns[1]){
					_i64toa_s(pquery->externalid, szText, 50, 10);
					pLvdi->item.pszText = szText;
				}
				break;

			case 2:
				if (displaycolumns[2])
					pLvdi->item.pszText = pquery->number;
				break;

			case 3:
				if (displaycolumns[3]){
					sprintf_s(szText, "%f", pquery->constant);
					pLvdi->item.pszText = szText;
				}
				break;

			case 4:   
				if (displaycolumns[4]){
					sprintf_s(szText, "%u", pquery->digits);
					pLvdi->item.pszText = szText;
				}

				break;
			case 5:   
				if (displaycolumns[5]){
					sprintf_s(szText, "%u", pquery->decimals);
					pLvdi->item.pszText = szText;
				}
				break;
			default:
				break;
			}
			break;


		default:
			break;
	}
	return 0L;
}
/****************************************************************************
*
*    FUNCTION: setup_connection()
*
*    PURPOSE: Connects to the server at SERVER_ADDR and SERVER_PORT, for communication
*			  with the server over tcp.
*
****************************************************************************/
int setup_connection(){

	if (network_init() == 0){
		if (network_create_socket() == 0){
			if (network_connect_to_host(SERVER_ADDR, SERVER_PORT) == 0){
				return 0;
			}
		}
	}
	
	return -1;

}
/****************************************************************************
*
*    FUNCTION: copy_reply_value(char *dest, char *src, int *srcpos, int maxlen)
*
*    PURPOSE: Copies the value between two " signs and pushes the srcpos after the last ",
*			  if at maxlen sets pos to maxlen -1.
*
****************************************************************************/
int copy_reply_value(char *dest, char *src, int *srcpos, int maxlen){

	int destpos = 0;
	while (src[(*srcpos)] != '\"' && (*srcpos) < maxlen) (*srcpos)++;
	(*srcpos)++;

	while (src[(*srcpos)] != '\"' && (*srcpos) < maxlen){
		dest[destpos] = src[(*srcpos)];
		(*srcpos)++;
		destpos++;
	}

	dest[destpos] = '\0';
	(*srcpos)++;

	if ((*srcpos) < maxlen){
		return 0;
	}		
	else{
		(*srcpos) = maxlen - 1;
		return 1;
	}
}
/****************************************************************************
*
*    FUNCTION: find_string(char *src, char *dest, int maxlen)
*
*    PURPOSE: Finds a string within another string, does not rely on terminating NULL.
*			  
*
****************************************************************************/
int find_string(char *src, char *dest, int maxlen){
	int pos = 0;
	int len = strlen(dest);
	char buffer[10];
	while (pos < maxlen){

		for (int i = 0; i < len; i++){
			buffer[i] = src[pos + i];
		}
		buffer[len] = '\0';
		if (!strcmp(buffer, dest)){
			return 1;
		}
	
		pos++;
	}

	return 0;
}
/****************************************************************************
*
*    FUNCTION: parse_show_result(HWND hWndListView)
*
*    PURPOSE: Gets information from tcp buffer, parses it and displays it.
*
*
****************************************************************************/
HWND parse_show_result(HWND hWndListView){

	const int maxlen = 500;
	char reply[maxlen];
	char value[maxlen];
	char header[maxlen];
	int index = 0;
	int pos = 0;
	bool bufferend = false;
	

	if (network_receive_packet(reply, maxlen) == -1){
		return hWndListView;
	}

	int replylen = strlen(reply);


	do{

		copy_reply_value(header, reply, &pos, replylen);
		if (!strcmp(header, "row")){
			while (reply[pos] != '}' && pos < replylen){

				copy_reply_value(header, reply, &pos, replylen);

				if (!strcmp(header, "date")){
					copy_reply_value(listofitem[index].date, reply, &pos, replylen);
					displaycolumns[0] = true;
				}
				else if (!strcmp(header, "externalid")){
					copy_reply_value(value, reply, &pos, replylen);
					listofitem[index].externalid = strtoll(value, NULL, 10);
					displaycolumns[1] = true;
				}
				else if (!strcmp(header, "number")){
					copy_reply_value(listofitem[index].number, reply, &pos, replylen);
					displaycolumns[2] = true;
				}
				else if (!strcmp(header, "constant")){
					copy_reply_value(value, reply, &pos, replylen);
					listofitem[index].constant = strtof(value, NULL);
					displaycolumns[3] = true;
				}
				else if (!strcmp(header, "digits")){
					copy_reply_value(value, reply, &pos, replylen);
					listofitem[index].digits = strtol(value, NULL, 10);
					displaycolumns[4] = true;
				}
				else if (!strcmp(header, "decimals")){
					copy_reply_value(value, reply, &pos, replylen);
					listofitem[index].decimals = strtol(value, NULL, 10);
					displaycolumns[5] = true;
				}


				if (bufferend == false && find_string(reply, "\"end\"", maxlen) == 0){
					memcpy(reply, reply + pos, sizeof(char) * (replylen - pos));
					network_receive_packet(reply + (replylen - pos), maxlen - (replylen - pos));
					replylen = strlen(reply);
					pos = 0;
				}
				else{
					bufferend = true;
				}



			}



			hWndListView = add_to_listview(hWndListView, index);
			index++;


		}


	} while (pos < (replylen - 1));

	return hWndListView;
}
