#pragma once
#include <QString>

class NormalizedUser {
public:
    NormalizedUser();
    NormalizedUser(const QString& original, const QString& normalized);
    
    // Getters
    QString getOriginalName() const { return m_originalName; }
    QString getNormalizedName() const { return m_normalizedName; }
    QString getFirstName() const { return m_firstName; }
    QString getLastName() const { return m_lastName; }
    QString getGeneratedLogin() const { return m_generatedLogin; }
    bool getIsValid() const { return m_isValid; }
    QString getValidationError() const { return m_validationError; }
    
    // Setters
    void setOriginalName(const QString& name) { m_originalName = name; }
    void setNormalizedName(const QString& name) { m_normalizedName = name; }
    void setFirstName(const QString& name) { m_firstName = name; }
    void setLastName(const QString& name) { m_lastName = name; }
    void setGeneratedLogin(const QString& login) { m_generatedLogin = login; }
    void setIsValid(bool valid) { m_isValid = valid; }
    void setValidationError(const QString& error) { m_validationError = error; }
    
    // Methods for name processing
    void parseFromNormalized();
    
private:
    QString m_originalName;
    QString m_normalizedName;
    QString m_firstName;
    QString m_lastName;
    QString m_generatedLogin;
    bool m_isValid;
    QString m_validationError;
};
