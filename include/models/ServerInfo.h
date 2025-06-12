#pragma once
#include <QString>
#include <QStringList>
#include <QJsonObject>
#include <QJsonDocument>

class ServerInfo {
public:
    ServerInfo();
    ServerInfo(const QString& name, const QString& distinguishedName);
    
    // Getters
    QString getName() const { return m_name; }
    QString getDistinguishedName() const { return m_distinguishedName; }
    QString getRdpAddress() const { return m_rdpAddress; }
    int getRdpPort() const { return m_rdpPort; }
    QString getEnvironment() const { return m_environment; }
    QStringList getUserList() const { return m_userList; }
    QJsonObject getMetadata() const { return m_metadata; }
    
    // Setters
    void setName(const QString& name) { m_name = name; }
    void setDistinguishedName(const QString& dn) { m_distinguishedName = dn; }
    void setRdpAddress(const QString& address) { m_rdpAddress = address; }
    void setRdpPort(int port) { m_rdpPort = port; }
    void setEnvironment(const QString& env) { m_environment = env; }
    void setUserList(const QStringList& users) { m_userList = users; }
    void setMetadata(const QJsonObject& metadata) { m_metadata = metadata; }
    
    // Add user to the list
    void addUser(const QString& userDN);
    
    // Remove user from the list
    bool removeUser(const QString& userDN);
    
    // JSON serialization
    QJsonObject toJson() const;
    static ServerInfo fromJson(const QJsonObject& json);
    
private:
    QString m_name;
    QString m_distinguishedName;
    QString m_rdpAddress;
    int m_rdpPort;
    QString m_environment; // prod, test, dev
    QStringList m_userList;
    QJsonObject m_metadata;
};
