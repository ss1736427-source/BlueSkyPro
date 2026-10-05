#include "PlanningBridge.h"

PlanningBridge::PlanningBridge(QObject *parent)
    : QObject(parent)
{
}

QVariantMap PlanningBridge::result() const
{
    return m_result;
}

void PlanningBridge::publishResult(const QVariantMap &result)
{
    if (m_result == result)
        return;

    m_result = result;
    emit resultChanged();
}

void PlanningBridge::clear()
{
    if (m_result.isEmpty())
        return;

    m_result.clear();
    emit resultChanged();
}
