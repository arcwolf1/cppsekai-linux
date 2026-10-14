// CppSekai - Linux stub for the Win32 surface used by the Windows build.
// Linux 上没有 Windows API，但上游代码里有一部分（单实例锁、窗口前置、
// 文件对话框、注册表字体、DWM 特效）没完全包在 #ifdef _WIN32 里。
// 全部 no-op 掉：能编过、能跑，功能缺失（这些本来就和音游核心无关）。
#pragma once

#ifdef __linux__

#include <cstdint>
#include <cstring>
#include <cwchar>
#include <string>

// ---- 基础类型
using HANDLE    = void*;
using HWND      = void*;
using HINSTANCE = void*;
using HMODULE   = void*;
using DWORD     = uint32_t;
using WORD      = uint16_t;
using BYTE      = uint8_t;
using BOOL      = int;
using UINT      = unsigned int;
using ATOM      = uint16_t;
using LRESULT   = long;
using WPARAM    = unsigned long;
using LPARAM    = long;
using LPVOID    = void*;
using LPCVOID   = const void*;
using LPWSTR    = wchar_t*;
using LPCWSTR   = const wchar_t*;
using LPSTR     = char*;
using LPCSTR    = const char*;
using HRESULT   = long;
using PCWSTR    = const wchar_t*;
using PCSTR     = const char*;
using LONG      = long;
using ULONGLONG = unsigned long long;
using HDC       = void*;
using FARPROC   = void(*)();

// ---- 宏
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#define CALLBACK
#define WINAPI
#define APIENTRY
#define ERROR_ALREADY_EXISTS 183
#define ERROR_SUCCESS 0
#define SW_SHOW    5
#define SW_RESTORE 9
#define SW_HIDE    0
#ifndef MAX_PATH
#define MAX_PATH 260
#endif
#define GW_OWNER 4
#define HKEY_CURRENT_USER  ((void*)0)
#define HKEY_LOCAL_MACHINE ((void*)1)
#define INVALID_HANDLE_VALUE ((HANDLE)(intptr_t)-1)
#define INVALID_FILE_ATTRIBUTES ((DWORD)-1)
#define FILE_MAP_ALL_ACCESS 0
#define PAGE_READWRITE 0
#define RRF_RT_REG_SZ 0
#define RRF_RT_REG_DWORD 0
#define SPI_GETNONCLIENTMETRICS 0
#define ROTATE_0 0

// ---- 函数 stub（no-op）
inline HANDLE CreateMutexW(void*, BOOL, const wchar_t*) { return nullptr; }
inline BOOL   CloseHandle(HANDLE) { return TRUE; }
inline DWORD  GetLastError() { return 0; }
inline void   SetLastError(DWORD) {}
inline HWND   FindWindowW(const wchar_t*, const wchar_t*) { return nullptr; }
inline HWND   FindWindowA(const char*, const char*) { return nullptr; }
inline HWND   GetForegroundWindow() { return nullptr; }
inline BOOL   ShowWindow(HWND, int) { return TRUE; }
inline BOOL   SetForegroundWindow(HWND) { return TRUE; }
inline DWORD  GetWindowThreadProcessId(HWND, DWORD*) { return 0; }
inline BOOL   AttachThreadInput(DWORD, DWORD, BOOL) { return TRUE; }
inline BOOL   SetWindowPos(HWND, HWND, int, int, int, int, UINT) { return TRUE; }
inline BOOL   IsWindowVisible(HWND) { return FALSE; }
inline BOOL   IsIconic(HWND) { return FALSE; }
inline HWND   GetActiveWindow() { return nullptr; }
inline HWND   GetWindow(HWND, UINT) { return nullptr; }
inline int    GetWindowTextW(HWND, wchar_t*, int) { return 0; }
inline BOOL   EnumWindows(BOOL(*)(HWND, LPARAM), LPARAM) { return TRUE; }
inline DWORD  GetModuleFileNameW(HMODULE, wchar_t*, DWORD) { return 0; }
inline UINT   RegisterWindowMessageW(const wchar_t*) { return 0; }
inline HMODULE LoadLibraryW(const wchar_t*) { return nullptr; }
inline HMODULE GetModuleHandleW(const wchar_t*) { return nullptr; }
inline FARPROC GetProcAddress(HMODULE, const char*) { return nullptr; }
inline DWORD  GetFileAttributesW(const wchar_t*) { return INVALID_FILE_ATTRIBUTES; }
inline DWORD  GetFileAttributesA(const char*) { return INVALID_FILE_ATTRIBUTES; }
inline UINT   GetWindowsDirectoryW(wchar_t*, UINT) { return 0; }
inline BOOL   SystemParametersInfoW(UINT, UINT, void*, UINT) { return FALSE; }
inline LONG   RegGetValueW(void*, const wchar_t*, const wchar_t*, DWORD, void*, void*, DWORD*) { return -1; }
inline LONG   RegOpenKeyExW(void*, const wchar_t*, DWORD, DWORD, void**) { return -1; }
inline LONG   RegQueryValueExW(void*, const wchar_t*, void*, DWORD*, BYTE*, DWORD*) { return -1; }
inline LONG   RegCloseKey(void*) { return 0; }
inline LONG   RegSetValueExW(void*, const wchar_t*, DWORD, DWORD, const BYTE*, DWORD) { return -1; }
inline LONG   RegCreateKeyExW(void*, const wchar_t*, DWORD, void*, DWORD, DWORD, void*, void**, DWORD*) { return -1; }
inline BOOL   CreateProcessW(const wchar_t*, wchar_t*, void*, void*, BOOL, DWORD, void*, const wchar_t*, void*, void*) { return FALSE; }
inline DWORD  WaitForSingleObject(HANDLE, DWORD) { return 0; }
inline BOOL   GetExitCodeProcess(HANDLE, DWORD*) { return TRUE; }
inline DWORD  GetTickCount() { return 0; }
inline ULONGLONG GetTickCount64() { return 0; }
inline void   Sleep(DWORD) {}
inline HRESULT CoInitializeEx(void*, DWORD) { return 0; }
inline void   CoUninitialize() {}
inline HRESULT DwmExtendFrameIntoClientArea(HWND, const void*) { return 0; }
inline HRESULT DwmEnableBlurBehindWindow(HWND, const void*) { return 0; }
inline HRESULT SetCurrentProcessExplicitAppUserModelID(PCWSTR) { return 0; }
inline int    ImmSetConversionStatus(HWND, DWORD, DWORD) { return 0; }
inline void   PostMessageW(HWND, UINT, WPARAM, LPARAM) {}
inline void   PostQuitMessage(int) {}
inline BOOL   GetClientRect(HWND, void*) { return TRUE; }
inline BOOL   GetWindowRect(HWND, void*) { return TRUE; }
inline BOOL   AdjustWindowRect(void*, DWORD, BOOL) { return TRUE; }
inline BOOL   SetWindowTextW(HWND, const wchar_t*) { return TRUE; }
inline BOOL   MoveWindow(HWND, int, int, int, int, BOOL) { return TRUE; }
inline BOOL   DestroyWindow(HWND) { return TRUE; }
inline LRESULT DefWindowProcW(HWND, UINT, WPARAM, LPARAM) { return 0; }
inline ATOM   RegisterClassExW(const void*) { return 0; }
inline HWND   CreateWindowExW(DWORD, const wchar_t*, const wchar_t*, DWORD, int, int, int, int, HWND, void*, HINSTANCE, void*) { return nullptr; }
inline BOOL   InvalidateRect(HWND, const void*, BOOL) { return TRUE; }
inline BOOL   UpdateWindow(HWND) { return TRUE; }
inline int    MessageBoxW(HWND, const wchar_t*, const wchar_t*, UINT) { return 0; }
inline HDC    GetDC(HWND) { return nullptr; }
inline int    ReleaseDC(HWND, HDC) { return 0; }

// ---- 结构体 stub
struct SECURITY_ATTRIBUTES { DWORD nLength; void* lpSecurityDescriptor; BOOL bInheritHandle; };
struct STARTUPINFOW { DWORD cb; };
struct PROCESS_INFORMATION { HANDLE hProcess; HANDLE hThread; DWORD dwProcessId; DWORD dwThreadId; };
struct OPENFILENAMEW {
    DWORD lStructSize = 0;
    HWND  hwndOwner = nullptr;
    wchar_t* lpstrFile = nullptr;
    DWORD nMaxFile = 0;
    const wchar_t* lpstrFilter = nullptr;
    const wchar_t* lpstrInitialDir = nullptr;
    const wchar_t* lpstrTitle = nullptr;
    DWORD Flags = 0;
};
#define OFN_FILEMUSTEXIST 0x1000
#define OFN_PATHMUSTEXIST 0x0800
#define OFN_EXPLORER      0x80000
#define OFN_NOCHANGEDIR   0x00000008
inline BOOL GetOpenFileNameW(OPENFILENAMEW*) { return FALSE; }
inline BOOL GetSaveFileNameW(OPENFILENAMEW*) { return FALSE; }

#endif
