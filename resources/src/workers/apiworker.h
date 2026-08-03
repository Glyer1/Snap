#ifndef APIWORKER_H
#define APIWORKER_H
#include <QObject>
#include <qqmlengine.h>
#include <QNetworkAccessManager>
#include "../types/types.h"
#include <QUrl>
#include <QNetworkReply>
#include "../utils/jsonutils.h"
#include "../utils/passwordutils.h"
class ApiWorker : public QObject
{
    Q_OBJECT
public:
    explicit ApiWorker(const QString &apiKey, QObject *parent = nullptr);
    void doRequest(const Request &request, const QString& modelUrl, const QString& modelName);
private:
    QString m_apiKey;
    QNetworkAccessManager* m_manager = nullptr;
    QString m_currentRequestId;
private slots:
    void onReplyFinished(QNetworkReply *reply);
signals:
    void finished(const Response &response);
};

#endif // APIWORKER_H
