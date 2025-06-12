#include "services/PasswordGenerator.h"
#include <QRandomGenerator>
#include <QTime>
#include <QCryptographicHash>

#ifdef _WIN32
#include <windows.h>
#include <wincrypt.h>
#pragma comment(lib, "Crypt32.lib")
#endif

PasswordGenerator::PasswordGenerator(QObject* parent) : QObject(parent) {
#ifdef _WIN32
    m_cryptoInitialized = false;
    m_hProvider = nullptr;
    initializeWindowsCrypto();
#endif

    // Seed the random generator
    QRandomGenerator::global()->seed(QTime::currentTime().msecsSinceStartOfDay());
}

PasswordGenerator::~PasswordGenerator() {
#ifdef _WIN32
    cleanupWindowsCrypto();
#endif
}

QString PasswordGenerator::generatePassword(const PasswordPolicy& policy) {
    const QString characterSet = getCharacterSet(policy);
    
    // Calculate the actual length
    int length = QRandomGenerator::global()->bounded(policy.minLength, policy.maxLength + 1);
    
    QString password;
    
    do {
        password.clear();
        
#ifdef _WIN32
        if (m_cryptoInitialized) {
            // Use Windows secure random
            QByteArray randomBytes = getSecureRandomBytes(length);
            
            for (int i = 0; i < length; ++i) {
                // Map random byte to character set
                unsigned char randomByte = static_cast<unsigned char>(randomBytes[i]);
                int index = randomByte % characterSet.length();
                password.append(characterSet[index]);
            }
        } else {
            // Fallback to Qt's random generator
            for (int i = 0; i < length; ++i) {
                int index = QRandomGenerator::global()->bounded(characterSet.length());
                password.append(characterSet[index]);
            }
        }
#else
        // Use Qt's random generator
        for (int i = 0; i < length; ++i) {
            int index = QRandomGenerator::global()->bounded(characterSet.length());
            password.append(characterSet[index]);
        }
#endif
        
    } while (policy.requireEachType && !hasRequiredTypes(password, policy));
    
    return password;
}

QStringList PasswordGenerator::generatePasswords(int count, const PasswordPolicy& policy) {
    QStringList passwords;
    
    for (int i = 0; i < count; ++i) {
        passwords << generatePassword(policy);
    }
    
    return passwords;
}

int PasswordGenerator::calculateStrength(const QString& password) {
    int strength = 0;
    
    // Length
    strength += password.length() * 4;
    
    // Character types
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    for (const QChar& c : password) {
        if (c.isLower()) hasLower = true;
        else if (c.isUpper()) hasUpper = true;
        else if (c.isDigit()) hasDigit = true;
        else hasSpecial = true;
    }
    
    // Bonuses for different character types
    if (hasLower) strength += 5;
    if (hasUpper) strength += 5;
    if (hasDigit) strength += 5;
    if (hasSpecial) strength += 10;
    
    // Middle numbers or symbols
    for (int i = 1; i < password.length() - 1; ++i) {
        if (!password[i].isLetter()) strength += 2;
    }
    
    // Penalties for consecutive same type
    int consecLower = 0;
    int consecUpper = 0;
    int consecDigit = 0;
    
    for (int i = 1; i < password.length(); ++i) {
        if (password[i].isLower() && password[i-1].isLower()) consecLower++;
        if (password[i].isUpper() && password[i-1].isUpper()) consecUpper++;
        if (password[i].isDigit() && password[i-1].isDigit()) consecDigit++;
    }
    
    strength -= consecLower * 2;
    strength -= consecUpper * 2;
    strength -= consecDigit * 2;
    
    // Penalty for repeated characters
    QMap<QChar, int> charCount;
    for (const QChar& c : password) {
        charCount[c]++;
    }
    
    for (auto it = charCount.begin(); it != charCount.end(); ++it) {
        if (it.value() > 1) strength -= it.value();
    }
    
    // Cap strength between 0 and 100
    return qBound(0, strength, 100);
}

bool PasswordGenerator::meetsPolicy(const QString& password, const PasswordPolicy& policy) {
    // Check length
    if (password.length() < policy.minLength || password.length() > policy.maxLength) {
        return false;
    }
    
    // Check for excluded characters
    for (int i = 0; i < policy.excludeChars.length(); ++i) {
        if (password.contains(policy.excludeChars[i])) {
            return false;
        }
    }
    
    // Check for required character types if needed
    return !policy.requireEachType || hasRequiredTypes(password, policy);
}

QString PasswordGenerator::getCharacterSet(const PasswordPolicy& policy) {
    QString characterSet;
    
    if (policy.includeLowercase) {
        characterSet += "abcdefghijkmnpqrstuvwxyz"; // Excluded l
    }
    
    if (policy.includeUppercase) {
        characterSet += "ABCDEFGHJKLMNPQRSTUVWXYZ"; // Excluded I, O
    }
    
    if (policy.includeNumbers) {
        characterSet += "23456789"; // Excluded 0, 1
    }
    
    if (policy.includeSymbols) {
        characterSet += "!@#$%^&*()-_=+[]{}|;:,.<>?";
    }
    
    // Remove excluded characters
    for (int i = 0; i < policy.excludeChars.length(); ++i) {
        characterSet.remove(policy.excludeChars[i]);
    }
    
    return characterSet;
}

bool PasswordGenerator::hasRequiredTypes(const QString& password, const PasswordPolicy& policy) {
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSymbol = false;
    
    for (const QChar& c : password) {
        if (c.isLower()) hasLower = true;
        else if (c.isUpper()) hasUpper = true;
        else if (c.isDigit()) hasDigit = true;
        else hasSymbol = true;
    }
    
    return (!policy.includeLowercase || hasLower) &&
           (!policy.includeUppercase || hasUpper) &&
           (!policy.includeNumbers || hasDigit) &&
           (!policy.includeSymbols || hasSymbol);
}

#ifdef _WIN32
void PasswordGenerator::initializeWindowsCrypto() {
    m_cryptoInitialized = CryptAcquireContext(
        reinterpret_cast<HCRYPTPROV*>(&m_hProvider),
        NULL,
        NULL,
        PROV_RSA_FULL,
        CRYPT_VERIFYCONTEXT
    );
    
    if (!m_cryptoInitialized) {
        qWarning() << "Failed to initialize Windows crypto provider, falling back to Qt random";
    }
}

void PasswordGenerator::cleanupWindowsCrypto() {
    if (m_cryptoInitialized && m_hProvider) {
        CryptReleaseContext(reinterpret_cast<HCRYPTPROV>(m_hProvider), 0);
        m_hProvider = nullptr;
        m_cryptoInitialized = false;
    }
}

QByteArray PasswordGenerator::getSecureRandomBytes(int count) {
    QByteArray randomBytes(count, 0);
    
    if (m_cryptoInitialized && m_hProvider) {
        if (!CryptGenRandom(
                reinterpret_cast<HCRYPTPROV>(m_hProvider),
                count,
                reinterpret_cast<BYTE*>(randomBytes.data())
            )) {
            qWarning() << "CryptGenRandom failed, falling back to Qt random";
            
            // Fallback to Qt's random generator
            for (int i = 0; i < count; ++i) {
                randomBytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
            }
        }
    } else {
        // Fallback to Qt's random generator
        for (int i = 0; i < count; ++i) {
            randomBytes[i] = static_cast<char>(QRandomGenerator::global()->bounded(256));
        }
    }
    
    return randomBytes;
}
#endif
