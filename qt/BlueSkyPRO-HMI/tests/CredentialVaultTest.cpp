#include "../src/CredentialVault.h"
#include <QString>

int main()
{
    CredentialVault vault(QStringLiteral("BlueSkyPRO/IntegrationCredentialVaultTest"));
    if (vault.hasSecret(QStringLiteral("unknown-provider")))
        return 1;
#ifdef Q_OS_WIN
    if (!vault.deleteSecret(QStringLiteral("notam"))) return 2;
    if (!vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token"))) return 3;
    if (!vault.hasSecret(QStringLiteral("notam"))) return 4;
    if (vault.readSecret(QStringLiteral("notam")) != QStringLiteral("vault-test-token")) return 5;
    if (!vault.deleteSecret(QStringLiteral("notam"))) return 6;
    if (vault.hasSecret(QStringLiteral("notam"))) return 7;
#else
    if (!vault.backendStatus().contains(QStringLiteral("UNSUPPORTED"))) return 8;
    if (vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token"))) return 9;
    if (vault.hasSecret(QStringLiteral("notam"))) return 10;
#endif
    return 0;
}
