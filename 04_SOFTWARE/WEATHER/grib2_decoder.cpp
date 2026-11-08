#include "grib2_decoder.hpp"

#include <eccodes.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <limits>
#include <map>
#include <sstream>
#include <tuple>
#include <utility>

namespace bluesky::weather {
namespace {

struct Message {
    int level{0};
    int date{0};
    int time{0};
    long ni{0};
    long nj{0};
    std::string short_name;
    std::string units;
    std::vector<double> latitudes;
    std::vector<double> longitudes;
    std::vector<double> values;
};

void setError(std::string* out, const std::string& message)
{
    if (out) *out = message;
}

bool getLong(codes_handle* h, const char* key, long& value)
{
    return codes_get_long(h, key, &value) == CODES_SUCCESS;
}

bool getString(codes_handle* h, const char* key, std::string& value)
{
    char buffer[128]{};
    std::size_t length = sizeof(buffer);
    if (codes_get_string(h, key, buffer, &length) != CODES_SUCCESS)
        return false;
    value.assign(buffer);
    return true;
}

double normalizeLongitude(double lon)
{
    while (lon > 180.0) lon -= 360.0;
    while (lon < -180.0) lon += 360.0;
    return lon;
}

std::vector<double> uniqueAxis(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    std::vector<double> result;
    for (double value : values) {
        if (result.empty() || std::abs(result.back() - value) > 1e-6)
            result.push_back(value);
    }
    return result;
}

std::size_t nearestAxisIndex(const std::vector<double>& axis, double value)
{
    auto it = std::lower_bound(axis.begin(), axis.end(), value);
    if (it == axis.end()) return axis.size() - 1;
    if (it == axis.begin()) return 0;
    const std::size_t hi = static_cast<std::size_t>(it - axis.begin());
    return std::abs(axis[hi] - value) < std::abs(axis[hi - 1] - value) ? hi : hi - 1;
}

bool readMessage(codes_handle* h, Message& m, std::string* error)
{
    long edition = 0, level = 0, ni = 0, nj = 0, validity_date = 0, validity_time = 0;
    if (!getLong(h, "edition", edition) || edition != 2 ||
        !getLong(h, "level", level) ||
        !getLong(h, "Ni", ni) || !getLong(h, "Nj", nj) ||
        !getLong(h, "validityDate", validity_date) ||
        !getLong(h, "validityTime", validity_time)) {
        setError(error, "GRIB message is missing required edition, level, grid or valid-time metadata");
        return false;
    }

    std::string type_of_level, grid_type;
    if (!getString(h, "typeOfLevel", type_of_level) ||
        !getString(h, "gridType", grid_type) ||
        !getString(h, "shortName", m.short_name) ||
        !getString(h, "units", m.units)) {
        setError(error, "GRIB message is missing type, grid, parameter or units metadata");
        return false;
    }
    if (type_of_level != "heightAboveGround" ||
        (m.short_name != "u" && m.short_name != "v") ||
        grid_type != "regular_ll" ||
        (m.units != "m s**-1" && m.units != "m/s")) {
        setError(error, "unsupported GRIB field: expected U/V in m/s on regular_ll heightAboveGround");
        return false;
    }
    if (level <= 0 || ni < 2 || nj < 2 || ni * nj > 20000000) {
        setError(error, "GRIB grid dimensions or AGL level are outside supported bounds");
        return false;
    }
    m.level = static_cast<int>(level);
    m.date = static_cast<int>(validity_date);
    m.time = static_cast<int>(validity_time);
    m.ni = ni;
    m.nj = nj;

    int iterator_error = CODES_SUCCESS;
    codes_iterator* iterator = codes_grib_iterator_new(h, 0, &iterator_error);
    if (!iterator || iterator_error != CODES_SUCCESS) {
        setError(error, "ecCodes cannot create a regular-grid iterator");
        return false;
    }

    std::vector<double> raw_lat, raw_lon, raw_values;
    raw_lat.reserve(static_cast<std::size_t>(ni * nj));
    raw_lon.reserve(static_cast<std::size_t>(ni * nj));
    raw_values.reserve(static_cast<std::size_t>(ni * nj));
    double lat = 0.0, lon = 0.0, value = 0.0, missing = 1.0e36;
    long bitmap_present = 0;
    if (getLong(h, "bitmapPresent", bitmap_present) && bitmap_present != 0)
        codes_set_double(h, "missingValue", missing);
    while (codes_grib_iterator_next(iterator, &lat, &lon, &value)) {
        raw_lat.push_back(lat);
        raw_lon.push_back(normalizeLongitude(lon));
        raw_values.push_back((std::isfinite(value) && std::abs(value) < 1e19 &&
                              std::abs(value - missing) > 1e-6)
                                 ? value : std::numeric_limits<double>::quiet_NaN());
    }
    codes_grib_iterator_delete(iterator);

    const std::size_t expected = static_cast<std::size_t>(ni * nj);
    if (raw_values.size() != expected) {
        setError(error, "GRIB iterator point count does not match Ni x Nj");
        return false;
    }

    m.latitudes = uniqueAxis(raw_lat);
    m.longitudes = uniqueAxis(raw_lon);
    if (m.latitudes.size() != static_cast<std::size_t>(nj) ||
        m.longitudes.size() != static_cast<std::size_t>(ni)) {
        setError(error, "GRIB grid is not a complete rectilinear regular_ll grid");
        return false;
    }
    m.values.assign(expected, std::numeric_limits<double>::quiet_NaN());
    std::vector<bool> occupied(expected, false);
    for (std::size_t i = 0; i < expected; ++i) {
        const std::size_t y = nearestAxisIndex(m.latitudes, raw_lat[i]);
        const std::size_t x = nearestAxisIndex(m.longitudes, raw_lon[i]);
        if (std::abs(m.latitudes[y] - raw_lat[i]) > 1e-4 ||
            std::abs(m.longitudes[x] - raw_lon[i]) > 1e-4) {
            setError(error, "GRIB coordinate cannot be mapped to normalized grid axes");
            return false;
        }
        const std::size_t index = y * m.longitudes.size() + x;
        if (occupied[index]) {
            setError(error, "GRIB grid contains duplicate coordinate cells");
            return false;
        }
        occupied[index] = true;
        m.values[index] = raw_values[i];
    }
    if (std::find(occupied.begin(), occupied.end(), false) != occupied.end()) {
        setError(error, "GRIB grid contains unpopulated coordinate cells");
        return false;
    }
    return true;
}

bool sameGrid(const Message& a, const Message& b)
{
    return a.ni == b.ni && a.nj == b.nj &&
           a.latitudes == b.latitudes && a.longitudes == b.longitudes;
}

} // namespace

bool Grib2Decoder::decodeWindFile(const std::string& path,
                                 std::vector<WindField>& fields,
                                 std::string* error)
{
    FILE* file = std::fopen(path.c_str(), "rb");
    if (!file) {
        setError(error, "cannot open GRIB2 input file");
        return false;
    }

    using Key = std::tuple<int, int, int>;
    struct Pair { bool has_u{false}; bool has_v{false}; Message u; Message v; };
    std::map<Key, Pair> pairs;
    std::size_t message_count = 0;
    bool failed = false;
    std::string failure;

    while (true) {
        int ec_error = CODES_SUCCESS;
        codes_handle* handle = codes_handle_new_from_file(nullptr, file, PRODUCT_GRIB, &ec_error);
        if (!handle) {
            if (ec_error != CODES_SUCCESS && !std::feof(file)) {
                failed = true;
                failure = "ecCodes failed while reading a GRIB message";
            }
            break;
        }
        Message message;
        std::string message_error;
        if (!readMessage(handle, message, &message_error)) {
            failed = true;
            failure = message_error;
            codes_handle_delete(handle);
            break;
        }
        codes_handle_delete(handle);
        ++message_count;
        Pair& pair = pairs[Key{message.level, message.date, message.time}];
        bool& present = message.short_name == "u" ? pair.has_u : pair.has_v;
        Message& target = message.short_name == "u" ? pair.u : pair.v;
        if (present) {
            failed = true;
            failure = "duplicate U or V message for the same AGL level and valid time";
            break;
        }
        target = std::move(message);
        present = true;
    }
    std::fclose(file);

    if (failed) {
        setError(error, failure);
        return false;
    }
    if (message_count == 0 || pairs.empty()) {
        setError(error, "GRIB2 file contains no supported wind messages");
        return false;
    }

    std::vector<WindField> decoded;
    decoded.reserve(pairs.size());
    for (const auto& entry : pairs) {
        const Pair& pair = entry.second;
        if (!pair.has_u || !pair.has_v) {
            setError(error, "GRIB2 U/V component pair is incomplete at one AGL level or valid time");
            return false;
        }
        if (!sameGrid(pair.u, pair.v)) {
            setError(error, "GRIB2 U/V components do not share identical grid coordinates");
            return false;
        }
        WindField f;
        f.level_agl_m = pair.u.level;
        f.valid_date_utc = pair.u.date;
        f.valid_time_utc = pair.u.time;
        f.rows = pair.u.latitudes.size();
        f.columns = pair.u.longitudes.size();
        f.latitudes_deg = pair.u.latitudes;
        f.longitudes_deg = pair.u.longitudes;
        f.eastward_mps = pair.u.values;
        f.northward_mps = pair.v.values;
        decoded.push_back(std::move(f));
    }

    std::sort(decoded.begin(), decoded.end(),
              [](const WindField& a, const WindField& b) {
                  return a.level_agl_m < b.level_agl_m;
              });
    fields = std::move(decoded);
    if (error) error->clear();
    return true;
}

} // namespace bluesky::weather
