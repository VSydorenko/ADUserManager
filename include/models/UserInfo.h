#pragma once
#include <QString>
#include <QDateTime>

class UserInfo {
public:
    UserInfo();
    UserInfo(const QString& login, const QString& fullName);
    
    // Getters
    QString getLogin() const { return m_login; }
    QString getFullName() const { return m_fullName; }
    QString getFirstName() const { return m_firstName; }
    QString getLastName() const { return m_lastName; }
    QString getDistinguishedName() const { return m_distinguishedName; }
    QString getServerName() const { return m_serverName; }
    QString getPassword() const { return m_password; }
    QDateTime getCreatedDate() const { return m_createdDate; }
    QDateTime getLastLogin() const { return m_lastLogin; }
    bool isActive() const { return m_isActive; }
    
    // Setters
    void setLogin(const QString& login) { m_login = login; }
    void setFullName(const QString& fullName) { m_fullName = fullName; }
    void setFirstName(const QString& firstName) { m_firstName = firstName; }
    void setLastName(const QString& lastName) { m_lastName = lastName; }
    void setDistinguishedName(const QString& dn) { m_distinguishedName = dn; }
    void setServerName(const QString& server) { m_serverName = server; }
    void setPassword(const QString& password) { m_password = password; }
    void setCreatedDate(const QDateTime& date) { m_createdDate = date; }
    void setLastLogin(const QDateTime& date) { m_lastLogin = date; }
    void setActive(bool active) { m_isActive = active; }
    
    // Helper methods
    QString getDisplayName() const;
    QString getRdpConnectionString(const QString& server, int port) const;
    
private:
    QString m_login;
    QString m_fullName;
    QString m_firstName;
    QString m_lastName;
    QString m_distinguishedName;
    QString m_serverName;
    QString m_password;
    QDateTime m_createdDate;
    QDateTime m_lastLogin;
    bool m_isActive;
};
