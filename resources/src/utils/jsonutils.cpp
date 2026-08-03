#include "jsonutils.h"

JsonUtils::JsonUtils(QObject *parent)
    : QObject{parent}
{}

//返回jsonobj格式的message
QJsonObject JsonUtils::messageToJson(const Message &message)
{
    QJsonObject msgObj;
    msgObj["content"] = message.content;
    msgObj["role"] = message.role;
    return msgObj;
}

//将对话表的所有信息全部转换成jsonArray并返回
QJsonArray JsonUtils::messagesToJsonArray(const QList<Message> &messages)
{
    QJsonArray array1;
    for(const Message& msg: messages)
    {
        array1.append(messageToJson(msg));
    }
    return array1;
}

// === Anthropic 格式 ===

// 构建 Anthropic Messages API 请求体
QJsonObject JsonUtils::buildAnthropicRequestBody(const Request &request)
{
    QJsonObject body;
    body["model"] = request.model;
    body["max_tokens"] = request.maxTokens;
    body["temperature"] = request.temperature;

    // system prompt 作为顶层字段（Anthropic 格式，非 messages 内）
    if (!request.systemPrompt.isEmpty()) {
        body["system"] = request.systemPrompt;
    }

    // messages 数组（只放 user/assistant，不含 system）
    QJsonArray messagesArr;
    for (const Message& msg : request.messages) {
        QJsonObject msgObj;
        msgObj["role"] = msg.role;
        msgObj["content"] = msg.content;
        messagesArr.append(msgObj);
    }
    body["messages"] = messagesArr;

    // 联网搜索工具
    if (request.webSearchEnabled) {
        QJsonObject webSearchTool;
        webSearchTool["type"] = QStringLiteral("web_search_20250305");
        webSearchTool["name"] = QStringLiteral("web_search");
        webSearchTool["input_schema"] = QJsonObject{
            {"type", "object"},
            {"properties", QJsonObject{
                {"query", QJsonObject{
                    {"type", "string"},
                    {"description", "The search query"}
                }}
            }},
            {"required", QJsonArray{"query"}}
        };

        QJsonArray tools;
        tools.append(webSearchTool);
        body["tools"] = tools;

        QJsonObject toolChoice;
        toolChoice["type"] = QStringLiteral("auto");
        body["tool_choice"] = toolChoice;
    }

    return body;
}

// 从 Anthropic 响应 content 数组中提取文本
QString JsonUtils::extractAnthropicContent(const QJsonArray &contentBlocks)
{
    QStringList textParts;
    QStringList sources;

    for (const QJsonValue &val : contentBlocks) {
        QJsonObject block = val.toObject();
        QString type = block["type"].toString();

        if (type == "text") {
            textParts.append(block["text"].toString());
        } else if (type == "web_search_tool_result") {
            // 提取搜索来源
            QJsonArray results = block["content"].toArray();
            for (const QJsonValue &rv : results) {
                QJsonObject result = rv.toObject();
                if (result["type"].toString() == "text") {
                    sources.append(result["text"].toString());
                }
            }
        }
        // server_tool_use 块跳过（服务端自动处理）
    }

    QString output = textParts.join(QStringLiteral("\n\n"));

    if (!sources.isEmpty()) {
        output += QStringLiteral("\n\n--- 搜索来源 ---\n");
        for (int i = 0; i < sources.size(); ++i) {
            output += QString("[%1] %2\n").arg(i + 1).arg(sources[i]);
        }
    }

    return output;
}

// 解析 Anthropic 错误格式: {"type":"error", "error":{"type":"...", "message":"..."}}
QString JsonUtils::extractAnthropicError(const QJsonObject &errorObj)
{
    if (errorObj.contains("error") && errorObj["error"].isObject()) {
        QJsonObject err = errorObj["error"].toObject();
        if (err.contains("message")) {
            return err["message"].toString();
        }
    }
    // 兜底：返回整个 JSON
    return QJsonDocument(errorObj).toJson(QJsonDocument::Compact);
}

//将数据转换为前端提取格式
QVariantList JsonUtils::parseApiResponse(const QString &text)
{
    QVariantList result;
    if (text.trimmed().isEmpty())
        return result;

    // 按 "api名字:" 分割,因为有多个。
    QStringList blocks = text.split("api名字:", Qt::SkipEmptyParts);
    for (const QString &block : blocks) {
        QVariantMap map;
        // 提取第一个换行前的部分作为 name
        int nameEnd = block.indexOf('\n');
        if (nameEnd < 0) continue;
        map["name"] = block.left(nameEnd).trimmed();

        // 定义函数按标签提取
        auto extractField = [&](const QString &tag) -> QString {
            QString pattern = tag + ":";
            int start = block.indexOf(pattern);
            if (start < 0) return QString();
            int valueStart = start + pattern.length();
            //从valueStart之后开始找\napi。
            int nextTag = block.indexOf("\napi", valueStart);
            if (nextTag < 0) nextTag = block.length();
            QString value = block.mid(valueStart, nextTag - valueStart).trimmed();
            // 去掉末尾可能的多余换行
            return value;
        };

        //每一个提示词定义的参数直接调用提取函数。
        map["head"] = extractField("api需引入头");
        map["desc"] = extractField("api解释");
        map["params"] = extractField("api参数解释");
        map["example"] = extractField("api示例");
        map["detail"] = extractField("api说明");

        // 如果 name 为空，跳过
        if (!map["name"].toString().isEmpty())
            result.append(map);
    }

    return result;
}
