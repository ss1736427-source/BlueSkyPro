#include "CredentialVault.h"
#include <QByteArray>
#include <QStringList>
#include <utility>
#ifdef Q_OS_WIN
#  define WIN32_LEAN_AND_MEAN
#  include <windows.h>
#  include <wincred.h>
#endif
CredentialVault::CredentialVault(QString targetPrefix, QObject *parent)
    : QObject(parent), m_targetPrefix(std::move(targetPrefix)) {}
bool CredentialVault::providerAllowed(const QString &provider) const {
    static const QStringList allowed = {QStringLiteral("notam"), QStringLiteral("icao"), QStringLiteral("maptiler"), QStringLiteral("weather")};
    return allowed.contains(provider);
}
QString CredentialVault::targetName(const QString &provider) const { return m_targetPrefix + QLatin1Char('/') + provider.toUpper(); }
QString CredentialVault::backendStatus() const {
#ifdef Q_OS_WIN
    return QStringLiteral("Windows Credential Manager");
#else
    return QStringLiteral("UNSUPPORTED: native credential vault is currently implemented for Windows only");
#endif
}
QString CredentialVault::lastError() const { return m_lastError; }
void CredentialVault::setLastError(const QString &error) const {
    if (m_lastError == error) return;
    m_lastError = error;
    emit const_cast<CredentialVault *>(this)->lastErrorChanged();
}
QString CredentialVault::readSecret(const QString &provider) const {
    if (!providerAllowed(provider)) { setLastError(QStringLiteral("Unknown provider identifier.")); return {}; }
#ifdef Q_OS_WIN
    PCREDENTIALW credential = nullptr;
    const QString target = targetName(provider);
    if (!CredReadW(reinterpret_cast<LPCWSTR>(target.utf16()), CRED_TYPE_GENERIC, 0, &credential)) {
        setLastError(QStringLiteral("Credential not found or unavailable (Windows error %1).").arg(GetLastError())); return {};
    }
    const QString value = QString::fromUtf8(reinterpret_cast<const char *>(credential->CredentialBlob), static_cast<qsizetype>(credential->CredentialBlobSize));
    CredFree(credential); setLastError({}); return value;
#else
    Q_UNUSED(provider); setLastError(QStringLiteral("Native credential vault is unsupported on this platform.")); return {};
#endif
}
bool CredentialVault::saveSecret(const QString &provider, const QString &secret) {
    if (!providerAllowed(provider)) { setLastError(QStringLiteral("Unknown provider identifier.")); return false; }
    QByteArray payload = secret.toUtf8();
    if (payload.isEmpty()) { setLastError(QStringLiteral("Refusing to save an empty credential.")); return false; }
#ifdef Q_OS_WIN
    if (payload.size() > CRED_MAX_CREDENTIAL_BLOB_SIZE) { payload.fill('\\0'); setLastError(QStringLiteral("Credential exceeds the Windows vault size limit.")); return false; }
    const QString target = targetName(provider);
    CREDENTIALW credential {};
    credential.Type = CRED_TYPE_GENERIC;
    credential.TargetName = const_cast<LPWSTR>(reinterpret_cast<LPCWSTR>(target.utf16()));
    credential.CredentialBlobSize = static_cast<DWORD>(payload.size());
    credential.CredentialBlob = reinterpret_cast<LPBYTE>(payload.data());
    credential.Persist = CRED_PERSIST_LOCAL_MACHINE;
    credential.UserName = const_cast<LPWSTR>(L"BlueSky PRO");
    const BOOL saved = CredWriteW(&credential, 0);
    const DWORD error = saved ? ERROR_SUCCESS : GetLastError();
    payload.fill('\\0');
    if (!saved) { setLastError(QStringLiteral("Windows Credential Manager write failed (error %1).").arg(error)); return false; }
    setLastError({}); return true;
#else
    payload.fill('\\0'); Q_UNUSED(provider);
    setLastError(QStringLiteral("Native credential vault is unsupported on this platform; no plaintext fallback is permitted.")); return false;
#endif
}
bool CredentialVault::deleteSecret(const QString &provider) {
    if (!providerAllowed(provider)) { setLastError(QStringLiteral("Unknown provider identifier.")); return false; }
#ifdef Q_OS_WIN
    const QString target = targetName(provider);
    if (CredDeleteW(reinterpret_cast<LPCWSTR>(target.utf16()), CRED_TYPE_GENERIC, 0)) { setLastError({}); return true; }
    const DWORD error = GetLastError();
    if (error == ERROR_NOT_FOUND) { setLastError({}); return true; }
    setLastError(QStringLiteral("Windows Credential Manager delete failed (error %1).").arg(error)); return false;
#else
    Q_UNUSED(provider); setLastError(QStringLiteral("Native credential vault is unsupported on this platform.")); return false;
#endif
}
bool CredentialVault::hasSecret(const QString &provider) const {
    if (!providerAllowed(provider)) { setLastError(QStringLiteral("Unknown provider identifier.")); return false; }
#ifdef Q_OS_WIN
    PCREDENTIALW credential = nullptr;
    const QString target = targetName(provider);
    if (!CredReadW(reinterpret_cast<LPCWSTR>(target.utf16()), CRED_TYPE_GENERIC, 0, &credential)) { setLastError({}); return false; }
    CredFree(credential); setLastError({}); return true;
#else
    Q_UNUSED(provider); return false;
#endif
}
