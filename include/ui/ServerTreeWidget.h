#pragma once
#include <QTreeWidget>
#include <QStringList>

class ServerTreeWidget : public QTreeWidget {
    Q_OBJECT
    
public:
    explicit ServerTreeWidget(QWidget* parent = nullptr);
    
    void setServers(const QStringList& servers);
    void clear();
    
signals:
    void serverSelected(const QString& serverName);
    
private slots:
    void onItemSelectionChanged();
    void onContextMenuRequested(const QPoint& pos);
    
private:
    void createEnvironmentGroups();
    QString getEnvironmentFromServer(const QString& serverName) const;
    
    QTreeWidgetItem* m_productionGroup;
    QTreeWidgetItem* m_testGroup;
    QTreeWidgetItem* m_devGroup;
};
