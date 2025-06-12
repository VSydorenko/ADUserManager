#include "services/ConfigManager.h"
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QCoreApplication>

ConfigManager::ConfigManager(QObject* parent) : QObject(parent) {
    initializeDefaultConfig();
}

bool ConfigManager::loadConfig(const QString& configPath) {
    // Use the provided path or the default one
    QString path = configPath.isEmpty() ? getDefaultConfigPath() : configPath;
    m_configPath = path;
    
    QFile file(path);
    if (!file.exists()) {
        qDebug() << "Configuration file does not exist at:" << path;
        emit configError("Configuration file not found. Creating default configuration.");
        return saveConfig(path);
    }
    
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Could not open configuration file:" << path;
        emit configError("Could not open configuration file");
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON parsing error:" << parseError.errorString();
        emit configError(QString("JSON parsing error: %1").arg(parseError.errorString()));
        return false;
    }
    
    if (!doc.isObject()) {
        qDebug() << "Configuration is not a valid JSON object";
        emit configError("Configuration is not a valid JSON object");
        return false;
    }
    
    m_config = doc.object();
    emit configLoaded();
    return true;
}

bool ConfigManager::saveConfig(const QString& configPath) {
    QString path = configPath.isEmpty() ? m_configPath : configPath;
    
    // If path is still empty, use default
    if (path.isEmpty()) {
        path = getDefaultConfigPath();
        m_configPath = path;
    }
    
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Could not open configuration file for writing:" << path;
        emit configError(QString("Could not save configuration to %1").arg(path));
        return false;
    }
    
    QJsonDocument doc(m_config);
    file.write(doc.toJson(QJsonDocument::Indented));
    file.close();
    
    emit configSaved();
    return true;
}

QString ConfigManager::getLlmProvider() const {
    if (!m_config.contains("llm") || !m_config["llm"].isObject()) {
        return "openai";
    }
    
    QJsonObject llmConfig = m_config["llm"].toObject();
    return llmConfig.value("provider").toString("openai");
}

QString ConfigManager::getLlmApiKey() const {
    if (!m_config.contains("llm") || !m_config["llm"].isObject()) {
        return QString();
    }
    
    QJsonObject llmConfig = m_config["llm"].toObject();
    return llmConfig.value("api_key").toString();
}

QString ConfigManager::getLlmEndpoint() const {
    if (!m_config.contains("llm") || !m_config["llm"].isObject()) {
        return "https://api.openai.com/v1/chat/completions";
    }
    
    QJsonObject llmConfig = m_config["llm"].toObject();
    return llmConfig.value("endpoint").toString("https://api.openai.com/v1/chat/completions");
}

QString ConfigManager::getLlmModel() const {
    if (!m_config.contains("llm") || !m_config["llm"].isObject()) {
        return "gpt-4";
    }
    
    QJsonObject llmConfig = m_config["llm"].toObject();
    return llmConfig.value("model").toString("gpt-4");
}

void ConfigManager::setLlmProvider(const QString& provider) {
    QJsonObject llmConfig = m_config.value("llm").toObject();
    llmConfig["provider"] = provider;
    m_config["llm"] = llmConfig;
}

void ConfigManager::setLlmApiKey(const QString& apiKey) {
    QJsonObject llmConfig = m_config.value("llm").toObject();
    llmConfig["api_key"] = apiKey;
    m_config["llm"] = llmConfig;
}

void ConfigManager::setLlmEndpoint(const QString& endpoint) {
    QJsonObject llmConfig = m_config.value("llm").toObject();
    llmConfig["endpoint"] = endpoint;
    m_config["llm"] = llmConfig;
}

void ConfigManager::setLlmModel(const QString& model) {
    QJsonObject llmConfig = m_config.value("llm").toObject();
    llmConfig["model"] = model;
    m_config["llm"] = llmConfig;
}

QString ConfigManager::getAdDomain() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return QString();
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("domain").toString();
}

QString ConfigManager::getAdUsersContainer() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "CN=Users";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("users_container").toString("CN=Users");
}

QString ConfigManager::getAdComputersContainer() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "CN=Computers";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("computers_container").toString("CN=Computers");
}

void ConfigManager::setAdDomain(const QString& domain) {
    QJsonObject adConfig = m_config.value("ad").toObject();
    adConfig["domain"] = domain;
    m_config["ad"] = adConfig;
}

void ConfigManager::setAdUsersContainer(const QString& container) {
    QJsonObject adConfig = m_config.value("ad").toObject();
    adConfig["users_container"] = container;
    m_config["ad"] = adConfig;
}

void ConfigManager::setAdComputersContainer(const QString& container) {
    QJsonObject adConfig = m_config.value("ad").toObject();
    adConfig["computers_container"] = container;
    m_config["ad"] = adConfig;
}

QJsonObject ConfigManager::getPasswordPolicy() const {
    if (!m_config.contains("password_policy") || !m_config["password_policy"].isObject()) {
        return QJsonObject(); // Default policy will be used
    }
    
    return m_config["password_policy"].toObject();
}

void ConfigManager::setPasswordPolicy(const QJsonObject& policy) {
    m_config["password_policy"] = policy;
}

QString ConfigManager::getDefaultConfigPath() const {
    // First check if there's a config file in the app directory
    QString appDirPath = QCoreApplication::applicationDirPath();
    QDir appDir(appDirPath);
    QString appDirConfig = appDir.filePath("app_config.json");
    
    if (QFile::exists(appDirConfig)) {
        return appDirConfig;
    }
    
    // Next check the resources directory
    QString resourceConfig = appDir.filePath("resources/config/app_config.json");
    if (QFile::exists(resourceConfig)) {
        return resourceConfig;
    }
    
    // Finally, use the config location in the user's directory
    QString configDir = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir(configDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    
    return dir.filePath("app_config.json");
}

void ConfigManager::initializeDefaultConfig() {
    QJsonObject config;
    
    // LLM settings
    QJsonObject llmConfig;
    llmConfig["provider"] = "openai";
    llmConfig["api_key"] = "";
    llmConfig["endpoint"] = "https://api.openai.com/v1/chat/completions";
    llmConfig["model"] = "gpt-4";
    config["llm"] = llmConfig;
    
    // AD settings
    QJsonObject adConfig;
    adConfig["domain"] = "example.local";
    adConfig["users_container"] = "CN=Users";
    adConfig["computers_container"] = "CN=Computers";
    config["ad"] = adConfig;
    
    // Password policy
    QJsonObject passwordPolicy;
    passwordPolicy["minLength"] = 12;
    passwordPolicy["maxLength"] = 16;
    passwordPolicy["includeUppercase"] = true;
    passwordPolicy["includeLowercase"] = true;
    passwordPolicy["includeNumbers"] = true;
    passwordPolicy["includeSymbols"] = true;
    passwordPolicy["excludeChars"] = "0O1lI";
    passwordPolicy["requireEachType"] = true;
    config["password_policy"] = passwordPolicy;
    
    m_config = config;
}
