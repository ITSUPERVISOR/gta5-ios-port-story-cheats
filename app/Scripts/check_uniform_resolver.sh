#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
mkdir -p Build/Probe
xcrun clang++ -std=c++20 -O2 -fobjc-arc -fblocks -I Source -I Tools/dawn/third_party/webgpu-headers/src \
  -framework Foundation -framework JavaScriptCore Tests/native_uniform_resolver.mm \
  -o Build/Probe/native-uniform-resolver
JSC_useJIT=false JSC_useRegExpJIT=false Build/Probe/native-uniform-resolver
