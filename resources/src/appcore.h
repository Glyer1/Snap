#ifndef APPCORE_H
#define APPCORE_H

#include "./utils/passwordutils.h"
#include "./workers/apiworker.h"
#include "./types/types.h"
#include <QObject>
#include <QThread>
#include <QQmlEngine>
#include <QVariantList>


class AppCore : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    QML_SINGLETON
public:
    static AppCore* instance();
    static AppCore* create(QQmlEngine* qmlEngine, QJSEngine* jsEngine);

    Q_INVOKABLE void searchApi(const QString &query, const QString company, const QString &modelUrl, const QString &modelName);
    Q_INVOKABLE void setWebSearchEnabled(bool enabled);
    Q_INVOKABLE bool isWebSearchEnabled() const;

    // AppCore.h 在 public 区域添加
    Q_INVOKABLE void saveApiSettings(const QString &company, const QString &apiKey, const QString &baseUrl, const QString &model);
    Q_INVOKABLE QString getApiKeyForCompany(const QString &company);
    Q_INVOKABLE QString getBaseUrlForCompany(const QString &company);
    Q_INVOKABLE QString getModelForCompany(const QString &company);
    Q_INVOKABLE void loadApiKey(const QString company);

signals:
    void searchResultReady(const QVariantList &results);
    void settingsChanged();

private:
    explicit AppCore(QObject *parent = nullptr);
    static AppCore* s_instance;
    void loadApiConfig();
    void loadApiKeyAndSearch(const QString &query, const QString company, const QString &modelUrl, const QString &modelName);
    void doSearch(const QString &query, const QString &modelUrl, const QString &modelName);

    PasswordUtils *m_passwordUtils = nullptr;
    QString m_apiKey;
    QString m_defaultModelUrl;
    bool m_webSearchEnabled = true;
};

#endif // APPCORE_H
