set(SOURCES
    APICompat.cpp
    ConsoleProgressBar.cpp
    ConsoleProgressIndicator.cpp
    ConsoleState.cpp
    DeviceHostProxy.cpp
    disk.cpp
    Distribution.cpp
    Dmesg.cpp
    DnsResolver.cpp
    DnsTunnelingChannel.cpp
    ExecutionContext.cpp
    filesystem.cpp
    GnsChannel.cpp
    GnsPortTrackerChannel.cpp
    GuestDeviceManager.cpp
    HandleConsoleProgressBar.cpp
    hcs.cpp
    helpers.cpp
    hvsocket.cpp
    interop.cpp
    HandleIO.cpp
    Localization.cpp
    lxssbusclient.cpp
    lxssclient.cpp
    LxssMessagePort.cpp
    LxssSecurity.cpp
    LxssServerPort.cpp
    MountSpecParsing.cpp
    NatNetworking.cpp
    notifications.cpp
    Redirector.cpp
    registry.cpp
    relay.cpp
    RingBuffer.cpp
    socket.cpp
    string.cpp
    SubProcess.cpp
    svccomm.cpp
    timestamp.cpp
    VTSupport.cpp
    WindowsUpdateIntegration.cpp
    WSLCContainerLauncher.cpp
    ConsommeNetworking.cpp
    WSLCProcessLauncher.cpp
    WslClient.cpp
    WslCoreConfig.cpp
    WslCoreFilesystem.cpp
    WSLPlusCommands.cpp
    WSLPlusNetworks.cpp
    WSLPlusImages.cpp
    WSLPlusDevices.cpp
    WSLPlusUsb.cpp
    WslCoreFirewallSupport.cpp
    WslCoreHostDnsInfo.cpp
    WslCoreNetworkEndpointSettings.cpp
    WslCoreNetworkingSupport.cpp
    WslInstall.cpp
    WslSecurity.cpp
    SlowOperationWatcher.cpp
    WslTelemetry.cpp
    wslutil.cpp
    install.cpp
    WSLCUserSettings.cpp
    notifications.cpp)

set(HEADERS
    ../../../generated/Localization.h
    APICompat.h
    ../inc/docker_schema.h
    ../inc/wslc_schema.h
    ../inc/lxssbusclient.h
    ../inc/lxssclient.h
    ../inc/LxssDynamicFunction.h
    ../inc/traceloggingconfig.h
    ../inc/wdk.h
    ../inc/wsl.h
    ../inc/wslconfig.h
    ../inc/wslhost.h
    ../inc/wslpolicies.h
    ../inc/WslPluginApi.h
    ../inc/wslrelay.h
    ../../shared/inc/CommandLine.h
    ../../shared/inc/defs.h
    ../../shared/inc/hns_schema.h
    ../../shared/inc/JsonUtils.h
    ../../shared/inc/lxfsshares.h
    ../../shared/inc/lxinitshared.h
    ../../shared/inc/message.h
    ../../shared/inc/prettyprintshared.h
    ../../shared/inc/retryshared.h
    ../../shared/inc/SocketChannel.h
    ../../shared/inc/socketshared.h
    ../../shared/inc/stringshared.h
    ConsoleProgressBar.h
    ConsoleProgressIndicator.h
    ConsoleState.h
    DeviceHostProxy.h
    disk.hpp
    Distribution.h
    Dmesg.h
    DnsResolver.h
    DnsTunnelingChannel.h
    ExecutionContext.h
    filesystem.hpp
    GnsChannel.h
    GnsPortTrackerChannel.h
    GuestDeviceManager.h
    HandleConsoleProgressBar.h
    hcs.hpp
    hcs_schema.h
    helpers.hpp
    hvsocket.hpp
    INetworkingEngine.h
    interop.hpp
    HandleIO.h
    LxssMessagePort.h
    LxssPort.h
    LxssSecurity.h
    LxssServerPort.h
    MountSpecParsing.h
    NatNetworking.h
    notifications.h
    precomp.h
    Redirector.h
    registry.hpp
    relay.hpp
    RingBuffer.h
    socket.hpp
    string.hpp
    Stringify.h
    SubProcess.h
    svccomm.hpp
    timestamp.hpp
    VTSupport.h
    WindowsUpdateIntegration.h
    WSLCContainerLauncher.h
    WSLCContainerEntry.h
    ConsommeNetworking.h
    WSLCProcessLauncher.h
    WslClient.h
    WslCoreConfig.h
    WslCoreFilesystem.h
    WslCoreFirewallSupport.h
    WslCoreHostDnsInfo.h
    WslCoreMessageQueue.h
    WslCoreNetworkEndpointSettings.h
    WslCoreNetworkingSupport.h
    WslInstall.h
    WslSecurity.h
    SlowOperationWatcher.h
    WslTelemetry.h
    wslutil.h
    EnumVariantMap.h
    WSLCUserSettings.h
    WSLCSessionDefaults.h
    )

add_library(common STATIC ${SOURCES} ${HEADERS})
add_dependencies(common wslserviceidl wsldevicehostidl localization wslservicemc wslinstalleridl yaml-cpp)

# WSL-Plus 本地改动: 可禁用 PCH（大 PCH 在编译期触发 C3859/C1076 虚拟内存限制）
if (NOT WSL_DISABLE_PCH)
    target_precompile_headers(common PRIVATE precomp.h)
else()
    # PCH 禁用时用 /FI 强制包含 precomp.h —— 复现"头先行"效果，消除
    # 依赖 PCH 掩盖的 include 顺序缺陷（如 DnsResolver.cpp 先 include LxssDynamicFunction.h），
    # 同时保持每编译单元内存平摊（无巨型 PCH 单次内存需求）
    target_compile_options(common PRIVATE "/FI${CMAKE_CURRENT_SOURCE_DIR}/precomp.h")
endif()
set_target_properties(common PROPERTIES FOLDER windows)
target_include_directories(common PUBLIC ${CMAKE_CURRENT_BINARY_DIR}/../service/mc/${TARGET_PLATFORM}/${CMAKE_BUILD_TYPE})

# common calls into HCS, HNS, the IP helper API, the VHD APIs, MSI, WinTrust and
# the config file parser, so anything linking common needs those libraries too.
# PUBLIC propagates them instead of making each consumer restate them.
target_link_libraries(common PUBLIC ${HCS_LINK_LIBRARIES} ${MSI_LINK_LIBRARIES} VirtDisk.lib configfile)

# WSLCUserSettings.cpp / WSLPlusNetworks.cpp / WSLPlusImages.cpp / WSLPlusDevices.cpp use yaml-cpp headers.
# The per-source include path and definition below only apply when the file is
# compiled on its own, so keep it out of the unity sources.
set_source_files_properties(WSLCUserSettings.cpp WSLPlusNetworks.cpp WSLPlusImages.cpp WSLPlusDevices.cpp PROPERTIES
    INCLUDE_DIRECTORIES "${yaml-cpp_SOURCE_DIR}/include"
    COMPILE_DEFINITIONS "YAML_CPP_STATIC_DEFINE"
    SKIP_UNITY_BUILD_INCLUSION ON)
target_link_libraries(common PRIVATE yaml-cpp)
