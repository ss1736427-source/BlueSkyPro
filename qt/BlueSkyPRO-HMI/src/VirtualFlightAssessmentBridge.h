#pragma once
#include <QObject>
#include <QVariantMap>
class VirtualFlightAssessmentBridge final : public QObject {
    Q_OBJECT
public:
    explicit VirtualFlightAssessmentBridge(QObject *parent = nullptr) : QObject(parent) {}
    Q_INVOKABLE QVariantMap assessSyntheticFixture(int scenarioIndex) const;
};
