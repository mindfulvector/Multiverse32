# CMakeLists.txt - Cross-Platform Win32 Compatibility Layer
cmake_minimum_required(VERSION 3.16)

# Project configuration
project(Win32HelloWorld 
    VERSION 1.0.0
    DESCRIPTION "Cross-platform Win32 API compatibility layer Hello World"
    LANGUAGES CXX
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Platform detection
if(WIN32)
    set(PLATFORM_NAME "Windows")
    set(PLATFORM_WINDOWS TRUE)
elseif(APPLE)
    set(PLATFORM_NAME "Apple")
    set(PLATFORM_APPLE TRUE)
    # Detect specific Apple platform
    if(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
        if(CMAKE_OSX_SYSROOT MATCHES ".*iphoneos.*")
            set(PLATFORM_IOS TRUE)
            set(PLATFORM_NAME "iOS")
        elseif(CMAKE_OSX_SYSROOT MATCHES ".*iphonesimulator.*")
            set(PLATFORM_IOS_SIMULATOR TRUE)
            set(PLATFORM_NAME "iOS Simulator")
        else()
            set(PLATFORM_MACOS TRUE)
            set(PLATFORM_NAME "macOS")
        endif()
    endif()
elseif(UNIX)
    set(PLATFORM_LINUX TRUE)
    set(PLATFORM_NAME "Linux")
else()
    set(PLATFORM_NAME "Unknown")
endif()

message(STATUS "Building for platform: ${PLATFORM_NAME}")

# Source files
set(SOURCES
    win32_hello.cpp
)

# Headers
set(HEADERS
    win32_compat.h
)

# Create executable
if(PLATFORM_WINDOWS)
    # Windows executable with Win32 subsystem
    add_executable(${PROJECT_NAME} WIN32 ${SOURCES} ${HEADERS})
else()
    # Regular executable for other platforms
    add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
endif()

# Platform-specific configurations
if(PLATFORM_WINDOWS)
    # Windows-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        WIN32_LEAN_AND_MEAN
        NOMINMAX
        UNICODE
        _UNICODE
    )
    
    # Link Windows libraries
    target_link_libraries(${PROJECT_NAME} PRIVATE
        user32
        gdi32
        kernel32
    )
    
    # Set Windows-specific compiler flags
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /W4          # Warning level 4
            /WX          # Treat warnings as errors
            /permissive- # Disable non-conforming code
        )
        
        # Set subsystem for MSVC
        set_target_properties(${PROJECT_NAME} PROPERTIES
            LINK_FLAGS "/SUBSYSTEM:WINDOWS"
        )
    elseif(MINGW)
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall
            -Wextra
            -Werror
        )
        
        # MinGW subsystem setting
        target_link_options(${PROJECT_NAME} PRIVATE
            -mwindows
        )
    endif()

elseif(PLATFORM_MACOS)
    # macOS-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLATFORM_MACOS
    )
    
    # Enable Objective-C++ compilation
    set_source_files_properties(${SOURCES} PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
    
    # Link macOS frameworks
    find_library(COCOA_FRAMEWORK Cocoa REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${COCOA_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
    )
    
    # Create macOS app bundle
    set_target_properties(${PROJECT_NAME} PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.multiverse.win32hello"
        MACOSX_BUNDLE_BUNDLE_NAME "Multiverse32"
        MACOSX_BUNDLE_DISPLAY_NAME "Multiverse32"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0.0"
        MACOSX_BUNDLE_COPYRIGHT "Copyright © 2025 Multiverse. All rights reserved."
        MACOSX_BUNDLE_ICON_FILE "Multiverse32"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
        MACOSX_DEPLOYMENT_TARGET "10.14"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
    )
    
    # Copy icon to app bundle
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Multiverse32.icns")
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Multiverse32.icns" 
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Multiverse32.icns")
    endif()
    
    # Copy Info.plist to app bundle
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist")
        set_source_files_properties("${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist" 
            PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
        target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist")
    endif()
    
    # Compiler flags for macOS
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter  # Allow unused parameters in compatibility layer
        -Wno-macro-redefined   # Allow macro redefinitions for compatibility
        -fobjc-arc  # Enable ARC for Objective-C++
    )

elseif(PLATFORM_IOS OR PLATFORM_IOS_SIMULATOR)
    # iOS-specific settings
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLATFORM_IOS
    )
    
    # Enable Objective-C++ compilation
    set_source_files_properties(${SOURCES} PROPERTIES
        COMPILE_FLAGS "-x objective-c++"
    )
    
    # Link iOS frameworks
    find_library(UIKIT_FRAMEWORK UIKit REQUIRED)
    find_library(FOUNDATION_FRAMEWORK Foundation REQUIRED)
    find_library(QUARTZCORE_FRAMEWORK QuartzCore REQUIRED)
    
    target_link_libraries(${PROJECT_NAME} PRIVATE
        ${UIKIT_FRAMEWORK}
        ${FOUNDATION_FRAMEWORK}
        ${QUARTZCORE_FRAMEWORK}
    )
    
    # Set iOS deployment target
    set_target_properties(${PROJECT_NAME} PROPERTIES
        XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET "12.0"
        XCODE_ATTRIBUTE_CLANG_ENABLE_OBJC_ARC YES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer"
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.example.win32hello"
        MACOSX_BUNDLE_BUNDLE_NAME "Win32 Hello World"
        MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
    )
    
    # iOS compiler flags
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter  # Allow unused parameters in compatibility layer
        -fobjc-arc
    )

elseif(PLATFORM_LINUX)
    # Linux-specific settings (fallback implementation)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        PLATFORM_LINUX
    )
    
    # Find X11 for potential Linux GUI implementation
    find_package(X11)
    if(X11_FOUND)
        target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIR})
        target_link_libraries(${PROJECT_NAME} PRIVATE ${X11_LIBRARIES})
        target_compile_definitions(${PROJECT_NAME} PRIVATE HAVE_X11)
    endif()
    
    # Compiler flags for Linux
    target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall
        -Wextra
        -Wno-unused-parameter  # Allow unused parameters in compatibility layer
    )
    
    # Link pthread for threading support
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)

endif()

# Cross-platform compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:GNU>:-fno-rtti>
    $<$<CXX_COMPILER_ID:Clang>:-fno-rtti>
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Debug/Release configurations
target_compile_definitions(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:DEBUG _DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

# Optimization flags
target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CONFIG:Debug>:-O0 -g>
    $<$<CONFIG:Release>:-O3 -DNDEBUG>
)

# Install configuration
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION bin
)

# Create separate targets for cross-compilation
if(PLATFORM_APPLE AND NOT PLATFORM_IOS)
    # macOS target
    add_custom_target(macos
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --config $<CONFIG>
        COMMENT "Building for macOS"
    )
    
    # iOS targets (only if Xcode is properly configured)
    find_program(XCODEBUILD_EXECUTABLE xcodebuild)
    if(XCODEBUILD_EXECUTABLE)
        execute_process(
            COMMAND ${XCODEBUILD_EXECUTABLE} -showsdks
            OUTPUT_VARIABLE XCODE_SDKS
            ERROR_QUIET
        )
        
        if(XCODE_SDKS MATCHES "iphoneos")
            # iOS device target
            add_custom_target(ios-device
                COMMAND ${CMAKE_COMMAND} 
                    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/ios.toolchain.cmake
                    -DPLATFORM=OS64
                    -DCMAKE_OSX_ARCHITECTURES=arm64
                    -S ${CMAKE_CURRENT_SOURCE_DIR}
                    -B ${CMAKE_BINARY_DIR}/ios-device
                COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/ios-device --config $<CONFIG>
                COMMENT "Building for iOS device"
            )
            
            # iOS simulator target
            add_custom_target(ios-simulator
                COMMAND ${CMAKE_COMMAND}
                    -DCMAKE_TOOLCHAIN_FILE=${CMAKE_CURRENT_SOURCE_DIR}/cmake/ios.toolchain.cmake
                    -DPLATFORM=SIMULATOR64
                    -DCMAKE_OSX_ARCHITECTURES=x86_64
                    -S ${CMAKE_CURRENT_SOURCE_DIR}
                    -B ${CMAKE_BINARY_DIR}/ios-simulator
                COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}/ios-simulator --config $<CONFIG>
                COMMENT "Building for iOS simulator"
            )
        else()
            message(STATUS "iOS SDK not found - iOS targets will not be available")
            message(STATUS "To enable iOS development, install Xcode with iOS support")
        endif()
    endif()
endif()

# Package configuration
set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_VENDOR "Example Corp")

if(PLATFORM_WINDOWS)
    set(CPACK_GENERATOR "ZIP;NSIS")
    set(CPACK_NSIS_DISPLAY_NAME "Win32 Hello World")
    set(CPACK_NSIS_PACKAGE_NAME "Win32HelloWorld")
elseif(PLATFORM_APPLE)
    set(CPACK_GENERATOR "DragNDrop;ZIP")
    set(CPACK_DMG_FORMAT "UDZO")
elseif(PLATFORM_LINUX)
    set(CPACK_GENERATOR "TGZ;DEB;RPM")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "example@example.com")
    set(CPACK_RPM_PACKAGE_SUMMARY "${PROJECT_DESCRIPTION}")
endif()

include(CPack)

# Print configuration summary
message(STATUS "=== Build Configuration Summary ===")
message(STATUS "Platform: ${PLATFORM_NAME}")
message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "Source directory: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "Binary directory: ${CMAKE_BINARY_DIR}")
if(PLATFORM_APPLE)
    message(STATUS "macOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    message(STATUS "iOS deployment target: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif()
message(STATUS "===================================")

# Development helper targets
add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${CMAKE_BINARY_DIR}
    COMMENT "Cleaning all build artifacts"
)

add_custom_target(rebuild
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target clean
    COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR}
    COMMENT "Clean rebuild"
)
