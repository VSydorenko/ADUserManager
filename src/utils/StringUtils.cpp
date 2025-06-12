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
    // Создаем мап для обратной транслитерации (латиница -> кириллица)
    // Обратите внимание на сложные случаи (многосимвольные комбинации)
    
    // Сначала обрабатываем многосимвольные комбинации
    QString result = latinText;
    
    // Специальные комбинации (многосимвольные)
    result.replace("Shch", "Щ");
    result.replace("shch", "щ");
    result.replace("Ye", "Є");
    result.replace("ye", "є");
    result.replace("Yi", "Ї");
    result.replace("yi", "ї");
    result.replace("Yu", "Ю");
    result.replace("yu", "ю");
    result.replace("Ya", "Я");
    result.replace("ya", "я");
    result.replace("Kh", "Х");
    result.replace("kh", "х");
    result.replace("Ts", "Ц");
    result.replace("ts", "ц");
    result.replace("Ch", "Ч");
    result.replace("ch", "ч");
    result.replace("Sh", "Ш");
    result.replace("sh", "ш");
    result.replace("Zh", "Ж");
    result.replace("zh", "ж");
    
    // Односимвольные соответствия
    static QMap<QChar, QChar> singleCharMap = {
        {'A', QChar(0x0410)}, {'a', QChar(0x0430)}, // А, а
        {'B', QChar(0x0411)}, {'b', QChar(0x0431)}, // Б, б
        {'V', QChar(0x0412)}, {'v', QChar(0x0432)}, // В, в
        {'G', QChar(0x0413)}, {'g', QChar(0x0433)}, // Г, г
        {'D', QChar(0x0414)}, {'d', QChar(0x0434)}, // Д, д
        {'E', QChar(0x0415)}, {'e', QChar(0x0435)}, // Е, е
        {'Z', QChar(0x0417)}, {'z', QChar(0x0437)}, // З, з
        {'Y', QChar(0x0418)}, {'y', QChar(0x0438)}, // И, и
        {'I', QChar(0x0406)}, {'i', QChar(0x0456)}, // І, і
        {'K', QChar(0x041A)}, {'k', QChar(0x043A)}, // К, к
        {'L', QChar(0x041B)}, {'l', QChar(0x043B)}, // Л, л
        {'M', QChar(0x041C)}, {'m', QChar(0x043C)}, // М, м
        {'N', QChar(0x041D)}, {'n', QChar(0x043D)}, // Н, н
        {'O', QChar(0x041E)}, {'o', QChar(0x043E)}, // О, о
        {'P', QChar(0x041F)}, {'p', QChar(0x043F)}, // П, п
        {'R', QChar(0x0420)}, {'r', QChar(0x0440)}, // Р, р
        {'S', QChar(0x0421)}, {'s', QChar(0x0441)}, // С, с
        {'T', QChar(0x0422)}, {'t', QChar(0x0442)}, // Т, т
        {'U', QChar(0x0423)}, {'u', QChar(0x0443)}, // У, у
        {'F', QChar(0x0424)}, {'f', QChar(0x0444)}, // Ф, ф
    };
    
    // Заменяем односимвольные соответствия
    QString finalResult;
    for (const QChar& c : result) {
        if (singleCharMap.contains(c)) {
            finalResult.append(singleCharMap[c]);
        } else {
            finalResult.append(c);
        }
    }
    
    return finalResult;
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
    // Генерация логина по схеме: первая буква имени + фамилия в латинице
    
    // Убедимся, что имена стандартизированы
    QString normalizedFirstName = normalizeUkrainianName(firstName);
    QString normalizedLastName = normalizeUkrainianName(lastName);
    
    // Обработка составных имен через дефис - берем первые буквы обеих частей
    QString firstLetters;
    if (normalizedFirstName.contains('-')) {
        QStringList parts = normalizedFirstName.split('-');
        for (const QString& part : parts) {
            if (!part.isEmpty()) {
                firstLetters += part.left(1);
            }
        }
    } else {
        firstLetters = normalizedFirstName.left(1);
    }
    
    // Транслитерация имени и фамилии
    QString firstLetterLatin = transliterateUkrToLatin(firstLetters);
    QString lastNameLatin = transliterateUkrToLatin(normalizedLastName);
    
    // Объединение и очистка от специальных символов
    QString login = firstLetterLatin + lastNameLatin;
    return sanitizeLoginName(login);
}

QString StringUtils::sanitizeLoginName(const QString& login) {
    QString result;
    
    // Оставляем только буквы и цифры, преобразуем к нижнему регистру для AD-совместимости
    for (const QChar& c : login) {
        if (c.isLetterOrNumber()) {
            result += c.toLower(); // Active Directory логины обычно в нижнем регистре
        }
    }
    
    // Проверка на длину для AD (обычно ограничена 20 символами)
    if (result.length() > 20) {
        result = result.left(20);
    }
    
    // Убедимся, что логин не начинается с цифры (это может вызывать проблемы в некоторых системах)
    if (!result.isEmpty() && result[0].isDigit()) {
        result = "u" + result;
        // Повторная проверка длины после добавления префикса
        if (result.length() > 20) {
            result = result.left(20);
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
    static const QRegularExpression validNamePattern(R"(^[\p{Cyrillic}'\s-]+$)");
    
    // Проверка формата имени с помощью регулярного выражения
    if (!validNamePattern.match(name).hasMatch()) {
        return false;
    }
    
    // Проверка на наличие специфических украинских символов
    bool hasUkrainianChar = false;
    for (const QChar& c : name) {
        if (!c.isSpace() && c != '\'' && c != '-' && !ukrChars.contains(c)) {
            return false;
        }
        if (ukrChars.contains(c)) {
            hasUkrainianChar = true;
        }
    }
    
    return hasUkrainianChar;
}

QString StringUtils::normalizeUkrainianName(const QString& name) {
    // Базовая нормализация - удаление лишних пробелов
    QString normalized = name.simplified();
    
    // Обработка апострофов (замена различных видов на стандартный)
    normalized.replace(QChar(0x2019), QChar('\'')); // Right Single Quotation Mark
    normalized.replace(QChar(0x02BC), QChar('\'')); // Modifier Letter Apostrophe
    normalized.replace(QChar(0x02B9), QChar('\'')); // Modifier Letter Prime
    
    // Правильное форматирование дефисов между частями двойных имен и фамилий
    normalized.replace(QRegularExpression("\\s*-\\s*"), "-");
    
    // Обеспечение правильной капитализации - первая буква каждого слова заглавная
    QStringList parts = normalized.split(' ', Qt::SkipEmptyParts);
    for (QString& part : parts) {
        if (!part.isEmpty()) {
            // Обработка составных имен через дефис (каждая часть должна начинаться с заглавной)
            if (part.contains('-')) {
                QStringList hyphenParts = part.split('-');
                for (QString& hyphenPart : hyphenParts) {
                    if (!hyphenPart.isEmpty()) {
                        hyphenPart[0] = hyphenPart[0].toUpper();
                        
                        // Остальную часть слова в нижний регистр
                        for (int i = 1; i < hyphenPart.length(); ++i) {
                            hyphenPart[i] = hyphenPart[i].toLower();
                        }
                    }
                }
                part = hyphenParts.join("-");
            } else {
                // Обычная обработка для слова
                part[0] = part[0].toUpper();
                
                // Остальную часть слова в нижний регистр
                for (int i = 1; i < part.length(); ++i) {
                    part[i] = part[i].toLower();
                }
            }
        }
    }
    
    return parts.join(" ");
}
