#pragma once
#include <QObject>
#include <QJsonObject>
#include <QString>

class ConfigManager : public QObject {
    Q_OBJECT
    
public:
    explicit ConfigManager(QObject* parent = nullptr);
    
    bool loadConfig(const QString& configPath = QString());
    bool saveConfig(const QString& configPath = QString());
    
    // LLM Settings
    QString getLlmProvider() const;
    QString getLlmApiKey() const;
    QString getLlmEndpoint() const;
    QString getLlmModel() const;
    
    void setLlmProvider(const QString& provider);
    void setLlmApiKey(const QString& apiKey);
    void setLlmEndpoint(const QString& endpoint);
    void setLlmModel(const QString& model);
      // AD Settings
    QString getAdDomain() const;
    QString getAdUsersContainer() const;
    QString getAdComputersContainer() const;
    QString getAdServerContainer() const;
    QString getAdDefaultUserGroup() const;
    QString getAdAdminGroup() const;
    QString getAdMetadataAttribute() const;
    
    void setAdDomain(const QString& domain);
    void setAdUsersContainer(const QString& container);
    void setAdComputersContainer(const QString& container);
    void setAdServerContainer(const QString& container);
    void setAdDefaultUserGroup(const QString& group);
    void setAdAdminGroup(const QString& group);
    void setAdMetadataAttribute(const QString& attribute);
    
    // Password Policy
    QJsonObject getPasswordPolicy() const;
    void setPasswordPolicy(const QJsonObject& policy);
    
    // Name Processing
    QString getTransliterationMode() const;
    bool getCapitalizeFirstLetter() const;
    QString getLoginPrefix() const;
    QString getLoginSuffix() const;
    int getMaxLoginLength() const;
    bool getAllowCompoundNames() const;
    QString getCompoundNameDelimiter() const;
    
    void setTransliterationMode(const QString& mode);
    void setCapitalizeFirstLetter(bool capitalize);
    void setLoginPrefix(const QString& prefix);
    void setLoginSuffix(const QString& suffix);
    void setMaxLoginLength(int length);
    void setAllowCompoundNames(bool allow);
    void setCompoundNameDelimiter(const QString& delimiter);
    
    // UI Settings
    QString getUiTheme() const;
    QString getUiLanguage() const;
    bool getExpandServerTree() const;
    int getAutoRefreshInterval() const;
    
    void setUiTheme(const QString& theme);
    void setUiLanguage(const QString& language);
    void setExpandServerTree(bool expand);
    void setAutoRefreshInterval(int interval);
    
signals:
    void configLoaded();
    void configSaved();
    void configError(const QString& error);
    
private:
    QString m_configPath;
    QJsonObject m_config;
    
    QString getDefaultConfigPath() const;
    void initializeDefaultConfig();
};
