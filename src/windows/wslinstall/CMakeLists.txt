set(SOURCES
    DllMain.cpp
    version.rc)

add_library(wslinstall SHARED ${SOURCES} wslinstall.def)

target_include_directories(wslinstall PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(wslinstall PROPERTIES FOLDER windows)
if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(wslinstall REUSE_FROM common)
endif()

target_link_libraries(wslinstall
                      ${COMMON_LINK_LIBRARIES}
                      ${MSI_LINK_LIBRARIES}
                      common
                      legacy_stdio_definitions
                      Crypt32.lib
                      sfc.lib)