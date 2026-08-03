#include "passwordutils.h"

PasswordUtils::PasswordUtils(QObject *parent)
    : QObject{parent}
{}


//存入api_key键值对。
void PasswordUtils::storeApiKey(const QString &key, const QString company)
{
    //创建对象并且存入键值对。
    QKeychain::WritePasswordJob job(QLatin1String("Snap"));
    job.setAutoDelete(false);
    //需要转接再打开条目
    job.setKey(QLatin1String((QStringLiteral("api_key")+company).toLatin1()));
    job.setTextData(key);//存储的实际apikey

    QObject::connect(&job, &QKeychain::WritePasswordJob::finished,
                     [](QKeychain::Job *job) {
                         if (job->error()) {
                             qDebug() << "存储失败:" << job->errorString();
                         } else {
                             qDebug() << "API Key 存储成功!";
                         }
                     }
                     );
    job.start();
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
