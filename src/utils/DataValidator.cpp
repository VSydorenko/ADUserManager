#include "utils/DataValidator.h"
#include <QRegularExpression>
#include "utils/StringUtils.h"

thread_local QString DataValidator::s_lastError;

bool DataValidator::isValidLogin(const QString& login) {
    // Check for emptiness
    if (login.isEmpty()) {
        s_lastError = "Login cannot be empty";
        return false;
    }
    
    // Check length (typical AD restrictions)
    if (login.length() > 20) {
        s_lastError = "Login cannot be longer than 20 characters";
        return false;
    }
    
    // Check for valid characters (letters, numbers, no spaces)
    QRegularExpression validChars("^[a-zA-Z0-9_-]+$");
    if (!validChars.match(login).hasMatch()) {
        s_lastError = "Login can only contain letters, numbers, underscores, and hyphens";
        return false;
    }
    
    // Check that it starts with a letter
    if (!login[0].isLetter()) {
        s_lastError = "Login must start with a letter";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidFullName(const QString& fullName) {
    // Check for emptiness
    if (fullName.trimmed().isEmpty()) {
        s_lastError = "Name cannot be empty";
        return false;
    }
    
    // Check length
    if (fullName.length() > 64) {
        s_lastError = "Name is too long (max 64 characters)";
        return false;
    }
    
    // Check that it contains at least two parts (first and last name)
    QStringList nameParts = fullName.split(' ', Qt::SkipEmptyParts);
    if (nameParts.size() < 2) {
        s_lastError = "Full name must contain both first and last names";
        return false;
    }
    
    // Check for invalid characters
    QRegularExpression invalidChars("[^\\p{L}\\s'-]");
    if (invalidChars.match(fullName).hasMatch()) {
        s_lastError = "Name contains invalid characters";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidPassword(const QString& password, int minLength) {
    // Check for emptiness
    if (password.isEmpty()) {
        s_lastError = "Password cannot be empty";
        return false;
    }
    
    // Check length
    if (password.length() < minLength) {
        s_lastError = QString("Password must be at least %1 characters long").arg(minLength);
        return false;
    }
    
    // Check for complexity (at least one uppercase, one lowercase, one digit)
    bool hasUpper = false;
    bool hasLower = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    for (const QChar& c : password) {
        if (c.isUpper()) hasUpper = true;
        if (c.isLower()) hasLower = true;
        if (c.isDigit()) hasDigit = true;
        if (!c.isLetterOrNumber()) hasSpecial = true;
    }
    
    if (!hasUpper || !hasLower || !hasDigit) {
        s_lastError = "Password must contain at least one uppercase letter, one lowercase letter, and one digit";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidServerName(const QString& serverName) {
    // Check for emptiness
    if (serverName.isEmpty()) {
        s_lastError = "Server name cannot be empty";
        return false;
    }
    
    // Check length (NetBIOS restrictions)
    if (serverName.length() > 15) {
        s_lastError = "Server name cannot be longer than 15 characters";
        return false;
    }
    
    // Check for valid characters (letters, numbers, hyphens)
    QRegularExpression validChars("^[a-zA-Z0-9-]+$");
    if (!validChars.match(serverName).hasMatch()) {
        s_lastError = "Server name can only contain letters, numbers, and hyphens";
        return false;
    }
    
    // Check that it doesn't start or end with a hyphen
    if (serverName[0] == '-' || serverName[serverName.length() - 1] == '-') {
        s_lastError = "Server name cannot start or end with a hyphen";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidDomainName(const QString& domainName) {
    // Check for emptiness
    if (domainName.isEmpty()) {
        s_lastError = "Domain name cannot be empty";
        return false;
    }
    
    // Check domain name format (simplified)
    QRegularExpression validDomain("^(?:[a-z0-9](?:[a-z0-9-]*[a-z0-9])?\\.)+[a-z0-9](?:[a-z0-9-]*[a-z0-9])?$", 
                                  QRegularExpression::CaseInsensitiveOption);
    if (!validDomain.match(domainName).hasMatch()) {
        s_lastError = "Invalid domain name format";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidDistinguishedName(const QString& dn) {
    // Check for emptiness
    if (dn.isEmpty()) {
        s_lastError = "Distinguished name cannot be empty";
        return false;
    }
    
    // Basic format validation for LDAP DN
    // This is a simplified check, real validation is more complex
    if (!dn.contains('=') || !dn.contains(',')) {
        s_lastError = "Distinguished name must be in LDAP format (e.g., CN=User,OU=Users,DC=example,DC=com)";
        return false;
    }
    
    return true;
}

bool DataValidator::isValidUkrainianName(const QString& name) {
    return StringUtils::isValidUkrainianName(name);
}

bool DataValidator::validateNormalizedUsers(const QList<NormalizedUser>& users) {
    if (users.isEmpty()) {
        s_lastError = "No users to validate";
        return false;
    }
    
    bool allValid = true;
    QStringList invalidUsers;
    
    for (const NormalizedUser& user : users) {
        if (!user.getIsValid()) {
            allValid = false;
            invalidUsers.append(user.getOriginalName() + ": " + user.getValidationError());
        }
    }
    
    if (!allValid) {
        s_lastError = "The following users are invalid:\n" + invalidUsers.join("\n");
    }
    
    return allValid;
}

QString DataValidator::getLastError() {
    return s_lastError;
}
