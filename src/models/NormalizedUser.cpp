#include "models/NormalizedUser.h"
#include <QStringList>

NormalizedUser::NormalizedUser() : m_isValid(false) {
}

NormalizedUser::NormalizedUser(const QString& original, const QString& normalized)
    : m_originalName(original), m_normalizedName(normalized), m_isValid(false) {
    
    parseFromNormalized();
}

void NormalizedUser::parseFromNormalized() {
    // Parse normalized name into first and last name components
    QStringList parts = m_normalizedName.split(' ', Qt::SkipEmptyParts);
    
    if (parts.size() >= 2) {
        m_firstName = parts.first();
        
        // Last name might be multi-part, so join all remaining parts
        parts.removeFirst();
        m_lastName = parts.join(" ");
        
        // Basic validation
        if (!m_firstName.isEmpty() && !m_lastName.isEmpty()) {
            m_isValid = true;
            
            // Generate simple login (will be improved by LLMService)
            m_generatedLogin = QString("%1%2")
                .arg(m_firstName.at(0).toUpper())
                .arg(m_lastName);
        } else {
            m_validationError = "Invalid name format: first or last name is empty";
        }
    } else {
        m_validationError = "Invalid name format: could not split into first and last name";
    }
}
