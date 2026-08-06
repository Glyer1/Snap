#include "passwordutils.h"

PasswordUtils::PasswordUtils(QObject *parent)
    : QObject{parent}
{}


//存入api_key键值对。
void PasswordUtils::storeApiKey(const QString &key, const QString company)
{
    qDebug() << "正在进行storeApiKey";

    auto *job = new QKeychain::WritePasswordJob(QLatin1String("Snap"));  // ← 堆上创建
    job->setAutoDelete(true);  // 完成后自动销毁
    job->setKey(QLatin1String((QStringLiteral("api_key") + company).toLatin1()));
    job->setTextData(key);

    QObject::connect(job, &QKeychain::WritePasswordJob::finished,
                     [](QKeychain::Job *job) {
                         if (job->error()) {
                             qDebug() << "存储失败:" << job->errorString();
                         } else {
                             qDebug() << "API Key 存储成功!";
                         }
                     });

    job->start();
    qDebug() << "存储apikey job.start();";
}

//读apikey
void PasswordUtils::readApiKey(const QString company)
{
    auto *job = new QKeychain::ReadPasswordJob(QLatin1String("Snap"), this);
    job->setAutoDelete(false);
    job->setKey(QLatin1String((QStringLiteral("api_key")+company).toLatin1()));

    //连接信号槽
    connect(job, &QKeychain::ReadPasswordJob::finished, this,
            [this, job](QKeychain::Job *j)
            {
               if (j->error())
               {
                   qDebug()<<"读取失败"<<j->errorString();
                   emit apiKeyLoadFailed(j->errorString());
               }
               else
               {
                   //读取
                   auto *readJob = static_cast<QKeychain::ReadPasswordJob*>(j);
                   QString apiKey = readJob->textData();
                   emit apiKeyLoaded(apiKey);
               }
               job->deleteLater();
            }
           );
    job->start();
}
