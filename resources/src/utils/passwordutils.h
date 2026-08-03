#ifndef PASSWORDUTILS_H
#define PASSWORDUTILS_H
#include <qt6keychain/keychain.h>
#include <QObject>
#include <QDebug>

class PasswordUtils : public QObject
{
    Q_OBJECT
public:
    explicit PasswordUtils(QObject *parent = nullptr);
    void storeApiKey(const QString &key , const QString company);
    void readApiKey(const QString company);
signals:

    // 读取api成功与否
    void apiKeyLoaded(const QString &key);
    void apiKeyLoadFailed(const QString &error);
};

#endif // PASSWORDUTILS_H
