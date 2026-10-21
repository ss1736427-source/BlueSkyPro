#include "../src/CredentialVault.h"
#include <QString>

int main(int argc, char *argv[])
{
    CredentialVault vault(QStringLiteral("BlueSkyPRO/IntegrationCredentialVaultTest"));
#ifdef Q_OS_WIN
    if (argc != 2)
        return 1;
    const QString action = QString::fromLocal8Bit(argv[1]);
    if (action == QStringLiteral("write")) {
        const QStringList providers = {QStringLiteral("notam"), QStringLiteral("faa_nms"), QStringLiteral("faa_nms_client_id"), QStringLiteral("faa_nms_client_secret")};
        for (const QString &provider : providers)
            if (!vault.deleteSecret(provider)) return 2;
        if (!vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token"))) return 3;
        if (!vault.saveSecret(QStringLiteral("faa_nms"), QStringLiteral("faa-nms-vault-test"))) return 12;
        if (!vault.saveSecret(QStringLiteral("faa_nms_client_id"), QStringLiteral("faa-client-id-test"))) return 15;
        return vault.saveSecret(QStringLiteral("faa_nms_client_secret"), QStringLiteral("faa-client-secret-test")) ? 0 : 16;
    }
    if (action == QStringLiteral("read")) {
        if (!vault.hasSecret(QStringLiteral("notam"))) return 4;
        if (vault.readSecret(QStringLiteral("notam")) != QStringLiteral("vault-test-token")) return 5;
        if (vault.readSecret(QStringLiteral("faa_nms")) != QStringLiteral("faa-nms-vault-test")) return 13;
        if (vault.readSecret(QStringLiteral("faa_nms_client_id")) != QStringLiteral("faa-client-id-test")) return 17;
        return vault.readSecret(QStringLiteral("faa_nms_client_secret")) == QStringLiteral("faa-client-secret-test") ? 0 : 18;
    }
    if (action == QStringLiteral("delete")) {
        const QStringList providers = {QStringLiteral("notam"), QStringLiteral("faa_nms"), QStringLiteral("faa_nms_client_id"), QStringLiteral("faa_nms_client_secret")};
        for (const QString &provider : providers)
            if (!vault.deleteSecret(provider)) return 6;
        for (const QString &provider : providers)
            if (vault.hasSecret(provider)) return 7;
        return 0;
    }
    return 8;
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    if (!vault.backendStatus().contains(QStringLiteral("UNSUPPORTED"))) return 9;
    const QStringList providers = {QStringLiteral("notam"), QStringLiteral("faa_nms"), QStringLiteral("faa_nms_client_id"), QStringLiteral("faa_nms_client_secret")};
    for (const QString &provider : providers) {
        if (vault.saveSecret(provider, QStringLiteral("vault-test-secret"))) return 10;
        if (vault.hasSecret(provider)) return 11;
    }
    if (vault.saveSecret(QStringLiteral("unknown_provider"), QStringLiteral("nope"))) return 19;
    return 0;
#endif
}
