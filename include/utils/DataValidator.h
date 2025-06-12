#pragma once
#include <QString>
#include <QStringList>
#include "models/UserInfo.h"
#include "models/NormalizedUser.h"

class DataValidator {
public:
    // User validation
    static bool isValidLogin(const QString& login);
    static bool isValidFullName(const QString& fullName);
    static bool isValidPassword(const QString& password, int minLength = 8);
    
    // Server validation
    static bool isValidServerName(const QString& serverName);
    static bool isValidDomainName(const QString& domainName);
    static bool isValidDistinguishedName(const QString& dn);
    
    // Ukrainian name validation
    static bool isValidUkrainianName(const QString& name);
    
    // LLM data validation
    static bool validateNormalizedUsers(const QList<NormalizedUser>& users);
    
    // Error reporting
    static QString getLastError();
    
private:
    static thread_local QString s_lastError;
};
