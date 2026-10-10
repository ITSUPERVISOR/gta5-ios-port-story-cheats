#!/bin/bash
# Build an unsigned IPA from YOUR OWN game files on YOUR OWN server.
# Usage: ./build_ipa.sh https://my-pc.tailnet-name.ts.net [--paid]
#   --paid  request the extra-memory entitlements (paid Apple developer account)
# Needs: an Apple Silicon Mac (or the GitHub Actions workflow), Xcode, xcodegen.
set -euo pipefail
SERVER="${1:?Usage: ./build_ipa.sh https://your-server [--paid]}"
ENTITLEMENTS=App/Free.entitlements
[ "${2:-}" = "--paid" ] && ENTITLEMENTS=App/Full.entitlements
cd "$(dirname "$0")/app"
JOBS="${JOBS:-$(sysctl -n hw.ncpu)}"
# The system Python runs natively on Apple Silicon (a Rosetta Python would
# make the engine tools build for the wrong CPU).
PY="${PYTHON:-/usr/bin/python3}"

"$PY" Scripts/fetch_game_files.py --server "$SERVER" --out Build/Inputs/mirror
"$PY" Scripts/prepare.py --mirror Build/Inputs/mirror
# The app opens with your server already filled in.
/usr/libexec/PlistBuddy -c "Set :PGDefaultAssetServer $SERVER" App/Info.plist

xcodebuild -project PlayGTA5Native.xcodeproj -scheme PlayGTA5Native \
  -configuration Release -sdk iphoneos -destination 'generic/platform=iOS' \
  -derivedDataPath Build/Release -jobs "$JOBS" \
  GCC_OPTIMIZATION_LEVEL=2 SWIFT_OPTIMIZATION_LEVEL=-O DEBUG_INFORMATION_FORMAT=dwarf \
  CODE_SIGNING_ALLOWED=NO CODE_SIGN_ENTITLEMENTS="$ENTITLEMENTS" build > Build/xcodebuild.log 2>&1 || {
  grep -E 'error:' Build/xcodebuild.log | head -20
  echo "Build failed; the full log is app/Build/xcodebuild.log"; exit 1; }
grep -E 'BUILD SUCCEEDED' Build/xcodebuild.log

APP=Build/Release/Build/Products/Release-iphoneos/PlayGTA5Native.app
# AltStore/SideStore re-sign the app; an ad-hoc signature with the chosen
# entitlements tells them what to request.
codesign --force --sign - --entitlements "$ENTITLEMENTS" --generate-entitlement-der "$APP"
STAGE=Build/IPA
mkdir -p "$STAGE/Payload"
ditto "$APP" "$STAGE/Payload/PlayGTA5Native.app"
(cd "$STAGE" && zip -qry ../../../GTA5Port.ipa Payload)
echo "Done: $(cd .. && pwd)/GTA5Port.ipa"
