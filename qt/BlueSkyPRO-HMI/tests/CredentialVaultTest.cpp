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
        if (!vault.deleteSecret(QStringLiteral("notam")) || !vault.deleteSecret(QStringLiteral("faa_nms")))
            return 2;
        if (!vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")))
            return 3;
        return vault.saveSecret(QStringLiteral("faa_nms"), QStringLiteral("faa-nms-vault-test")) ? 0 : 12;
    }
    if (action == QStringLiteral("read")) {
        if (!vault.hasSecret(QStringLiteral("notam")))
            return 4;
        if (vault.readSecret(QStringLiteral("notam")) != QStringLiteral("vault-test-token"))
            return 5;
        return vault.readSecret(QStringLiteral("faa_nms")) == QStringLiteral("faa-nms-vault-test") ? 0 : 13;
    }
    if (action == QStringLiteral("delete")) {
        if (!vault.deleteSecret(QStringLiteral("notam")) || !vault.deleteSecret(QStringLiteral("faa_nms")))
            return 6;
        return (vault.hasSecret(QStringLiteral("notam")) || vault.hasSecret(QStringLiteral("faa_nms"))) ? 7 : 0;
    }
    return 8;
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    if (!vault.backendStatus().contains(QStringLiteral("UNSUPPORTED")))
        return 9;
    if (vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")))
        return 10;
    if (vault.saveSecret(QStringLiteral("faa_nms"), QStringLiteral("faa-nms-vault-test")))
        return 14;
    return (vault.hasSecret(QStringLiteral("notam")) || vault.hasSecret(QStringLiteral("faa_nms"))) ? 11 : 0;
#endif
}
