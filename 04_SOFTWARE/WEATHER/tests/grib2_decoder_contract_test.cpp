#include "grib2_decoder.hpp"

#include <eccodes.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

namespace {
bool writeWindMessage(const char* path, const char* shortName,
                      double value, const char* mode)
{
    codes_handle* handle = codes_handle_new_from_samples(nullptr, "regular_ll_sfc_grib2");
    if (!handle) return false;

    std::size_t nameLength = std::string(shortName).size();
    const bool metadataOk =
        codes_set_string(handle, "shortName", shortName, &nameLength) == CODES_SUCCESS &&
        codes_set_string(handle, "typeOfLevel", "heightAboveGround",
                         nullptr) == CODES_SUCCESS &&
        codes_set_long(handle, "level", 50) == CODES_SUCCESS &&
        codes_set_long(handle, "dataDate", 20261108) == CODES_SUCCESS &&
        codes_set_long(handle, "dataTime", 600) == CODES_SUCCESS &&
        codes_set_long(handle, "forecastTime", 0) == CODES_SUCCESS;

    std::size_t count = 0;
    bool valuesOk = metadataOk &&
                    codes_get_size(handle, "values", &count) == CODES_SUCCESS;
    std::vector<double> values(count, value);
    if (valuesOk)
        valuesOk = codes_set_double_array(handle, "values", values.data(), values.size()) == CODES_SUCCESS;

    const bool writeOk = valuesOk &&
                         codes_write_message(handle, path, mode) == CODES_SUCCESS;
    codes_handle_delete(handle);
    return writeOk;
}
}

int main()
{
    const std::string path = "weather_decoder_fixture.grib2";
    std::remove(path.c_str());

    assert(writeWindMessage(path.c_str(), "u", 4.0, "w"));
    assert(writeWindMessage(path.c_str(), "v", 2.0, "a"));

    std::vector<bluesky::weather::WindField> fields;
    std::string error;
    const bool decoded = bluesky::weather::Grib2Decoder::decodeWindFile(path, fields, &error);
    assert(decoded && error.empty());
    assert(fields.size() == 1);
    assert(fields[0].level_agl_m == 50);
    assert(fields[0].valid_date_utc == 20261108);
    assert(fields[0].valid_time_utc == 600);
    assert(fields[0].rows >= 2 && fields[0].columns >= 2);
    assert(fields[0].eastward_mps.size() == fields[0].rows * fields[0].columns);
    assert(fields[0].northward_mps.size() == fields[0].rows * fields[0].columns);
    for (double value : fields[0].eastward_mps)
        assert(std::abs(value - 4.0) < 1e-6);
    for (double value : fields[0].northward_mps)
        assert(std::abs(value - 2.0) < 1e-6);

    fields.clear();
    assert(!bluesky::weather::Grib2Decoder::decodeWindFile(
        "this-file-does-not-exist.grib2", fields, &error));
    assert(fields.empty() && !error.empty());

    std::remove(path.c_str());
    return 0;
}
