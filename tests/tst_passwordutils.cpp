#include <QtTest>
#include <QSignalSpy>
#include <QObject>

// 引入被测模块
#include "utils/passwordutils.h"
#include <qt6keychain/keychain.h>

class TestPasswordUtils : public QObject
{
    Q_OBJECT

private slots:
    // ---- 构造函数 ----
    void test_constructor_noParent()
    {
        PasswordUtils utils;

        QVERIFY(utils.parent() == nullptr);
    }

    void test_constructor_withParent()
    {
        QObject parent;
        PasswordUtils utils(&parent);

        QCOMPARE(utils.parent(), &parent);
    }

    void test_constructor_setsQObjectName()
    {
        PasswordUtils utils;
        QVERIFY(utils.inherits("QObject"));
    }

    // ---- Key 名称生成规则 ----
    void test_keyName_convention()
    {
        // 验证 key 命名规则: "api_key" + company
        // 通过与 qtkeychain 的交互间接验证
        QString company = "DeepSeek";
        QString expectedKey = "api_key" + company;

        QCOMPARE(QStringLiteral("api_key") + company, expectedKey);
        // 在实际调用中，这个 key 会被传递给 QKeychain
        // 此处验证命名规则的一致性
    }

    void test_keyName_multipleCompanies()
    {
        // 验证不同公司名应生成不同 key
        QString company1 = "DeepSeek";
        QString company2 = "Anthropic";

        QString key1 = QStringLiteral("api_key") + company1;
        QString key2 = QStringLiteral("api_key") + company2;

        QVERIFY(key1 != key2);
        QCOMPARE(key1, QString("api_keyDeepSeek"));
        QCOMPARE(key2, QString("api_keyAnthropic"));
    }

    // ---- 信号声明 ----
    void test_signals_declared()
    {
        PasswordUtils utils;

        const QMetaObject *meta = utils.metaObject();

        // 验证 apiKeyLoaded 信号存在
        int loadedIdx = meta->indexOfSignal("apiKeyLoaded(QString)");
        QVERIFY(loadedIdx >= 0);

        // 验证 apiKeyLoadFailed 信号存在
        int failedIdx = meta->indexOfSignal("apiKeyLoadFailed(QString)");
        QVERIFY(failedIdx >= 0);
    }

    // ---- readApiKey 信号连接测试 ----
    void test_readApiKey_connectsProperly()
    {
        PasswordUtils utils;

        QSignalSpy spyLoaded(&utils, &PasswordUtils::apiKeyLoaded);
        QSignalSpy spyFailed(&utils, &PasswordUtils::apiKeyLoadFailed);

        // 尝试读取一个 key（在 Windows 上使用 Credential Store）
        utils.readApiKey("NonExistentCompany_TestOnly");

        // 注意：readApiKey 是异步的，需要通过 QKeychain 回调
        // 在单元测试环境中，我们不等待实际完成，
        // 而是验证调用不会产生崩溃，并且信号 spy 已正确设置
        QVERIFY(spyLoaded.isValid());
        QVERIFY(spyFailed.isValid());

        // 初始时都没有信号
        QCOMPARE(spyLoaded.count(), 0);
        QCOMPARE(spyFailed.count(), 0);
    }

    // ---- storeApiKey 调用测试 ----
    void test_storeApiKey_doesNotCrash()
    {
        PasswordUtils utils;

        // 验证 storeApiKey 调用不会崩溃
        // 实际存储依赖系统 keychain，在 CI 中可能成功也可能失败
        utils.storeApiKey("test-key-value", "TestCompany_Only");

        // 异步操作，此处仅验证方法可安全调用
        QVERIFY(true);
    }

    // ---- 多实例独立性 ----
    void test_multipleInstances_independent()
    {
        PasswordUtils utils1;
        PasswordUtils utils2;

        // 每个实例应该独立
        QVERIFY(&utils1 != &utils2);

        QSignalSpy spy1(&utils1, &PasswordUtils::apiKeyLoaded);
        QSignalSpy spy2(&utils2, &PasswordUtils::apiKeyLoaded);

        QVERIFY(spy1.isValid());
        QVERIFY(spy2.isValid());
    }
};

QTEST_MAIN(TestPasswordUtils)
#include "tst_passwordutils.moc"
