#pragma once
#include <QDialog>
#include <QComboBox>
#include <QTextEdit>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QCheckBox>

#include "services/LLMService.h"
#include "services/ADManager.h"
#include "services/PasswordGenerator.h"
#include "models/NormalizedUser.h"

class CreateUsersDialog : public QDialog {
    Q_OBJECT
    
public:
    explicit CreateUsersDialog(const QStringList& servers, QWidget* parent = nullptr);
    
    void setLLMService(LLMService* llmService);
    void setADManager(ADManager* adManager);
    void setPasswordGenerator(PasswordGenerator* passwordGenerator);
    
    QString getSelectedServer() const;
    
private slots:
    void onProcessClicked();
    void onCreateUsersClicked();
    void onCancelClicked();
    
    void onUserListProcessed(const QList<NormalizedUser>& users);
    void onProcessingError(const QString& error);
    void onProcessingProgress(int percentage);
    
private:
    void setupUI();
    void updateTable(const QList<NormalizedUser>& users);
    
    // UI components
    QComboBox* m_serverComboBox;
    QTextEdit* m_userListEdit;
    QTableWidget* m_resultsTable;
    QProgressBar* m_progressBar;
    QCheckBox* m_createPasswordsCheckbox;
    
    QPushButton* m_processButton;
    QPushButton* m_createButton;
    QPushButton* m_cancelButton;
    
    // Service references
    LLMService* m_llmService;
    ADManager* m_adManager;
    PasswordGenerator* m_passwordGenerator;
    
    // Internal data
    QList<NormalizedUser> m_processedUsers;
};
