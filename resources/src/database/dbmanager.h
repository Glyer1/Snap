#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QVariantList>

class DBManager : public QObject
{
    Q_OBJECT
public:
    static DBManager* instance();

    // 初始化数据库（在 AppCore 构造函数或 main 中调用）
    bool initDatabase(const QString &dbPath = "snap_history.db");

    // 存储 API 结果列表
    bool saveApiResults(const QVariantList &results);

    // 后续扩展：按名字查询
    // QVariantList searchByName(const QString &keyword);

    //获取历史api列表
    void getHistoryList();
private:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    bool createTable();
    bool insertOrReplace(const QVariantMap &apiData);

    QSqlDatabase m_db;
    static DBManager* s_instance;

signals:
    void getHistoryListSuccess(QVariantList result);
    void getHistoryListFailed(QVariantList result);
};

#endif // DBMANAGER_H
