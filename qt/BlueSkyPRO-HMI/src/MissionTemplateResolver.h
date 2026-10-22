#pragma once

#include <QObject>
#include <QVariantMap>

class MissionTemplateResolver final : public QObject
{
    Q_OBJECT

public:
    explicit MissionTemplateResolver(QObject *parent = nullptr);

    // Explainable lexical baseline only; this does not validate mission safety or feasibility.
    Q_INVOKABLE QVariantMap resolve(const QString &taskText) const;
};
