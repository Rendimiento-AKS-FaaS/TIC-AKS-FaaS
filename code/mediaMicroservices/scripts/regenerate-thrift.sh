#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT"

if command -v thrift >/dev/null 2>&1; then
  THRIFT=thrift
elif command -v docker >/dev/null 2>&1; then
  THRIFT="docker run --rm -v ${ROOT}:/work -w /work yg397/thrift-microservice-deps:xenial thrift"
else
  echo "Install thrift or Docker to regenerate gen-cpp and gen-lua." >&2
  exit 1
fi

rm -rf gen-cpp gen-lua
mkdir -p gen-cpp gen-lua

$THRIFT -r --gen cpp -out gen-cpp media_service.thrift
$THRIFT -r --gen lua -out gen-lua media_service.thrift

# Keep only nginx client stubs for the read gateway.
find gen-lua -maxdepth 1 -type f ! -name 'media_service_PageService.lua' \
  ! -name 'media_service_ttypes.lua' ! -name 'media_service_constants.lua' -delete

echo "Regenerated read-only Thrift artifacts."
