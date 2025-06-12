#include "services/LLMService.h"
#include <QNetworkRequest>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QUrl>
#include <QDebug>

LLMService::LLMService(QObject* parent) : QObject(parent), m_model("gpt-4") {
    m_networkManager = new QNetworkAccessManager(this);
    connect(m_networkManager, &QNetworkAccessManager::finished, this, &LLMService::handleNetworkReply);
}

LLMService::~LLMService() {
    // Network manager will be deleted automatically when parent is deleted
}

void LLMService::setApiKey(const QString& apiKey) {
    m_apiKey = apiKey;
}

void LLMService::setEndpoint(const QString& endpoint) {
    m_endpoint = endpoint;
}

void LLMService::setModel(const QString& model) {
    m_model = model;
}

void LLMService::processUserList(const QString& rawUserList) {
    if (m_apiKey.isEmpty() || m_endpoint.isEmpty()) {
        emit processingError("API key or endpoint not set");
        return;
    }
    QJsonObject requestData = buildPrompt(rawUserList);
    QJsonDocument doc(requestData);
    QByteArray data = doc.toJson();
    
    // Используем фигурные скобки вместо круглых, чтобы избежать "most vexing parse"
    QNetworkRequest request{QUrl(m_endpoint)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());
    
    emit processingProgress(10);
    m_networkManager->post(request, data);
}

void LLMService::handleNetworkReply(QNetworkReply* reply) {
    emit processingProgress(50);
    
    if (reply->error() != QNetworkReply::NoError) {
        emit processingError(QString("Network error: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }
    
    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);
    
    if (doc.isNull() || !doc.isObject()) {
        emit processingError("Invalid JSON response");
        reply->deleteLater();
        return;
    }
    
    QJsonObject responseObj = doc.object();
    QList<NormalizedUser> users = parseResponse(responseObj);
    
    emit processingProgress(100);
    emit userListProcessed(users);
    
    reply->deleteLater();
}

QJsonObject LLMService::buildPrompt(const QString& userList) {
    // Build the GPT prompt for normalizing Ukrainian names
    QString systemPrompt = QString(
        "Ты эксперт по нормализации украинских имен и фамилий. "
        "Твоя задача - преобразовать список имен в структуру 'Имя Фамилия' "
        "и сгенерировать логины в виде YFamiliya, где Y - первая буква имени."
    );
    
    QString userPrompt = QString(
        "Нормализуй список украинских имен в формат «Имя Фамилия» "
        "и сгенерируй логины в формате YFamiliya, где Y - первая буква имени.\n\n"
        "Входные данные:\n%1\n\n"
        "Верни результат только в виде массива JSON объектов:\n"
        "[\n"
        "  {\n"
        "    \"original\": \"Баришовець Ірана\",\n"
        "    \"normalized\": \"Ірина Баришовець\",\n"
        "    \"firstName\": \"Ірина\",\n"
        "    \"lastName\": \"Баришовець\",\n"
        "    \"login\": \"IBaryshoveats\"\n"
        "  }\n"
        "]\n"
    ).arg(userList);
    
    // Create the full request payload
    QJsonObject requestData;
    requestData["model"] = m_model;
    
    QJsonArray messages;
    
    QJsonObject systemMessage;
    systemMessage["role"] = "system";
    systemMessage["content"] = systemPrompt;
    messages.append(systemMessage);
    
    QJsonObject userMessage;
    userMessage["role"] = "user";
    userMessage["content"] = userPrompt;
    messages.append(userMessage);
    
    requestData["messages"] = messages;
    requestData["temperature"] = 0.1; // Low temperature for more deterministic results
    
    return requestData;
}

QList<NormalizedUser> LLMService::parseResponse(const QJsonObject& response) {
    QList<NormalizedUser> result;
    
    if (!response.contains("choices") || !response["choices"].isArray()) {
        qDebug() << "Invalid response format: missing 'choices' array";
        return result;
    }
    
    QJsonArray choices = response["choices"].toArray();
    if (choices.isEmpty()) {
        qDebug() << "Empty choices array in response";
        return result;
    }
    
    QJsonObject messageObj = choices[0].toObject()["message"].toObject();
    QString content = messageObj["content"].toString();
    
    // Parse the content as JSON
    QJsonDocument doc = QJsonDocument::fromJson(content.toUtf8());
    if (doc.isNull() || !doc.isArray()) {
        qDebug() << "Invalid JSON content in response";
        return result;
    }
    
    QJsonArray usersArray = doc.array();
    for (const QJsonValue& value : usersArray) {
        QJsonObject userObj = value.toObject();
        
        NormalizedUser user;
        user.setOriginalName(userObj["original"].toString());
        user.setNormalizedName(userObj["normalized"].toString());
        user.setFirstName(userObj["firstName"].toString());
        user.setLastName(userObj["lastName"].toString());
        user.setGeneratedLogin(userObj["login"].toString());
        user.setIsValid(true);
        
        result.append(user);
    }
    
    return result;
}

bool LLMService::isValidUkrainianName(const QString& name) {
    // Basic validation for Ukrainian names
    // Check if name contains Ukrainian characters
    static const QString ukrChars = "АаБбВвГгҐґДдЕеЄєЖжЗзИиІіЇїЙйКкЛлМмНнОоПпРрСсТтУуФфХхЦцЧчШшЩщЬьЮюЯя'";
    
    for (QChar c : name) {
        if (!c.isSpace() && !c.isPunct() && !ukrChars.contains(c)) {
            return false;
        }
    }
    
    return true;
}

QString LLMService::normalizeUkrainianName(const QString& name) {
    // This is a simplified normalization, the LLM will do the actual work
    return name.simplified();
}
