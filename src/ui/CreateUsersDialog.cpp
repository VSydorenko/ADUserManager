#include "ui/CreateUsersDialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QHeaderView>
#include <QApplication>

CreateUsersDialog::CreateUsersDialog(const QStringList& servers, QWidget* parent)
    : QDialog(parent), m_llmService(nullptr), m_adManager(nullptr), m_passwordGenerator(nullptr)
{
    setWindowTitle(tr("Create Users"));
    
    setupUI();
    
    // Populate server list
    m_serverComboBox->addItems(servers);
}

void CreateUsersDialog::setLLMService(LLMService* llmService)
{
    m_llmService = llmService;
    
    if (m_llmService) {
        connect(m_llmService, &LLMService::userListProcessed, this, &CreateUsersDialog::onUserListProcessed);
        connect(m_llmService, &LLMService::processingError, this, &CreateUsersDialog::onProcessingError);
        connect(m_llmService, &LLMService::processingProgress, this, &CreateUsersDialog::onProcessingProgress);
    }
}

void CreateUsersDialog::setADManager(ADManager* adManager)
{
    m_adManager = adManager;
}

void CreateUsersDialog::setPasswordGenerator(PasswordGenerator* passwordGenerator)
{
    m_passwordGenerator = passwordGenerator;
}

QString CreateUsersDialog::getSelectedServer() const
{
    return m_serverComboBox->currentText();
}

void CreateUsersDialog::setupUI()
{
    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    
    // Server selection
    QGroupBox* serverGroup = new QGroupBox(tr("Server"));
    QVBoxLayout* serverLayout = new QVBoxLayout(serverGroup);
    
    m_serverComboBox = new QComboBox();
    serverLayout->addWidget(m_serverComboBox);
    
    mainLayout->addWidget(serverGroup);
    
    // User list input
    QGroupBox* inputGroup = new QGroupBox(tr("User List"));
    QVBoxLayout* inputLayout = new QVBoxLayout(inputGroup);
    
    QLabel* instructionLabel = new QLabel(tr("Enter a list of Ukrainian names (one per line):"));
    inputLayout->addWidget(instructionLabel);
    
    m_userListEdit = new QTextEdit();
    m_userListEdit->setPlaceholderText(tr("Enter names here...\nExample:\nІван Петренко\nОлена Коваленко"));
    inputLayout->addWidget(m_userListEdit);
    
    mainLayout->addWidget(inputGroup);
    
    // Process button
    m_processButton = new QPushButton(tr("Process with LLM"));
    mainLayout->addWidget(m_processButton);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setMinimum(0);
    m_progressBar->setMaximum(100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    // Results table
    QGroupBox* resultsGroup = new QGroupBox(tr("Processing Results"));
    QVBoxLayout* resultsLayout = new QVBoxLayout(resultsGroup);
    
    m_resultsTable = new QTableWidget(0, 5);
    m_resultsTable->setHorizontalHeaderLabels({
        tr("Original"), tr("Normalized"), tr("First Name"), 
        tr("Last Name"), tr("Login")
    });
    m_resultsTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_resultsTable->setSelectionBehavior(QTableWidget::SelectRows);
    m_resultsTable->setEditTriggers(QAbstractItemView::DoubleClicked);
    
    resultsLayout->addWidget(m_resultsTable);
    
    // Password checkbox
    m_createPasswordsCheckbox = new QCheckBox(tr("Generate random passwords"));
    m_createPasswordsCheckbox->setChecked(true);
    resultsLayout->addWidget(m_createPasswordsCheckbox);
    
    mainLayout->addWidget(resultsGroup);
    
    // Buttons
    QHBoxLayout* buttonLayout = new QHBoxLayout();
    
    m_createButton = new QPushButton(tr("Create Users"));
    m_createButton->setEnabled(false);
    
    m_cancelButton = new QPushButton(tr("Cancel"));
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(m_createButton);
    buttonLayout->addWidget(m_cancelButton);
    
    mainLayout->addLayout(buttonLayout);
    
    // Connect signals
    connect(m_processButton, &QPushButton::clicked, this, &CreateUsersDialog::onProcessClicked);
    connect(m_createButton, &QPushButton::clicked, this, &CreateUsersDialog::onCreateUsersClicked);
    connect(m_cancelButton, &QPushButton::clicked, this, &CreateUsersDialog::onCancelClicked);
    
    // Set dialog size
    resize(800, 600);
}

void CreateUsersDialog::onProcessClicked()
{
    QString userList = m_userListEdit->toPlainText().trimmed();
    
    if (userList.isEmpty()) {
        QMessageBox::warning(this, tr("Empty Input"), tr("Please enter a list of user names to process."));
        return;
    }
    
    if (!m_llmService) {
        QMessageBox::critical(this, tr("Service Error"), tr("LLM service is not available."));
        return;
    }
    
    // Clear previous results
    m_resultsTable->setRowCount(0);
    m_processedUsers.clear();
    
    // Show progress
    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);
    
    // Disable inputs while processing
    m_userListEdit->setEnabled(false);
    m_serverComboBox->setEnabled(false);
    m_processButton->setEnabled(false);
    
    // Process user list
    m_llmService->processUserList(userList);
}

void CreateUsersDialog::onCreateUsersClicked()
{
    if (m_processedUsers.isEmpty()) {
        QMessageBox::warning(this, tr("No Users"), tr("No users to create. Please process a user list first."));
        return;
    }
    
    if (!m_adManager) {
        QMessageBox::critical(this, tr("Service Error"), tr("AD manager service is not available."));
        return;
    }
    
    QString serverName = m_serverComboBox->currentText();
    if (serverName.isEmpty()) {
        QMessageBox::warning(this, tr("No Server"), tr("Please select a server."));
        return;
    }
    
    // Show progress
    m_progressBar->setValue(0);
    m_progressBar->setVisible(true);
    
    // Disable inputs
    m_resultsTable->setEnabled(false);
    m_createButton->setEnabled(false);
    
    // Generate passwords if needed
    bool generatePasswords = m_createPasswordsCheckbox->isChecked();
    
    // Create users
    int created = 0;
    int total = m_processedUsers.size();
    
    for (int i = 0; i < m_processedUsers.size(); i++) {
        NormalizedUser& normalizedUser = m_processedUsers[i];
        
        if (!normalizedUser.getIsValid()) {
            continue;
        }
        
        UserInfo user;
        user.setLogin(normalizedUser.getGeneratedLogin());
        user.setFirstName(normalizedUser.getFirstName());
        user.setLastName(normalizedUser.getLastName());
        user.setFullName(normalizedUser.getNormalizedName());
        user.setServerName(serverName);
        
        // Create password if needed
        if (generatePasswords && m_passwordGenerator) {
            QString password = m_passwordGenerator->generatePassword();
            user.setPassword(password);
        }
        
        // Create user in AD
        bool success = m_adManager->createUser(user, serverName);
        
        if (success) {
            created++;
            
            // Update table to show success
            m_resultsTable->item(i, 0)->setBackground(QBrush(QColor(200, 255, 200)));
        } else {
            // Update table to show failure
            m_resultsTable->item(i, 0)->setBackground(QBrush(QColor(255, 200, 200)));
        }
        
        // Update progress
        int progress = (i + 1) * 100 / total;
        m_progressBar->setValue(progress);
        QApplication::processEvents();
    }
    
    // Show results
    QMessageBox::information(this, tr("Users Created"),
                           tr("Created %1 of %2 users on server %3.")
                           .arg(created)
                           .arg(total)
                           .arg(serverName));
    
    // Re-enable inputs
    m_resultsTable->setEnabled(true);
    m_createButton->setEnabled(true);
    m_progressBar->setVisible(false);
    
    // Close if successful
    if (created > 0) {
        accept();
    }
}

void CreateUsersDialog::onCancelClicked()
{
    reject();
}

void CreateUsersDialog::onUserListProcessed(const QList<NormalizedUser>& users)
{
    m_processedUsers = users;
    updateTable(users);
    
    m_progressBar->setVisible(false);
    m_userListEdit->setEnabled(true);
    m_serverComboBox->setEnabled(true);
    m_processButton->setEnabled(true);
    m_createButton->setEnabled(!users.isEmpty());
}

void CreateUsersDialog::onProcessingError(const QString& error)
{
    QMessageBox::critical(this, tr("Processing Error"), error);
    
    m_progressBar->setVisible(false);
    m_userListEdit->setEnabled(true);
    m_serverComboBox->setEnabled(true);
    m_processButton->setEnabled(true);
}

void CreateUsersDialog::onProcessingProgress(int percentage)
{
    m_progressBar->setValue(percentage);
}

void CreateUsersDialog::updateTable(const QList<NormalizedUser>& users)
{
    m_resultsTable->setRowCount(users.size());
    
    for (int i = 0; i < users.size(); i++) {
        const NormalizedUser& user = users[i];
        
        m_resultsTable->setItem(i, 0, new QTableWidgetItem(user.getOriginalName()));
        m_resultsTable->setItem(i, 1, new QTableWidgetItem(user.getNormalizedName()));
        m_resultsTable->setItem(i, 2, new QTableWidgetItem(user.getFirstName()));
        m_resultsTable->setItem(i, 3, new QTableWidgetItem(user.getLastName()));
        m_resultsTable->setItem(i, 4, new QTableWidgetItem(user.getGeneratedLogin()));
        
        // Set background color based on validation
        QBrush background = user.getIsValid() 
            ? QBrush(QColor(255, 255, 255))  // White for valid
            : QBrush(QColor(255, 220, 220)); // Light red for invalid
        
        for (int col = 0; col < 5; col++) {
            m_resultsTable->item(i, col)->setBackground(background);
        }
    }
}
