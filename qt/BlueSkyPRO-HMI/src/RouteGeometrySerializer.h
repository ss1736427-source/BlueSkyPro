#pragma once

#include "../../../04_SOFTWARE/PLANNING/model/route_model.hpp"

#include <QJsonObject>
#include <QString>

namespace bluesky::planning {

// Converts canonical WGS84 Route geometry to the Planning Core request contract.
// Returns false without modifying output when the route is incomplete or invalid.
bool serializeRouteGeometry(const Route &route,
                            QJsonObject &output,
                            QString *error = nullptr);

// Adds canonical route geometry to an already assembled Planning Core request.
// Existing authoritative request inputs are preserved; no placeholder inputs are created.
bool attachRouteGeometryToRequest(const Route &route,
                                  QJsonObject &request,
                                  QString *error = nullptr);

} // namespace bluesky::planning
