#pragma once
#include <QString>
#include <QJsonObject>

class JsonHelper {
public:
    // Convert between JsonObject and QString
    static QString jsonToString(const QJsonObject& json, bool indented = true);
    static QJsonObject stringToJson(const QString& jsonStr);
    
    // Extract values with defaults
    static QString getString(const QJsonObject& json, const QString& key, const QString& defaultValue = QString());
    static int getInt(const QJsonObject& json, const QString& key, int defaultValue = 0);
    static bool getBool(const QJsonObject& json, const QString& key, bool defaultValue = false);
    static QStringList getStringArray(const QJsonObject& json, const QString& key);
    
    // Path operations
    static QJsonObject getObjectAtPath(const QJsonObject& root, const QString& path, const QString& separator = "/");
    static void setObjectAtPath(QJsonObject& root, const QString& path, const QJsonObject& value, const QString& separator = "/");
    
    // File operations
    static QJsonObject loadFromFile(const QString& filePath);
    static bool saveToFile(const QString& filePath, const QJsonObject& json);
};
