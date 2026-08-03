#ifndef CLIPBOARDUTILS_H
#define CLIPBOARDUTILS_H

#include <QObject>
#include <QGuiApplication>
#include <QClipboard>

class ClipboardUtils : public QObject
{
    Q_OBJECT
public:
    explicit ClipboardUtils(QObject *parent = nullptr) : QObject(parent) {}

    // 使用 Q_INVOKABLE 使方法可在 QML 中调用
    Q_INVOKABLE void setText(const QString &text) {
        QGuiApplication::clipboard()->setText(text);
    }

    Q_INVOKABLE QString getText() const {
        return QGuiApplication::clipboard()->text();
    }
};
#endif // CLIPBOARDUTILS_H
