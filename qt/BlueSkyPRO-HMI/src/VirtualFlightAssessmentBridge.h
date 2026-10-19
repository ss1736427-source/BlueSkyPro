#pragma once
#include <QObject>
#include <QVariantMap>
#include <QStringList>
class VirtualFlightAssessmentBridge final : public QObject {
    Q_OBJECT
public:
    explicit VirtualFlightAssessmentBridge(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE QVariantMap assessSyntheticFixture(int scenarioIndex) const;
    Q_INVOKABLE QVariantMap assessManualGeometry(const QString &geometryJson,
                                                const QStringList &selectedTemplateIds,
                                                const QString &missionId,
                                                int missionVersion,
                                                bool compositionComplete,
                                                int scenarioIndex) const;
};
