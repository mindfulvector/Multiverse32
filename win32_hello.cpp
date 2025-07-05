// win32_hello.cpp - Hello World application using Win32 API
#include "win32_compat.h"
#include <iostream>

// Window procedure
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Set up text drawing
            SetBkMode(hdc, TRANSPARENT);
            SetTextColor(hdc, RGB(0, 0, 0));
            
            // Get client rectangle
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Draw "Hello World!" centered in the window
            DrawText(hdc, "Hello World!", -1, &rect, DT_SINGLELINE | DT_CENTER | DT_VCENTER);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
        
        case WM_CLOSE:
            DestroyWindow(hwnd);
            return 0;
            
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    
#ifdef _WIN32
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
#else
    return 0; // Default processing for non-Windows platforms
#endif
}

// Main application entry point
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    std::cout << "Hello World - Cross Platform Win32" << std::endl;

    // Register window class
    WNDCLASSEX wc = {};
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    std::cout << "LoadCursor..." << std::endl;
    wc.hCursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "HelloWorldWindowClass";
    
    std::cout << "Registering class..." << std::endl;
    if (!RegisterClassEx(&wc)) {
        return -1;
    }
    
    // Create window
    std::cout << "Creating window..." << std::endl;
    HWND hwnd = CreateWindowEx(
        0,
        "HelloWorldWindowClass",
        "Hello World - Cross Platform Win32",
        WS_OVERLAPPEDWINDOW,
        300, 300, 500, 400,
        NULL, NULL, hInstance, NULL
    );
    std::cout << "HWND: " << hwnd << std::endl;
    
    if (!hwnd) {
        std::cout << "Failed to create window." << std::endl;
        return -1;
    }
    
    // Show window
    std::cout << "Showing window..." << std::endl;
    if(!ShowWindow(hwnd, nCmdShow)) {
        std::cout << "Failed to show window." << std::endl;
        DestroyWindow(hwnd);
        return -1;
    } else {
        std::cout << "Window shown successfully." << std::endl;
    }
    std::cout << "Updating window..." << std::endl;
    if(!UpdateWindow(hwnd)) {
        std::cout << "Failed to update window." << std::endl;
        DestroyWindow(hwnd);
        return -1;
    } else {
        std::cout << "Window updated successfully." << std::endl;
    }
    
    // Message loop
    MSG msg = {};
    std::cout << "Entering message loop..." << std::endl;
    while (GetMessage(&msg, NULL, 0, 0)) {
        //std::cout << "Message received: " << msg.message << std::endl;
        TranslateMessage(&msg);
        //std::cout << "Dispatching message..." << std::endl;
        DispatchMessage(&msg);
    }
    
    std::cout << "Exiting message loop." << std::endl;
    std::cout << "Result: " << msg.wParam << std::endl;
    std::cout << "Goodbye!" << std::endl;
    return (int)msg.wParam;
}
