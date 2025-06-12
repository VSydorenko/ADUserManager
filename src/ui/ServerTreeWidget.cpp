#include "ui/ServerTreeWidget.h"
#include <QHeaderView>
#include <QMenu>
#include <QAction>

ServerTreeWidget::ServerTreeWidget(QWidget* parent)
    : QTreeWidget(parent)
{
    // Setup tree widget
    setHeaderLabel(tr("Servers"));
    setSelectionMode(QAbstractItemView::SingleSelection);
    setSortingEnabled(true);
    setContextMenuPolicy(Qt::CustomContextMenu);
    
    // Create environment groups
    createEnvironmentGroups();
    
    // Connect signals
    connect(this, &QTreeWidget::itemSelectionChanged, this, &ServerTreeWidget::onItemSelectionChanged);
    connect(this, &QTreeWidget::customContextMenuRequested, this, &ServerTreeWidget::onContextMenuRequested);
}

void ServerTreeWidget::setServers(const QStringList& servers)
{
    // Clear existing servers (but keep environment groups)
    for (int i = 0; i < m_productionGroup->childCount(); i++) {
        m_productionGroup->removeChild(m_productionGroup->child(i));
    }
    
    for (int i = 0; i < m_testGroup->childCount(); i++) {
        m_testGroup->removeChild(m_testGroup->child(i));
    }
    
    for (int i = 0; i < m_devGroup->childCount(); i++) {
        m_devGroup->removeChild(m_devGroup->child(i));
    }
    
    // Add servers to appropriate groups
    for (const QString& server : servers) {
        QTreeWidgetItem* item = new QTreeWidgetItem();
        item->setText(0, server);
        item->setIcon(0, QIcon(":/icons/server.svg"));
        
        QString env = getEnvironmentFromServer(server);
        if (env == "prod") {
            m_productionGroup->addChild(item);
        } else if (env == "test") {
            m_testGroup->addChild(item);
        } else {
            m_devGroup->addChild(item);
        }
    }
    
    // Expand all items
    expandAll();
}

void ServerTreeWidget::clear()
{
    QTreeWidget::clear();
    createEnvironmentGroups();
}

void ServerTreeWidget::onItemSelectionChanged()
{
    QList<QTreeWidgetItem*> selectedItems = this->selectedItems();
    if (selectedItems.isEmpty()) {
        return;
    }
    
    QTreeWidgetItem* item = selectedItems.first();
    
    // Ignore if an environment group is selected
    if (item == m_productionGroup || item == m_testGroup || item == m_devGroup) {
        return;
    }
    
    QString serverName = item->text(0);
    emit serverSelected(serverName);
}

void ServerTreeWidget::onContextMenuRequested(const QPoint& pos)
{
    QTreeWidgetItem* item = itemAt(pos);
    if (!item) {
        return;
    }
    
    // Ignore if an environment group is selected
    if (item == m_productionGroup || item == m_testGroup || item == m_devGroup) {
        return;
    }
    
    QString serverName = item->text(0);
    
    QMenu contextMenu;
    
    QAction* selectAction = contextMenu.addAction(tr("Select"));
    QAction* refreshAction = contextMenu.addAction(tr("Refresh"));
    
    QAction* result = contextMenu.exec(mapToGlobal(pos));
    
    if (result == selectAction) {
        setCurrentItem(item);
        emit serverSelected(serverName);
    } else if (result == refreshAction) {
        emit serverSelected(serverName);
    }
}

void ServerTreeWidget::createEnvironmentGroups()
{
    // Create the environment groups
    m_productionGroup = new QTreeWidgetItem(this);
    m_productionGroup->setText(0, tr("Production"));
    m_productionGroup->setIcon(0, QIcon(":/icons/folder.svg"));
    
    m_testGroup = new QTreeWidgetItem(this);
    m_testGroup->setText(0, tr("Test"));
    m_testGroup->setIcon(0, QIcon(":/icons/folder.svg"));
    
    m_devGroup = new QTreeWidgetItem(this);
    m_devGroup->setText(0, tr("Development"));
    m_devGroup->setIcon(0, QIcon(":/icons/folder.svg"));
    
    addTopLevelItem(m_productionGroup);
    addTopLevelItem(m_testGroup);
    addTopLevelItem(m_devGroup);
}

QString ServerTreeWidget::getEnvironmentFromServer(const QString& serverName) const
{
    // Simple heuristic to determine server environment from name
    QString lowerName = serverName.toLower();
    
    if (lowerName.contains("prod") || lowerName.contains("prd")) {
        return "prod";
    } else if (lowerName.contains("test") || lowerName.contains("tst") || lowerName.contains("stg")) {
        return "test";
    } else {
        return "dev"; // Default to dev environment
    }
}
