#include "models/ServerInfo.h"
#include <QJsonArray>

ServerInfo::ServerInfo() : m_rdpPort(3389), m_environment("dev") {
}

ServerInfo::ServerInfo(const QString& name, const QString& distinguishedName) 
    : m_name(name), m_distinguishedName(distinguishedName), m_rdpPort(3389), m_environment("dev") {
}

void ServerInfo::addUser(const QString& userDN) {
    if (!m_userList.contains(userDN)) {
        m_userList.append(userDN);
    }
}

bool ServerInfo::removeUser(const QString& userDN) {
    return m_userList.removeOne(userDN);
}

QJsonObject ServerInfo::toJson() const {
    QJsonObject json;
    json["name"] = m_name;
    json["distinguishedName"] = m_distinguishedName;
    json["rdpAddress"] = m_rdpAddress;
    json["rdpPort"] = m_rdpPort;
    json["environment"] = m_environment;
    
    // Convert user list to JSON array
    QJsonArray userArray;
    for (const QString& user : m_userList) {
        userArray.append(user);
    }
    json["userList"] = userArray;
    
    // Add metadata directly
    json["metadata"] = m_metadata;
    
    return json;
}

ServerInfo ServerInfo::fromJson(const QJsonObject& json) {
    ServerInfo info;
    
    info.m_name = json["name"].toString();
    info.m_distinguishedName = json["distinguishedName"].toString();
    info.m_rdpAddress = json["rdpAddress"].toString();
    info.m_rdpPort = json["rdpPort"].toInt(3389);
    info.m_environment = json["environment"].toString("dev");
    
    // Parse user list from JSON array
    QJsonArray userArray = json["userList"].toArray();
    for (const QJsonValue& value : userArray) {
        info.m_userList.append(value.toString());
    }
    
    // Parse metadata
    if (json.contains("metadata")) {
        info.m_metadata = json["metadata"].toObject();
    }
    
    return info;
}
