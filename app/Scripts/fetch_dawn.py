#!/usr/bin/env python3
"""Download the prebuilt Dawn graphics library (BSD-3-Clause) from this project's release."""
import hashlib
import io
import os
import sys
import tarfile
import urllib.request
from pathlib import Path

root = Path(__file__).resolve().parents[1]
# Set when the project is published; a fork can override it with DAWN_PREBUILT_URL.
URL = os.environ.get('DAWN_PREBUILT_URL') or 'https://github.com/kevinerikjs/gta5-ios-port/releases/download/dawn-1/dawn-ios-prebuilt.tar.gz'
SHA256 = '9ab9733b7da3f1b263f9aa48964736fee95038c4c125954a3a214bca5cfbad88'
if (root / 'Build/Dawn-iOS/src/dawn/native/libwebgpu_dawn.a').exists():
    print('Dawn already present.')
    sys.exit(0)
print('Downloading prebuilt Dawn...', flush=True)
data = urllib.request.urlopen(URL, timeout=300).read()
if hashlib.sha256(data).hexdigest() != SHA256:
    sys.exit('Prebuilt Dawn checksum mismatch.')
from safe_extract import extract
with tarfile.open(fileobj=io.BytesIO(data)) as archive:
    extract(archive, root)
print('Dawn ready.')
