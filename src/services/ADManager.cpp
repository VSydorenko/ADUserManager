#include "services/ADManager.h"
#include <QDebug>
#include <QJsonDocument>
#include <QJsonArray>

ADManager::ADManager(QObject* parent) : QObject(parent), m_connected(false) {
#ifdef _WIN32
    // Initialize COM on creation
    CoInitialize(NULL);
#endif
}

ADManager::~ADManager() {
#ifdef _WIN32
    // Uninitialize COM on destruction
    CoUninitialize();
#endif
}

bool ADManager::connectToAD(const QString& domain) {
    bool result = false;
    
#ifdef _WIN32
    try {
        // If domain is not provided, use the default domain
        if (domain.isEmpty()) {
            IADsNamespaces* pNamespaces = nullptr;
            HRESULT hr = CoCreateInstance(CLSID_ADSystemInfo, NULL, CLSCTX_INPROC_SERVER, 
                                         IID_IADsNamespaces, (void**)&pNamespaces);
            
            if (SUCCEEDED(hr)) {
                m_domainDN = "LDAP://DC=example,DC=com"; // Default domain DN
                releaseInterface(pNamespaces);
            } else {
                emit error("Failed to get default domain info");
                return false;
            }
        } else {
            m_domainDN = "LDAP://" + domain;
        }
        
        // Try to bind to the domain
        IDirectorySearch* pDirSearch = nullptr;
        HRESULT hr = ADsOpenObject(
            reinterpret_cast<LPCWSTR>(m_domainDN.utf16()),
            NULL,
            NULL,
            ADS_SECURE_AUTHENTICATION,
            IID_IDirectorySearch,
            (void**)&pDirSearch
        );
        
        if (SUCCEEDED(hr)) {
            m_connected = true;
            // Set up container paths
            m_serverContainer = "CN=Computers," + domain;
            m_userContainer = "CN=Users," + domain;
            
            releaseInterface(pDirSearch);
            result = true;
            emit connectionStatusChanged(true);
        } else {
            handleADError("Connect to AD", hr);
        }
    } catch (...) {
        emit error("Unknown exception when connecting to AD");
    }
#else
    emit error("AD functionality is only available on Windows");
#endif

    return result;
}

QStringList ADManager::getServerList() {
    QStringList serverList;
    
    if (!m_connected) {
        emit error("Not connected to AD");
        return serverList;
    }
    
#ifdef _WIN32
    // Implementation for fetching server list from AD
    // This would query the container where servers are stored
    
    // Sample mock data
    serverList << "SERVER01" << "SERVER02" << "DEVSERVER" << "TESTSERVER";
#endif

    return serverList;
}

ServerInfo ADManager::getServerInfo(const QString& serverName) {
    ServerInfo serverInfo;
    
    if (!m_connected) {
        emit error("Not connected to AD");
        return serverInfo;
    }
    
    if (!serverExists(serverName)) {
        emit error(QString("Server %1 does not exist").arg(serverName));
        return serverInfo;
    }
    
#ifdef _WIN32
    // Implementation for fetching server information from AD
    // This would query AD for the server object and its attributes
    
    // For now, create mock data
    serverInfo.setName(serverName);
    serverInfo.setDistinguishedName(buildServerOUDN(serverName));
    serverInfo.setRdpAddress(serverName + ".example.com");
    serverInfo.setRdpPort(3389);
    serverInfo.setEnvironment(serverName.toLower().contains("dev") ? "dev" : 
                             (serverName.toLower().contains("test") ? "test" : "prod"));
    
    // Get users for this server
    QStringList users = getUsersForServer(serverName);
    serverInfo.setUserList(users);
    
    // Get metadata
    QJsonObject metadata = getServerMetadata(serverName);
    serverInfo.setMetadata(metadata);
#endif

    return serverInfo;
}

bool ADManager::createServerOU(const QString& serverName) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    if (serverExists(serverName)) {
        // Server already exists, no need to create
        return true;
    }
    
#ifdef _WIN32
    // Implementation for creating a new OU for the server in AD
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::createServerGroup(const QString& serverName) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    QString groupDN = buildServerGroupDN(serverName);
    
#ifdef _WIN32
    // Implementation for creating a new group for the server in AD
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

QStringList ADManager::getUsersForServer(const QString& serverName) {
    QStringList userList;
    
    if (!m_connected) {
        emit error("Not connected to AD");
        return userList;
    }
    
    if (!serverExists(serverName)) {
        emit error(QString("Server %1 does not exist").arg(serverName));
        return userList;
    }
    
#ifdef _WIN32
    // Implementation for fetching users for a specific server from AD
    // This would query the group or OU associated with the server and get its members
    
    // Sample mock data
    if (serverName == "SERVER01") {
        userList << "CN=User1,OU=SERVER01,DC=example,DC=com" 
                << "CN=User2,OU=SERVER01,DC=example,DC=com";
    } else if (serverName == "DEVSERVER") {
        userList << "CN=DevUser1,OU=DEVSERVER,DC=example,DC=com"
                << "CN=DevUser2,OU=DEVSERVER,DC=example,DC=com"
                << "CN=DevUser3,OU=DEVSERVER,DC=example,DC=com";
    }
#endif

    return userList;
}

UserInfo ADManager::getUserInfo(const QString& userDN) {
    UserInfo userInfo;
    
    if (!m_connected) {
        emit error("Not connected to AD");
        return userInfo;
    }
    
#ifdef _WIN32
    // Implementation for fetching user information from AD
    // This would query AD for the user object and its attributes
    
    // Extract server name from the DN
    QRegExp serverRegex("OU=([^,]+),");
    if (serverRegex.indexIn(userDN) != -1) {
        QString serverName = serverRegex.cap(1);
        userInfo.setServerName(serverName);
    }
    
    // Extract login from the DN
    QRegExp loginRegex("CN=([^,]+),");
    if (loginRegex.indexIn(userDN) != -1) {
        QString login = loginRegex.cap(1);
        userInfo.setLogin(login);
    }
    
    // Set distinguished name
    userInfo.setDistinguishedName(userDN);
    
    // For now, create mock data
    if (userDN.contains("User1")) {
        userInfo.setFullName("Ivan Petrov");
        userInfo.setFirstName("Ivan");
        userInfo.setLastName("Petrov");
        userInfo.setCreatedDate(QDateTime::currentDateTime().addDays(-30));
        userInfo.setLastLogin(QDateTime::currentDateTime().addDays(-2));
        userInfo.setActive(true);
    } else if (userDN.contains("User2")) {
        userInfo.setFullName("Olga Sidorova");
        userInfo.setFirstName("Olga");
        userInfo.setLastName("Sidorova");
        userInfo.setCreatedDate(QDateTime::currentDateTime().addDays(-15));
        userInfo.setLastLogin(QDateTime::currentDateTime().addDays(-5));
        userInfo.setActive(true);
    } else if (userDN.contains("DevUser")) {
        userInfo.setFullName("Developer User");
        userInfo.setFirstName("Developer");
        userInfo.setLastName("User");
        userInfo.setCreatedDate(QDateTime::currentDateTime().addDays(-60));
        userInfo.setLastLogin(QDateTime::currentDateTime().addDays(-1));
        userInfo.setActive(true);
    }
#endif

    return userInfo;
}

bool ADManager::createUser(const UserInfo& user, const QString& serverName) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    // Create server OU and group if they don't exist
    if (!serverExists(serverName)) {
        if (!createServerOU(serverName)) {
            emit error(QString("Failed to create server OU for %1").arg(serverName));
            return false;
        }
        
        if (!createServerGroup(serverName)) {
            emit error(QString("Failed to create server group for %1").arg(serverName));
            return false;
        }
    }
    
    // Check if the user already exists
    if (userExists(user.getLogin())) {
        emit error(QString("User %1 already exists").arg(user.getLogin()));
        return false;
    }
    
#ifdef _WIN32
    // Implementation for creating a new user in AD
    // This would create a user in the server's OU and add them to the server's group
    
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::updateUser(const UserInfo& user) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    if (!userExists(user.getLogin())) {
        emit error(QString("User %1 does not exist").arg(user.getLogin()));
        return false;
    }
    
#ifdef _WIN32
    // Implementation for updating user information in AD
    // This would update the user's attributes in AD
    
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::deactivateUser(const QString& userDN) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
#ifdef _WIN32
    // Implementation for deactivating a user in AD
    // This would typically set the user's account to disabled
    
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::changePassword(const QString& userDN, const QString& newPassword) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
#ifdef _WIN32
    // Implementation for changing a user's password in AD
    
    // For now, return true as if it was successful
    return true;
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::setServerMetadata(const QString& serverName, const QJsonObject& metadata) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    if (!serverExists(serverName)) {
        emit error(QString("Server %1 does not exist").arg(serverName));
        return false;
    }
    
    QString groupDN = buildServerGroupDN(serverName);
    QString jsonString = QJsonDocument(metadata).toJson(QJsonDocument::Compact);
    
#ifdef _WIN32
    // Implementation for storing metadata in AD
    // This would set an extensionAttribute on the server's group or OU
    return setADAttribute(groupDN, "extensionAttribute1", jsonString);
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

QJsonObject ADManager::getServerMetadata(const QString& serverName) {
    QJsonObject metadata;
    
    if (!m_connected) {
        emit error("Not connected to AD");
        return metadata;
    }
    
    if (!serverExists(serverName)) {
        emit error(QString("Server %1 does not exist").arg(serverName));
        return metadata;
    }
    
    QString groupDN = buildServerGroupDN(serverName);
    
#ifdef _WIN32
    // Implementation for retrieving metadata from AD
    // This would get the extensionAttribute from the server's group or OU
    
    QString jsonString = getADAttribute(groupDN, "extensionAttribute1");
    if (!jsonString.isEmpty()) {
        QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());
        if (!doc.isNull() && doc.isObject()) {
            metadata = doc.object();
        }
    }
#endif

    return metadata;
}

bool ADManager::serverExists(const QString& serverName) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
    QString ouDN = buildServerOUDN(serverName);
    
#ifdef _WIN32
    // Implementation for checking if a server exists in AD
    // This would check if the server's OU exists
    
    // For now, we'll say all servers exist
    QStringList knownServers = {"SERVER01", "SERVER02", "DEVSERVER", "TESTSERVER"};
    return knownServers.contains(serverName);
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

bool ADManager::userExists(const QString& login) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return false;
    }
    
#ifdef _WIN32
    // Implementation for checking if a user exists in AD
    
    // For now, say these users already exist
    QStringList existingUsers = {"user1", "user2", "devuser1", "devuser2", "devuser3"};
    return existingUsers.contains(login.toLower());
#else
    emit error("AD functionality is only available on Windows");
    return false;
#endif
}

QString ADManager::generateUniqueLogin(const QString& firstName, const QString& lastName) {
    if (!m_connected) {
        emit error("Not connected to AD");
        return QString();
    }
    
    // Basic algorithm: first letter of first name + last name
    QString baseLogin = firstName.left(1).toUpper() + lastName;
    
    // Remove spaces and special characters
    baseLogin.remove(QRegExp("[^a-zA-Z0-9]"));
    
    // If the login already exists, append a number
    QString login = baseLogin;
    int suffix = 1;
    
    while (userExists(login)) {
        login = baseLogin + QString::number(suffix++);
    }
    
    return login;
}

void ADManager::handleADError(const QString& operation, HRESULT hr) {
#ifdef _WIN32
    _com_error err(hr);
    LPCTSTR errMsg = err.ErrorMessage();
    QString qErrorMsg = QString::fromWCharArray(errMsg);
    
    emit error(QString("AD Error during %1: %2 (0x%3)")
              .arg(operation)
              .arg(qErrorMsg)
              .arg(QString::number(hr, 16)));
#else
    Q_UNUSED(operation);
    Q_UNUSED(hr);
#endif
}

QString ADManager::buildUserDN(const QString& login, const QString& serverName) {
    return QString("CN=%1,OU=%2,%3")
           .arg(login)
           .arg(serverName)
           .arg(m_domainDN.mid(7)); // Remove "LDAP://" prefix
}

QString ADManager::buildServerGroupDN(const QString& serverName) {
    return QString("CN=%1-Group,OU=%2,%3")
           .arg(serverName)
           .arg(serverName)
           .arg(m_domainDN.mid(7)); // Remove "LDAP://" prefix
}

QString ADManager::buildServerOUDN(const QString& serverName) {
    return QString("OU=%1,%2")
           .arg(serverName)
           .arg(m_domainDN.mid(7)); // Remove "LDAP://" prefix
}

bool ADManager::setADAttribute(const QString& objectDN, const QString& attribute, const QString& value) {
#ifdef _WIN32
    bool result = false;
    IDispatch* pObject = nullptr;
    
    HRESULT hr = getObject(objectDN, &pObject);
    if (SUCCEEDED(hr) && pObject) {
        VARIANT var;
        VariantInit(&var);
        var.vt = VT_BSTR;
        var.bstrVal = SysAllocString(reinterpret_cast<const OLECHAR*>(value.utf16()));
        
        hr = setObjectAttribute(pObject, attribute, &var);
        result = SUCCEEDED(hr);
        
        VariantClear(&var);
        releaseInterface(pObject);
    }
    
    return result;
#else
    Q_UNUSED(objectDN);
    Q_UNUSED(attribute);
    Q_UNUSED(value);
    return false;
#endif
}

QString ADManager::getADAttribute(const QString& objectDN, const QString& attribute) {
#ifdef _WIN32
    QString result;
    IDispatch* pObject = nullptr;
    
    HRESULT hr = getObject(objectDN, &pObject);
    if (SUCCEEDED(hr) && pObject) {
        VARIANT var;
        VariantInit(&var);
        
        hr = getObjectAttribute(pObject, attribute, &var);
        if (SUCCEEDED(hr)) {
            result = variantToString(var);
        }
        
        VariantClear(&var);
        releaseInterface(pObject);
    }
    
    return result;
#else
    Q_UNUSED(objectDN);
    Q_UNUSED(attribute);
    return QString();
#endif
}

#ifdef _WIN32
HRESULT ADManager::getObject(const QString& distinguishedName, IDispatch** ppObject) {
    return ADsOpenObject(
        reinterpret_cast<LPCWSTR>(QString("LDAP://" + distinguishedName).utf16()),
        NULL,
        NULL,
        ADS_SECURE_AUTHENTICATION,
        IID_IDispatch,
        (void**)ppObject
    );
}

HRESULT ADManager::getObjectAttribute(IDispatch* pObject, const QString& attributeName, VARIANT* pvAttribute) {
    if (!pObject || !pvAttribute) {
        return E_INVALIDARG;
    }
    
    HRESULT hr = E_FAIL;
    DISPID dispid;
    OLECHAR* szMember = (OLECHAR*)reinterpret_cast<const OLECHAR*>(attributeName.utf16());
    
    hr = pObject->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_USER_DEFAULT, &dispid);
    if (SUCCEEDED(hr)) {
        DISPPARAMS dp = {NULL, NULL, 0, 0};
        hr = pObject->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYGET, &dp, pvAttribute, NULL, NULL);
    }
    
    return hr;
}

HRESULT ADManager::setObjectAttribute(IDispatch* pObject, const QString& attributeName, VARIANT* pvAttribute) {
    if (!pObject || !pvAttribute) {
        return E_INVALIDARG;
    }
    
    HRESULT hr = E_FAIL;
    DISPID dispid;
    OLECHAR* szMember = (OLECHAR*)reinterpret_cast<const OLECHAR*>(attributeName.utf16());
    
    hr = pObject->GetIDsOfNames(IID_NULL, &szMember, 1, LOCALE_USER_DEFAULT, &dispid);
    if (SUCCEEDED(hr)) {
        DISPPARAMS dp = {pvAttribute, NULL, 1, 0};
        DISPID dispidPut = DISPID_PROPERTYPUT;
        dp.rgdispidNamedArgs = &dispidPut;
        dp.cNamedArgs = 1;
        
        hr = pObject->Invoke(dispid, IID_NULL, LOCALE_USER_DEFAULT, DISPATCH_PROPERTYPUT, &dp, NULL, NULL, NULL);
        
        // If property was set successfully, save changes
        if (SUCCEEDED(hr)) {
            IADs* pIADs = nullptr;
            hr = pObject->QueryInterface(IID_IADs, (void**)&pIADs);
            if (SUCCEEDED(hr) && pIADs) {
                hr = pIADs->SetInfo();
                releaseInterface(pIADs);
            }
        }
    }
    
    return hr;
}

QString ADManager::variantToString(VARIANT& var) {
    QString result;
    
    if (var.vt == VT_BSTR) {
        result = QString::fromWCharArray(var.bstrVal);
    } else if (var.vt == VT_ARRAY | VT_VARIANT) {
        // Handle array types if needed
    }
    
    return result;
}

void ADManager::releaseInterface(IUnknown* pInterface) {
    if (pInterface) {
        pInterface->Release();
        pInterface = nullptr;
    }
}
#endif
