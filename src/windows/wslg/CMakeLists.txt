set(SOURCES
    main.cpp
    main.rc)

set(HEADERS
    resource.h
    )

add_executable(wslg WIN32 ${SOURCES} ${HEADERS})
add_dependencies(wslg
                 common)

target_link_libraries(wslg
                      ${COMMON_LINK_LIBRARIES}
                      ${MSI_LINK_LIBRARIES}
                      common
                      delayimp.lib)

set_target_properties(wslg PROPERTIES LINK_FLAGS "/DELAYLOAD:msi.dll /DELAYLOAD:WINTRUST.dll")
if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(wslg REUSE_FROM common)
endif()
set_target_properties(wslg PROPERTIES FOLDER windows)