#include <QApplication>
#include <QMessageBox>
#include "MainWindow.h"
#include "PluginManager.h"
#include "HostAppAdapter.h"
#include <memory>

using namespace PluginFramework;

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);

    app.setApplicationName("Plugin Host Application");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("PluginFramework");

    // 初始化宿主适配器
    auto hostAdapter = std::make_shared<HostAppAdapter>();
    if (!hostAdapter->Initialize()) {
        QMessageBox::critical(nullptr, "错误", "Failed to initialize host adapter");
        return 1;
    }

    // 初始化插件管理器
    auto pluginManager = std::make_shared<PluginManager>(hostAdapter.get());
    if (!pluginManager->Initialize()) {
        QString errorMsg = QString("Failed to initialize plugin manager: %1")
            .arg(QString::fromStdString(pluginManager->GetLastError()));
        QMessageBox::critical(nullptr, "错误", errorMsg);
        return 1;
    }

    // 创建并显示主窗口
    MainWindow mainWindow(hostAdapter, pluginManager);
    mainWindow.show();

    int result = app.exec();

    // 关闭插件管理器
    pluginManager->Shutdown();

    return result;
}
