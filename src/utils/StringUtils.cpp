#include "utils/StringUtils.h"
#include <QRegularExpression>
#include <QMap>

QString StringUtils::transliterateUkrToLatin(const QString& ukrText) {
    static QMap<QChar, QString> transliterationMap = {
        {QChar(0x0410), "A"},    // А
        {QChar(0x0411), "B"},    // Б
        {QChar(0x0412), "V"},    // В
        {QChar(0x0413), "G"},    // Г
        {QChar(0x0490), "G"},    // Ґ
        {QChar(0x0414), "D"},    // Д
        {QChar(0x0415), "E"},    // Е
        {QChar(0x0404), "Ye"},   // Є
        {QChar(0x0416), "Zh"},   // Ж
        {QChar(0x0417), "Z"},    // З
        {QChar(0x0418), "Y"},    // И
        {QChar(0x0406), "I"},    // І
        {QChar(0x0407), "Yi"},   // Ї
        {QChar(0x0419), "Y"},    // Й
        {QChar(0x041A), "K"},    // К
        {QChar(0x041B), "L"},    // Л
        {QChar(0x041C), "M"},    // М
        {QChar(0x041D), "N"},    // Н
        {QChar(0x041E), "O"},    // О
        {QChar(0x041F), "P"},    // П
        {QChar(0x0420), "R"},    // Р
        {QChar(0x0421), "S"},    // С
        {QChar(0x0422), "T"},    // Т
        {QChar(0x0423), "U"},    // У
        {QChar(0x0424), "F"},    // Ф
        {QChar(0x0425), "Kh"},   // Х
        {QChar(0x0426), "Ts"},   // Ц
        {QChar(0x0427), "Ch"},   // Ч
        {QChar(0x0428), "Sh"},   // Ш
        {QChar(0x0429), "Shch"}, // Щ
        {QChar(0x042C), ""},     // Ь
        {QChar(0x042E), "Yu"},   // Ю
        {QChar(0x042F), "Ya"},   // Я
        
        // Lowercase
        {QChar(0x0430), "a"},    // а
        {QChar(0x0431), "b"},    // б
        {QChar(0x0432), "v"},    // в
        {QChar(0x0433), "g"},    // г
        {QChar(0x0491), "g"},    // ґ
        {QChar(0x0434), "d"},    // д
        {QChar(0x0435), "e"},    // е
        {QChar(0x0454), "ye"},   // є
        {QChar(0x0436), "zh"},   // ж
        {QChar(0x0437), "z"},    // з
        {QChar(0x0438), "y"},    // и
        {QChar(0x0456), "i"},    // і
        {QChar(0x0457), "yi"},   // ї
        {QChar(0x0439), "y"},    // й
        {QChar(0x043A), "k"},    // к
        {QChar(0x043B), "l"},    // л
        {QChar(0x043C), "m"},    // м
        {QChar(0x043D), "n"},    // н
        {QChar(0x043E), "o"},    // о
        {QChar(0x043F), "p"},    // п
        {QChar(0x0440), "r"},    // р
        {QChar(0x0441), "s"},    // с
        {QChar(0x0442), "t"},    // т
        {QChar(0x0443), "u"},    // у
        {QChar(0x0444), "f"},    // ф
        {QChar(0x0445), "kh"},   // х
        {QChar(0x0446), "ts"},   // ц
        {QChar(0x0447), "ch"},   // ч
        {QChar(0x0448), "sh"},   // ш
        {QChar(0x0449), "shch"}, // щ
        {QChar(0x044C), ""},     // ь
        {QChar(0x044E), "yu"},   // ю
        {QChar(0x044F), "ya"}    // я
    };
    
    QString result;
    for (const QChar& c : ukrText) {
        if (transliterationMap.contains(c)) {
            result += transliterationMap[c];
        } else {
            result += c;
        }
    }
    
    return result;
}

QString StringUtils::transliterateLatinToUkr(const QString& latinText) {
    // This is a more complex operation due to multi-character transliteration
    // For simplicity, we'll implement a basic version
    
    // Not fully implemented in this example
    return latinText;
}

QString StringUtils::toCamelCase(const QString& text, bool capitalizeFirstLetter) {
    QString result;
    bool nextUpper = capitalizeFirstLetter;
    
    for (const QChar& c : text) {
        if (c.isLetterOrNumber()) {
            result += nextUpper ? c.toUpper() : c.toLower();
            nextUpper = false;
        } else {
            nextUpper = true;
        }
    }
    
    return result;
}

QString StringUtils::toSnakeCase(const QString& text) {
    QString result;
    
    for (int i = 0; i < text.length(); ++i) {
        QChar c = text[i];
        if (c.isUpper() && i > 0) {
            result += '_';
        }
        result += c.toLower();
    }
    
    // Replace any sequences of non-alphanumeric with single underscore
    result.replace(QRegularExpression("[^a-z0-9]+"), "_");
    
    return result;
}

QString StringUtils::toKebabCase(const QString& text) {
    QString result = toSnakeCase(text);
    result.replace('_', '-');
    return result;
}

QString StringUtils::generateLoginFromName(const QString& firstName, const QString& lastName) {
    // First letter of first name + last name in Latin
    QString firstLetter = transliterateUkrToLatin(firstName.left(1)).toUpper();
    QString lastNameLatin = transliterateUkrToLatin(lastName);
    
    // Remove any spaces or special characters
    QString login = firstLetter + lastNameLatin;
    return sanitizeLoginName(login);
}

QString StringUtils::sanitizeLoginName(const QString& login) {
    QString result;
    
    // Keep only letters and numbers
    for (const QChar& c : login) {
        if (c.isLetterOrNumber()) {
            result += c;
        }
    }
    
    return result;
}

QString StringUtils::removeAccents(const QString& text) {
    QString result;
    QString decomposed = text.normalized(QString::NormalizationForm_D);
    
    for (const QChar& c : decomposed) {
        if (c.category() != QChar::Mark_NonSpacing) {
            result += c;
        }
    }
    
    return result;
}

QString StringUtils::normalizeSpaces(const QString& text) {
    return text.simplified();
}

QString StringUtils::removeSpecialCharacters(const QString& text, bool keepSpaces) {
    QString result;
    
    for (const QChar& c : text) {
        if (c.isLetterOrNumber() || (keepSpaces && c.isSpace())) {
            result += c;
        }
    }
    
    return result;
}

bool StringUtils::isValidUkrainianName(const QString& name) {
    // Check if name contains Ukrainian characters
    static const QString ukrChars = "АаБбВвГгҐґДдЕеЄєЖжЗзИиІіЇїЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЬьЮюЯя'";
    
    bool hasUkrainianChar = false;
    for (const QChar& c : name) {
        if (!c.isSpace() && !ukrChars.contains(c)) {
            return false;
        }
        if (ukrChars.contains(c)) {
            hasUkrainianChar = true;
        }
    }
    
    return hasUkrainianChar;
}

QString StringUtils::normalizeUkrainianName(const QString& name) {
    // Basic normalization - remove extra spaces
    QString normalized = name.simplified();
    
    // Ensure proper capitalization - first letter of each word is uppercase
    QStringList parts = normalized.split(' ', Qt::SkipEmptyParts);
    for (QString& part : parts) {
        if (!part.isEmpty()) {
            part[0] = part[0].toUpper();
            
            // Make rest of the word lowercase
            for (int i = 1; i < part.length(); ++i) {
                part[i] = part[i].toLower();
            }
        }
    }
    
    return parts.join(" ");
}
