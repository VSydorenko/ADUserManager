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
    
    // Базовые критерии - длина пароля
    const int length = password.length();
    
    // Бонус за длину пароля: чем длиннее, тем лучше
    strength += length * 4;
    
    // Характеристики пароля
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSpecial = false;
    
    // Подсчет каждого типа символа
    int lowerCount = 0;
    int upperCount = 0;
    int digitCount = 0;
    int specialCount = 0;
    
    for (const QChar& c : password) {
        if (c.isLower()) {
            hasLower = true;
            lowerCount++;
        }
        else if (c.isUpper()) {
            hasUpper = true;
            upperCount++;
        }
        else if (c.isDigit()) {
            hasDigit = true;
            digitCount++;
        }
        else {
            hasSpecial = true;
            specialCount++;
        }
    }
    
    // Бонусы за наличие разных типов символов    // Бонусы за разнообразие символов
    if (hasLower) strength += 5;
    if (hasUpper) strength += 5;
    if (hasDigit) strength += 5;
    if (hasSpecial) strength += 10;
    
    // Дополнительные бонусы за количество каждого типа символов
    strength += (lowerCount > 0) ? (length - lowerCount) : 0;
    strength += (upperCount > 0) ? (length - upperCount) * 1.5 : 0;
    strength += (digitCount > 0) ? digitCount * 2 : 0;
    strength += (specialCount > 0) ? specialCount * 3 : 0;
    
    // Бонус за символы в середине строки (не по краям)
    for (int i = 1; i < length - 1; ++i) {
        if (password[i].isDigit() || !password[i].isLetterOrNumber()) 
            strength += 2;
    }
    
    // Штрафы за последовательности одинакового типа
    int consecLower = 0;
    int consecUpper = 0;
    int consecDigit = 0;
    
    // Проверка последовательностей
    for (int i = 1; i < length; ++i) {
        if (password[i].isLower() && password[i-1].isLower()) consecLower++;
        if (password[i].isUpper() && password[i-1].isUpper()) consecUpper++;
        if (password[i].isDigit() && password[i-1].isDigit()) consecDigit++;
    }
    
    // Штрафы за последовательности одинакового типа
    strength -= consecLower * 2;
    strength -= consecUpper * 2;
    strength -= consecDigit * 2;
    
    // Штраф за повторяющиеся символы
    QMap<QChar, int> charCount;
    for (const QChar& c : password) {
        charCount[c]++;
    }
      // Рассчитываем штрафы за повторения
    int repeatPenalty = 0;
    QMapIterator<QChar, int> i(charCount);
    while (i.hasNext()) {
        i.next();
        if (i.value() > 1) {
            repeatPenalty += i.value() - 1;
        }
    }
    strength -= repeatPenalty * 2;
    
    // Штраф за последовательности в алфавите/клавиатуре
    static const QStringList sequences = {
        "qwertyuiop", "asdfghjkl", "zxcvbnm", // клавиатура QWERTY
        "abcdefghijklmnopqrstuvwxyz",         // алфавит
        "01234567890"                          // числовая последовательность
    };
    
    QString lowerPass = password.toLower();
    
    for (const QString& seq : sequences) {
        for (int i = 0; i <= seq.length() - 3; ++i) {
            QString subSeq = seq.mid(i, 3);
            if (lowerPass.contains(subSeq)) {
                strength -= 5;
            }
            QString revSubSeq = QString(subSeq);
            std::reverse(revSubSeq.begin(), revSubSeq.end());
            if (lowerPass.contains(revSubSeq)) {
                strength -= 5;
            }
        }
    }
    
    // Ограничение силы пароля от 0 до 100
    return qBound(0, strength, 100);
}

bool PasswordGenerator::meetsPolicy(const QString& password, const PasswordPolicy& policy) {
    // Проверка длины
    if (password.length() < policy.minLength || password.length() > policy.maxLength) {
        return false;
    }
    
    // Проверка на исключенные символы
    for (const QChar& c : policy.excludeChars) {
        if (password.contains(c)) {
            return false;
        }
    }
    
    // Проверка на требуемые типы символов (если требуется)
    if (policy.requireEachType) {
        if (!hasRequiredTypes(password, policy)) {
            return false;
        }
    }
    
    // Проверка на простые последовательности
    static const QStringList commonSequences = {
        "123", "abc", "qwe", "password", "admin"
    };
    
    QString lowerPass = password.toLower();
    for (const QString& seq : commonSequences) {
        if (lowerPass.contains(seq)) {
            return false;
        }
    }
    
    return true;
}

QString PasswordGenerator::getCharacterSet(const PasswordPolicy& policy) {
    QString characterSet;
    
    // Создаем полные наборы символов для каждого типа
    const QString lowercaseChars = "abcdefghijklmnopqrstuvwxyz";
    const QString uppercaseChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const QString numberChars = "0123456789";
    const QString symbolChars = "!@#$%^&*()-_=+[]{}|;:,.<>?/~`\"\\";
    
    // Добавляем в набор символы соответствующего типа согласно политике
    if (policy.includeLowercase) {
        for (const QChar& c : lowercaseChars) {
            if (!policy.excludeChars.contains(c)) {
                characterSet += c;
            }
        }
    }
    
    if (policy.includeUppercase) {
        for (const QChar& c : uppercaseChars) {
            if (!policy.excludeChars.contains(c)) {
                characterSet += c;
            }
        }
    }
    
    if (policy.includeNumbers) {
        for (const QChar& c : numberChars) {
            if (!policy.excludeChars.contains(c)) {
                characterSet += c;
            }
        }
    }
    
    if (policy.includeSymbols) {
        for (const QChar& c : symbolChars) {
            if (!policy.excludeChars.contains(c)) {
                characterSet += c;
            }
        }
    }
      // Проверка на пустой набор символов (если все типы были отключены)
    if (characterSet.isEmpty()) {
        // Аварийный вариант - используем только строчные буквы
        characterSet = "abcdefghijklmnopqrstuvwxyz";
        
        // И всё равно удаляем исключенные символы для безопасности
        for (const QChar& c : policy.excludeChars) {
            characterSet.remove(c);
        }
    }
    
    return characterSet;
}

bool PasswordGenerator::hasRequiredTypes(const QString& password, const PasswordPolicy& policy) {
    // Проверка наличия всех требуемых типов символов в пароле
    bool hasLower = false;
    bool hasUpper = false;
    bool hasDigit = false;
    bool hasSymbol = false;
    
    // Определяем наборы символов для проверки
    const QString lowerChars = "abcdefghijklmnopqrstuvwxyz";
    const QString upperChars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
    const QString digitChars = "0123456789";
    // Всё, что не буква и не цифра, считаем символом
    
    // Проверяем каждый символ пароля
    for (const QChar& c : password) {
        if (lowerChars.contains(c)) {
            hasLower = true;
        } else if (upperChars.contains(c)) {
            hasUpper = true;
        } else if (digitChars.contains(c)) {
            hasDigit = true;
        } else {
            hasSymbol = true;
        }
        
        // Если нашли все требуемые типы, можно прервать цикл для оптимизации
        if ((!policy.includeLowercase || hasLower) &&
            (!policy.includeUppercase || hasUpper) &&
            (!policy.includeNumbers || hasDigit) &&
            (!policy.includeSymbols || hasSymbol)) {
            break;
        }
    }
    
    // Проверяем соответствие политике
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
