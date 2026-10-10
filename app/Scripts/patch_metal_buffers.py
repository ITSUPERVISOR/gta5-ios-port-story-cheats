"""Apply the local buffer-offset cache to the pinned Dawn source."""
from pathlib import Path
import subprocess
root = Path(__file__).resolve().parents[1]
dawn = root / 'Tools/dawn'
patches = root / 'Scripts/patches'
source = dawn / 'src/dawn/native/metal/CommandBufferMTL.mm'
marker = 'PlayGTA5: vertex inputs are packed after'
if marker in source.read_text():
    subprocess.run(['patch', '-p1', '--batch', '--dry-run', '-R', '--fuzz=0',
                    '-i', str(patches / 'metal-buffer-offsets.patch')],
                   cwd=dawn, check=True, stdout=subprocess.PIPE)
else:
    subprocess.run(['patch', '-p1', '--batch', '-i', str(patches / 'metal-buffer-offsets.patch')], cwd=dawn, check=True)
header = source.parent / 'PlayGTA5MetalBufferCache.h'
content = (patches / header.name).read_bytes()
if not header.exists() or header.read_bytes() != content:
    header.write_bytes(content)
