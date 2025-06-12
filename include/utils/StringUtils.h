#pragma once
#include <QString>

class StringUtils {
public:
    // Ukrainian <-> Latin transliteration
    static QString transliterateUkrToLatin(const QString& ukrText);
    static QString transliterateLatinToUkr(const QString& latinText);
    
    // Case conversions
    static QString toCamelCase(const QString& text, bool capitalizeFirstLetter = false);
    static QString toSnakeCase(const QString& text);
    static QString toKebabCase(const QString& text);
    
    // Login name generators
    static QString generateLoginFromName(const QString& firstName, const QString& lastName);
    static QString sanitizeLoginName(const QString& login);
    
    // String cleaners
    static QString removeAccents(const QString& text);
    static QString normalizeSpaces(const QString& text);
    static QString removeSpecialCharacters(const QString& text, bool keepSpaces = true);
    
    // Ukrainian name processing
    static bool isValidUkrainianName(const QString& name);
    static QString normalizeUkrainianName(const QString& name);
};
