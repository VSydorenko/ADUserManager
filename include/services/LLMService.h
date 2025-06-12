#pragma once
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include "models/NormalizedUser.h"

class LLMService : public QObject {
    Q_OBJECT
    
public:
    explicit LLMService(QObject* parent = nullptr);
    ~LLMService();
    
    void setApiKey(const QString& apiKey);
    void setEndpoint(const QString& endpoint);
    void setModel(const QString& model);
    
    // Main processing method
    void processUserList(const QString& rawUserList);
    
signals:
    void userListProcessed(const QList<NormalizedUser>& users);
    void processingError(const QString& error);
    void processingProgress(int percentage);
    
private slots:
    void handleNetworkReply(QNetworkReply* reply);
    
private:
    QNetworkAccessManager* m_networkManager;
    QString m_apiKey;
    QString m_endpoint;
    QString m_model;
    
    // LLM Processing
    QJsonObject buildPrompt(const QString& userList);
    QList<NormalizedUser> parseResponse(const QJsonObject& response);
    QString generateLogin(const QString& firstName, const QString& lastName);
    
    // Ukrainian name processing
    bool isValidUkrainianName(const QString& name);
    QString normalizeUkrainianName(const QString& name);
};
