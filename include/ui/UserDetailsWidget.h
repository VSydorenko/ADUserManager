#pragma once
#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

#include "models/UserInfo.h"
#include "models/ServerInfo.h"

class UserDetailsWidget : public QWidget {
    Q_OBJECT
    
public:
    explicit UserDetailsWidget(QWidget* parent = nullptr);
    
    void setUser(const UserInfo& user);
    void setServerInfo(const ServerInfo& serverInfo);
    void clearUser();
    
signals:
    void copyRdpRequested();
    void changePasswordRequested();
    void deactivateUserRequested();
    
private slots:
    void onCopyRdpClicked();
    void onChangePasswordClicked();
    void onDeactivateUserClicked();
    
private:
    void setupUI();
    
    // UI components
    QGridLayout* m_layout;
    
    // Info fields
    QLabel* m_loginLabel;
    QLineEdit* m_loginField;
    
    QLabel* m_fullNameLabel;
    QLineEdit* m_fullNameField;
    
    QLabel* m_serverLabel;
    QLineEdit* m_serverField;
    
    QLabel* m_serverAddressLabel;
    QLineEdit* m_serverAddressField;
    
    QLabel* m_createdLabel;
    QLineEdit* m_createdField;
    
    QLabel* m_lastLoginLabel;
    QLineEdit* m_lastLoginField;
    
    QLabel* m_statusLabel;
    QLineEdit* m_statusField;
    
    QLabel* m_rdpInfoLabel;
    QLineEdit* m_rdpInfoField;
    
    // Actions
    QPushButton* m_copyRdpButton;
    QPushButton* m_changePasswordButton;
    QPushButton* m_deactivateUserButton;
    
    // Data
    UserInfo m_currentUser;
    ServerInfo m_currentServer;
};
