#include "utils/JsonHelper.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QStringList>
#include <functional>
#include <QDebug>

QString JsonHelper::jsonToString(const QJsonObject& json, bool indented) {
    QJsonDocument doc(json);
    return QString::fromUtf8(doc.toJson(indented ? QJsonDocument::Indented : QJsonDocument::Compact));
}

QJsonObject JsonHelper::stringToJson(const QString& jsonStr) {
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8(), &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << error.errorString();
        return QJsonObject();
    }
    
    if (!doc.isObject()) {
        qWarning() << "JSON document is not an object";
        return QJsonObject();
    }
    
    return doc.object();
}

QString JsonHelper::getString(const QJsonObject& json, const QString& key, const QString& defaultValue) {
    if (json.contains(key) && json[key].isString()) {
        return json[key].toString();
    }
    return defaultValue;
}

int JsonHelper::getInt(const QJsonObject& json, const QString& key, int defaultValue) {
    if (json.contains(key) && json[key].isDouble()) {
        return json[key].toInt();
    }
    return defaultValue;
}

bool JsonHelper::getBool(const QJsonObject& json, const QString& key, bool defaultValue) {
    if (json.contains(key) && json[key].isBool()) {
        return json[key].toBool();
    }
    return defaultValue;
}

QStringList JsonHelper::getStringArray(const QJsonObject& json, const QString& key) {
    QStringList result;
    
    if (json.contains(key) && json[key].isArray()) {
        QJsonArray array = json[key].toArray();
        for (const QJsonValue& value : array) {
            if (value.isString()) {
                result.append(value.toString());
            }
        }
    }
    
    return result;
}

QJsonObject JsonHelper::getObjectAtPath(const QJsonObject& root, const QString& path, const QString& separator) {
    QStringList parts = path.split(separator, Qt::SkipEmptyParts);
    
    QJsonObject current = root;
    for (const QString& part : parts) {
        if (!current.contains(part) || !current[part].isObject()) {
            return QJsonObject();
        }
        current = current[part].toObject();
    }
    
    return current;
}

void JsonHelper::setObjectAtPath(QJsonObject& root, const QString& path, const QJsonObject& value, const QString& separator) {
    QStringList parts = path.split(separator, Qt::SkipEmptyParts);
    
    if (parts.isEmpty()) {
        // Replace the root with the value
        root = value;
        return;
    }
    
    // Recursively create missing objects along the path
    std::function<void(QJsonObject&, int)> setHelper = [&](QJsonObject& obj, int index) {
        const QString& part = parts[index];

        if (index == parts.size() - 1) {
            obj[part] = value;
            return;
        }

        QJsonObject child = obj.value(part).toObject();
        setHelper(child, index + 1);
        obj[part] = child;
    };

    setHelper(root, 0);
}

QJsonObject JsonHelper::loadFromFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning() << "Could not open file for reading:" << filePath;
        return QJsonObject();
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error in file" << filePath << ":" << error.errorString();
        return QJsonObject();
    }
    
    if (!doc.isObject()) {
        qWarning() << "JSON document in file" << filePath << "is not an object";
        return QJsonObject();
    }
    
    return doc.object();
}

bool JsonHelper::saveToFile(const QString& filePath, const QJsonObject& json) {
    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        qWarning() << "Could not open file for writing:" << filePath;
        return false;
    }
    
    QJsonDocument doc(json);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    return true;
}
