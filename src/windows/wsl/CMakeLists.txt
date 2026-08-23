set(SOURCES
    main.cpp
    main.rc)

set(HEADERS
    resource.h)

add_executable(wsl ${SOURCES} ${HEADERS})

target_link_libraries(wsl
                      ${COMMON_LINK_LIBRARIES}
                      ${MSI_LINK_LIBRARIES}
                      common
                      runtimeobject.lib
                      delayimp.lib)

set_target_properties(wsl PROPERTIES LINK_FLAGS "/DELAYLOAD:msi.dll /DELAYLOAD:WINTRUST.dll")
if (NOT WSL_DISABLE_PCH)
    if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(wsl REUSE_FROM common)
endif()
endif()
set_target_properties(wsl PROPERTIES FOLDER windows)