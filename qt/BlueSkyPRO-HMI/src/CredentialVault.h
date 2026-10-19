#pragma once
#include <QObject>
#include <QString>
class CredentialVault final : public QObject {
    Q_OBJECT
    Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)
public:
    explicit CredentialVault(QString targetPrefix = QStringLiteral("BlueSkyPRO/ExternalIntegrations"), QObject *parent = nullptr);
    Q_INVOKABLE QString readSecret(const QString &provider) const;
    Q_INVOKABLE bool saveSecret(const QString &provider, const QString &secret);
    Q_INVOKABLE bool deleteSecret(const QString &provider);
    Q_INVOKABLE bool hasSecret(const QString &provider) const;
    Q_INVOKABLE QString backendStatus() const;
    QString lastError() const;
signals:
    void lastErrorChanged();
private:
    bool providerAllowed(const QString &provider) const;
    QString targetName(const QString &provider) const;
    void setLastError(const QString &error) const;
    QString m_targetPrefix;
    mutable QString m_lastError;
};
