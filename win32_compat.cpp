// win32_compat.cpp - Win32 API Compatibility Layer Implementation
// This file contains the cross-platform implementation of Win32 API functions

#include <unistd.h> // for usleep
#include <stdio.h>  // for printf
#include <iostream>

// ==============================================================================
// PLATFORM-SPECIFIC IMPLEMENTATIONS (continued from header)
// ==============================================================================

#ifndef _WIN32

// Suppress unused parameter warnings for compatibility layer functions
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"

#ifdef __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_OS_MAC
        #define PLATFORM_MACOS_IMPL
        #import <Cocoa/Cocoa.h>
        // Include our header after Cocoa to avoid BOOL conflicts
        #include "win32_compat.h"
    #elif TARGET_OS_IPHONE || TARGET_OS_SIMULATOR
        #define PLATFORM_IOS_IMPL
        #import <UIKit/UIKit.h>
        #include "win32_compat.h"
    #endif
#else
    #include "win32_compat.h"
#endif

#include <map>
#include <string>
#include <vector>
#include <memory>

// Internal structures for emulation
struct WindowData {
    std::string title;
    int x, y, width, height;
    bool visible;
    LRESULT (*wndProc)(HWND, UINT, WPARAM, LPARAM);
    void* platformWindow;
    
    WindowData() : x(0), y(0), width(0), height(0), visible(false), wndProc(nullptr), platformWindow(nullptr) {}
};

struct DeviceContext {
    HWND window;
    void* platformContext;
    
    DeviceContext(HWND w = nullptr) : window(w), platformContext(nullptr) {}
};

// Global state for emulation
static std::map<HWND, std::unique_ptr<WindowData>> g_windows;
static std::map<HDC, std::unique_ptr<DeviceContext>> g_deviceContexts;
static std::map<std::string, LRESULT (*)(HWND, UINT, WPARAM, LPARAM)> g_windowClasses;
static uintptr_t g_nextWindowHandle = 1;
static uintptr_t g_nextDCHandle = 1;
static std::vector<MSG> g_messageQueue;
static bool g_quitPosted = false;

// Forward declarations for platform-specific helpers
void* CreatePlatformWindow(const char* title, int x, int y, int width, int height);
void ShowPlatformWindow(void* window);
void DestroyPlatformWindow(void* window);
void* BeginPlatformPaint(void* window);
void EndPlatformPaint(void* window, void* context);
void DrawPlatformText(void* context, const char* text, int x, int y);
void InvalidatePlatformWindow(void* window);
void ProcessPlatformEvents();

// Win32 API implementations
HWND CreateWindowEx(DWORD dwExStyle, LPCSTR lpClassName, LPCSTR lpWindowName,
                   DWORD dwStyle, int X, int Y, int nWidth, int nHeight,
                   HWND hWndParent, void* hMenu, HINSTANCE hInstance, LPVOID lpParam) {
    
    // Silence unused parameter warnings
    (void)dwExStyle; (void)dwStyle; (void)hWndParent; (void)hMenu; (void)hInstance; (void)lpParam;
    
    HWND hwnd = (HWND)g_nextWindowHandle++;
    auto windowData = std::make_unique<WindowData>();
    windowData->title = lpWindowName ? lpWindowName : "";
    windowData->x = X;
    windowData->y = Y;
    windowData->width = nWidth;
    windowData->height = nHeight;
    
    // Find window procedure
    auto classIt = g_windowClasses.find(lpClassName);
    if (classIt != g_windowClasses.end()) {
        windowData->wndProc = classIt->second;
    }
    
    // Create platform-specific window
    windowData->platformWindow = CreatePlatformWindow(windowData->title.c_str(), X, Y, nWidth, nHeight);
    
    g_windows[hwnd] = std::move(windowData);
    return hwnd;
}

BOOL ShowWindow(HWND hWnd, int nCmdShow) {
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end()) {
        it->second->visible = (nCmdShow != 0);
        if (it->second->visible) {
            ShowPlatformWindow(it->second->platformWindow);
        }
        return TRUE;
    }
    return FALSE;
}

BOOL UpdateWindow(HWND hWnd) {
    // Send WM_PAINT message
    MSG msg = {};
    msg.hwnd = hWnd;
    msg.message = WM_PAINT;
    g_messageQueue.push_back(msg);
    return TRUE;
}

BOOL DestroyWindow(HWND hWnd) {
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end()) {
        DestroyPlatformWindow(it->second->platformWindow);
        g_windows.erase(it);
        return TRUE;
    }
    return FALSE;
}

BOOL InvalidateRect(HWND hWnd, const RECT* lpRect, BOOL bErase) {
    (void)lpRect; (void)bErase; // Silence unused parameter warnings
    
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end()) {
        InvalidatePlatformWindow(it->second->platformWindow);
        // Queue a paint message
        MSG msg = {};
        msg.hwnd = hWnd;
        msg.message = WM_PAINT;
        g_messageQueue.push_back(msg);
        return TRUE;
    }
    return FALSE;
}

BOOL GetClientRect(HWND hWnd, RECT* lpRect) {
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end() && lpRect) {
        lpRect->left = 0;
        lpRect->top = 0;
        lpRect->right = it->second->width;
        lpRect->bottom = it->second->height;
        return TRUE;
    }
    return FALSE;
}

BOOL SetWindowText(HWND hWnd, LPCSTR lpString) {
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end()) {
        it->second->title = lpString ? lpString : "";
        return TRUE;
    }
    return FALSE;
}

HINSTANCE GetModuleHandle(LPCSTR lpModuleName) {
    (void)lpModuleName; // Silence unused parameter warning
    return (HINSTANCE)1; // Dummy handle
}

void* LoadCursor(HINSTANCE hInstance, LPCSTR lpCursorName) {
    (void)hInstance; (void)lpCursorName; // Silence unused parameter warnings
    return (void*)1; // Dummy cursor
}

BOOL RegisterClassEx(const WNDCLASSEX* lpWndClass) {
    if (lpWndClass && lpWndClass->lpszClassName) {
        g_windowClasses[lpWndClass->lpszClassName] = lpWndClass->lpfnWndProc;
        return TRUE;
    }
    return FALSE;
}

BOOL GetMessage(MSG* lpMsg, HWND hWnd, UINT wMsgFilterMin, UINT wMsgFilterMax) {
    (void)hWnd; (void)wMsgFilterMin; (void)wMsgFilterMax; // Silence unused parameter warnings
    
    if (g_quitPosted) {
        return FALSE;
    }
    
    // Process platform-specific events
    ProcessPlatformEvents();
    
    // Process platform messages here
    // For now, return messages from queue
    if (!g_messageQueue.empty()) {
        *lpMsg = g_messageQueue.front();
        g_messageQueue.erase(g_messageQueue.begin());
        return TRUE;
    }
    
    // Create a synthetic paint message to keep the loop alive
    if (g_messageQueue.empty() && !g_windows.empty()) {
        MSG msg = {};
        msg.hwnd = g_windows.begin()->first; // First window
        msg.message = WM_PAINT;
        g_messageQueue.push_back(msg);
    }
    
    // Small delay to prevent busy waiting
    usleep(16000); // ~60 FPS
    return TRUE;
}

BOOL TranslateMessage(const MSG* lpMsg) {
    (void)lpMsg; // Silence unused parameter warning
    return TRUE; // No-op for now
}

LRESULT DispatchMessage(const MSG* lpMsg) {
    if (lpMsg && lpMsg->hwnd) {
        auto it = g_windows.find(lpMsg->hwnd);
        if (it != g_windows.end() && it->second->wndProc) {
            return it->second->wndProc(lpMsg->hwnd, lpMsg->message, lpMsg->wParam, lpMsg->lParam);
        }
    }
    return 0;
}

void PostQuitMessage(int nExitCode) {
    g_quitPosted = true;
    MSG msg = {};
    msg.message = WM_QUIT;
    msg.wParam = nExitCode;
    g_messageQueue.push_back(msg);
}

HDC BeginPaint(HWND hWnd, PAINTSTRUCT* lpPaint) {
    auto it = g_windows.find(hWnd);
    if (it != g_windows.end()) {
        HDC hdc = (HDC)g_nextDCHandle++;
        auto dc = std::make_unique<DeviceContext>(hWnd);
        dc->platformContext = BeginPlatformPaint(it->second->platformWindow);
        g_deviceContexts[hdc] = std::move(dc);
        
        if (lpPaint) {
            lpPaint->hdc = hdc;
            lpPaint->fErase = FALSE;
            GetClientRect(hWnd, &lpPaint->rcPaint);
        }
        
        return hdc;
    }
    return nullptr;
}

BOOL EndPaint(HWND hWnd, const PAINTSTRUCT* lpPaint) {
    if (lpPaint && lpPaint->hdc) {
        auto it = g_deviceContexts.find(lpPaint->hdc);
        if (it != g_deviceContexts.end()) {
            auto windowIt = g_windows.find(hWnd);
            if (windowIt != g_windows.end()) {
                EndPlatformPaint(windowIt->second->platformWindow, it->second->platformContext);
            }
            g_deviceContexts.erase(it);
            return TRUE;
        }
    }
    return FALSE;
}

int DrawText(HDC hdc, LPCSTR lpchText, int cchText, RECT* lpRect, UINT format) {
    auto it = g_deviceContexts.find(hdc);
    if (it != g_deviceContexts.end() && lpchText && lpRect) {
        int len = (cchText == -1) ? strlen(lpchText) : cchText;
        std::string text(lpchText, len);
        
        int x = lpRect->left;
        int y = lpRect->top;
        
        if (format & DT_CENTER) {
            x = lpRect->left + (lpRect->right - lpRect->left) / 2;
        }
        if (format & DT_VCENTER) {
            y = lpRect->top + (lpRect->bottom - lpRect->top) / 2;
        }
        
        DrawPlatformText(it->second->platformContext, text.c_str(), x, y);
        return len;
    }
    return 0;
}

BOOL TextOut(HDC hdc, int x, int y, LPCSTR lpString, int c) {
    auto it = g_deviceContexts.find(hdc);
    if (it != g_deviceContexts.end() && lpString) {
        int len = (c == -1) ? strlen(lpString) : c;
        std::string text(lpString, len);
        DrawPlatformText(it->second->platformContext, text.c_str(), x, y);
        return TRUE;
    }
    return FALSE;
}

BOOL Rectangle(HDC hdc, int left, int top, int right, int bottom) {
    // Stub implementation
    return TRUE;
}

BOOL FillRect(HDC hdc, const RECT* lpRect, HBRUSH hBrush) {
    // Stub implementation
    return TRUE;
}

HFONT CreateFont(int cHeight, int cWidth, int cEscapement, int cOrientation,
                int cWeight, DWORD bItalic, DWORD bUnderline, DWORD bStrikeOut,
                DWORD iCharSet, DWORD iOutPrecision, DWORD iClipPrecision,
                DWORD iQuality, DWORD iPitchAndFamily, LPCSTR pszFaceName) {
    return (HFONT)1; // Dummy font handle
}

HGDIOBJ SelectObject(HDC hdc, HGDIOBJ h) {
    return (HGDIOBJ)1; // Dummy previous object
}

BOOL DeleteObject(HGDIOBJ ho) {
    return TRUE;
}

DWORD SetTextColor(HDC hdc, DWORD color) {
    return 0; // Previous color
}

int SetBkMode(HDC hdc, int mode) {
    return 0; // Previous mode
}

#ifndef _WIN32
// Provide DefWindowProc for non-Windows platforms
LRESULT DefWindowProc(HWND hWnd, UINT Msg, WPARAM wParam, LPARAM lParam) {
    return 0; // Default processing
}
#endif

// ==============================================================================
// PLATFORM-SPECIFIC HELPER IMPLEMENTATIONS
// ==============================================================================

#ifdef PLATFORM_MACOS_IMPL

// Custom NSView subclass for text rendering
@interface CustomTextView : NSView
@property (nonatomic, strong) NSString* textToRender;
@end

@implementation CustomTextView

- (void)drawRect:(NSRect)dirtyRect {
    [super drawRect:dirtyRect];
    
    if (self.textToRender) {
        // Clear the background
        [[NSColor whiteColor] setFill];
        NSRectFill(dirtyRect);
        
        // Create drawing attributes
        NSDictionary* attributes = @{
            NSFontAttributeName: [NSFont systemFontOfSize:24.0],
            NSForegroundColorAttributeName: [NSColor blackColor]
        };
        
        // Calculate text size and center it
        NSSize textSize = [self.textToRender sizeWithAttributes:attributes];
        NSRect bounds = [self bounds];
        NSPoint drawPoint = NSMakePoint(
            (bounds.size.width - textSize.width) / 2.0,
            (bounds.size.height - textSize.height) / 2.0
        );
        
        // Draw the text
        [self.textToRender drawAtPoint:drawPoint withAttributes:attributes];
    }
}

@end

// Global storage for text to be rendered
static NSMutableDictionary* g_windowTexts = nil;

void ProcessPlatformEvents() {
    @autoreleasepool {
        NSEvent* event;
        while ((event = [NSApp nextEventMatchingMask:NSEventMaskAny
                                           untilDate:[NSDate distantPast]
                                              inMode:NSDefaultRunLoopMode
                                             dequeue:YES])) {
            [NSApp sendEvent:event];
        }
    }
}

// Setup macOS menu bar
void SetupMacOSMenuBar() {
    @autoreleasepool {
        // Create the main menu bar
        NSMenu* mainMenu = [[NSMenu alloc] initWithTitle:@"MainMenu"];
        
        // Create the application menu (first menu item)
        NSMenuItem* appMenuItem = [[NSMenuItem alloc] init];
        NSMenu* appMenu = [[NSMenu alloc] initWithTitle:@"Multiverse32"];
        
        // About menu item
        NSMenuItem* aboutMenuItem = [[NSMenuItem alloc] initWithTitle:@"About Multiverse32"
                                                               action:@selector(orderFrontStandardAboutPanel:)
                                                        keyEquivalent:@""];
        [aboutMenuItem setTarget:NSApp];
        [appMenu addItem:aboutMenuItem];
        
        [appMenu addItem:[NSMenuItem separatorItem]];
        
        // Services menu
        NSMenuItem* servicesMenuItem = [[NSMenuItem alloc] initWithTitle:@"Services"
                                                                  action:nil
                                                           keyEquivalent:@""];
        NSMenu* servicesMenu = [[NSMenu alloc] initWithTitle:@"Services"];
        [servicesMenuItem setSubmenu:servicesMenu];
        [NSApp setServicesMenu:servicesMenu];
        [appMenu addItem:servicesMenuItem];
        
        [appMenu addItem:[NSMenuItem separatorItem]];
        
        // Hide application
        NSMenuItem* hideMenuItem = [[NSMenuItem alloc] initWithTitle:@"Hide Multiverse32"
                                                              action:@selector(hide:)
                                                       keyEquivalent:@"h"];
        [hideMenuItem setTarget:NSApp];
        [appMenu addItem:hideMenuItem];
        
        // Hide others
        NSMenuItem* hideOthersMenuItem = [[NSMenuItem alloc] initWithTitle:@"Hide Others"
                                                                    action:@selector(hideOtherApplications:)
                                                             keyEquivalent:@"h"];
        [hideOthersMenuItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand | NSEventModifierFlagOption];
        [hideOthersMenuItem setTarget:NSApp];
        [appMenu addItem:hideOthersMenuItem];
        
        // Show all
        NSMenuItem* showAllMenuItem = [[NSMenuItem alloc] initWithTitle:@"Show All"
                                                                 action:@selector(unhideAllApplications:)
                                                          keyEquivalent:@""];
        [showAllMenuItem setTarget:NSApp];
        [appMenu addItem:showAllMenuItem];
        
        [appMenu addItem:[NSMenuItem separatorItem]];
        
        // Quit application
        NSMenuItem* quitMenuItem = [[NSMenuItem alloc] initWithTitle:@"Quit Multiverse32"
                                                              action:@selector(terminate:)
                                                       keyEquivalent:@"q"];
        [quitMenuItem setTarget:NSApp];
        [appMenu addItem:quitMenuItem];
        
        [appMenuItem setSubmenu:appMenu];
        [mainMenu addItem:appMenuItem];
        
        // Create Window menu
        NSMenuItem* windowMenuItem = [[NSMenuItem alloc] initWithTitle:@"Window"
                                                                action:nil
                                                         keyEquivalent:@""];
        NSMenu* windowMenu = [[NSMenu alloc] initWithTitle:@"Window"];
        
        // Minimize
        NSMenuItem* minimizeMenuItem = [[NSMenuItem alloc] initWithTitle:@"Minimize"
                                                                  action:@selector(performMiniaturize:)
                                                           keyEquivalent:@"m"];
        [windowMenu addItem:minimizeMenuItem];
        
        // Zoom
        NSMenuItem* zoomMenuItem = [[NSMenuItem alloc] initWithTitle:@"Zoom"
                                                              action:@selector(performZoom:)
                                                       keyEquivalent:@""];
        [windowMenu addItem:zoomMenuItem];
        
        [windowMenu addItem:[NSMenuItem separatorItem]];
        
        // Bring All to Front
        NSMenuItem* bringAllToFrontMenuItem = [[NSMenuItem alloc] initWithTitle:@"Bring All to Front"
                                                                         action:@selector(arrangeInFront:)
                                                                  keyEquivalent:@""];
        [bringAllToFrontMenuItem setTarget:NSApp];
        [windowMenu addItem:bringAllToFrontMenuItem];
        
        [windowMenuItem setSubmenu:windowMenu];
        [mainMenu addItem:windowMenuItem];
        
        // Set the window menu for the application
        [NSApp setWindowsMenu:windowMenu];
        
        // Set the main menu
        [NSApp setMainMenu:mainMenu];
        
        printf("Menu bar setup completed\n");
    }
}

void* CreatePlatformWindow(const char* title, int x, int y, int width, int height) {
    @autoreleasepool {
        printf("=== CreatePlatformWindow Debug ===\n");
        printf("Title: %s\n", title ? title : "(null)");
        printf("Position: (%d, %d)\n", x, y);
        printf("Size: %dx%d\n", width, height);
        
        // Initialize global text storage if needed
        if (!g_windowTexts) {
            g_windowTexts = [[NSMutableDictionary alloc] init];
        }
        
        // Initialize NSApplication if needed
        printf("Checking NSApp state...\n");
        if (![NSApp isRunning]) {
            printf("NSApp not running, initializing...\n");
            [NSApplication sharedApplication];
            printf("Created shared application\n");
            
            // Set activation policy to regular (shows in dock)
            [NSApp setActivationPolicy:NSApplicationActivationPolicyRegular];
            printf("Set activation policy to regular\n");
            
            // Setup menu bar
            SetupMacOSMenuBar();
            printf("Setup menu bar\n");
            
            // Finish launching
            [NSApp finishLaunching];
            printf("Finished launching NSApp\n");
        } else {
            printf("NSApp already running\n");
        }
        
        printf("Creating window frame...\n");
        NSRect frame = NSMakeRect(x, y, width, height);
        printf("Frame: origin(%.1f, %.1f) size(%.1f, %.1f)\n", 
               frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
        
        printf("Allocating NSWindow...\n");
        NSWindow* window = [[NSWindow alloc] initWithContentRect:frame
                                                       styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable | NSWindowStyleMaskResizable
                                                         backing:NSBackingStoreBuffered
                                                           defer:NO];
        
        if (window) {
            printf("NSWindow created successfully: %p\n", (__bridge void*)window);
            
            printf("Setting window title...\n");
            [window setTitle:[NSString stringWithUTF8String:title]];
            
            // Create and set custom content view
            printf("Creating custom content view...\n");
            CustomTextView* customView = [[CustomTextView alloc] initWithFrame:frame];
            [window setContentView:customView];
            printf("Set custom content view\n");
            
            printf("Centering window...\n");
            [window center];
            
            NSRect finalFrame = [window frame];
            printf("Final frame: origin(%.1f, %.1f) size(%.1f, %.1f)\n", 
                   finalFrame.origin.x, finalFrame.origin.y, finalFrame.size.width, finalFrame.size.height);
            
            printf("Window visible: %s\n", [window isVisible] ? "YES" : "NO");
            printf("Window key: %s\n", [window isKeyWindow] ? "YES" : "NO");
            printf("Window main: %s\n", [window isMainWindow] ? "YES" : "NO");
            
        } else {
            printf("ERROR: Failed to create NSWindow!\n");
        }
        
        printf("Returning bridge-retained pointer: %p\n", (__bridge void*)window);
        printf("=== End CreatePlatformWindow Debug ===\n\n");
        
        return (__bridge_retained void*)window;
    }
}

void ShowPlatformWindow(void* window) {
    @autoreleasepool {
        printf("=== ShowPlatformWindow Debug ===\n");
        printf("Window pointer: %p\n", window);
        
        NSWindow* nsWindow = (__bridge NSWindow*)window;
        if (nsWindow) {
            printf("NSWindow object: %p\n", (__bridge void*)nsWindow);
            printf("Window title: %s\n", [[nsWindow title] UTF8String]);
            printf("Before show - visible: %s\n", [nsWindow isVisible] ? "YES" : "NO");
            
            [nsWindow makeKeyAndOrderFront:nil];
            
            // Activate the application to bring it to front
            [NSApp activateIgnoringOtherApps:YES];
            
            printf("After makeKeyAndOrderFront - visible: %s\n", [nsWindow isVisible] ? "YES" : "NO");
            printf("Window key: %s\n", [nsWindow isKeyWindow] ? "YES" : "NO");
            printf("Window main: %s\n", [nsWindow isMainWindow] ? "YES" : "NO");
        } else {
            printf("ERROR: NSWindow is null!\n");
        }
        printf("=== End ShowPlatformWindow Debug ===\n\n");
    }
}

void DestroyPlatformWindow(void* window) {
    @autoreleasepool {
        NSWindow* nsWindow = (__bridge_transfer NSWindow*)window;
        [nsWindow close];
    }
}

void* BeginPlatformPaint(void* window) {
    @autoreleasepool {
        NSWindow* nsWindow = (__bridge NSWindow*)window;
        NSView* contentView = [nsWindow contentView];
        return (__bridge_retained void*)contentView;
    }
}

void EndPlatformPaint(void* window, void* context) {
    @autoreleasepool {
        NSView* view = (__bridge_transfer NSView*)context;
        [view setNeedsDisplay:YES];
    }
}

void DrawPlatformText(void* context, const char* text, int x, int y) {
    @autoreleasepool {
        NSView* view = (__bridge NSView*)context;
        NSString* nsText = [NSString stringWithUTF8String:text];
        
        // Check if this is our custom text view
        if ([view isKindOfClass:[CustomTextView class]]) {
            CustomTextView* customView = (CustomTextView*)view;
            customView.textToRender = nsText;
            [customView setNeedsDisplay:YES];
            printf("Set text '%s' on custom view and triggered redraw\n", text);
        } else {
            // Fallback for other view types
            [view setNeedsDisplay:YES];
            printf("Triggered redraw on standard view for text '%s'\n", text);
        }
    }
}

void InvalidatePlatformWindow(void* window) {
    @autoreleasepool {
        NSWindow* nsWindow = (__bridge NSWindow*)window;
        [[nsWindow contentView] setNeedsDisplay:YES];
    }
}

#elif defined(PLATFORM_IOS_IMPL)

// iOS implementations would go here
// Similar structure but using UIKit instead of Cocoa

void ProcessPlatformEvents() {
    // Stub for non-MacOS platforms
}

void* CreatePlatformWindow(const char* title, int x, int y, int width, int height) {
    printf("=== CreatePlatformWindow Debug ===\n");
    printf("Title: %s\n", title ? title : "(null)");
    printf("Position: (%d, %d)\n", x, y);
    printf("Size: %dx%d\n", width, height);

    @autoreleasepool {
        UIWindow* window = [[UIWindow alloc] initWithFrame:CGRectMake(x, y, width, height)];
        if (window) {
            printf("UIWindow created successfully: %p\n", (__bridge void*)window);
            [window setBackgroundColor:[UIColor whiteColor]];
            [window setHidden:NO];
            [window setRootViewController:[[UIViewController alloc] init]];
        } else {
            printf("ERROR: Failed to create UIWindow!\n");
        }
        return (__bridge_retained void*)window;
    }
}

void ShowPlatformWindow(void* window) {
    @autoreleasepool {
        UIWindow* uiWindow = (__bridge UIWindow*)window;
        [uiWindow makeKeyAndVisible];
    }
}

void DestroyPlatformWindow(void* window) {
    @autoreleasepool {
        UIWindow* uiWindow = (__bridge_transfer UIWindow*)window;
        [uiWindow setHidden:YES];
    }
}

void* BeginPlatformPaint(void* window) {
    @autoreleasepool {
        UIWindow* uiWindow = (__bridge UIWindow*)window;
        return (__bridge_retained void*)uiWindow;
    }
}

void EndPlatformPaint(void* window, void* context) {
    @autoreleasepool {
        UIWindow* uiWindow = (__bridge_transfer UIWindow*)context;
        [uiWindow setNeedsDisplay];
    }
}

void DrawPlatformText(void* context, const char* text, int x, int y) {
    @autoreleasepool {
        // iOS text drawing would require a graphics context
        // This is a placeholder implementation
        UIWindow* uiWindow = (__bridge UIWindow*)context;
        // Would need proper Core Graphics implementation
    }
}

void InvalidatePlatformWindow(void* window) {
    @autoreleasepool {
        UIWindow* uiWindow = (__bridge UIWindow*)window;
        [uiWindow setNeedsDisplay];
    }
}

#else

// Default implementations for other platforms

void ProcessPlatformEvents() {
    // Stub
}

void* CreatePlatformWindow(const char* title, int x, int y, int width, int height) {
    return (void*)9999; // Stub
}

void ShowPlatformWindow(void* window) {
    // Stub
}

void DestroyPlatformWindow(void* window) {
    // Stub
}

void* BeginPlatformPaint(void* window) {
    return (void*)1; // Stub
}

void EndPlatformPaint(void* window, void* context) {
    // Stub
}

void DrawPlatformText(void* context, const char* text, int x, int y) {
    // Stub - would need platform-specific text rendering
}

void InvalidatePlatformWindow(void* window) {
    // Stub
}

#endif

#pragma GCC diagnostic pop // Restore warning settings


// Cross-platform main function
#ifndef _WIN32
int main(int argc, char* argv[]) {
    return WinMain(GetModuleHandle(NULL), NULL, NULL, SW_SHOW);
}
#endif


#endif // !_WIN32
