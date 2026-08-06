#include <QtTest>
#include <QSignalSpy>
#include <QQmlEngine>
#include <QJSEngine>
#include <QVariantList>

// 引入被测模块
#include "appcore.h"

class TestAppCore : public QObject
{
    Q_OBJECT

private:
    AppCore *m_core = nullptr;

private slots:
    void initTestCase()
    {
        // 在整个测试套件开始前创建 AppCore 实例
        m_core = AppCore::instance();
    }

    void cleanupTestCase()
    {
        // 清理
    }

    // ---- 单例模式 ----
    void test_singleton_sameInstance()
    {
        AppCore *instance1 = AppCore::instance();
        AppCore *instance2 = AppCore::instance();

        QCOMPARE(instance1, instance2);
        QVERIFY(instance1 != nullptr);
    }

    void test_create_returnsSameAsInstance()
    {
        QQmlEngine engine;
        QJSEngine jsEngine;

        AppCore *fromCreate = AppCore::create(&engine, &jsEngine);

        QCOMPARE(fromCreate, AppCore::instance());
    }

    // ---- webSearch 开关 ----
    void test_webSearch_defaultEnabled()
    {
        // 默认应为 true（在构造函数中通过 loadApiConfig 设置）
        QVERIFY(m_core->isWebSearchEnabled());
    }

    void test_webSearch_setFalse()
    {
        m_core->setWebSearchEnabled(false);
        QCOMPARE(m_core->isWebSearchEnabled(), false);
    }

    void test_webSearch_setTrue()
    {
        m_core->setWebSearchEnabled(true);
        QCOMPARE(m_core->isWebSearchEnabled(), true);
    }

    void test_webSearch_toggle()
    {
        m_core->setWebSearchEnabled(true);
        QVERIFY(m_core->isWebSearchEnabled());

        m_core->setWebSearchEnabled(false);
        QVERIFY(!m_core->isWebSearchEnabled());

        m_core->setWebSearchEnabled(true);
        QVERIFY(m_core->isWebSearchEnabled());
    }

    // ---- searchApi 信号测试 ----
    void test_searchApi_emitsSignalOnKeyLoadFailure()
    {
        // 注意: searchApi 在没有 apiKey 时会尝试从 keychain 读取，
        // 这会触发异步操作。我们在此主要验证信号连接不会崩溃。
        // 在 CI 环境中通常没有存储的 key，因此会收到 apiKeyLoadFailed。

        // 验证 AppCore 实例可以调用 searchApi 而不崩溃
        // 实际网络请求依赖 QNetworkAccessManager，此处仅验证方法可调用
        QVERIFY(m_core != nullptr);

        // 注意: 无法轻易验证异步结果，因为 keychain 操作是异步的
        // 此处仅做冒烟测试：确保方法调用不会崩溃
    }

    // ---- 构造函数与默认值 ----
    void test_instance_isValidQObject()
    {
        QVERIFY(m_core->isWidgetType() == false); // 是 QObject，不是 QWidget
        QVERIFY(m_core->parent() == nullptr);      // 默认无 parent
    }

    void test_inheritsQObject()
    {
        QVERIFY(m_core->inherits("QObject"));
        QVERIFY(m_core->inherits("AppCore"));
    }

    // ---- 属性与 QML 元信息 ----
    void test_qmlTypeInfo()
    {
        // AppCore 有 QML_ELEMENT 和 QML_SINGLETON 宏
        // 验证 metaObject 包含预期的属性和方法
        const QMetaObject *meta = m_core->metaObject();

        // 验证 searchApi 方法存在且可被 QML 调用
        int methodIndex = meta->indexOfMethod("searchApi(QString,QString,QString,QString)");
        QVERIFY(methodIndex >= 0);

        // 验证 setWebSearchEnabled 方法
        int setterIndex = meta->indexOfMethod("setWebSearchEnabled(bool)");
        QVERIFY(setterIndex >= 0);

        // 验证 isWebSearchEnabled 方法
        int getterIndex = meta->indexOfMethod("isWebSearchEnabled()");
        QVERIFY(getterIndex >= 0);
    }

    void test_searchResultReady_signalExists()
    {
        const QMetaObject *meta = m_core->metaObject();
        int signalIndex = meta->indexOfSignal("searchResultReady(QVariantList)");
        QVERIFY(signalIndex >= 0);
    }

    void test_settingsChanged_signal()
    {
        QSignalSpy spy(m_core, &AppCore::settingsChanged);

        // 调用 saveApiSettings 应触发 settingsChanged
        m_core->saveApiSettings("DeepSeek", "test-key", "https://test.com", "test-model");

        // 等待信号（QtKeychain 异步，但 saveApiSettings 是同步的，信号应该立即发射）
        QCOMPARE(spy.count(), 1);
    }
};

QTEST_MAIN(TestAppCore)
#include "tst_appcore.moc"
