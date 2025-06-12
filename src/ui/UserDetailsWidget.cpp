#include "ui/UserDetailsWidget.h"
#include <QClipboard>
#include <QApplication>

UserDetailsWidget::UserDetailsWidget(QWidget* parent)
    : QWidget(parent)
{
    setupUI();
    clearUser();
}

void UserDetailsWidget::setupUI()
{
    m_layout = new QGridLayout(this);
    
    // Login
    m_loginLabel = new QLabel(tr("Login:"));
    m_loginField = new QLineEdit();
    m_loginField->setReadOnly(true);
    m_layout->addWidget(m_loginLabel, 0, 0);
    m_layout->addWidget(m_loginField, 0, 1);
    
    // Full name
    m_fullNameLabel = new QLabel(tr("Full Name:"));
    m_fullNameField = new QLineEdit();
    m_fullNameField->setReadOnly(true);
    m_layout->addWidget(m_fullNameLabel, 1, 0);
    m_layout->addWidget(m_fullNameField, 1, 1);
    
    // Server
    m_serverLabel = new QLabel(tr("Server:"));
    m_serverField = new QLineEdit();
    m_serverField->setReadOnly(true);
    m_layout->addWidget(m_serverLabel, 2, 0);
    m_layout->addWidget(m_serverField, 2, 1);
    
    // Server address
    m_serverAddressLabel = new QLabel(tr("Server Address:"));
    m_serverAddressField = new QLineEdit();
    m_serverAddressField->setReadOnly(true);
    m_layout->addWidget(m_serverAddressLabel, 3, 0);
    m_layout->addWidget(m_serverAddressField, 3, 1);
    
    // Created
    m_createdLabel = new QLabel(tr("Created:"));
    m_createdField = new QLineEdit();
    m_createdField->setReadOnly(true);
    m_layout->addWidget(m_createdLabel, 4, 0);
    m_layout->addWidget(m_createdField, 4, 1);
    
    // Last login
    m_lastLoginLabel = new QLabel(tr("Last Login:"));
    m_lastLoginField = new QLineEdit();
    m_lastLoginField->setReadOnly(true);
    m_layout->addWidget(m_lastLoginLabel, 5, 0);
    m_layout->addWidget(m_lastLoginField, 5, 1);
    
    // Status
    m_statusLabel = new QLabel(tr("Status:"));
    m_statusField = new QLineEdit();
    m_statusField->setReadOnly(true);
    m_layout->addWidget(m_statusLabel, 6, 0);
    m_layout->addWidget(m_statusField, 6, 1);
    
    // RDP info
    m_rdpInfoLabel = new QLabel(tr("RDP Connection:"));
    m_rdpInfoField = new QLineEdit();
    m_rdpInfoField->setReadOnly(true);
    m_layout->addWidget(m_rdpInfoLabel, 7, 0);
    m_layout->addWidget(m_rdpInfoField, 7, 1);
    
    // Action buttons
    m_copyRdpButton = new QPushButton(tr("Copy RDP Info"));
    m_changePasswordButton = new QPushButton(tr("Change Password"));
    m_deactivateUserButton = new QPushButton(tr("Deactivate User"));
    
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(m_copyRdpButton);
    buttonLayout->addWidget(m_changePasswordButton);
    buttonLayout->addWidget(m_deactivateUserButton);
    
    m_layout->addLayout(buttonLayout, 8, 0, 1, 2);
    
    // Connect signals
    connect(m_copyRdpButton, &QPushButton::clicked, this, &UserDetailsWidget::onCopyRdpClicked);
    connect(m_changePasswordButton, &QPushButton::clicked, this, &UserDetailsWidget::onChangePasswordClicked);
    connect(m_deactivateUserButton, &QPushButton::clicked, this, &UserDetailsWidget::onDeactivateUserClicked);
    
    // Add some spacing
    m_layout->setVerticalSpacing(10);
    m_layout->setContentsMargins(10, 10, 10, 10);
}

void UserDetailsWidget::setUser(const UserInfo& user)
{
    m_currentUser = user;
    
    m_loginField->setText(user.getLogin());
    m_fullNameField->setText(user.getFullName());
    m_serverField->setText(user.getServerName());
    m_createdField->setText(user.getCreatedDate().toString("yyyy-MM-dd hh:mm:ss"));
    m_lastLoginField->setText(user.getLastLogin().toString("yyyy-MM-dd hh:mm:ss"));
    
    QString status = user.isActive() ? tr("Active") : tr("Disabled");
    m_statusField->setText(status);
    
    // Update RDP info if server info is available
    if (!m_currentServer.getName().isEmpty()) {
        QString rdpInfo = user.getRdpConnectionString(m_currentServer.getRdpAddress(), m_currentServer.getRdpPort());
        m_rdpInfoField->setText(rdpInfo);
        m_serverAddressField->setText(m_currentServer.getRdpAddress());
    }
    
    // Enable buttons
    m_copyRdpButton->setEnabled(true);
    m_changePasswordButton->setEnabled(true);
    m_deactivateUserButton->setEnabled(user.isActive());
}

void UserDetailsWidget::setServerInfo(const ServerInfo& serverInfo)
{
    m_currentServer = serverInfo;
    
    m_serverField->setText(serverInfo.getName());
    m_serverAddressField->setText(serverInfo.getRdpAddress());
    
    // Update RDP info if user info is available
    if (!m_currentUser.getLogin().isEmpty()) {
        QString rdpInfo = m_currentUser.getRdpConnectionString(serverInfo.getRdpAddress(), serverInfo.getRdpPort());
        m_rdpInfoField->setText(rdpInfo);
    }
}

void UserDetailsWidget::clearUser()
{
    m_currentUser = UserInfo();
    m_currentServer = ServerInfo();
    
    m_loginField->clear();
    m_fullNameField->clear();
    m_serverField->clear();
    m_serverAddressField->clear();
    m_createdField->clear();
    m_lastLoginField->clear();
    m_statusField->clear();
    m_rdpInfoField->clear();
    
    // Disable buttons
    m_copyRdpButton->setEnabled(false);
    m_changePasswordButton->setEnabled(false);
    m_deactivateUserButton->setEnabled(false);
}

void UserDetailsWidget::onCopyRdpClicked()
{
    QString rdpInfo = m_rdpInfoField->text();
    if (!rdpInfo.isEmpty()) {
        QClipboard* clipboard = QApplication::clipboard();
        clipboard->setText(rdpInfo);
        
        emit copyRdpRequested();
    }
}

void UserDetailsWidget::onChangePasswordClicked()
{
    emit changePasswordRequested();
}

void UserDetailsWidget::onDeactivateUserClicked()
{
    emit deactivateUserRequested();
}
