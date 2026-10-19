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
        if (!vault.deleteSecret(QStringLiteral("notam")))
            return 2;
        return vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")) ? 0 : 3;
    }
    if (action == QStringLiteral("read")) {
        if (!vault.hasSecret(QStringLiteral("notam")))
            return 4;
        return vault.readSecret(QStringLiteral("notam")) == QStringLiteral("vault-test-token") ? 0 : 5;
    }
    if (action == QStringLiteral("delete")) {
        if (!vault.deleteSecret(QStringLiteral("notam")))
            return 6;
        return vault.hasSecret(QStringLiteral("notam")) ? 7 : 0;
    }
    return 8;
#else
    Q_UNUSED(argc);
    Q_UNUSED(argv);
    if (!vault.backendStatus().contains(QStringLiteral("UNSUPPORTED")))
        return 9;
    if (vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")))
        return 10;
    return vault.hasSecret(QStringLiteral("notam")) ? 11 : 0;
#endif
}
