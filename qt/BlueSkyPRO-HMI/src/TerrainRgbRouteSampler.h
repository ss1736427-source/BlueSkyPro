#pragma once

#include <QHash>
#include <QImage>
#include <QString>
#include <QVariantList>
#include <QVariantMap>

class TerrainRgbRouteSampler final
{
public:
    // Samples a canonical EPSG:4326 route against cached MapTiler Terrain RGB tiles.
    // Output is visual context only and is not DEM or clearance evidence.
    static bool sampleRoute(const QVariantMap &routeSnapshot,
                            const QVariantList &tileManifest,
                            QVariantList *samples,
                            QString *error = nullptr,
                            double maximumSpacingM = 25.0,
                            int maximumSamples = 100000);
};
