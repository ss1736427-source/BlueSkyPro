#!/usr/bin/env python3
"""Download a bounded NOAA/NCEP GFS 0.25-degree GRIB2 subset.

This is a data-acquisition probe, not a meteorological decoder or flight-release
service. It requires only Python's standard library. Use --self-test for offline
contract checks; use explicit model date/cycle/forecast hour for a live request.
"""
from __future__ import annotations

import argparse
import hashlib
import json
import re
import sys
import time
import urllib.error
import urllib.parse
import urllib.request
from datetime import datetime, timezone
from pathlib import Path
from typing import Iterable

ENDPOINT = "https://nomads.ncep.noaa.gov/cgi-bin/filter_gfs_0p25.pl"
USER_AGENT = "BlueSkyPRO-GRIB2-Prototype/0.1 (weather data acquisition; contact: project maintainers)"
MAX_BYTES_DEFAULT = 256 * 1024 * 1024


class ProbeError(RuntimeError):
    """Expected input, transport, or GRIB container validation failure."""


def validate_bbox(west: float, south: float, east: float, north: float) -> None:
    values = (west, south, east, north)
    if not all(-180 <= west_value <= 180 for west_value in (west, east)):
        raise ProbeError("Longitude must be within [-180, 180].")
    if not all(-90 <= lat <= 90 for lat in (south, north)):
        raise ProbeError("Latitude must be within [-90, 90].")
    if west >= east:
        raise ProbeError("west must be less than east; antimeridian-crossing boxes are not supported.")
    if south >= north:
        raise ProbeError("south must be less than north.")
    if not all(map(lambda value: value == value and abs(value) != float("inf"), values)):
        raise ProbeError("Bounding box values must be finite numbers.")


def parse_levels(text: str) -> tuple[int, ...]:
    try:
        levels = tuple(dict.fromkeys(int(part.strip()) for part in text.split(",") if part.strip()))
    except ValueError as exc:
        raise ProbeError("Levels must be comma-separated integer metres, e.g. 10,80,100.") from exc
    if not levels or any(level < 1 or level > 1000 for level in levels):
        raise ProbeError("Specify one or more positive above-ground levels from 1 to 1000 m.")
    return levels


def build_request_url(date: str, cycle: str, forecast_hour: int, bbox: tuple[float, float, float, float],
                      levels: Iterable[int]) -> str:
    if not re.fullmatch(r"\d{8}", date):
        raise ProbeError("Date must use YYYYMMDD.")
    try:
        datetime.strptime(date, "%Y%m%d")
    except ValueError as exc:
        raise ProbeError("Date is not a valid calendar date.") from exc
    if cycle not in {"00", "06", "12", "18"}:
        raise ProbeError("GFS cycle must be one of 00, 06, 12, 18 UTC.")
    if not 0 <= forecast_hour <= 384:
        raise ProbeError("Forecast hour must be between 0 and 384.")
    west, south, east, north = bbox
    validate_bbox(west, south, east, north)

    params = [
        ("file", f"gfs.t{cycle}z.pgrb2.0p25.f{forecast_hour:03d}"),
        ("var_UGRD", "on"),
        ("var_VGRD", "on"),
        ("subregion", ""),
        ("leftlon", f"{west:g}"),
        ("rightlon", f"{east:g}"),
        ("toplat", f"{north:g}"),
        ("bottomlat", f"{south:g}"),
    ]
    params.extend((f"lev_{level}_m_above_ground", "on") for level in sorted(set(levels)))
    params.append(("dir", f"/gfs.{date}/{cycle}/atmos"))
    return ENDPOINT + "?" + urllib.parse.urlencode(params)


def validate_grib2(data: bytes) -> None:
    if len(data) < 20:
        raise ProbeError("Response is too small to be a GRIB2 file.")
    if data[:4] != b"GRIB":
        preview = data[:120].decode("utf-8", errors="replace").replace("\n", " ")
        raise ProbeError(f"Response is not GRIB data (possible HTML/API error): {preview!r}")
    if data[7] != 2:
        raise ProbeError(f"Expected GRIB edition 2, received edition byte {data[7]}.")
    if not data.endswith(b"7777"):
        raise ProbeError("GRIB2 end marker 7777 is missing; response may be truncated.")


def download(url: str, destination: Path, timeout: float, max_bytes: int) -> tuple[int, str]:
    request = urllib.request.Request(url, headers={"User-Agent": USER_AGENT, "Accept": "application/octet-stream"})
    destination.parent.mkdir(parents=True, exist_ok=True)
    temporary = destination.with_suffix(destination.suffix + ".part")
    digest = hashlib.sha256()
    received = 0
    try:
        with urllib.request.urlopen(request, timeout=timeout) as response, temporary.open("wb") as output:
            status = getattr(response, "status", 200)
            if status != 200:
                raise ProbeError(f"NOMADS returned HTTP {status}.")
            while True:
                chunk = response.read(1024 * 1024)
                if not chunk:
                    break
                received += len(chunk)
                if received > max_bytes:
                    raise ProbeError(f"Download exceeded configured limit ({max_bytes} bytes); narrow the region/levels.")
                output.write(chunk)
                digest.update(chunk)
        with temporary.open("rb") as source:
            data_prefix = source.read(16)
        # Validate the container without loading the complete file into memory.
        if len(data_prefix) < 8 or data_prefix[:4] != b"GRIB" or data_prefix[7] != 2:
            with temporary.open("rb") as source:
                preview = source.read(120).decode("utf-8", errors="replace").replace("\\n", " ")
            raise ProbeError(f"Response is not a GRIB2 payload (possible unavailable cycle/level): {preview!r}")
        if received < 20:
            raise ProbeError("Response is too small to be a valid GRIB2 payload.")
        with temporary.open("rb") as source:
            source.seek(-4, 2)
            if source.read(4) != b"7777":
                raise ProbeError("GRIB2 end marker 7777 is missing; response may be truncated.")
        temporary.replace(destination)
        return received, digest.hexdigest()
    except urllib.error.HTTPError as exc:
        raise ProbeError(f"NOMADS returned HTTP {exc.code}: {exc.reason}") from exc
    except urllib.error.URLError as exc:
        raise ProbeError(f"Network/TLS error while contacting NOMADS: {exc.reason}") from exc
    finally:
        if temporary.exists():
            temporary.unlink()


def run_self_test() -> int:
    bbox = (-123.0, 37.0, -121.0, 39.0)
    url = build_request_url("20261019", "00", 3, bbox, (2, 10, 20, 30, 40, 50, 80, 100))
    parsed = urllib.parse.urlparse(url)
    query = urllib.parse.parse_qs(parsed.query)
    assert parsed.netloc == "nomads.ncep.noaa.gov"
    assert query["file"] == ["gfs.t00z.pgrb2.0p25.f003"]
    for level in (2, 10, 20, 30, 40, 50, 80, 100):
        assert query[f"lev_{level}_m_above_ground"] == ["on"]
    assert query["var_UGRD"] == ["on"] and query["var_VGRD"] == ["on"]
    validate_grib2(b"GRIB" + b"\x00\x00\x00\x02" + b"payload-data" + b"7777")
    for invalid in (b"<html>not grib</html>", b"GRIB" + b"\x00\x00\x00\x01" + b"payload-data" + b"7777",
                    b"GRIB" + b"\x00\x00\x00\x02" + b"truncated-data"):
        try:
            validate_grib2(invalid)
        except ProbeError:
            pass
        else:
            raise AssertionError("invalid GRIB payload was accepted")
    try:
        validate_bbox(10, 95, 20, 96)
    except ProbeError:
        pass
    else:
        raise AssertionError("invalid latitude was accepted")
    print("SELF-TEST PASS: URL construction, selected levels/variables, bbox and GRIB2 guards.")
    return 0


def main(argv: list[str] | None = None) -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--self-test", action="store_true", help="run offline contract tests; no network access")
    parser.add_argument("--date", help="GFS cycle date in UTC, YYYYMMDD")
    parser.add_argument("--cycle", choices=("00", "06", "12", "18"), help="GFS model initialization cycle in UTC")
    parser.add_argument("--forecast-hour", type=int, help="forecast lead hour, e.g. 3, 6, 12")
    parser.add_argument("--bbox", nargs=4, type=float, metavar=("WEST", "SOUTH", "EAST", "NORTH"),
                        help="WGS84 bounding box; antimeridian-crossing boxes are not supported")
    parser.add_argument("--levels", default="2,10,20,30,40,50,80,100", help="comma-separated metres above ground (default: 2,10,20,30,40,50,80,100)")
    parser.add_argument("--output", type=Path, help="output GRIB2 path")
    parser.add_argument("--timeout", type=float, default=90.0, help="network timeout seconds (default: 90)")
    parser.add_argument("--max-bytes", type=int, default=MAX_BYTES_DEFAULT, help="download ceiling (default: 256 MiB)")
    args = parser.parse_args(argv)

    if args.self_test:
        return run_self_test()
    if not all((args.date, args.cycle is not None, args.forecast_hour is not None, args.bbox, args.output)):
        parser.error("live download requires --date, --cycle, --forecast-hour, --bbox and --output")
    if args.timeout <= 0 or args.max_bytes < 20:
        parser.error("--timeout must be positive and --max-bytes must be at least 20")
    levels = parse_levels(args.levels)
    bbox = tuple(args.bbox)
    validate_bbox(*bbox)
    url = build_request_url(args.date, args.cycle, args.forecast_hour, bbox, levels)
    print(f"Source: NOAA/NCEP GFS 0.25-degree NOMADS subset")
    print(f"Cycle: {args.date} {args.cycle} UTC; forecast hour: {args.forecast_hour:03d}")
    print(f"BBox W,S,E,N: {bbox}; wind levels AGL (m): {levels}")
    print(f"Request: {url}")
    started = time.monotonic()
    size, sha256 = download(url, args.output, args.timeout, args.max_bytes)
    elapsed = round(time.monotonic() - started, 3)
    manifest = {
        "classification": "LIVE_MODEL_DATA_UNDECODED",
        "provider": "NOAA/NCEP NOMADS",
        "model": "GFS 0.25 degree",
        "product": "GRIB2 subset",
        "cycle_utc": f"{args.date}T{args.cycle}:00:00Z",
        "forecast_hour": args.forecast_hour,
        "bbox_wgs84_west_south_east_north": list(bbox),
        "requested_variables": ["UGRD", "VGRD"],
        "requested_levels_m_above_ground": list(levels),
        "retrieved_at_utc": datetime.now(timezone.utc).isoformat(),
        "file": args.output.name,
        "bytes": size,
        "sha256": sha256,
        "download_seconds": elapsed,
        "decode_status": "NOT_PERFORMED",
        "flight_use_status": "NOT_APPROVED",
    }
    manifest_path = args.output.with_suffix(args.output.suffix + ".manifest.json")
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n", encoding="utf-8")
    print(f"GRIB2 container validated: {size} bytes; SHA-256 {sha256}; {elapsed}s")
    print(f"Manifest: {manifest_path}")
    print("NOTE: This probe validates acquisition/container only. Decode and scientific checks are still required.")
    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except ProbeError as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        raise SystemExit(2)
