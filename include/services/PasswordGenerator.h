#pragma once
#include <QString>
#include <QObject>
#include <QVector>

struct PasswordPolicy {
    int minLength = 12;
    int maxLength = 16;
    bool includeUppercase = true;
    bool includeLowercase = true;
    bool includeNumbers = true;
    bool includeSymbols = true;
    QString excludeChars = "0O1lI";
    bool requireEachType = true;
};

class PasswordGenerator : public QObject {
    Q_OBJECT
    
public:
    explicit PasswordGenerator(QObject* parent = nullptr);
    ~PasswordGenerator();
    
    QString generatePassword(const PasswordPolicy& policy = PasswordPolicy());
    QStringList generatePasswords(int count, const PasswordPolicy& policy = PasswordPolicy());
    
    // Password strength validation
    int calculateStrength(const QString& password);
    bool meetsPolicy(const QString& password, const PasswordPolicy& policy);
    
private:
    QString getCharacterSet(const PasswordPolicy& policy);
    bool hasRequiredTypes(const QString& password, const PasswordPolicy& policy);
    
#ifdef _WIN32
    void initializeWindowsCrypto();
    void cleanupWindowsCrypto();
    QByteArray getSecureRandomBytes(int count);
    
    // Windows-specific members for crypto
    bool m_cryptoInitialized;
    void* m_hProvider; // HCRYPTPROV
#endif
};
