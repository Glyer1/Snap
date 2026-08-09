#include "DBManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QCoreApplication>

DBManager* DBManager::s_instance = nullptr;

DBManager::DBManager(QObject *parent) : QObject(parent)
{
}

DBManager::~DBManager()
{
    if (m_db.isOpen()) {
        m_db.close();
    }
}

DBManager* DBManager::instance()
{
    if (!s_instance) {
        s_instance = new DBManager();
    }
    return s_instance;
}

//初始化数据库-创建文件
bool DBManager::initDatabase(const QString &dbPath)
{
    // 确定数据库文件路径（默认在应用程序目录下）
    QString path = dbPath;
    if (path.isEmpty()) {
        path = QCoreApplication::applicationDirPath() + "/snap_history.db";
    }

    // 检查目录是否存在，不存在则创建
    QFileInfo info(path);
    if (!info.absoluteDir().exists()) {
        if (!QDir().mkpath(info.absolutePath())) {
            qDebug() << "创建数据库目录失败:" << info.absolutePath();
            return false;
        }
    }

    m_db = QSqlDatabase::addDatabase("QSQLITE");
    m_db.setDatabaseName(path);

    if (!m_db.open()) {
        qDebug() << "打开数据库失败:" << m_db.lastError().text();
        return false;
    }

    qDebug() << "数据库已打开:" << path;

    // 设置词典路径（相对于应用程序目录或绝对路径）
    QString appDir = QCoreApplication::applicationDirPath();
    QString dictPath = appDir + "/dict/";

    // 使用 cppjieba 的词典文件
    m_jieba = std::make_shared<cppjieba::Jieba>(
        (dictPath + "jieba.dict.utf8").toStdString(),
        (dictPath + "hmm_model.utf8").toStdString(),
        (dictPath + "user.dict.utf8").toStdString(),
        (dictPath + "idf.utf8").toStdString(),
        (dictPath + "stop_words.utf8").toStdString()
        );

    return createTable();
}

//创建api查询后收录结果的表
bool DBManager::createTable()
{
    QString sql = R"(
        CREATE TABLE IF NOT EXISTS api_cache (
            name TEXT PRIMARY KEY,
            head TEXT,
            desc TEXT,
            example TEXT,
            params TEXT,
            detail TEXT,
            created_at DATETIME DEFAULT CURRENT_TIMESTAMP,
            updated_at DATETIME DEFAULT CURRENT_TIMESTAMP
        )
    )";

    QSqlQuery query;
    if (!query.exec(sql)) {
        qDebug() << "创建表失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "api_cache 表已准备就绪";
    return true;
}

//插入或者替代
bool DBManager::insertOrReplace(const QVariantMap &apiData)
{
    QString sql = R"(
        INSERT OR REPLACE INTO api_cache (
            name, head, desc, example, params, detail, updated_at
        ) VALUES (?, ?, ?, ?, ?, ?, CURRENT_TIMESTAMP)
    )";

    QSqlQuery query;
    query.prepare(sql);

    query.addBindValue(apiData["name"].toString());
    query.addBindValue(apiData["head"].toString());
    query.addBindValue(apiData["desc"].toString());
    query.addBindValue(apiData["example"].toString());
    query.addBindValue(apiData["params"].toString());
    query.addBindValue(apiData["detail"].toString());

    if (!query.exec()) {
        qDebug() << "插入数据失败:" << query.lastError().text()
            << "name:" << apiData["name"].toString();
        return false;
    }

    return true;
}

//获取历史搜索过的api列表
void DBManager::getHistoryList()
{
    QString sql = R"(SELECT * FROM api_cache)";

    QVariantList result;

    QSqlQuery query;
    query.prepare(sql);

    //执行，并且进行失败处理
    if(!query.exec())
    {
        qDebug()<<"查询数据失败";
        QVariantMap map;
        map["name"] = "";
        map["head"] = "";
        map["desc"] = "";
        map["example"] = "";
        map["params"] = "";
        map["detail"] = "";
        result.append(map);
        emit getHistoryListFailed(result);
    }

    //存储
    while(query.next())
    {
        QVariantMap map;
        map["name"] = query.value(0);
        map["head"] = query.value(1);
        map["desc"] = query.value(2);
        map["example"] = query.value(3);
        map["params"] = query.value(4);
        map["detail"] = query.value(5);

        result.append(map);
    }

    emit getHistoryListSuccess(result);

}

//保存api查询结果
bool DBManager::saveApiResults(const QVariantList &results)
{
    if (results.isEmpty()) {
        return false;
    }

    // 检查第一条是否是“查询失败”或“AI 返回内容”
    QVariantMap first = results[0].toMap();
    if (first["name"].toString() == "查询失败" ||
        first["name"].toString() == "AI 返回内容") {
        qDebug() << "跳过存储无效结果:" << first["name"].toString();
        return false;
    }

    int successCount = 0;
    for (const QVariant &item : results) {
        QVariantMap map = item.toMap();
        if (map["name"].toString().isEmpty()) {
            continue;
        }
        if (insertOrReplace(map)) {
            successCount++;
        }
    }

    qDebug() << "存储 API 结果完成:" << successCount << "/" << results.size();
    return successCount > 0;
}

//查询记录
QVariantList DBManager::recordsInHistory(const QString& query)
{
    QVariantList result;
    //将QString进行分割
    QStringList keywords = splictQuery(query);
    if(keywords.isEmpty())return result;

    //查询sql用or进行查找关键字段。
    QString sql = "SELECT * FROM api_cache WHERE";

    QStringList conditions;
    for (const QString& kw : keywords) {
        // 防止 SQL 注入（简单转义）
        QString safeKw = kw;
        safeKw.replace("'", "''");
        conditions << "name LIKE '%" + safeKw + "%'";
        conditions << "head LIKE '%" + safeKw + "%'";
        conditions << "desc LIKE '%" + safeKw + "%'";
        conditions << "example LIKE '%" + safeKw + "%'";
        conditions << "params LIKE '%" + safeKw + "%'";
        conditions << "detail LIKE '%" + safeKw + "%'";

    }
    sql += "(" + conditions.join(" OR ") + ")";

    //执行查询并填充结果
    QSqlQuery queryObj;
    queryObj.prepare(sql);
    if (!queryObj.exec()) {
        qDebug() << "searchLocal 执行失败:" << queryObj.lastError().text();
        return result;
    }

    while (queryObj.next()) {
        QVariantMap map;
        map["name"] = queryObj.value(0);
        map["head"] = queryObj.value(1);
        map["desc"] = queryObj.value(2);
        map["example"] = queryObj.value(3);
        map["params"] = queryObj.value(4);
        map["detail"] = queryObj.value(5);
        result.append(map);
    }

    return result;

}

QStringList DBManager::splictQuery(const QString &query)
{
    if (!m_jieba) {
        qWarning() << "Jieba 未初始化";
        return {};
    }

    std::string utf8Query = query.toUtf8().toStdString();
    std::vector<std::string> words;

    // 使用精确模式分词
    m_jieba->Cut(utf8Query, words, true);  // true 表示使用 HMM

    QStringList result;
    for (const auto& w : words) {
        QString word = QString::fromUtf8(w.c_str());
        if (word.length() >= 2) {  // 过滤单字和标点
            result.append(word);
        }
    }

    return result;
}
