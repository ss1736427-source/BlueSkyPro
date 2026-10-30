#pragma once
#include <QVariantMap>
class NotamGeometryEngine final {
public:
    // Parse explicit NOTAM geometry and altitude bounds. Q-line search radius is never treated as restriction geometry.
    static QVariantMap normalize(const QVariantMap &source);

    // Keep the NOTAM ceiling as one flat plane derived from the source height.
    // Terrain peaks must not raise or reshape that ceiling. Inputs use metres.
    static QVariantMap conservativeUpperPlane(double minTerrainMeters,
                                               double maxTerrainMeters,
                                               double totalRestrictionHeightMeters);
};
