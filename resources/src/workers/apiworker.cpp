#include "apiworker.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
ApiWorker::ApiWorker(const QString &apiKey, QObject *parent)
    : QObject{parent}
    , m_apiKey(apiKey)
{
    m_manager = new QNetworkAccessManager(this);
    connect(m_manager, &QNetworkAccessManager::finished, this, &ApiWorker::onReplyFinished);
}

void ApiWorker::doRequest(const Request &request, const QString& modelUrl, const QString& modelName)
{
    //设置api节点以及关键信息
    QUrl url(modelUrl);
    QNetworkRequest req(url);

    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    req.setRawHeader("Authorization", QString("Bearer %1").arg(m_apiKey).toUtf8());

    m_currentRequestId = request.requestId;

    // 构建 Anthropic 格式请求体（含 web_search tool）
    Request reqCopy = request;
    if (!modelName.isEmpty()) {
        reqCopy.model = modelName;
    }
    QJsonObject body = JsonUtils::buildAnthropicRequestBody(reqCopy);

    //发送
    QByteArray jsonRequst = QJsonDocument(body).toJson();
    m_manager->post(req, jsonRequst);
}

//接受到networkaccess的完成信号
void ApiWorker::onReplyFinished(QNetworkReply *reply)
{
    Response response;
    response.requestId = m_currentRequestId;

    //网络错误检测
    if(reply->error() != QNetworkReply::NoError)
    {
        response.success = false;

        // 尝试从响应体中提取 API 级错误信息
        QByteArray data = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(data);
        if (doc.isObject()) {
            QString apiError = JsonUtils::extractAnthropicError(doc.object());
            if (!apiError.isEmpty()) {
                response.errorMessage = apiError;
            } else {
                response.errorMessage = reply->errorString();
            }
        } else {
            response.errorMessage = reply->errorString();
        }
        response.content = "";
        emit finished(response);

        reply->deleteLater();
        return;
    }

    //读取响应
    QByteArray data = reply->readAll();

    //解析json
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    //解析json错误处理
    if(parseError.error != QJsonParseError::NoError)
    {
        response.success = false;
        response.errorMessage = "JsonParseError:"+parseError.errorString();
        response.content = "";
        emit finished(response);

        reply->deleteLater();
        return;
    }

    QJsonObject obj = doc.object();

    // Anthropic 格式响应解析
    // 格式: {"type":"message", "role":"assistant", "content":[...], "stop_reason":"..."}
    if (obj.contains("content") && obj["content"].isArray())
    {
        QJsonArray contentBlocks = obj["content"].toArray();
        response.content = JsonUtils::extractAnthropicContent(contentBlocks);
        response.success = !response.content.isEmpty();
        response.errorMessage = response.content.isEmpty() ? "Model returned empty response" : "";
    }
    // Anthropic 错误响应: {"type":"error", "error":{...}}
    else if (obj.contains("type") && obj["type"].toString() == "error")
    {
        response.success = false;
        response.errorMessage = JsonUtils::extractAnthropicError(obj);
        response.content = "";
    }
    else
    {
        response.success = false;
        response.errorMessage = "Unknown response format";
        response.content = "";
    }

    emit finished(response);
    reply->deleteLater();
}
