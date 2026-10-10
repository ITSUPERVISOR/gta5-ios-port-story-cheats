#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p Build/Probe
xcrun clang -std=c11 -O2 Tests/bc_textures.c Source/BCTextureDecoder.c -o Build/Probe/bc-textures-check
PG_BC_MICROBENCH=1 Build/Probe/bc-textures-check
xcrun clang -std=c11 -O1 -g -fsanitize=address Tests/bc_textures.c Source/BCTextureDecoder.c -o Build/Probe/bc-textures-asan
Build/Probe/bc-textures-asan
xcrun clang++ -std=c++17 -O2 -fobjc-arc Tests/texture_views.mm -framework Foundation -framework JavaScriptCore -o Build/Probe/texture-views-check
Build/Probe/texture-views-check
xcrun clang++ -std=c++17 -O2 -g -fsanitize=address -fobjc-arc Tests/texture_rows.mm -framework Foundation -framework JavaScriptCore -o Build/Probe/texture-rows-check
JSC_useJIT=false JSC_useRegExpJIT=false PG_TEXTURE_ROWS_MICROBENCH=1 Build/Probe/texture-rows-check
