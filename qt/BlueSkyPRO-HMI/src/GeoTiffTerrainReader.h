#pragma once

#include "terrain_profile_validator.hpp"

#include <QString>

namespace bluesky::planning {

// Reads GeoTIFF into a decoded grid for inspection only. The returned grid is
// deliberately marked UNKNOWN for surface, vertical datum and accuracy: file
// metadata alone is not a trusted source attestation and cannot grant READY.
class GeoTiffTerrainReader final {
public:
    static bool readForInspection(const QString &filePath,
                                  TerrainDemGrid *grid,
                                  QString *error = nullptr);
};

} // namespace bluesky::planning
