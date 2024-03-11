// injector.cpp : Defines the entry point for the application.
//
#pragma comment(lib,"Shcore.lib")
#include "framework.h"
#include "injector.h"
#include "injectrelate.h"
#include "getrelate.h"
#define MAX_LOADSTRING 100
#define BUFFER_SIZE 512
//#define _UNICODE
//#define UNICODE
// 目录关联变量
wchar_t path[MAX_PATH];
wchar_t drive[_MAX_DRIVE];
wchar_t dir[_MAX_DIR];
wchar_t fname[_MAX_FNAME];
wchar_t ext[_MAX_EXT];
wchar_t configFilePath[MAX_PATH];
wchar_t line[BUFFER_SIZE] = { 0 };
// Global Variables:
const wchar_t* text_1 = L"请先在 “更多选项” 中导入DLL";
wchar_t* DllPath = NULL;
int g_FontSize = 16;
HFONT hFont;
COLORREF oldColor;
HDC hdc;
BOOL useRedText = FALSE;
HWND hWnd;
HWND hDlg2;
HWND injectButton;
DWORD PID;
TCHAR PID_display[128];
TCHAR CustomInject_display[128];
TCHAR convertPID[64];
int CustomPID_State_0 = 0;
int CustomPID_State = 0;
int FileState = 0;
BOOL TextoutHighDPI_Support = FALSE;
wchar_t* TEMP[MAX_PATH];
UINT dpi;
UINT dpiX;
HANDLE hMutex;
HFONT hFontGlobal; // 用于存储调整后的字体
//#define TEMP L"\DLL.dll"
const wchar_t* processName = L"minecraft.windows.exe";
HINSTANCE hInstance;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];  // the main window class name
int screenWidth,screenHeight,startX, startY;
HFONT g_hFont = NULL;
HFONT g_hFont_Text = NULL;
// DPI感知处理
void DrawScaledText(HDC hdc, LPCWSTR text, int baseX, int baseY, UINT dpi) {

	int fontSize = -MulDiv(g_FontSize, dpi, 96);
	// 创建适应高DPI的字体
	g_hFont_Text = CreateFont(fontSize, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET,
		OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
		DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft YaHei"));
	SelectObject(hdc, g_hFont_Text);
	float scalingFactor = (float)dpi / 96;
	int scaledXDrawText = (int)(baseX * scalingFactor);
	int scaledYDrawText = (int)(baseY * scalingFactor);

	TextOut(hdc, scaledXDrawText, scaledYDrawText, text, wcslen(text));
}
void AdjustWindowSizeForDPI(HWND hWnd, int baseWidth, int baseHeight, UINT dpi)
{
	float scalingFactor = (float)dpi / 96;
	int scaledWidth = (int)(baseWidth * scalingFactor);
	int scaledHeight = (int)(baseHeight * scalingFactor);
	// 获取屏幕分辨率
	screenWidth = GetSystemMetrics(SM_CXSCREEN);
	screenHeight = GetSystemMetrics(SM_CYSCREEN);

	// 计算左上角的位置，使窗口居中
	startX = (screenWidth - scaledWidth) / 2;
	startY = (screenHeight - scaledHeight) / 2;
	SetWindowPos(hWnd, NULL, startX, startY, scaledWidth, scaledHeight, SWP_NOZORDER);
}
void AdjustButtonForDPI(HWND hWnd, HWND hButton, int baseX, int baseY, int baseWidth, int baseHeight, UINT dpi) {
	float scalingFactor = (float)dpi / 96;
	int scaledX = (int)(baseX * scalingFactor);
	int scaledY = (int)(baseY * scalingFactor);
	int scaledWidth = (int)(baseWidth * scalingFactor);
	int scaledHeight = (int)(baseHeight * scalingFactor);

	SetWindowPos(hButton, NULL, scaledX, scaledY, scaledWidth, scaledHeight, SWP_NOZORDER);
}
// 系统版本检测处理
BOOL IsWindows10OrGreater() {
	OSVERSIONINFOEX osvi;
	DWORDLONG dwlConditionMask = 0;

	// 初始化OSVERSIONINFOEX结构体
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	osvi.dwMajorVersion = 10; // Windows 10的主版本号

	// 初始化条件掩码
	VER_SET_CONDITION(dwlConditionMask, VER_MAJORVERSION, VER_GREATER_EQUAL);

	// 检查系统版本是否满足条件
	return VerifyVersionInfo(&osvi, VER_MAJORVERSION, dwlConditionMask);
}
// 字体应用到按钮
void SetButtonFont(HWND hwndButton, UINT dpi) {
	LOGFONT lf = { 0 };
	lf.lfHeight = -MulDiv(10, dpi, 72); // 根据 dpiY 动态计算 10pt 字体大小
	lf.lfWeight = FW_NORMAL; // 字体粗细
	lf.lfCharSet = DEFAULT_CHARSET; // 字符集
	wcscpy_s(lf.lfFaceName, L"Microsoft YaHei"); // 字体名称

	// 检查是否已有字体对象
	if (g_hFont) {
		DeleteObject(g_hFont);
	}
	g_hFont = CreateFontIndirect(&lf);
	if (g_hFont) {
		SendMessage(hwndButton, WM_SETFONT, WPARAM(g_hFont), TRUE);
	}
}

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
// 窗口声明
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    CustomPIDProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	// TODO: Place code here, but don't place file scope here.
	// 多实例检测部分
	hMutex = CreateMutexW(NULL, FALSE, L"Global\\Onlyoneinstance_injector_ac403da3eea9");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		// 如果存在，则说明程序已在运行
		MessageBoxW(NULL, L"注入器已经在运行中！", L"错误", MB_ICONEXCLAMATION | MB_OK);
		CloseHandle(hMutex);
		return 0;
	}
	//版本检测
	if (!IsWindows10OrGreater()) {
		MessageBoxW(NULL, L"此程序只能在NT10或更高版本的Windows系统上运行。", L"错误", MB_ICONERROR);
		return -1;
	}
	// 初始化DPI感知
	SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
	// Initialize global strings
	// 从资源文件读取类名
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_INJECTOR, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);
	// Perform application initialization:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}
	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_INJECTOR));

	MSG msg;

	// Main message loop:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_INJECTOR));
	//wcex.hIcon = NULL;
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_INJECTOR);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	//wcex.hIconSm = NULL;

	// 注册主窗口类
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
	//hInst = hInstance; // Store instance handle in our global variable

	// 预设窗口样式 (WS_OVERLAPPEDWINDOW)
	DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

	// 创建窗口
	hWnd = CreateWindowW(szWindowClass, szTitle, style,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, nullptr, nullptr, hInstance, nullptr);
	dpi = GetDpiForWindow(hWnd); // 获取窗口所在显示器的DPI
	// 高DPI适配 根据DPI调整窗口大小
	AdjustWindowSizeForDPI(hWnd, 300, 130, dpi);
	injectButton = CreateWindow(L"BUTTON", L"注入", WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON, 10, 30, 265, 30, hWnd, (HMENU)ID_BUTTON_START, (HINSTANCE)GetWindowLongPtr(hWnd, GWLP_HINSTANCE), NULL);
	SetButtonFont(injectButton,dpi);
	// 根据DPI调整按钮位置
	AdjustButtonForDPI(hWnd, injectButton, 10, 30, 265, 30,dpi);
	// 显示窗口
	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//

// 检查文件是否存在并且不是目录
int fileExists(const wchar_t* path) {
	DWORD fileAttributes = GetFileAttributesW(path);
	if (fileAttributes == INVALID_FILE_ATTRIBUTES) {
		return 0;
	}
	if (fileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
		return 0;
	}
	return 1;
}

void StartInject() {
	// 参数定义
	//TCHAR tempPath[MAX_PATH];
	DWORD PID = GetMCProcID(processName);
	if (fileExists(line) == 1) {
		// 检查文件
		if (fileExists(line) == 0)
		{
			text_1 = L"请先导入DLL!";
			g_FontSize = 16;
			useRedText = TRUE;
			// 重绘窗口
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return;
		}
		else if (PID == 0)
		{
			//const wchar_t PID_replace_display[128] = L"未找到目标进程，确保 Minecraft 已经打开。";
			//wcscpy_s(PID_display, PID_replace_display);
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			text_1 = L"未找到目标进程，确保 Minecraft 已经打开";
			g_FontSize = 13;
			useRedText = TRUE;
			//wcscpy_s(PID_display, configFilePath);
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			// 重绘窗口
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return;
		}
		else {
			if(DllPath!=NULL){
				char ansiPath[MAX_PATH];
				// 文本处理
				wsprintf(PID_display, TEXT("进程 %lu 已被注入。"), PID);
				//GetTempPath(MAX_PATH, tempPath);
				// 提取DLL到 "%TEMP%\DLL.dll"
				//wcscat_s(tempPath, MAX_PATH, TEMP);
				//
				//宽字符转窄字符
				WideCharToMultiByte(CP_ACP, 0, DllPath, -1, ansiPath, MAX_PATH, NULL, NULL);
				//ExtractResource(GetModuleHandle(NULL), dll_DLL, ansiPath);
				size_t len = strlen(ansiPath);
				std::wstring ws(&ansiPath[0], &ansiPath[len]);
				SetAccessControl(ws, L"S-1-15-2-1");
				performInjection(PID, ansiPath);
				//MessageBox(hWnd, PID_display, L"提示", MB_OK);
				useRedText = FALSE;
				g_FontSize = 16;
				text_1 = PID_display;
				// 重绘窗口
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
			else {
				char ansiPath[MAX_PATH];
				// 文本处理
				wsprintf(PID_display, TEXT("进程 %lu 已被注入。"), PID);
				//GetTempPath(MAX_PATH, tempPath);
				// 提取DLL到 "%TEMP%\DLL.dll"
				//wcscat_s(tempPath, MAX_PATH, TEMP);
				//
				//宽字符转窄字符
				WideCharToMultiByte(CP_ACP, 0, line, -1, ansiPath, MAX_PATH, NULL, NULL);
				//ExtractResource(GetModuleHandle(NULL), dll_DLL, ansiPath);
				size_t len = strlen(ansiPath);
				std::wstring ws(&ansiPath[0], &ansiPath[len]);
				SetAccessControl(ws, L"S-1-15-2-1");
				performInjection(PID, ansiPath);
				//MessageBox(hWnd, PID_display, L"提示", MB_OK);
				useRedText = FALSE;
				g_FontSize = 16;
				text_1 = PID_display;
				// 重绘窗口
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}
		}
	}
	else {
		// 如果传进来的参数是NULL，则说明没有导入DLL
		if (DllPath == NULL || fileExists(DllPath) == 0)
		{
			text_1 = L"请先导入DLL!";
			g_FontSize = 16;
			useRedText = TRUE;
			// 重绘窗口
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return;
		}
		else if (PID == 0)
		{
			//wcscpy_s(PID_display, PID_replace_display);
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			text_1 = L"未找到目标进程，确保 Minecraft 已经打开";
			g_FontSize = 13;
			useRedText = TRUE;
			//wcscpy_s(PID_display, configFilePath);
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			// 重绘窗口
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
			return;
		}
		else {
			char ansiPath[MAX_PATH];
			// 文本处理
			wsprintf(PID_display, TEXT("进程 %lu 已被注入。"), PID);
			//GetTempPath(MAX_PATH, tempPath);
			// 提取DLL到 "%TEMP%\DLL.dll"
			//wcscat_s(tempPath, MAX_PATH, TEMP);
			// 
			//宽字符转窄字符
			WideCharToMultiByte(CP_ACP, 0, DllPath, -1, ansiPath, MAX_PATH, NULL, NULL);
			//ExtractResource(GetModuleHandle(NULL), dll_DLL, ansiPath);
			size_t len = strlen(ansiPath);
			std::wstring ws(&ansiPath[0], &ansiPath[len]);
			SetAccessControl(ws, L"S-1-15-2-1");
			performInjection(PID, ansiPath);
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			useRedText = FALSE;
			g_FontSize = 16;
			text_1 = PID_display;
			// 重绘窗口
			InvalidateRect(hWnd, NULL, TRUE);
			UpdateWindow(hWnd);
		}
	}
}
void CustomPIDInject(DWORD PID) {
	// 参数定义
	TCHAR tempPath[MAX_PATH];
	TCHAR szProcessName[MAX_PATH];
	// 转换PID
	TCHAR convertPID[32];
	if (fileExists(line) == 1) {
		if (fileExists(line) == 0) {
			CustomPID_State = 1;
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			return;
		}
		else if (PID == 0)
		{
			//MessageBox(hWnd, L"PID不能为空或零。", L"注意", MB_OK);
			CustomPID_State = 2;
			return;
		}
		//!=NULL , bSuccess=TRUE
		else if (GetProcessNameByPID(PID, szProcessName, MAX_PATH)) { 
			// 比较进程名是否为 Minecraft 进程
			//wcscmp
			if (_tcsicmp(szProcessName, TEXT("minecraft.windows.exe")) == 0)
			{
				if (DllPath != NULL) {
					char ansiPath[MAX_PATH];
					// 文本处理
					//GetTempPath(MAX_PATH, tempPath);
					// 提取DLL到 "%TEMP%\DLL.dll"
					//wcscat_s(tempPath, MAX_PATH, TEMP);
					WideCharToMultiByte(CP_ACP, 0, DllPath, -1, ansiPath, MAX_PATH, NULL, NULL);
					//ExtractResource(GetModuleHandle(NULL), MENU_DLL, ansiPath);
					size_t len = strlen(ansiPath);
					std::wstring ws(&ansiPath[0], &ansiPath[len]);
					SetAccessControl(ws, L"S-1-15-2-1");
					performInjection(PID, ansiPath);
					CustomPID_State = 4;
					wsprintf(convertPID, TEXT("进程 %lu 已被注入。"), PID);
					wcscpy_s(CustomInject_display, convertPID);
					//MessageBox(hWnd, convertPID, L"提示", MB_OK);
					return;
				}
				else {
					char ansiPath[MAX_PATH];
					// 文本处理
					//GetTempPath(MAX_PATH, tempPath);
					// 提取DLL到 "%TEMP%\DLL.dll"
					//wcscat_s(tempPath, MAX_PATH, TEMP);
					WideCharToMultiByte(CP_ACP, 0, line, -1, ansiPath, MAX_PATH, NULL, NULL);
					//ExtractResource(GetModuleHandle(NULL), KUNGFU_DLL, ansiPath);
					size_t len = strlen(ansiPath);
					std::wstring ws(&ansiPath[0], &ansiPath[len]);
					SetAccessControl(ws, L"S-1-15-2-1");
					performInjection(PID, ansiPath);
					CustomPID_State = 4;
					wsprintf(convertPID, TEXT("进程 %lu 已被注入。"), PID);
					wcscpy_s(CustomInject_display, convertPID);
					//MessageBox(hWnd, convertPID, L"提示", MB_OK);
					return;
				}
			}
			else
			{
				CustomPID_State = 5;
				//MessageBox(hWnd, TEXT("该进程不是 Minecraft 进程！"), TEXT("错误"), MB_OK);
				return;
			}
		}
		// ==NULL
		// bSuccess=FALSE
		else {
			CustomPID_State = 3;
			//MessageBox(NULL, TEXT("找不到该进程。"), TEXT("错误"), MB_OK);
			return;
		}
	}
	else {
		if (DllPath == NULL || fileExists(DllPath) == 0)
		{
			//const wchar_t PID_replace_display[128] = L"请先导入DLL!";
			//wcscpy_s(PID_display, PID_replace_display);
			CustomPID_State = 1;
			//MessageBox(hWnd, PID_display, L"提示", MB_OK);
			return;
		}
		else if (PID == 0)
		{
			//MessageBox(hWnd, L"PID不能为空或零。", L"注意", MB_OK);
			CustomPID_State = 2;
			return;
		}
		//!=NULL , bSuccess=TRUE
		else if (GetProcessNameByPID(PID, szProcessName, MAX_PATH)) { 
			// 比较进程名是否为 Minecraft 进程
			if (_tcsicmp(szProcessName, TEXT("minecraft.windows.exe")) == 0)
			{
				char ansiPath[MAX_PATH];
				// 文本处理
				//GetTempPath(MAX_PATH, tempPath);
				// 提取DLL到 "%TEMP%\DLL.dll"
				//wcscat_s(tempPath, MAX_PATH, TEMP);
				WideCharToMultiByte(CP_ACP, 0, DllPath, -1, ansiPath, MAX_PATH, NULL, NULL);
				//ExtractResource(GetModuleHandle(NULL), KUNGFU_DLL, ansiPath);
				size_t len = strlen(ansiPath);
				std::wstring ws(&ansiPath[0], &ansiPath[len]);
				SetAccessControl(ws, L"S-1-15-2-1");
				performInjection(PID, ansiPath);
				CustomPID_State = 4;
				wsprintf(convertPID, TEXT("进程 %lu 已被注入。"), PID);
				wcscpy_s(CustomInject_display, convertPID);
				//MessageBox(hWnd, convertPID, L"提示", MB_OK);
				return;
			}
			else
			{
				CustomPID_State = 5;
				//MessageBox(hWnd, TEXT("该进程不是 Minecraft 进程！"), TEXT("错误"), MB_OK);
				return;
			}
		}
		// ==NULL
		// bSuccess=FALSE
		else {
			CustomPID_State = 3;
			//MessageBox(NULL, TEXT("找不到该进程。"), TEXT("错误"), MB_OK);
			return;
		}
	}
}
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DPICHANGED: {
    dpiX = LOWORD(wParam);
    AdjustWindowSizeForDPI(hWnd, 300, 130, dpiX);
    AdjustButtonForDPI(hWnd, injectButton, 10, 30, 265, 30, dpiX);
	SetButtonFont(injectButton, dpiX);
	TextoutHighDPI_Support = TRUE;
	InvalidateRect(hWnd, NULL, TRUE);
	UpdateWindow(hWnd);
	// 获取程序的可执行文件路径
	//TCHAR szAppPath[MAX_PATH];
	//GetModuleFileName(NULL, szAppPath, MAX_PATH);
	}
	break;
	case WM_CREATE: {
		// 获取当前程序完整路径
		GetModuleFileNameW(NULL, path, MAX_PATH);
		// 分解路径为驱动器/目录/文件名/扩展名
		_wsplitpath_s(path, drive, dir, fname, ext);
		// 构建配置文件路径
		_snwprintf_s(configFilePath, sizeof(configFilePath) / sizeof(wchar_t), _TRUNCATE, L"%s%sinjector_config.ini", drive, dir);

		FILE* file;

		errno_t err;
		// 读取配置文件
		err = _wfopen_s(&file, configFilePath, L"r");
		if (err != 0) {
			// 读取不到配置文件

		}
		else {
			// 文件成功打开
			if (fgetws(line, BUFFER_SIZE, file) != NULL) {
				// 移除字符串末尾换行符
				line[wcscspn(line, L"\n")] = 0;
				// 在这里使用configDLLPath
				//MessageBoxW(NULL, line, L"配置文件路径", MB_ICONINFORMATION | MB_OK);
			}
			else {
				// 空配置文件处理

			}
			fclose(file);
		}

		// 检查 DLL 文件是否存在
		if (fileExists(line)) {
			text_1 = L"已读取到先前导入的DLL!";
			g_FontSize = 16;
		}
		else {
			text_1 = L"请先在 “更多选项” 中导入DLL";
			g_FontSize = 16;
		}
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 参数预定义
		TCHAR tempPath[MAX_PATH];
		// Parse the menu selections:
		switch (wmId)
		{
		case ID_BUTTON_START:
			StartInject();
			break;
		case IDM_CUSTOMPID:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_CUSTOM_PID), hWnd, CustomPIDProc);
			break;
		case IDM_IMPORT_DLL: {
			// 获取DLL路径
			wchar_t* tempDllPath = OnImportDLL(hWnd);
			if (tempDllPath == NULL)
			{

			}
			else {
				if (DllPath != NULL) {
					free(DllPath);
					DllPath = NULL;
				}
				DllPath = tempDllPath;
				// 创建配置文件
				FILE* configFile = NULL;
				errno_t err;
				err = _wfopen_s(&configFile, configFilePath, L"w");
				if (err != 0) {
					MessageBoxW(NULL, L"无法创建配置文件。", L"错误", MB_OK | MB_ICONERROR);
				}
				else {
					fwprintf(configFile, L"%s", DllPath);
					fclose(configFile);
					fflush(configFile);
				}

				text_1 = L"已导入DLL!";
				g_FontSize = 16;
				useRedText = FALSE;
				CustomPID_State_0 = -1;
				// 重绘窗口
				InvalidateRect(hWnd, NULL, TRUE);
				UpdateWindow(hWnd);
			}

		}
		break;
		case IDM_ABOUT:
			DialogBox(hInstance, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		hdc = BeginPaint(hWnd, &ps);
		// TODO: Add any drawing code that uses hdc here...
		//hFont = CreateFont(MulDiv(g_FontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72), 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_TT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_DONTCARE, TEXT("Microsoft YaHei"));
		//SelectObject(hdc, hFont);
		if (useRedText)
		{
			oldColor = SetTextColor(hdc, RGB(255, 0, 0)); // 红
		}
		else
		{
			oldColor = SetTextColor(hdc, RGB(0, 0, 0)); // 黑
		}
		// TEXT部分
		//TextOut(hdc, 26, 4, text_1, wcslen(text_1));
		if (!TextoutHighDPI_Support) {
			DrawScaledText(hdc, text_1, 26, 4, dpi);
		}
		else {
			DrawScaledText(hdc, text_1, 26, 4, dpiX);
		}
		DeleteObject(g_hFont_Text);
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_CLOSE:
		DestroyWindow(hWnd); //销毁窗口发送WM_DESTROY消息
		break;
	case WM_DESTROY: {
		if (g_hFont) {
			DeleteObject(g_hFont);
			g_hFont = NULL;
		}
		if (g_hFont_Text) {
			DeleteObject(g_hFont_Text);
			g_hFont_Text = NULL;
		}
		CloseHandle(hMutex);
		PostQuitMessage(0);
	}
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}
// 自定义PID 对话框消息处理函数
INT_PTR CALLBACK CustomPIDProc(HWND hDlg2, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		if (CustomPID_State_0 == -1) {
			SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("已导入DLL!"));
		}
		else if (fileExists(line) == 1) {
			SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("已读取到先前导入的DLL!"));
		}
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDC_PIDBUTTON_OK)
		{
			TCHAR szText[11];
			DWORD szPID;
			GetDlgItemText(hDlg2, IDC_EDITBOX_PID, szText, 10);
			// 文本转换为数字
			szPID = _ttoi(szText);
			// 传递PID，并注入
			CustomPIDInject(szPID);
			if (CustomPID_State == 1) {
				SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("请先导入DLL!"));
			}
			else if (CustomPID_State == 2) {
				SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("PID不能为空或零"));
			}
			else if (CustomPID_State == 3) {
				SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("找不到该进程"));
			}
			else if (CustomPID_State == 4) {
				SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, CustomInject_display);
			}
			else if (CustomPID_State == 5) {
				SetDlgItemText(hDlg2, IDC_CUSTOMPIDTEXT, TEXT("该进程不是 Minecraft 进程！"));
			}
			return (INT_PTR)TRUE;
		}
		break;
	case WM_CLOSE:
		EndDialog(hDlg2, 0);
		return (INT_PTR)TRUE;
		break;
	}
	return (INT_PTR)FALSE;
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
