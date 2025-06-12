#include "models/UserInfo.h"
#include <QCoreApplication>

UserInfo::UserInfo() : m_isActive(true), m_createdDate(QDateTime::currentDateTime()) {
}

UserInfo::UserInfo(const QString& login, const QString& fullName) 
    : m_login(login), m_fullName(fullName), m_isActive(true), m_createdDate(QDateTime::currentDateTime()) {
    
    // Try to parse first and last name from full name
    QStringList nameParts = fullName.split(' ', Qt::SkipEmptyParts);
    if (nameParts.size() >= 2) {
        m_firstName = nameParts.first();
        m_lastName = nameParts.last();
    } else if (nameParts.size() == 1) {
        m_firstName = nameParts.first();
        m_lastName = "";
    }
}

QString UserInfo::getDisplayName() const {
    if (m_fullName.isEmpty()) {
        return m_login;
    }
    return QString("%1 (%2)").arg(m_fullName, m_login);
}

QString UserInfo::getRdpConnectionString(const QString& server, int port) const {
    // Format: user:password@server:port
    return QString("%1:%2@%3:%4").arg(
        m_login, 
        m_password, 
        server.isEmpty() ? m_serverName : server,
        QString::number(port)
    );
}
