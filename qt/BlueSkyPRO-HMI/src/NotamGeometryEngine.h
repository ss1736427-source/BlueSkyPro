#pragma once
#include <QVariantMap>
class NotamGeometryEngine final {
public:
    // Parse explicit NOTAM geometry and altitude bounds. Q-line search radius is never treated as restriction geometry.
    static QVariantMap normalize(const QVariantMap &source);
};
