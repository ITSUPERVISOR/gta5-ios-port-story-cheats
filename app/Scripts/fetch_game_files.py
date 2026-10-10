#!/usr/bin/env python3
"""Download the two game files the build needs from YOUR OWN server.

The build needs only the engine (b/8b0b5899ed/game.wasm) and the file list
(data/manifest.json). Everything else streams to the phone while you play.
Nothing is downloaded from anywhere except the server you give.
"""
import argparse
import hashlib
import sys
import urllib.request
from pathlib import Path

ENGINE = 'b/8b0b5899ed/game.wasm'
ENGINE_SHA256 = '11ca8d2c04c5e843d18ff4aea4899d72c86973c6b031df334e67c446b2ae83e0'

parser = argparse.ArgumentParser()
parser.add_argument('--server', required=True, help='e.g. https://my-pc.tailnet-name.ts.net')
parser.add_argument('--out', type=Path, required=True, help='folder to place a minimal mirror in')
args = parser.parse_args()
server = args.server.rstrip('/')

def fetch(relative):
    target = args.out / relative
    target.parent.mkdir(parents=True, exist_ok=True)
    print(f'Downloading {relative} from your server...', flush=True)
    try:
        with urllib.request.urlopen(f'{server}/{relative}', timeout=120) as response, target.open('wb') as stream:
            while chunk := response.read(1 << 20):
                stream.write(chunk)
    except Exception as error:
        sys.exit(f'Could not download {server}/{relative}: {error}\n'
                 'Check that the server is running, reachable, and serves the folder that contains data/ and b/.')
    return target

engine = fetch(ENGINE)
if hashlib.sha256(engine.read_bytes()).hexdigest() != ENGINE_SHA256:
    sys.exit('Your game.wasm is not the build this port supports (8b0b5899ed). '
             'The engine patches are made for that exact file.')
fetch('data/manifest.json')
print('Game files OK.')
