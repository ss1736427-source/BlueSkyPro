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

    // Produces the complete v2 mission aggregate used by MT-01.
    // missionContext must carry explicit task/intent and exact equipment bindings.
    Q_INVOKABLE QVariantMap buildMissionV2(const QString &geometryJson,
                                            const QStringList &selectedTemplateIds,
                                            const QString &missionId,
                                            int missionVersion,
                                            bool compositionComplete,
                                            const QVariantMap &missionContext) const;

    Q_INVOKABLE QVariantMap buildAutoMissionV2(const QStringList &selectedTemplateIds,
                                                const QString &missionId,
                                                int missionVersion,
                                                const QVariantMap &missionContext) const;
};
