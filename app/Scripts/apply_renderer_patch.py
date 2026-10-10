#!/usr/bin/env python3
"""Fetch the original renderer script from the public playgta5 repository and
apply this project's changes. The original is not redistributed here."""
import hashlib
import subprocess
import sys
import urllib.request
from pathlib import Path

root = Path(__file__).resolve().parents[1]
ORIGINAL = 'https://raw.githubusercontent.com/shadany7824/playgta5/a3bad032c92cf929d3f4fa2fb236ff0476878b51/wgpu_worker.js'
ORIGINAL_SHA256 = 'e1b83d2b10a98d29d8f7ac45a4e42f68486144a8046df688667891af034716c7'
target = root / 'Resources/wgpu_worker.js'
source = urllib.request.urlopen(ORIGINAL, timeout=120).read()
if hashlib.sha256(source).hexdigest() != ORIGINAL_SHA256:
    sys.exit('The original wgpu_worker.js changed upstream; this patch targets commit a3bad03.')
target.write_bytes(source)
subprocess.run(['patch', '-s', str(target), str(root / 'Patches/wgpu_worker.js.patch')], check=True)
print('Renderer script patched.')
