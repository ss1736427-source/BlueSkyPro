#pragma once

#include "weather_field.hpp"

#include <string>
#include <vector>

namespace bluesky::weather {

class Grib2Decoder final {
public:
    // Decodes paired instantaneous U/V fields on regular latitude/longitude
    // grids at heightAboveGround levels. Unsupported grids/levels are rejected.
    static bool decodeWindFile(const std::string& path,
                               std::vector<WindField>& fields,
                               std::string* error = nullptr);
};

} // namespace bluesky::weather
