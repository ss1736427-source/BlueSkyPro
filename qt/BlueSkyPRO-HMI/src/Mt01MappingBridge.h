#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

// Qt boundary for the canonical MT-01 planning pipeline.
// The bridge accepts a complete canonical Mission JSON document and explicit
// camera/flight inputs. It does not fill missing assignments or infer safety data.
class Mt01MappingBridge final : public QObject
{
    Q_OBJECT

public:
    explicit Mt01MappingBridge(QObject *parent = nullptr) : QObject(parent) {}

    Q_INVOKABLE QVariantMap calculate(const QString &missionJson,
                                      const QVariantMap &parameters) const;
};
