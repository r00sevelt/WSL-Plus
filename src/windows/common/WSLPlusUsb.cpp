/*++

Copyright (c) WSL-Plus contributors.

Module Name:

    WSLPlusUsb.cpp

Abstract:

    WSL-Plus: USB 后端实现（v1 = usbipd-win）；
    实现为进程封装（usbipd.exe 标准命令: list/attach/detach），
    后端可用性检测 + 命令错误转换为明确错误信息。

--*/

#include "precomp.h"
#include "WSLPlusUsb.h"

namespace wsl::windows::common::wslplus::usb
{
namespace
{
    constexpr const wchar_t* kUsbipdExe = L"usbipd.exe";

    std::wstring RunCommand(_In_ const std::wstring& args)
    {
        std::wstring cmdLine = std::wstring(kUsbipdExe) + L" " + args;
        DWORD result = 0;
        wil::unique_handle pipeRead;
        wil::unique_handle pipeWrite;
        SECURITY_ATTRIBUTES sa{sizeof(SECURITY_ATTRIBUTES), nullptr, true};
        THROW_IF_WIN32_BOOL_FALSE(CreatePipe(&pipeRead, &pipeWrite, &sa, 0));
        SetHandleInformation(pipeRead.get(), HANDLE_FLAG_INHERIT, 0);

        STARTUPINFOW si{};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdOutput = pipeWrite.get();
        si.hStdError = pipeWrite.get();
        si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

        PROCESS_INFORMATION pi{};
        std::wstring env;
        BOOL ok = CreateProcessW(nullptr, cmdLine.data(), nullptr, nullptr, TRUE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi);
        pipeWrite.reset();
        THROW_LAST_ERROR_IF(!ok);

        wil::unique_handle proc(pi.hProcess);
        wil::unique_handle thread(pi.hThread);

        std::string output;
        char buf[4096];
        DWORD read = 0;
        while (ReadFile(pipeRead.get(), buf, sizeof(buf), &read, nullptr) && read > 0)
        {
            output.append(buf, read);
        }
        WaitForSingleObject(proc.get(), INFINITE);
        GetExitCodeProcess(proc.get(), &result);
        if (result != 0)
        {
            WSL_LOG("WSL-Plus usbipd command failed", TraceLoggingValue(result, "exitCode"));
            THROW_HR(E_FAIL);
        }
        return wsl::windows::common::string::MultiByteToWide(output);
    }

    class UsbipdBackend final : public IUsbBackend
    {
    public:
        BackendKind Kind() const noexcept override
        {
            return BackendKind::Usbipd;
        }

        bool Available() const noexcept override
        {
            return (GetFileAttributesW(kUsbipdExe) != INVALID_FILE_ATTRIBUTES) ||
                (GetFileAttributesW(L"C:\\Windows\\System32\\usbipd.exe") != INVALID_FILE_ATTRIBUTES);
        }

        std::vector<std::string> Enumerate() const override
        {
            // usbipd list 输出解析: 每行 "BUSID STATE DEVICE_ID ..."，仅取状态==Attached 不涉及（目录层面）
            std::vector<std::string> result;
            std::wstring output = RunCommand(L"list");
            std::wistringstream lines{wsl::windows::common::string::WideToMultiByte(output)};
            std::string line;
            while (std::getline(lines, line))
            {
                std::istringstream parts{line};
                std::string busId, state;
                if (parts >> busId >> state)
                {
                    if (busId.rfind("USB", 0) == 0 || busId.rfind("1-", 0) == 0 || busId.rfind("2-", 0) == 0)
                    {
                        result.emplace_back(busId);
                    }
                }
            }
            return result;
        }

        void Attach(_In_ const std::string& busId, _In_ const GUID& vmId) override
        {
            const auto vmGuid = wsl::shared::string::GuidToString<wchar_t>(vmId);
            RunCommand(std::format(L"attach --busid {} --device {}", busId, vmGuid));
        }

        void Detach(_In_ const std::string& busId) override
        {
            RunCommand(std::format(L"detach --busid {}", busId));
        }
    };
} // namespace

std::unique_ptr<IUsbBackend> CreateDefault()
{
    return std::make_unique<UsbipdBackend>();
}

void WatchLoop(_In_ const std::unique_ptr<IUsbBackend>& backend, _In_ int intervalSeconds, _In_ const WatchCallback& onNewDevice)
{
    std::vector<std::string> lastSeen;
    if (backend->Available())
    {
        try
        {
            lastSeen = backend->Enumerate();
        }
        CATCH_LOG()
    }

    for (;;)
    {
        std::this_thread::sleep_for(std::chrono::seconds(intervalSeconds));

        if (!backend->Available())
        {
            continue;
        }

        std::vector<std::string> current;
        try
        {
            current = backend->Enumerate();
        }
        CATCH_LOG()
        if (current.empty() && lastSeen.empty())
        {
            continue;
        }

        // diff: 新出现的设备
        for (const auto& busId : current)
        {
            if (std::find(lastSeen.begin(), lastSeen.end(), busId) == lastSeen.end())
            {
                if (!onNewDevice(busId))
                {
                    return;
                }
            }
        }

        lastSeen = std::move(current);
    }
}
} // namespace wsl::windows::common::wslplus::usb
