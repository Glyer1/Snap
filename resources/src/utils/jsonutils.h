#ifndef JSONUTILS_H
#define JSONUTILS_H

#include <QObject>
#include <QJsonObject>
#include <QJsonArray>
#include "../types/types.h"
class JsonUtils : public QObject
{
    Q_OBJECT
public:
    explicit JsonUtils(QObject *parent = nullptr);

    //消息转换
    static QJsonObject messageToJson(const Message& message);
    static QJsonArray messagesToJsonArray(const QList<Message>& messages);

    // Anthropic 格式请求/响应处理
    static QJsonObject buildAnthropicRequestBody(const Request& request);
    static QString extractAnthropicContent(const QJsonArray& contentBlocks);
    static QString extractAnthropicError(const QJsonObject& errorObj);

    //前后端联调文本输出格式适配
    static QVariantList parseApiResponse(const QString &text);
signals:
};

#endif // JSONUTILS_H
