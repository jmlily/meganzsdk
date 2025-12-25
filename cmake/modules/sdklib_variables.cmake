# No configurable values.
set(USE_SQLITE 1)
set(USE_SODIUM 1)
set(USE_CRYPTOPP 1)

if (NOT WIN32)
    set(USE_PTHREAD 1)
    set(USE_CPPTHREAD 0)
else()
    set(USE_CPPTHREAD 1)
    # Only set toolset for Visual Studio generators, allowing automatic selection of appropriate version
if(CMAKE_GENERATOR MATCHES "Visual Studio")
    # Allow CMake to automatically select the appropriate platform toolset
    # This avoids forcing v142 when using VS2022 which supports v143
endif()

# Add compiler flags to handle code page issues and nodiscard warnings
if(MSVC)
    # Disable C4819 warning (code page issues)
    add_compile_options(/wd4819)
    # Disable C4834 warning (discarded nodiscard return values)
    add_compile_options(/wd4834)
endif()
endif()

include(detect_host_architecture)

# Configure CMAKE_OSX_DEPLOYMENT_TARGET if not already set
include(set_osx_deployment_target)
set_osx_deployment_target(
    ARM64 "11.1"
    x86_64 "10.15"
)

if (CMAKE_SYSTEM_NAME STREQUAL "Android")
    # Ensure that compatibility with Android devices that use a 16KiB page size is enabled
    set(ANDROID_SUPPORT_FLEXIBLE_PAGE_SIZES ON)
    set(CMAKE_SYSTEM_VERSION 26)
endif()
