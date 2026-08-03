#ifndef TYPES_H
#define TYPES_H
#include <QString>
#include <QList>

struct Message{
    QString role;   //user/system/assistant
    QString content;
};

struct Request{
    QString requestId;
    QString model = "deepseek-v4-flash";//速度，如果需要质量改pro
    QList<Message> messages;
    double temperature;
    // Anthropic format fields
    QString systemPrompt;              // 顶层 system prompt（不再放 messages 里）
    int maxTokens = 4096;
    bool webSearchEnabled = true;      // 是否启用联网搜索
};

struct Response{
    QString requestId;
    QString content;
    bool success;
    QString errorMessage;
};



#endif // TYPES_H
