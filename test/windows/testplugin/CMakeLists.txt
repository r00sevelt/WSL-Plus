set(SOURCES Plugin.cpp)

add_library(testplugin SHARED ${SOURCES})
set_target_properties(testplugin PROPERTIES EXCLUDE_FROM_ALL FALSE)
include_directories(testplugin ${CMAKE_CURRENT_SOURCE_DIR}/..)
target_link_libraries(testplugin ${COMMON_LINK_LIBRARIES} legacy_stdio_definitions common)
if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(testplugin REUSE_FROM common)
endif()