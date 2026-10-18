#include "ManualGeometryBridge.h"

#include <QJsonDocument>
#include <QJsonObject>

void ManualGeometryBridge::publishGeometry(const QString &json, bool complete)
{
    const QJsonDocument document = QJsonDocument::fromJson(json.toUtf8());
    if (!document.isObject()
        || !document.object().value(QStringLiteral("objects")).isArray()) {
        emit bridgeError(QStringLiteral("MANUAL_GEOMETRY_JSON_INVALID"));
        return;
    }

    if (m_geometryJson != json) {
        m_geometryJson = json;
        emit geometryChanged();
    }
    if (m_compositionComplete != complete) {
        m_compositionComplete = complete;
        emit compositionCompleteChanged();
    }
}
