// win32_compat.h - Win32 API Compatibility Layer
#pragma once

#include <iostream>
 
#ifdef _WIN32
    // On Windows, use real Win32 API
    #include <windows.h>
    #include <windowsx.h>
#else
    // On other platforms, provide Win32 API emulation
    #include <stdint.h>
    #include <string.h>
    
    // Win32 type definitions
    typedef void* HWND;
    typedef void* HINSTANCE;
    typedef void* HDC;
    typedef void* HFONT;
    typedef void* HBRUSH;
    typedef void* HPEN;
    typedef void* HGDIOBJ;
    typedef unsigned int UINT;
    typedef intptr_t LONG_PTR;
    typedef uintptr_t UINT_PTR;
    typedef UINT_PTR WPARAM;
    typedef LONG_PTR LPARAM;
    typedef LONG_PTR LRESULT;
    typedef unsigned long DWORD;
    typedef const char* LPCSTR;
    typedef char* LPSTR;
    typedef void* LPVOID;
    
    // Handle BOOL conflict with Objective-C on Apple platforms
    #ifdef __OBJC__
    // BOOL is already defined by Objective-C as 'signed char', we'll use our own type
    typedef int WIN32_BOOL;
    #define BOOL WIN32_BOOL
    #else
    typedef int BOOL;
    #endif
    
    #define TRUE 1
    #define FALSE 0
    #ifndef NULL
    #define NULL 0
    #endif
    
    // Win32 constants
    #define WM_PAINT 0x000F
    #define WM_CLOSE 0x0010
    #define WM_DESTROY 0x0002
    #define WM_SIZE 0x0005
    #define WM_KEYDOWN 0x0100
    #define WM_KEYUP 0x0101
    #define WM_LBUTTONDOWN 0x0201
    #define WM_LBUTTONUP 0x0202
    #define WM_MOUSEMOVE 0x0200
    #define WM_QUIT 0x0012
    
    #define WS_OVERLAPPEDWINDOW 0x00CF0000L
    #define CS_HREDRAW 0x0002
    #define CS_VREDRAW 0x0001
    #define COLOR_WINDOW 5
    #define SW_SHOW 5
    #define IDC_ARROW 32512
    #define DT_SINGLELINE 0x00000020
    #define DT_CENTER 0x00000001
    #define DT_VCENTER 0x00000004
    
    // Resource handling macros
    #define MAKEINTRESOURCE(i) ((LPCSTR)((uintptr_t)((unsigned short)(i))))
    #define IS_INTRESOURCE(r) ((((uintptr_t)(r)) >> 16) == 0)

    // Win32 structures
    typedef struct {
        UINT cbSize;
        UINT style;
        LRESULT (*lpfnWndProc)(HWND, UINT, WPARAM, LPARAM);
        int cbClsExtra;
        int cbWndExtra;
        HINSTANCE hInstance;
        void* hIcon;
        void* hCursor;
        HBRUSH hbrBackground;
        LPCSTR lpszMenuName;
        LPCSTR lpszClassName;
        void* hIconSm;
    } WNDCLASSEX;
    
    typedef struct {
        int left;
        int top;
        int right;
        int bottom;
    } RECT;
    
    typedef struct {
        HDC hdc;
        BOOL fErase;
        RECT rcPaint;
        BOOL fRestore;
        BOOL fIncUpdate;
        unsigned char rgbReserved[32];
    } PAINTSTRUCT;
    
    typedef struct {
        HWND hwnd;
        UINT message;
        WPARAM wParam;
        LPARAM lParam;
        DWORD time;
        struct { int x, y; } pt;
    } MSG;
    
    // Win32 API function declarations
    HWND CreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
                       DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                       HWND hWndParent, void* hMenu, HINSTANCE hInstance, LPVOID lpParam);
    
    BOOL ShowWindow(HWND hWnd, int nCmdShow);
    BOOL UpdateWindow(HWND hWnd);
    BOOL DestroyWindow(HWND hWnd);
    BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase);
    BOOL GetClientRect(HWND hWnd, RECT* lpRect);
    BOOL SetWindowText(HWND hWnd, LPCSTR lpString);
    
    HINSTANCE GetModuleHandle(LPCSTR lpModuleName);
    void* LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName);
    BOOL RegisterClassEx(const WNDCLASSEX* lpWndClass);
    
    BOOL GetMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax);
    BOOL TranslateMessage(const MSG* lpMsg);
    LRESULT DispatchMessage(const MSG* lpMsg);
    void PostQuitMessage(int nExitCode);
    
    HDC BeginPaint(HWND hWnd, PAINTSTRUCT* lpPaint);
    BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint);
    
    int DrawText(HDC hdc, LPCSTR lpchText, int cchText, RECT* lpRect, UINT format);
    BOOL TextOut(HDC hdc, int x, int y, LPCSTR lpString, int c);
    BOOL Rectangle(HDC hdc, int left, int top, int right, int bottom);
    BOOL FillRect(HDC hdc, const RECT* lpRect, HBRUSH hBrush);
    
    HFONT CreateFont(int cHeight, int cWidth, int cEscapement, int cOrientation,
                    int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut,
                    DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
                    DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName);
    
    HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h);
    BOOL DeleteObject(HGDIOBJ ho);
    
    DWORD SetTextColor(HDC hdc, DWORD color);
    int SetBkMode(HDC hdc, int mode);
    
    LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam);
    
    #define TRANSPARENT 1
    #define RGB(r,g,b) ((DWORD)(((unsigned char)(r)|((unsigned short)((unsigned char)(g))<<8))|(((DWORD)(unsigned char)(b))<<16)))
    
    // Win32 callback function type
    #define CALLBACK
    #define WINAPI
    
#endif // !_WIN32
