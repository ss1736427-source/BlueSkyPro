#pragma once
#include <QVariantMap>
class NotamGeometryEngine final {
public:
    // Parse explicit NOTAM geometry and altitude bounds. Q-line search radius is never treated as restriction geometry.
    static QVariantMap normalize(const QVariantMap &source);

    // Build one flat upper plane. The source restriction height is measured from the lowest terrain point.
    // If that plane is below the highest terrain point, use the highest terrain elevation instead.
    // All values must already use the same vertical datum and units (metres).
    static QVariantMap conservativeUpperPlane(double minTerrainMeters,
                                               double maxTerrainMeters,
                                               double totalRestrictionHeightMeters);
};
