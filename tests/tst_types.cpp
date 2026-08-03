#include <QtTest>
#include <QString>
#include <QList>

// 引入被测模块
#include "types/types.h"

class TestTypes : public QObject
{
    Q_OBJECT

private slots:
    // ---- Message ----
    void test_message_defaultConstruction()
    {
        Message msg;

        // 默认构造后 role 和 content 应该为空字符串
        QVERIFY(msg.role.isEmpty());
        QVERIFY(msg.content.isEmpty());
    }

    void test_message_assignment()
    {
        Message msg;
        msg.role = "user";
        msg.content = "Hello";

        QCOMPARE(msg.role, QString("user"));
        QCOMPARE(msg.content, QString("Hello"));
    }

    void test_message_copy()
    {
        Message msg1;
        msg1.role = "assistant";
        msg1.content = "Response";

        Message msg2 = msg1;

        QCOMPARE(msg2.role, QString("assistant"));
        QCOMPARE(msg2.content, QString("Response"));
    }

    void test_message_inList()
    {
        QList<Message> messages;

        Message msg;
        msg.role = "user";
        msg.content = "Question";
        messages.append(msg);

        QCOMPARE(messages.size(), 1);
        QCOMPARE(messages[0].role, QString("user"));
        QCOMPARE(messages[0].content, QString("Question"));
    }

    // ---- Request ----
    void test_request_defaultValues()
    {
        Request req;

        // 验证默认值
        QCOMPARE(req.model, QString("deepseek-v4-flash"));
        QCOMPARE(req.maxTokens, 4096);
        QCOMPARE(req.webSearchEnabled, true);
        QVERIFY(req.messages.isEmpty());
        QVERIFY(req.systemPrompt.isEmpty());
        QVERIFY(req.requestId.isEmpty());
    }

    void test_request_temperatureDefault()
    {
        Request req;
        // temperature 是 double，未显式初始化时为 0.0
        QCOMPARE(req.temperature, 0.0);
    }

    void test_request_customValues()
    {
        Request req;
        req.requestId = "req-001";
        req.model = "custom-model";
        req.temperature = 0.8;
        req.maxTokens = 8192;
        req.webSearchEnabled = false;
        req.systemPrompt = "System instructions";

        QCOMPARE(req.requestId, QString("req-001"));
        QCOMPARE(req.model, QString("custom-model"));
        QCOMPARE(req.temperature, 0.8);
        QCOMPARE(req.maxTokens, 8192);
        QCOMPARE(req.webSearchEnabled, false);
        QCOMPARE(req.systemPrompt, QString("System instructions"));
    }

    void test_request_addMessages()
    {
        Request req;

        Message msg1;
        msg1.role = "system";
        msg1.content = "System prompt";
        req.messages.append(msg1);

        Message msg2;
        msg2.role = "user";
        msg2.content = "User query";
        req.messages.append(msg2);

        QCOMPARE(req.messages.size(), 2);
        QCOMPARE(req.messages[0].role, QString("system"));
        QCOMPARE(req.messages[1].role, QString("user"));
    }

    // ---- Response ----
    void test_response_defaultValues()
    {
        Response resp;

        QVERIFY(resp.requestId.isEmpty());
        QVERIFY(resp.content.isEmpty());
        // bool 成员默认值在 C++ 中可能是未定义的，但我们测试的是结构体行为
        QVERIFY(resp.errorMessage.isEmpty());
    }

    void test_response_success()
    {
        Response resp;
        resp.requestId = "req-001";
        resp.content = "Here is the answer.";
        resp.success = true;
        resp.errorMessage = "";

        QCOMPARE(resp.requestId, QString("req-001"));
        QCOMPARE(resp.content, QString("Here is the answer."));
        QCOMPARE(resp.success, true);
        QVERIFY(resp.errorMessage.isEmpty());
    }

    void test_response_failure()
    {
        Response resp;
        resp.requestId = "req-002";
        resp.content = "";
        resp.success = false;
        resp.errorMessage = "Network timeout";

        QCOMPARE(resp.requestId, QString("req-002"));
        QVERIFY(resp.content.isEmpty());
        QCOMPARE(resp.success, false);
        QCOMPARE(resp.errorMessage, QString("Network timeout"));
    }

    void test_response_copy()
    {
        Response resp1;
        resp1.requestId = "req-003";
        resp1.content = "Data";
        resp1.success = true;
        resp1.errorMessage = "No error";

        Response resp2 = resp1;

        QCOMPARE(resp2.requestId, QString("req-003"));
        QCOMPARE(resp2.content, QString("Data"));
        QCOMPARE(resp2.success, true);
    }
};

QTEST_MAIN(TestTypes)
#include "tst_types.moc"
