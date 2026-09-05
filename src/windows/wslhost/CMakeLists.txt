set(SOURCES
    main.cpp
    main.rc)

set(HEADERS
    resource.h)

add_executable(wslhost WIN32 ${SOURCES} ${HEADERS})
add_dependencies(wslhost
                 common)

target_link_libraries(wslhost
                      ${COMMON_LINK_LIBRARIES}
                      common
                      runtimeobject.lib
                      delayimp.lib)

set_target_properties(wslhost PROPERTIES LINK_FLAGS "/DELAYLOAD:msi.dll /DELAYLOAD:WINTRUST.dll")

if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(wslhost REUSE_FROM common)
endif()
set_target_properties(wslhost PROPERTIES FOLDER windows)