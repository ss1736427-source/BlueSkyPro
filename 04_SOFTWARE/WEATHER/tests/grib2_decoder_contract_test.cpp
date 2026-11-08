#include "grib2_decoder.hpp"
#include <cassert>
#include <string>
#include <vector>
int main()
{
    std::vector<bluesky::weather::WindField> fields;
    std::string error;
    const bool decoded = bluesky::weather::Grib2Decoder::decodeWindFile(
        "this-file-does-not-exist.grib2", fields, &error);
    assert(!decoded && fields.empty() && !error.empty());
    return 0;
}
