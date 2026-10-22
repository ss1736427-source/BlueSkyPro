#pragma once
#include <QVariantMap>
class NotamGeometryEngine final {
public:
    // Parse explicit NOTAM geometry and altitude bounds. Q-line search radius is never treated as restriction geometry.
    static QVariantMap normalize(const QVariantMap &source);

    // Conservative flat upper plane: highest terrain elevation + clearance.
    // totalRestrictionHeight is measured from the lowest terrain point in the zone.
    // All values must already use the same vertical datum and units (metres).
    static QVariantMap conservativeUpperPlane(double minTerrainMeters,
                                               double maxTerrainMeters,
                                               double totalRestrictionHeightMeters,
                                               double clearanceMeters = 100.0);
};
