#pragma once

#include <QObject>
#include <QProcess>
#include <QVariantMap>

class PlanningBridge final : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap result READ result NOTIFY resultChanged)
    Q_PROPERTY(bool running READ running NOTIFY runningChanged)

public:
    explicit PlanningBridge(QObject *parent = nullptr);

    QVariantMap result() const;
    bool running() const;

    Q_INVOKABLE bool startProcess(const QString &program, const QStringList &arguments = {});
    Q_INVOKABLE void stopProcess();
    Q_INVOKABLE bool sendRequest(const QString &json);
    Q_INVOKABLE bool publishJson(const QString &json);
    Q_INVOKABLE void clear();

public slots:
    void publishResult(const QVariantMap &result);

signals:
    void resultChanged();
    void runningChanged();
    void bridgeError(const QString &code);

private:
    void consumeStdout();

    QProcess m_process;
    QByteArray m_stdoutBuffer;
    QVariantMap m_result;
};
