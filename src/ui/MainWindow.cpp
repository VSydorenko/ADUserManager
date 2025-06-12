#include "ui/MainWindow.h"
#include "ui/ServerTreeWidget.h"
#include "ui/UserDetailsWidget.h"
#include "ui/CreateUsersDialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QDockWidget>
#include <QHeaderView>
#include <QMenu>
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>
#include <QVBoxLayout>
#include <QDateTime>
#include <QClipboard>
#include <QFileDialog>
#include <QStandardPaths>
#include <QSettings>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    // Initialize services
    m_configManager = std::make_unique<ConfigManager>(this);
    m_configManager->loadConfig();
    
    m_adManager = std::make_unique<ADManager>(this);
    m_llmService = std::make_unique<LLMService>(this);
    m_passwordGenerator = std::make_unique<PasswordGenerator>(this);
    
    // Configure services from config
    m_llmService->setApiKey(m_configManager->getLlmApiKey());
    m_llmService->setEndpoint(m_configManager->getLlmEndpoint());
    m_llmService->setModel(m_configManager->getLlmModel());
    
    // Set up the UI
    setupUI();
    setupMenus();
    setupToolbar();
    setupStatusBar();
    setupConnections();
    
    // Initialize with AD connection
    m_adManager->connectToAD(m_configManager->getAdDomain());
    
    // Load servers 
    loadServers();
    
    // Set window properties
    setWindowTitle(tr("AD User Manager"));
    setMinimumSize(800, 600);
    resize(1200, 800);
}

MainWindow::~MainWindow()
{
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    // Save any settings or state before closing
    QSettings settings;
    settings.setValue("MainWindow/geometry", saveGeometry());
    settings.setValue("MainWindow/state", saveState());
    
    event->accept();
}

void MainWindow::setupUI()
{
    // Create main layout with splitters
    m_mainSplitter = new QSplitter(Qt::Horizontal, this);
    m_rightSplitter = new QSplitter(Qt::Vertical, m_mainSplitter);
    
    // Create server tree
    m_serverTree = new ServerTreeWidget(m_mainSplitter);
    m_mainSplitter->addWidget(m_serverTree);
    
    // Create user table
    m_userTable = new QTableWidget(m_rightSplitter);
    m_userTable->setColumnCount(4);
    m_userTable->setHorizontalHeaderLabels({tr("Full Name"), tr("Login"), tr("Status"), tr("Created Date")});
    m_userTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_userTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_userTable->setSelectionMode(QTableWidget::SingleSelection);
    m_userTable->setSortingEnabled(true);
    m_rightSplitter->addWidget(m_userTable);
    
    // Create user details pane
    m_userDetails = new UserDetailsWidget(m_rightSplitter);
    m_rightSplitter->addWidget(m_userDetails);
    
    // Add right splitter to main splitter
    m_mainSplitter->addWidget(m_rightSplitter);
    
    // Set splitter sizes
    m_mainSplitter->setSizes({200, 600});
    m_rightSplitter->setSizes({400, 200});
    
    // Create log output as dock widget
    m_logOutput = new QTextEdit();
    m_logOutput->setReadOnly(true);
    
    m_logDock = new QDockWidget(tr("Log"), this);
    m_logDock->setWidget(m_logOutput);
    m_logDock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, m_logDock);
    
    // Set central widget
    setCentralWidget(m_mainSplitter);
    
    // Restore any saved state
    QSettings settings;
    if (settings.contains("MainWindow/geometry")) {
        restoreGeometry(settings.value("MainWindow/geometry").toByteArray());
    }
    if (settings.contains("MainWindow/state")) {
        restoreState(settings.value("MainWindow/state").toByteArray());
    }
}

void MainWindow::setupMenus()
{
    // File menu
    QMenu* fileMenu = menuBar()->addMenu(tr("&File"));
    
    QAction* createUsersAction = fileMenu->addAction(tr("&Create Users..."));
    createUsersAction->setIcon(QIcon(":/icons/user-plus.svg"));
    connect(createUsersAction, &QAction::triggered, this, &MainWindow::onCreateUsers);
    
    fileMenu->addSeparator();
    
    QAction* exportUsersAction = fileMenu->addAction(tr("&Export Users..."));
    exportUsersAction->setIcon(QIcon(":/icons/export.svg"));
    connect(exportUsersAction, &QAction::triggered, this, &MainWindow::onExportUsers);
    
    fileMenu->addSeparator();
    
    QAction* settingsAction = fileMenu->addAction(tr("&Settings..."));
    settingsAction->setIcon(QIcon(":/icons/settings.svg"));
    connect(settingsAction, &QAction::triggered, this, &MainWindow::onSettings);
    
    fileMenu->addSeparator();
    
    QAction* exitAction = fileMenu->addAction(tr("E&xit"));
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    
    // View menu
    QMenu* viewMenu = menuBar()->addMenu(tr("&View"));
    
    QAction* refreshAction = viewMenu->addAction(tr("&Refresh Servers"));
    refreshAction->setIcon(QIcon(":/icons/refresh.svg"));
    connect(refreshAction, &QAction::triggered, this, &MainWindow::onRefreshServers);
    
    viewMenu->addSeparator();
    
    QAction* logAction = viewMenu->addAction(tr("Show &Log"));
    logAction->setCheckable(true);
    logAction->setChecked(true);
    connect(logAction, &QAction::toggled, m_logDock, &QDockWidget::setVisible);
    
    // User menu
    QMenu* userMenu = menuBar()->addMenu(tr("&User"));
    
    QAction* deactivateAction = userMenu->addAction(tr("&Deactivate User"));
    deactivateAction->setIcon(QIcon(":/icons/user-minus.svg"));
    connect(deactivateAction, &QAction::triggered, this, &MainWindow::onDeactivateUser);
    
    QAction* changePasswordAction = userMenu->addAction(tr("Change &Password..."));
    changePasswordAction->setIcon(QIcon(":/icons/key.svg"));
    connect(changePasswordAction, &QAction::triggered, this, &MainWindow::onChangePassword);
    
    QAction* copyConnectionAction = userMenu->addAction(tr("Copy &RDP Connection Info"));
    copyConnectionAction->setIcon(QIcon(":/icons/copy.svg"));
    connect(copyConnectionAction, &QAction::triggered, this, &MainWindow::onCopyConnectionInfo);
    
    // Help menu
    QMenu* helpMenu = menuBar()->addMenu(tr("&Help"));
    
    QAction* aboutAction = helpMenu->addAction(tr("&About..."));
    connect(aboutAction, &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolbar()
{
    QToolBar* toolbar = addToolBar(tr("Main Toolbar"));
    toolbar->setMovable(false);
    
    toolbar->addAction(QIcon(":/icons/user-plus.svg"), tr("Create Users"), this, &MainWindow::onCreateUsers);
    toolbar->addAction(QIcon(":/icons/refresh.svg"), tr("Refresh"), this, &MainWindow::onRefreshServers);
    toolbar->addSeparator();
    toolbar->addAction(QIcon(":/icons/user-minus.svg"), tr("Deactivate User"), this, &MainWindow::onDeactivateUser);
    toolbar->addAction(QIcon(":/icons/key.svg"), tr("Change Password"), this, &MainWindow::onChangePassword);
    toolbar->addAction(QIcon(":/icons/copy.svg"), tr("Copy RDP Info"), this, &MainWindow::onCopyConnectionInfo);
    toolbar->addSeparator();
    toolbar->addAction(QIcon(":/icons/settings.svg"), tr("Settings"), this, &MainWindow::onSettings);
}

void MainWindow::setupStatusBar()
{
    QStatusBar* status = statusBar();
    
    // Connection status indicator
    m_connectionStatus = new QLabel(tr("Not Connected"));
    m_connectionStatus->setStyleSheet("color: red");
    status->addWidget(m_connectionStatus);
    
    // Server count
    m_serverCount = new QLabel(tr("Servers: 0"));
    status->addWidget(m_serverCount);
    
    // User count
    m_userCount = new QLabel(tr("Users: 0"));
    status->addWidget(m_userCount);
    
    // Progress bar (hidden by default)
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setMaximumWidth(150);
    m_progressBar->setVisible(false);
    status->addPermanentWidget(m_progressBar);
}

void MainWindow::setupConnections()
{
    // Server tree signals
    connect(m_serverTree, &ServerTreeWidget::serverSelected, this, &MainWindow::onServerSelected);
    
    // User table signals
    connect(m_userTable, &QTableWidget::itemSelectionChanged, [this]() {
        if (m_userTable->selectedItems().isEmpty()) {
            return;
        }
        
        int row = m_userTable->selectedItems().first()->row();
        QString userDN = m_userTable->item(row, 0)->data(Qt::UserRole).toString();
        onUserSelected(userDN);
    });
    
    connect(m_userTable, &QTableWidget::itemDoubleClicked, [this](QTableWidgetItem* item) {
        int row = item->row();
        QString userDN = m_userTable->item(row, 0)->data(Qt::UserRole).toString();
        onUserDoubleClicked(userDN);
    });
    
    // AD Manager signals
    connect(m_adManager.get(), &ADManager::connectionStatusChanged, this, &MainWindow::onADConnectionChanged);
    connect(m_adManager.get(), &ADManager::operationProgress, this, &MainWindow::onOperationProgress);
    connect(m_adManager.get(), &ADManager::error, this, &MainWindow::onADError);
}

void MainWindow::loadServers()
{
    if (!m_adManager->isConnected()) {
        return;
    }
    
    QStringList servers = m_adManager->getServerList();
    m_serverTree->setServers(servers);
    
    m_serverCount->setText(tr("Servers: %1").arg(servers.count()));
    log(tr("Loaded %1 servers").arg(servers.count()));
}

void MainWindow::loadUsers(const QString& serverName)
{
    if (!m_adManager->isConnected() || serverName.isEmpty()) {
        return;
    }
    
    m_userTable->setRowCount(0);
    m_userTable->setSortingEnabled(false);
    
    QStringList users = m_adManager->getUsersForServer(serverName);
    m_userTable->setRowCount(users.count());
    
    for (int i = 0; i < users.count(); i++) {
        UserInfo user = m_adManager->getUserInfo(users[i]);
        
        QTableWidgetItem* nameItem = new QTableWidgetItem(user.getFullName());
        nameItem->setData(Qt::UserRole, user.getDistinguishedName());
        m_userTable->setItem(i, 0, nameItem);
        
        m_userTable->setItem(i, 1, new QTableWidgetItem(user.getLogin()));
        
        QTableWidgetItem* statusItem = new QTableWidgetItem(user.isActive() ? tr("Active") : tr("Disabled"));
        statusItem->setForeground(user.isActive() ? QBrush(Qt::darkGreen) : QBrush(Qt::red));
        m_userTable->setItem(i, 2, statusItem);
        
        m_userTable->setItem(i, 3, new QTableWidgetItem(user.getCreatedDate().toString("yyyy-MM-dd")));
    }
    
    m_userTable->setSortingEnabled(true);
    m_userTable->sortItems(0, Qt::AscendingOrder);
    
    m_userCount->setText(tr("Users: %1").arg(users.count()));
    log(tr("Loaded %1 users for server %2").arg(users.count()).arg(serverName));
}

void MainWindow::updateStatusBar()
{
    if (m_adManager->isConnected()) {
        m_connectionStatus->setText(tr("Connected to AD"));
        m_connectionStatus->setStyleSheet("color: green");
    } else {
        m_connectionStatus->setText(tr("Not Connected"));
        m_connectionStatus->setStyleSheet("color: red");
    }
}

void MainWindow::showConnectionStatus(bool connected)
{
    if (connected) {
        m_connectionStatus->setText(tr("Connected to AD"));
        m_connectionStatus->setStyleSheet("color: green");
    } else {
        m_connectionStatus->setText(tr("Not Connected"));
        m_connectionStatus->setStyleSheet("color: red");
    }
}

void MainWindow::displayError(const QString& message)
{
    QMessageBox::critical(this, tr("Error"), message);
    log(tr("ERROR: %1").arg(message));
}

void MainWindow::log(const QString& message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    m_logOutput->append(QString("[%1] %2").arg(timestamp, message));
}

void MainWindow::onCreateUsers()
{
    if (!m_adManager->isConnected()) {
        displayError(tr("Not connected to AD. Please check your connection."));
        return;
    }
    
    CreateUsersDialog dialog(m_adManager->getServerList(), this);
    dialog.setLLMService(m_llmService.get());
    dialog.setADManager(m_adManager.get());
    dialog.setPasswordGenerator(m_passwordGenerator.get());
    
    if (dialog.exec() == QDialog::Accepted) {
        // Refresh current server if it matches the one users were created for
        QString createdForServer = dialog.getSelectedServer();
        if (createdForServer == m_currentServer) {
            loadUsers(m_currentServer);
        }
        
        log(tr("Users created for server %1").arg(createdForServer));
    }
}

void MainWindow::onRefreshServers()
{
    loadServers();
    
    // Also reload users for the current server
    if (!m_currentServer.isEmpty()) {
        loadUsers(m_currentServer);
    }
}

void MainWindow::onServerSelected(const QString& serverName)
{
    m_currentServer = serverName;
    m_userDetails->clearUser();
    loadUsers(serverName);
}

void MainWindow::onUserSelected(const QString& userDN)
{
    m_currentUser = userDN;
    
    if (userDN.isEmpty()) {
        m_userDetails->clearUser();
        return;
    }
    
    UserInfo user = m_adManager->getUserInfo(userDN);
    m_userDetails->setUser(user);
    
    // If server info is available, set RDP information
    if (!m_currentServer.isEmpty()) {
        ServerInfo serverInfo = m_adManager->getServerInfo(m_currentServer);
        m_userDetails->setServerInfo(serverInfo);
    }
}

void MainWindow::onUserDoubleClicked(const QString& userDN)
{
    // Could show a detailed user info dialog or similar
    if (userDN.isEmpty()) {
        return;
    }
    
    UserInfo user = m_adManager->getUserInfo(userDN);
    QMessageBox::information(this, tr("User Details"), 
                           tr("User: %1\nLogin: %2\nServer: %3\nCreated: %4\nLast Login: %5\nStatus: %6")
                           .arg(user.getFullName())
                           .arg(user.getLogin())
                           .arg(user.getServerName())
                           .arg(user.getCreatedDate().toString("yyyy-MM-dd"))
                           .arg(user.getLastLogin().toString("yyyy-MM-dd"))
                           .arg(user.isActive() ? tr("Active") : tr("Disabled")));
}

void MainWindow::onDeactivateUser()
{
    if (m_currentUser.isEmpty()) {
        QMessageBox::warning(this, tr("No User Selected"), tr("Please select a user to deactivate."));
        return;
    }
    
    UserInfo user = m_adManager->getUserInfo(m_currentUser);
    
    QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Confirm Deactivation"),
                                                             tr("Are you sure you want to deactivate user %1?")
                                                             .arg(user.getFullName()));
    
    if (confirm == QMessageBox::Yes) {
        if (m_adManager->deactivateUser(m_currentUser)) {
            log(tr("User %1 has been deactivated").arg(user.getFullName()));
            
            // Refresh user list
            loadUsers(m_currentServer);
        } else {
            displayError(tr("Failed to deactivate user %1").arg(user.getFullName()));
        }
    }
}

void MainWindow::onChangePassword()
{
    if (m_currentUser.isEmpty()) {
        QMessageBox::warning(this, tr("No User Selected"), tr("Please select a user to change password."));
        return;
    }
    
    UserInfo user = m_adManager->getUserInfo(m_currentUser);
    
    // Generate a new password according to policy
    QJsonObject policyJson = m_configManager->getPasswordPolicy();
    PasswordPolicy policy;
    
    if (!policyJson.isEmpty()) {
        policy.minLength = policyJson["minLength"].toInt(12);
        policy.maxLength = policyJson["maxLength"].toInt(16);
        policy.includeUppercase = policyJson["includeUppercase"].toBool(true);
        policy.includeLowercase = policyJson["includeLowercase"].toBool(true);
        policy.includeNumbers = policyJson["includeNumbers"].toBool(true);
        policy.includeSymbols = policyJson["includeSymbols"].toBool(true);
        policy.excludeChars = policyJson["excludeChars"].toString("0O1lI");
        policy.requireEachType = policyJson["requireEachType"].toBool(true);
    }
    
    QString newPassword = m_passwordGenerator->generatePassword(policy);
    
    QString message = tr("Change password for user %1?\n\nNew password: %2\n\n"
                      "Password strength: %3%")
                    .arg(user.getFullName())
                    .arg(newPassword)
                    .arg(m_passwordGenerator->calculateStrength(newPassword));
    
    QMessageBox::StandardButton confirm = QMessageBox::question(this, tr("Change Password"), message);
    
    if (confirm == QMessageBox::Yes) {
        if (m_adManager->changePassword(m_currentUser, newPassword)) {
            log(tr("Password changed for user %1").arg(user.getFullName()));
            
            // Update the user object
            user.setPassword(newPassword);
            onUserSelected(m_currentUser); // Refresh display
            
            // Also copy to clipboard
            QClipboard* clipboard = QApplication::clipboard();
            clipboard->setText(newPassword);
            
            QMessageBox::information(this, tr("Password Changed"),
                                  tr("Password has been changed and copied to clipboard."));
        } else {
            displayError(tr("Failed to change password for user %1").arg(user.getFullName()));
        }
    }
}

void MainWindow::onCopyConnectionInfo()
{
    if (m_currentUser.isEmpty() || m_currentServer.isEmpty()) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select a user and server."));
        return;
    }
    
    UserInfo user = m_adManager->getUserInfo(m_currentUser);
    ServerInfo server = m_adManager->getServerInfo(m_currentServer);
    
    QString connectionInfo = user.getRdpConnectionString(server.getRdpAddress(), server.getRdpPort());
    
    QClipboard* clipboard = QApplication::clipboard();
    clipboard->setText(connectionInfo);
    
    QMessageBox::information(this, tr("Connection Info Copied"),
                          tr("RDP connection info has been copied to clipboard."));
    
    log(tr("Copied RDP connection info for user %1 on server %2")
        .arg(user.getFullName())
        .arg(server.getName()));
}

void MainWindow::onExportUsers()
{
    if (m_currentServer.isEmpty()) {
        QMessageBox::warning(this, tr("No Server Selected"), tr("Please select a server to export users from."));
        return;
    }
    
    QString defaultPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
    QString fileName = QFileDialog::getSaveFileName(this, tr("Export Users"),
                                                 defaultPath + "/" + m_currentServer + "-users.csv",
                                                 tr("CSV Files (*.csv);;All Files (*.*)"));
    
    if (fileName.isEmpty()) {
        return;
    }
    
    QStringList users = m_adManager->getUsersForServer(m_currentServer);
    
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        displayError(tr("Could not open file for writing: %1").arg(fileName));
        return;
    }
    
    QTextStream out(&file);
    out << "Full Name,Login,Status,Created Date,Last Login\n";
    
    for (const QString& userDN : users) {
        UserInfo user = m_adManager->getUserInfo(userDN);
        out << QString("%1,%2,%3,%4,%5\n")
               .arg(user.getFullName())
               .arg(user.getLogin())
               .arg(user.isActive() ? "Active" : "Disabled")
               .arg(user.getCreatedDate().toString("yyyy-MM-dd"))
               .arg(user.getLastLogin().toString("yyyy-MM-dd"));
    }
    
    file.close();
    
    log(tr("Exported %1 users to %2").arg(users.count()).arg(fileName));
    QMessageBox::information(this, tr("Export Complete"), 
                          tr("%1 users exported to %2").arg(users.count()).arg(fileName));
}

void MainWindow::onSettings()
{
    // This would show a settings dialog
    QMessageBox::information(this, tr("Settings"), tr("Settings dialog not yet implemented"));
}

void MainWindow::onAbout()
{
    QString aboutText = tr(
        "<h3>AD User Manager</h3>"
        "<p>Version 1.0.0</p>"
        "<p>A tool for managing users and servers in Active Directory.</p>"
        "<p>Copyright © 2025</p>"
    );
    
    QMessageBox::about(this, tr("About AD User Manager"), aboutText);
}

void MainWindow::onADConnectionChanged(bool connected)
{
    showConnectionStatus(connected);
    
    if (connected) {
        loadServers();
    } else {
        m_serverTree->clear();
        m_userTable->setRowCount(0);
        m_userDetails->clearUser();
        
        m_serverCount->setText(tr("Servers: 0"));
        m_userCount->setText(tr("Users: 0"));
    }
}

void MainWindow::onADError(const QString& error)
{
    displayError(error);
}

void MainWindow::onOperationProgress(const QString& operation, int progress)
{
    // Show progress in status bar
    if (progress < 0 || progress >= 100) {
        m_progressBar->setVisible(false);
        statusBar()->showMessage(tr("%1: Complete").arg(operation), 3000);
    } else {
        m_progressBar->setVisible(true);
        m_progressBar->setValue(progress);
        statusBar()->showMessage(tr("%1: %2%").arg(operation).arg(progress));
    }
}
