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
    
    void setAdDomain(const QString& domain);
    void setAdUsersContainer(const QString& container);
    void setAdComputersContainer(const QString& container);
    
    // Password Policy
    QJsonObject getPasswordPolicy() const;
    void setPasswordPolicy(const QJsonObject& policy);
    
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
