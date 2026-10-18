#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>

class ManualMissionModelAdapter final : public QObject
{
    Q_OBJECT

public:
    explicit ManualMissionModelAdapter(QObject *parent = nullptr) : QObject(parent) {}

    // Converts map-captured geometry into the canonical typed Mission model.
    // A valid result means structural completeness only, never flight readiness.
    Q_INVOKABLE QVariantMap buildMission(const QString &geometryJson,
                                         const QStringList &selectedTemplateIds,
                                         const QString &missionId,
                                         int missionVersion,
                                         bool compositionComplete) const;
};
