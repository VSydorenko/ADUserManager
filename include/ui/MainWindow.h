#pragma once
#include <QMainWindow>
#include <QTreeWidget>
#include <QTableWidget>
#include <QTextEdit>
#include <QProgressBar>
#include <QLabel>
#include <QSplitter>
#include <memory>

#include "services/ADManager.h"
#include "services/LLMService.h"
#include "services/PasswordGenerator.h"
#include "services/ConfigManager.h"

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QStatusBar;
class QToolBar;
class QDockWidget;
QT_END_NAMESPACE

class ServerTreeWidget;
class UserDetailsWidget;

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();
    
protected:
    void closeEvent(QCloseEvent* event) override;
    
private slots:
    void onCreateUsers();
    void onRefreshServers();
    void onServerSelected(const QString& serverName);
    void onUserSelected(const QString& userDN);
    void onUserDoubleClicked(const QString& userDN);
    void onDeactivateUser();
    void onChangePassword();
    void onCopyConnectionInfo();
    void onExportUsers();
    void onSettings();
    void onAbout();
    
    // AD Manager slots
    void onADConnectionChanged(bool connected);
    void onADError(const QString& error);
    void onOperationProgress(const QString& operation, int progress);
    
private:
    void setupUI();
    void setupMenus();
    void setupToolbar();
    void setupStatusBar();
    void setupConnections();
    
    void loadServers();
    void loadUsers(const QString& serverName);
    void updateStatusBar();
    void showConnectionStatus(bool connected);
    void displayError(const QString& message);
    void log(const QString& message);
    
    // UI Components
    QSplitter* m_mainSplitter;
    QSplitter* m_rightSplitter;
    
    ServerTreeWidget* m_serverTree;
    QTableWidget* m_userTable;
    UserDetailsWidget* m_userDetails;
    QTextEdit* m_logOutput;
    QDockWidget* m_logDock;
    
    // Status Bar
    QLabel* m_connectionStatus;
    QLabel* m_serverCount;
    QLabel* m_userCount;
    QProgressBar* m_progressBar;
    
    // Services
    std::unique_ptr<ADManager> m_adManager;
    std::unique_ptr<LLMService> m_llmService;
    std::unique_ptr<PasswordGenerator> m_passwordGenerator;
    std::unique_ptr<ConfigManager> m_configManager;
    
    // Current state
    QString m_currentServer;
    QString m_currentUser;
};
