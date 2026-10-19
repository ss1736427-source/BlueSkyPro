#include "../src/CredentialVault.h"
#include <cassert>
#include <QString>
int main() {
    CredentialVault vault(QStringLiteral("BlueSkyPRO/IntegrationCredentialVaultTest"));
    assert(!vault.hasSecret(QStringLiteral("unknown-provider")));
#ifdef Q_OS_WIN
    assert(vault.deleteSecret(QStringLiteral("notam")));
    assert(vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")));
    assert(vault.hasSecret(QStringLiteral("notam")));
    assert(vault.readSecret(QStringLiteral("notam")) == QStringLiteral("vault-test-token"));
    assert(vault.deleteSecret(QStringLiteral("notam")));
    assert(!vault.hasSecret(QStringLiteral("notam")));
#else
    assert(vault.backendStatus().contains(QStringLiteral("UNSUPPORTED")));
    assert(!vault.saveSecret(QStringLiteral("notam"), QStringLiteral("vault-test-token")));
    assert(!vault.hasSecret(QStringLiteral("notam")));
#endif
    return 0;
}
