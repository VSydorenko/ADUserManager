#pragma once
#include <QObject>
#include <QStringList>
#include <memory>
#include "models/ServerInfo.h"
#include "models/UserInfo.h"

#ifdef _WIN32
#include <windows.h>
#include <activeds.h>
#include <adshlp.h>
#include <comdef.h>
#pragma comment(lib, "activeds.lib")
#pragma comment(lib, "adsiid.lib")
#endif

class ADManager : public QObject {
    Q_OBJECT
    
public:
    explicit ADManager(QObject* parent = nullptr);
    ~ADManager();
    
    bool connectToAD(const QString& domain = "");
    bool isConnected() const { return m_connected; }
    
    // Server/OU Management
    QStringList getServerList();
    ServerInfo getServerInfo(const QString& serverName);
    bool createServerOU(const QString& serverName);
    bool createServerGroup(const QString& serverName);
    
    // User Management
    QStringList getUsersForServer(const QString& serverName);
    UserInfo getUserInfo(const QString& userDN);
    bool createUser(const UserInfo& user, const QString& serverName);
    bool updateUser(const UserInfo& user);
    bool deactivateUser(const QString& userDN);
    bool changePassword(const QString& userDN, const QString& newPassword);
    
    // Metadata Storage (using extensionAttributes)
    bool setServerMetadata(const QString& serverName, const QJsonObject& metadata);
    QJsonObject getServerMetadata(const QString& serverName);
    
    // Validation
    bool serverExists(const QString& serverName);
    bool userExists(const QString& login);
    QString generateUniqueLogin(const QString& firstName, const QString& lastName);
    
signals:
    void connectionStatusChanged(bool connected);
    void operationProgress(const QString& operation, int progress);
    void error(const QString& errorMessage);
    
private slots:
    void handleADError(const QString& operation, HRESULT hr);
    
private:
    bool m_connected;
    QString m_domainDN;
    QString m_serverContainer;
    QString m_userContainer;
    
    // AD Helper methods
    QString buildUserDN(const QString& login, const QString& serverName);
    QString buildServerGroupDN(const QString& serverName);
    QString buildServerOUDN(const QString& serverName);
    bool setADAttribute(const QString& objectDN, const QString& attribute, const QString& value);
    QString getADAttribute(const QString& objectDN, const QString& attribute);
    
#ifdef _WIN32
    HRESULT getObject(const QString& distinguishedName, IDispatch** ppObject);
    HRESULT getObjectAttribute(IDispatch* pObject, const QString& attributeName, VARIANT* pvAttribute);
    HRESULT setObjectAttribute(IDispatch* pObject, const QString& attributeName, VARIANT* pvAttribute);
    QString variantToString(VARIANT& var);
    void releaseInterface(IUnknown* pInterface);
#endif
};
