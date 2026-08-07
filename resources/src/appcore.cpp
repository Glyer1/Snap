#include "AppCore.h"
#include "./database/dbmanager.h"
#include <QDebug>
#include <QFile>
#include <QCoreApplication>
#include <QDir>
#include <QSettings>

AppCore* AppCore::s_instance = nullptr;

AppCore::AppCore(QObject *parent) : QObject(parent)
{
    loadApiConfig();

}

AppCore* AppCore::instance()
{
    if (!s_instance) {
        s_instance = new AppCore();
    }
    return s_instance;
}

AppCore* AppCore::create(QQmlEngine* qmlEngine, QJSEngine* jsEngine)
{
    Q_UNUSED(qmlEngine)
    Q_UNUSED(jsEngine)
    return instance();
}

//加载apiUrl的默认地址
void AppCore::loadApiConfig()
{
    // 默认值（硬编码兜底）
    m_defaultModelUrl = QStringLiteral("https://api.deepseek.com/anthropic/v1/messages");
    m_webSearchEnabled = true;

    // 尝试从 configs/api.txt 读取
    QFile configFile(QStringLiteral("resources/configs/api.txt"));
    if (!configFile.exists()) {
        // 尝试相对于应用程序目录
        configFile.setFileName(
            QCoreApplication::applicationDirPath() + QStringLiteral("/configs/api.txt"));
    }
    if (!configFile.exists()) {
        configFile.setFileName(
            QDir::currentPath() + QStringLiteral("/resources/configs/api.txt"));
    }

    if (configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QString line = QString::fromUtf8(configFile.readAll()).trimmed();
        // 格式: "company:url"
        int colonIdx = line.indexOf(':');
        if (colonIdx > 0) {
            m_defaultModelUrl = line.mid(colonIdx + 1);
        }
        configFile.close();
    }
    // 文件找不到也不报错，用默认值
}

void AppCore::setWebSearchEnabled(bool enabled)
{
    m_webSearchEnabled = enabled;
}

bool AppCore::isWebSearchEnabled() const
{
    return m_webSearchEnabled;
}

void AppCore::searchApi(const QString &query, const QString company, const QString &modelUrl, const QString &modelName)
{

    qDebug() << "进入AppCore.searchApi\n" ;

    // 如果 Key 还没加载过，先加载
    if (m_apiKey.isEmpty()) {
        qDebug() << "如果 Key 还没加载过，先加载\n" ;
        loadApiKeyAndSearch(query, company, modelUrl, modelName);
        return;
    }

    qDebug() << "直接dosearch\n" ;
    // Key 已存在，直接发起请求
    doSearch(query, modelUrl, modelName);
}

void AppCore::loadApiKeyAndSearch(const QString &query, const QString company, const QString &modelUrl, const QString &modelName)
{


    if (!m_passwordUtils) {
        qDebug() << "creating PasswordUtils";
        m_passwordUtils = new PasswordUtils(this);
    }

    // 断开旧的连接，防止重复连接
    disconnect(m_passwordUtils, &PasswordUtils::apiKeyLoaded, this, nullptr);
    disconnect(m_passwordUtils, &PasswordUtils::apiKeyLoadFailed, this, nullptr);

    // 连接信号
    connect(m_passwordUtils, &PasswordUtils::apiKeyLoaded,
            this, [this, query, modelUrl, modelName](const QString &key) {
                m_apiKey = key;
                doSearch(query, modelUrl, modelName);
            });

    qDebug() << "qtkeychain连接成功\n" ;

    connect(m_passwordUtils, &PasswordUtils::apiKeyLoadFailed,
            this, [this](const QString &error) {
                qDebug() << "读取 API Key 失败:" << error;
            });

    qDebug() << "qtkeychain获取连接成功\n" ;

    //然后读取
    m_passwordUtils->readApiKey(company);
}


void AppCore::loadApiKey(const QString company)
{
    qDebug() << "成功进入loadApiKey";

    if (!m_passwordUtils) {
        qDebug() << "creating PasswordUtils";
        m_passwordUtils = new PasswordUtils(this);
        qDebug() << "没有PasswordUtils，先创建一个";
    }

    // 断开旧的连接，防止重复连接
    disconnect(m_passwordUtils, &PasswordUtils::apiKeyLoaded, this, nullptr);
    disconnect(m_passwordUtils, &PasswordUtils::apiKeyLoadFailed, this, nullptr);

    // 连接信号
    connect(m_passwordUtils, &PasswordUtils::apiKeyLoaded,
            this, [this](const QString &key) {
                m_apiKey = key;
                qDebug() << "API Key 已自动加载";
            });

    qDebug() << "qtkeychain连接成功\n" ;

    connect(m_passwordUtils, &PasswordUtils::apiKeyLoadFailed,
            this, [this](const QString &error) {
                qDebug() << "加载 API Key 失败:" << error;
            });

    qDebug() << "qtkeychain获取连接成功\n" ;

    //然后读取
    m_passwordUtils->readApiKey(company);
}

void AppCore::doSearch(const QString &query, const QString &modelUrl, const QString &modelName)
{
    // 确定实际 URL
    QString effectiveUrl = modelUrl.isEmpty() ? m_defaultModelUrl : modelUrl;

    // 构造 Request
    Request request;
    request.model = modelName.isEmpty() ? QStringLiteral("deepseek-v4-flash") : modelName;
    request.temperature = 0.7;
    request.requestId = QUuid::createUuid().toString();
    request.webSearchEnabled = m_webSearchEnabled;

    // system prompt 作为顶层字段（Anthropic 格式），不放入 messages
    request.systemPrompt = QStringLiteral(
        "你是一个编程助手，根据用户的问题，返回可能用到的相关api并给出示例代码。"
        "若非提供问题而是询问api，直接进行联网搜索查询api内容，含义等信息。除了'xx:yy...'不要输出其他内容。"
        "每个api按以下格式输出（可返回多个api，格式同理，若下面结构为为api1,那么第二个api也是从'api名字:xx'开始，重复一整块，加一空行再到下一个api，以此类推，就为api1+空行+api2+空行+...，每个api之间用空行分隔,若为普通问题则一个即可）：\n"
        "每块需要严格按照以下格式\n\n"
        "api名字:名字\n"
        "api需引入头:引入头文件代码，如 #include <QFile> \n"
        "api解释:一句话解释这个API是做什么的\n"
        "api参数解释:参数说明\n"
        "api示例:示例代码\n"
        "api说明:示例的注释解释\n"
        "若是提问而非询问api也按照以上格式，但是api名字改成几个字总结问题回答，api解释为回答，api示例为举例，api说明为简单解释回答。除了以上'xx:{{yy}}...'不要输出其他内容。"
    );

    // 用户消息
    Message userMsg;
    userMsg.role = "user";
    userMsg.content = query;
    request.messages.append(userMsg);

    // 创建 Worker（传入 Key）
    ApiWorker *worker = new ApiWorker(m_apiKey, this);
    connect(worker, &ApiWorker::finished, this, [this, worker](const Response &response) {
        QVariantList results;
        if (response.success) {
            // 解析 AI 返回的内容
            results = JsonUtils::parseApiResponse(response.content);
            // 如果解析结果为空，说明格式不匹配，把原始内容作为一条放进去
            if (results.isEmpty()) {
                QVariantMap fallback;
                fallback["name"] = "AI 返回内容";
                fallback["desc"] = "";
                fallback["head"] = "";
                fallback["params"] = "";
                fallback["example"] = "";
                fallback["detail"] = response.content;
                results.append(fallback);
            }
        } else {
            // 错误情况也包装成一条
            QVariantMap err;
            err["name"] = "查询失败";
            err["desc"] = response.errorMessage;
            err["head"] = "";
            err["params"] = "";
            err["example"] = "";
            err["detail"] = response.errorMessage;
            results.append(err);
        }

        //存储到数据库
        if (results.size() > 0) {
            DBManager::instance()->saveApiResults(results);
        }
        emit searchResultReady(results);
        worker->deleteLater();
    });

    // 把 worker 放到线程中执行
    QThread *thread = new QThread(this);
    worker->moveToThread(thread);
    connect(thread, &QThread::started, [worker, request, effectiveUrl, modelName]() {
        worker->doRequest(request, effectiveUrl, modelName);
    });
    connect(worker, &ApiWorker::finished, thread, &QThread::quit);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

// AppCore.cpp 末尾添加

void AppCore::saveApiSettings(const QString &company, const QString &apiKey, const QString &baseUrl, const QString &model)
{
    // 1. 存 API Key 到系统凭据
    if (!m_passwordUtils) {
        m_passwordUtils = new PasswordUtils(this);
    }
    m_passwordUtils->storeApiKey(apiKey, company);

    // 2. 存 URL 和 Model 到 QSettings（非敏感信息）
    QSettings settings("Snap", "ApiConfig");
    settings.setValue("company", company);
    settings.setValue("baseUrl_" + company, baseUrl);
    settings.setValue("model_" + company, model);

    // 3. 更新内存中的值
    m_defaultModelUrl = baseUrl;
    m_apiKey = apiKey;

    qDebug() << "设置已保存: company=" << company << "url=" << baseUrl << "model=" << model;
    emit settingsChanged();
}

QString AppCore::getApiKeyForCompany(const QString &company)
{
    // 从系统凭据读取
    if (!m_passwordUtils) {
        m_passwordUtils = new PasswordUtils(this);
    }
    // 注意：这是同步读取，为了简化这里直接返回内存中的值
    // 实际使用中，如果 m_apiKey 为空，可以触发异步读取
    return m_apiKey;
}

QString AppCore::getBaseUrlForCompany(const QString &company)
{
    QSettings settings("Snap", "ApiConfig");
    return settings.value("baseUrl_" + company, "https://api.deepseek.com/anthropic/v1/messages").toString();
}

QString AppCore::getModelForCompany(const QString &company)
{
    QSettings settings("Snap", "ApiConfig");
    return settings.value("model_" + company, "deepseek-v4-flash").toString();
}
