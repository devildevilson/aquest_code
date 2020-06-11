#ifndef SHARED_APPLICATION_CONSTANTS_H
#define SHARED_APPLICATION_CONSTANTS_H

// версию лучше сделать size_t
#define MAKE_VERSION(major, minor, patch) \
((major << 22) | (minor << 12) | patch)

#define VERSION_TO_STR(ver) \
(std::to_string((ver >> 22) & 0xffc) + "." + \
 std::to_string((ver >> 12) & 0x3ff) + "." + \
 std::to_string(ver & 0xfff))

// возможно эти константы придут из cmake
#define APPLICATION_NAME "VulkanTest"
#define ENGINE_NAME "test_engine"

#define EGINE_VERSION MAKE_VERSION(0, 0, 1)
//#define APP_VERSION   MAKE_VERSION(0, 0, 1)

#define ENGINE_VERSION_STR VERSION_TO_STR(EGINE_VERSION)
#define APP_VERSION_STR    VERSION_TO_STR(APP_VERSION)

#endif
