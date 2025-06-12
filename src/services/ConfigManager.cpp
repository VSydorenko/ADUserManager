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

QString ConfigManager::getAdServerContainer() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "OU=Servers";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("server_container").toString("OU=Servers");
}

QString ConfigManager::getAdDefaultUserGroup() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "CN=Users,CN=Builtin";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("default_user_group").toString("CN=Users,CN=Builtin");
}

QString ConfigManager::getAdAdminGroup() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "CN=Administrators,CN=Builtin";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("admin_group").toString("CN=Administrators,CN=Builtin");
}

QString ConfigManager::getAdMetadataAttribute() const {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        return "extensionAttribute1";
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    return adConfig.value("metadata_attribute").toString("extensionAttribute1");
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

void ConfigManager::setAdServerContainer(const QString& container) {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        m_config["ad"] = QJsonObject();
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    adConfig["server_container"] = container;
    m_config["ad"] = adConfig;
}

void ConfigManager::setAdDefaultUserGroup(const QString& group) {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        m_config["ad"] = QJsonObject();
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    adConfig["default_user_group"] = group;
    m_config["ad"] = adConfig;
}

void ConfigManager::setAdAdminGroup(const QString& group) {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        m_config["ad"] = QJsonObject();
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    adConfig["admin_group"] = group;
    m_config["ad"] = adConfig;
}

void ConfigManager::setAdMetadataAttribute(const QString& attribute) {
    if (!m_config.contains("ad") || !m_config["ad"].isObject()) {
        m_config["ad"] = QJsonObject();
    }
    
    QJsonObject adConfig = m_config["ad"].toObject();
    adConfig["metadata_attribute"] = attribute;
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
    adConfig["server_container"] = "OU=Servers";
    adConfig["default_user_group"] = "CN=Users,CN=Builtin";
    adConfig["admin_group"] = "CN=Administrators,CN=Builtin";
    adConfig["metadata_attribute"] = "extensionAttribute1";
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
    
    // Name processing settings
    QJsonObject nameConfig;
    nameConfig["transliteration_mode"] = "standard_ukrainian";
    nameConfig["capitalize_first_letter"] = true;
    nameConfig["login_prefix"] = "";
    nameConfig["login_suffix"] = "";
    nameConfig["max_login_length"] = 20;
    nameConfig["allow_compound_names"] = true;
    nameConfig["compound_name_delimiter"] = "-";
    config["name_processing"] = nameConfig;
    
    // UI settings
    QJsonObject uiConfig;
    uiConfig["theme"] = "light";
    uiConfig["language"] = "ua";
    uiConfig["expand_server_tree"] = true;
    uiConfig["auto_refresh_interval"] = 300;
    config["ui"] = uiConfig;
    
    m_config = config;
}

// Name Processing Methods

QString ConfigManager::getTransliterationMode() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return "standard_ukrainian";
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("transliteration_mode").toString("standard_ukrainian");
}

bool ConfigManager::getCapitalizeFirstLetter() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return true;
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("capitalize_first_letter").toBool(true);
}

QString ConfigManager::getLoginPrefix() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return "";
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("login_prefix").toString("");
}

QString ConfigManager::getLoginSuffix() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return "";
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("login_suffix").toString("");
}

int ConfigManager::getMaxLoginLength() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return 20;
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("max_login_length").toInt(20);
}

bool ConfigManager::getAllowCompoundNames() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return true;
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("allow_compound_names").toBool(true);
}

QString ConfigManager::getCompoundNameDelimiter() const {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        return "-";
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    return nameConfig.value("compound_name_delimiter").toString("-");
}

void ConfigManager::setTransliterationMode(const QString& mode) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["transliteration_mode"] = mode;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setCapitalizeFirstLetter(bool capitalize) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["capitalize_first_letter"] = capitalize;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setLoginPrefix(const QString& prefix) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["login_prefix"] = prefix;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setLoginSuffix(const QString& suffix) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["login_suffix"] = suffix;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setMaxLoginLength(int length) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["max_login_length"] = length;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setAllowCompoundNames(bool allow) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["allow_compound_names"] = allow;
    m_config["name_processing"] = nameConfig;
}

void ConfigManager::setCompoundNameDelimiter(const QString& delimiter) {
    if (!m_config.contains("name_processing") || !m_config["name_processing"].isObject()) {
        m_config["name_processing"] = QJsonObject();
    }
    
    QJsonObject nameConfig = m_config["name_processing"].toObject();
    nameConfig["compound_name_delimiter"] = delimiter;
    m_config["name_processing"] = nameConfig;
}

// UI Settings Methods

QString ConfigManager::getUiTheme() const {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        return "light";
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    return uiConfig.value("theme").toString("light");
}

QString ConfigManager::getUiLanguage() const {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        return "ua";
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    return uiConfig.value("language").toString("ua");
}

bool ConfigManager::getExpandServerTree() const {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        return true;
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    return uiConfig.value("expand_server_tree").toBool(true);
}

int ConfigManager::getAutoRefreshInterval() const {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        return 300;
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    return uiConfig.value("auto_refresh_interval").toInt(300);
}

void ConfigManager::setUiTheme(const QString& theme) {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        m_config["ui"] = QJsonObject();
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    uiConfig["theme"] = theme;
    m_config["ui"] = uiConfig;
}

void ConfigManager::setUiLanguage(const QString& language) {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        m_config["ui"] = QJsonObject();
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    uiConfig["language"] = language;
    m_config["ui"] = uiConfig;
}

void ConfigManager::setExpandServerTree(bool expand) {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        m_config["ui"] = QJsonObject();
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    uiConfig["expand_server_tree"] = expand;
    m_config["ui"] = uiConfig;
}

void ConfigManager::setAutoRefreshInterval(int interval) {
    if (!m_config.contains("ui") || !m_config["ui"].isObject()) {
        m_config["ui"] = QJsonObject();
    }
    
    QJsonObject uiConfig = m_config["ui"].toObject();
    uiConfig["auto_refresh_interval"] = interval;
    m_config["ui"] = uiConfig;
}
