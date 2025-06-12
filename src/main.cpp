#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QString>
#include "ui/MainWindow.h"
#include "services/ConfigManager.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setOrganizationName("ADUserManager");
    app.setApplicationName("ADUserManager");
    app.setApplicationVersion("1.0.0");
    
    // Load application style
    QFile styleFile(":/styles/default.qss");
    if (styleFile.exists()) {
        styleFile.open(QFile::ReadOnly);
        QString style(styleFile.readAll());
        app.setStyleSheet(style);
    }
    
    // Initialize configuration
    ConfigManager configManager;
    if (!configManager.loadConfig()) {
        QMessageBox::critical(nullptr, "Configuration Error", 
            "Failed to load configuration. Please check app_config.json file.");
        return 1;
    }
    
    // Create and show main window
    MainWindow mainWindow;
    mainWindow.show();
    
    return app.exec();
}
