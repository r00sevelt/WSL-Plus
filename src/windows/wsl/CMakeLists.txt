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
else()
    target_compile_options(wsl PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/../common/precomp.h")
endif()
endif()
set_target_properties(wsl PROPERTIES FOLDER windows)