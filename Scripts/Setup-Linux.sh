#!/bin/bash
set -e
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$ROOT_DIR"
Vendor/Binaries/Premake/Linux/premake5 --cc=clang --file=Build.lua gmake2