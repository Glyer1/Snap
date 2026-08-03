#include <QtTest>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>

// 引入被测模块
#include "utils/jsonutils.h"
#include "types/types.h"

class TestJsonUtils : public QObject
{
    Q_OBJECT

private slots:
    // ---- messageToJson ----
    void test_messageToJson_basic()
    {
        Message msg;
        msg.role = "user";
        msg.content = "Hello, world!";

        QJsonObject obj = JsonUtils::messageToJson(msg);

        QCOMPARE(obj["role"].toString(), QString("user"));
        QCOMPARE(obj["content"].toString(), QString("Hello, world!"));
    }

    void test_messageToJson_empty()
    {
        Message msg;
        // role 和 content 都为空

        QJsonObject obj = JsonUtils::messageToJson(msg);

        QVERIFY(obj.contains("role"));
        QVERIFY(obj.contains("content"));
        QCOMPARE(obj["role"].toString(), QString(""));
        QCOMPARE(obj["content"].toString(), QString(""));
    }

    void test_messageToJson_systemRole()
    {
        Message msg;
        msg.role = "system";
        msg.content = "You are a helpful assistant.";

        QJsonObject obj = JsonUtils::messageToJson(msg);

        QCOMPARE(obj["role"].toString(), QString("system"));
        QCOMPARE(obj["content"].toString(), QString("You are a helpful assistant."));
    }

    void test_messageToJson_assistantRole()
    {
        Message msg;
        msg.role = "assistant";
        msg.content = "Here is the answer.";

        QJsonObject obj = JsonUtils::messageToJson(msg);

        QCOMPARE(obj["role"].toString(), QString("assistant"));
        QCOMPARE(obj["content"].toString(), QString("Here is the answer."));
    }

    // ---- messagesToJsonArray ----
    void test_messagesToJsonArray_empty()
    {
        QList<Message> messages;

        QJsonArray arr = JsonUtils::messagesToJsonArray(messages);

        QCOMPARE(arr.size(), 0);
    }

    void test_messagesToJsonArray_single()
    {
        QList<Message> messages;
        Message msg;
        msg.role = "user";
        msg.content = "Single message";
        messages.append(msg);

        QJsonArray arr = JsonUtils::messagesToJsonArray(messages);

        QCOMPARE(arr.size(), 1);
        QJsonObject obj = arr[0].toObject();
        QCOMPARE(obj["role"].toString(), QString("user"));
        QCOMPARE(obj["content"].toString(), QString("Single message"));
    }

    void test_messagesToJsonArray_multiple()
    {
        QList<Message> messages;

        Message msg1;
        msg1.role = "system";
        msg1.content = "System prompt";
        messages.append(msg1);

        Message msg2;
        msg2.role = "user";
        msg2.content = "User question";
        messages.append(msg2);

        Message msg3;
        msg3.role = "assistant";
        msg3.content = "Assistant reply";
        messages.append(msg3);

        QJsonArray arr = JsonUtils::messagesToJsonArray(messages);

        QCOMPARE(arr.size(), 3);
        QCOMPARE(arr[0].toObject()["role"].toString(), QString("system"));
        QCOMPARE(arr[0].toObject()["content"].toString(), QString("System prompt"));
        QCOMPARE(arr[1].toObject()["role"].toString(), QString("user"));
        QCOMPARE(arr[1].toObject()["content"].toString(), QString("User question"));
        QCOMPARE(arr[2].toObject()["role"].toString(), QString("assistant"));
        QCOMPARE(arr[2].toObject()["content"].toString(), QString("Assistant reply"));
    }

    // ---- buildAnthropicRequestBody ----
    void test_buildAnthropicRequestBody_basic()
    {
        Request req;
        req.model = "test-model";
        req.temperature = 0.5;
        req.maxTokens = 2048;
        req.webSearchEnabled = false;

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        QCOMPARE(body["model"].toString(), QString("test-model"));
        QCOMPARE(body["max_tokens"].toInt(), 2048);
        QCOMPARE(body["temperature"].toDouble(), 0.5);
        // messages 应该是空数组
        QCOMPARE(body["messages"].toArray().size(), 0);
        // 没有 tools（webSearchEnabled=false）
        QVERIFY(!body.contains("tools"));
        QVERIFY(!body.contains("tool_choice"));
    }

    void test_buildAnthropicRequestBody_withSystemPrompt()
    {
        Request req;
        req.model = "test-model";
        req.temperature = 0.7;
        req.maxTokens = 4096;
        req.webSearchEnabled = false;
        req.systemPrompt = "You are a coding assistant.";

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        QVERIFY(body.contains("system"));
        QCOMPARE(body["system"].toString(), QString("You are a coding assistant."));
    }

    void test_buildAnthropicRequestBody_withoutSystemPrompt()
    {
        Request req;
        req.model = "test-model";
        req.temperature = 0.7;
        req.maxTokens = 4096;
        req.webSearchEnabled = false;
        // systemPrompt 为空

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        QVERIFY(!body.contains("system"));
    }

    void test_buildAnthropicRequestBody_withMessages()
    {
        Request req;
        req.model = "claude";
        req.temperature = 1.0;
        req.maxTokens = 1024;
        req.webSearchEnabled = false;

        Message msg;
        msg.role = "user";
        msg.content = "What is Qt?";
        req.messages.append(msg);

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        QJsonArray msgs = body["messages"].toArray();
        QCOMPARE(msgs.size(), 1);
        QCOMPARE(msgs[0].toObject()["role"].toString(), QString("user"));
        QCOMPARE(msgs[0].toObject()["content"].toString(), QString("What is Qt?"));
    }

    void test_buildAnthropicRequestBody_withWebSearch()
    {
        Request req;
        req.model = "test-model";
        req.temperature = 0.7;
        req.maxTokens = 4096;
        req.webSearchEnabled = true;

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        // 应该包含 tools 数组
        QVERIFY(body.contains("tools"));
        QJsonArray tools = body["tools"].toArray();
        QCOMPARE(tools.size(), 1);

        QJsonObject tool = tools[0].toObject();
        QCOMPARE(tool["type"].toString(), QString("web_search_20250305"));
        QCOMPARE(tool["name"].toString(), QString("web_search"));
        QVERIFY(tool.contains("input_schema"));

        // 应该包含 tool_choice
        QVERIFY(body.contains("tool_choice"));
        QCOMPARE(body["tool_choice"].toObject()["type"].toString(), QString("auto"));
    }

    void test_buildAnthropicRequestBody_defaultValues()
    {
        // 测试 Request 的默认值在 buildAnthropicRequestBody 中的表现
        Request req; // 使用所有默认值
        req.temperature = 0.5;

        QJsonObject body = JsonUtils::buildAnthropicRequestBody(req);

        QCOMPARE(body["model"].toString(), QString("deepseek-v4-flash"));
        QCOMPARE(body["max_tokens"].toInt(), 4096);
        // webSearchEnabled 默认为 true
        QVERIFY(body.contains("tools"));
    }

    // ---- extractAnthropicContent ----
    void test_extractAnthropicContent_empty()
    {
        QJsonArray contentBlocks;

        QString result = JsonUtils::extractAnthropicContent(contentBlocks);

        QVERIFY(result.isEmpty());
    }

    void test_extractAnthropicContent_textOnly()
    {
        QJsonArray contentBlocks;
        QJsonObject block;
        block["type"] = "text";
        block["text"] = "Hello from Claude!";
        contentBlocks.append(block);

        QString result = JsonUtils::extractAnthropicContent(contentBlocks);

        QCOMPARE(result, QString("Hello from Claude!"));
    }

    void test_extractAnthropicContent_multipleTextBlocks()
    {
        QJsonArray contentBlocks;

        QJsonObject block1;
        block1["type"] = "text";
        block1["text"] = "First paragraph.";
        contentBlocks.append(block1);

        QJsonObject block2;
        block2["type"] = "text";
        block2["text"] = "Second paragraph.";
        contentBlocks.append(block2);

        QString result = JsonUtils::extractAnthropicContent(contentBlocks);

        QVERIFY(result.contains("First paragraph."));
        QVERIFY(result.contains("Second paragraph."));
        QVERIFY(result.contains("\n\n"));
    }

    void test_extractAnthropicContent_withSearchResults()
    {
        QJsonArray contentBlocks;

        // 模拟文本块
        QJsonObject textBlock;
        textBlock["type"] = "text";
        textBlock["text"] = "Here is the answer:";
        contentBlocks.append(textBlock);

        // 模拟搜索工具结果块
        QJsonObject searchBlock;
        searchBlock["type"] = "web_search_tool_result";

        QJsonArray searchContent;
        QJsonObject source1;
        source1["type"] = "text";
        source1["text"] = "Source A: Qt documentation";
        searchContent.append(source1);

        QJsonObject source2;
        source2["type"] = "text";
        source2["text"] = "Source B: Qt forum";
        searchContent.append(source2);

        searchBlock["content"] = searchContent;
        contentBlocks.append(searchBlock);

        QString result = JsonUtils::extractAnthropicContent(contentBlocks);

        QVERIFY(result.contains("Here is the answer:"));
        QVERIFY(result.contains("搜索来源"));
        QVERIFY(result.contains("[1] Source A: Qt documentation"));
        QVERIFY(result.contains("[2] Source B: Qt forum"));
    }

    void test_extractAnthropicContent_skipsServerToolUse()
    {
        QJsonArray contentBlocks;

        QJsonObject textBlock;
        textBlock["type"] = "text";
        textBlock["text"] = "Final answer";
        contentBlocks.append(textBlock);

        // server_tool_use 块应被跳过
        QJsonObject toolUseBlock;
        toolUseBlock["type"] = "server_tool_use";
        toolUseBlock["name"] = "web_search";
        contentBlocks.append(toolUseBlock);

        QString result = JsonUtils::extractAnthropicContent(contentBlocks);

        QCOMPARE(result, QString("Final answer"));
    }

    // ---- extractAnthropicError ----
    void test_extractAnthropicError_standard()
    {
        QJsonObject errorObj;
        QJsonObject innerError;
        innerError["type"] = "authentication_error";
        innerError["message"] = "Invalid API key";
        errorObj["error"] = innerError;

        QString result = JsonUtils::extractAnthropicError(errorObj);

        QCOMPARE(result, QString("Invalid API key"));
    }

    void test_extractAnthropicError_noErrorKey()
    {
        QJsonObject errorObj;
        errorObj["some_other_field"] = "value";

        QString result = JsonUtils::extractAnthropicError(errorObj);

        // 应返回整个 JSON 的 compact 形式
        QVERIFY(!result.isEmpty());
        QVERIFY(result.contains("some_other_field"));
    }

    void test_extractAnthropicError_errorNotObject()
    {
        QJsonObject errorObj;
        errorObj["error"] = "just a string, not an object";

        QString result = JsonUtils::extractAnthropicError(errorObj);

        // 应返回整个 JSON 的 compact 形式
        QVERIFY(!result.isEmpty());
    }

    void test_extractAnthropicError_empty()
    {
        QJsonObject errorObj;

        QString result = JsonUtils::extractAnthropicError(errorObj);

        // 空对象 → "{}"
        QCOMPARE(result, QString("{}"));
    }

    // ---- 构造函数 ----
    void test_constructor()
    {
        JsonUtils utils;
        QVERIFY(utils.parent() == nullptr);
    }

    void test_constructor_withParent()
    {
        QObject parent;
        JsonUtils utils(&parent);
        QCOMPARE(utils.parent(), &parent);
    }
};

QTEST_MAIN(TestJsonUtils)
#include "tst_jsonutils.moc"
