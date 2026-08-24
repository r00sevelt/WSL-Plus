// WSL-Plus Desktop (N0-GUI) —— Qt Widgets 骨架
// v0.1: 标签页框架（设置/网络/设备）+ wsl.exe 命令面对话（后端零耦合）
// 后续: 网络拓扑 QML 画布 + 直接链接模块（DLL 化后替换命令面）

#include <QApplication>
#include <QComboBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QProcess>
#include <QPushButton>
#include <QTabWidget>
#include <QWidget>

namespace
{
    QString RunWsl(const QString& args)
    {
        QProcess proc;
        proc.setProgram(QStringLiteral("wsl.exe"));
        proc.setArguments(QStringList() << args.split(QLatin1Char(' '), Qt::SkipEmptyParts));
        proc.start();
        proc.waitForFinished(15000);
        return QString::fromLocal8Bit(proc.readAllStandardOutput());
    }

    QWidget* CreateSettingsTab(QPlainTextEdit* output)
    {
        auto* tab = new QWidget;
        auto* layout = new QVBoxLayout(tab);
        auto* row = new QHBoxLayout;
        auto* distro = new QComboBox;
        distro->addItems(RunWsl(QStringLiteral("--list -q")).split(QLatin1Char('\n'), Qt::SkipEmptyParts));
        auto* refresh = new QPushButton(QStringLiteral("刷新"));
        auto* run = new QPushButton(QStringLiteral("打开终端"));
        row->addWidget(new QLabel(QStringLiteral("发行版:")));
        row->addWidget(distro);
        row->addWidget(refresh);
        row->addWidget(run);
        layout->addLayout(row);
        layout->addWidget(output);
        QObject::connect(refresh, &QPushButton::clicked, [distro] {
            distro->clear();
            distro->addItems(RunWsl(QStringLiteral("--list -q")).split(QLatin1Char('\n'), Qt::SkipEmptyParts));
        });
        QObject::connect(run, &QPushButton::clicked, [distro] {
            RunWsl(QStringLiteral("--cd") + QStringLiteral(" ~ ") + distro->currentText() + QStringLiteral(" -- bash"));
        });
        return tab;
    }

    QWidget* CreateNetworkTab(QPlainTextEdit* output)
    {
        auto* tab = new QWidget;
        auto* layout = new QVBoxLayout(tab);
        auto* row = new QHBoxLayout;
        auto* refresh = new QPushButton(QStringLiteral("WSL-Plus network ls"));
        row->addWidget(refresh);
        row->addStretch();
        layout->addLayout(row);
        layout->addWidget(output);
        QObject::connect(refresh, &QPushButton::clicked, [output] {
            output->setPlainText(RunWsl(QStringLiteral("network ls")));
        });
        return tab;
    }

    QWidget* CreateDevicesTab(QPlainTextEdit* output)
    {
        auto* tab = new QWidget;
        auto* layout = new QVBoxLayout(tab);
        auto* refresh = new QPushButton(QStringLiteral("WSL-Plus device list"));
        layout->addWidget(refresh);
        layout->addWidget(output);
        QObject::connect(refresh, &QPushButton::clicked, [output] {
            output->setPlainText(RunWsl(QStringLiteral("device list")));
        });
        return tab;
    }
} // namespace

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    app.setApplicationName(QStringLiteral("WSL-Plus Desktop"));

    auto* window = new QWidget;
    window->setWindowTitle(QStringLiteral("WSL-Plus Desktop"));
    window->resize(760, 520);

    auto* tabs = new QTabWidget(window);
    auto* layout = new QVBoxLayout(window);
    layout->addWidget(tabs);

    auto* settingsOut = new QPlainTextEdit;
    settingsOut->setReadOnly(true);
    auto* networkOut = new QPlainTextEdit;
    networkOut->setReadOnly(true);
    auto* devicesOut = new QPlainTextEdit;
    devicesOut->setReadOnly(true);

    tabs->addTab(CreateSettingsTab(settingsOut), QStringLiteral("发行版"));
    tabs->addTab(CreateNetworkTab(networkOut), QStringLiteral("网络 · WSL-Plus"));
    tabs->addTab(CreateDevicesTab(devicesOut), QStringLiteral("设备 · WSL-Plus"));

    window->show();
    return app.exec();
}
