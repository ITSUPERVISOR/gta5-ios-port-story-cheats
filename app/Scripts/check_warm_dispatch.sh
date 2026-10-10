#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p Build/Probe
xcrun clang++ -std=c++20 -O2 -fobjc-arc -fblocks -I Source -I Tools/dawn/third_party/webgpu-headers/src \
  -framework Foundation -framework JavaScriptCore Tests/warm_dispatch.mm -o Build/Probe/warm-dispatch
JSC_useJIT=false JSC_useRegExpJIT=false Build/Probe/warm-dispatch
xcrun clang++ -std=c++20 -O2 -fobjc-arc -fblocks -I Source -I Tools/dawn/third_party/webgpu-headers/src \
  -framework Foundation -framework JavaScriptCore Tests/warm_javascript.mm -o Build/Probe/warm-javascript
JSC_useJIT=false JSC_useRegExpJIT=false Build/Probe/warm-javascript
