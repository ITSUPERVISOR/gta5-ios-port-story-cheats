"""Prepare pinned Dawn/Metal and BC decoding dependencies for the native app."""
import argparse
from pathlib import Path
import subprocess
import tarfile
import urllib.request
parser = argparse.ArgumentParser()
parser.add_argument('--simulator', action='store_true')
options = parser.parse_args()
root = Path(__file__).resolve().parents[1]
build = root / ('Build/Dawn-Simulator' if options.simulator else 'Build/Dawn-iOS')
sdk = 'iphonesimulator' if options.simulator else 'iphoneos'
tools = root / 'Tools'
tools.mkdir(exist_ok=True)
dawn_commit = '8594916844e4890c669c8fde887d9b8fb663695f'
bc_commit = '80859ed3b7afb1c527a2a99d70c61457bea72d0c'

def source_archive(repo, commit, destination):
    if destination.exists():
        return
    archive = tools / (destination.name + '.tar.gz')
    urllib.request.urlretrieve(f'https://codeload.github.com/{repo}/tar.gz/{commit}', archive)
    with tarfile.open(archive) as tar:
        tar.extractall(tools, filter='data')
    (tools / (repo.split('/')[-1] + '-' + commit)).rename(destination)

source_archive('google/dawn', dawn_commit, tools / 'dawn')
source_archive('iOrange/bcdec', bc_commit, tools / 'bcdec')
# D3D shaders in this mirror use 23 sampled textures and at most one storage
# texture. Keep Metal's existing total binding budget and the default storage
# allowance, assigning the spare slots to sampled textures.
metal_limits = tools / 'dawn/src/dawn/native/metal/PhysicalDeviceMTL.mm'
original = ('limits->v1.maxSampledTexturesPerShaderStage += additional / 2;\n'
            '        limits->v1.maxStorageTexturesPerShaderStage += (additional - additional / 2);')
replacement = 'limits->v1.maxSampledTexturesPerShaderStage += additional; // PlayGTA5: preserve total Metal texture budget.'
metal_source = metal_limits.read_text()
if original in metal_source:
    metal_limits.write_text(metal_source.replace(original, replacement))
elif replacement not in metal_source:
    raise RuntimeError('Pinned Dawn texture-limit patch no longer matches.')
subprocess.run(['/usr/bin/python3', str(root / 'Scripts/patch_metal_buffers.py')], check=True)
metal_patch_inputs = [metal_limits, tools / 'dawn/src/dawn/native/metal/CommandBufferMTL.mm',
                      tools / 'dawn/src/dawn/native/metal/PlayGTA5MetalBufferCache.h']
if not (build / 'src/dawn/native/libwebgpu_dawn.a').exists():
    if not (tools / 'dawn/third_party/abseil-cpp/absl').exists():
        subprocess.run(['python3', 'tools/fetch_dawn_dependencies.py'], cwd=tools / 'dawn', check=True)
    flags = ['-DCMAKE_SYSTEM_NAME=iOS', '-DCMAKE_OSX_SYSROOT=' + sdk,
             '-DCMAKE_OSX_ARCHITECTURES=arm64', '-DCMAKE_OSX_DEPLOYMENT_TARGET=17.0',
             '-DCMAKE_BUILD_TYPE=Release', '-DDAWN_BUILD_MONOLITHIC_LIBRARY=STATIC',
             '-DDAWN_ENABLE_METAL=ON', '-DTINT_BUILD_WGSL_READER=ON',
             '-DTINT_BUILD_WGSL_WRITER=ON', '-DTINT_BUILD_MSL_WRITER=ON']
    disabled = ['DAWN_BUILD_TESTS', 'DAWN_BUILD_SAMPLES', 'DAWN_BUILD_BENCHMARKS',
                'DAWN_BUILD_FUZZERS', 'DAWN_BUILD_NODE_BINDINGS', 'DAWN_BUILD_PROTOBUF',
                'DAWN_ENABLE_D3D11', 'DAWN_ENABLE_D3D12', 'DAWN_ENABLE_VULKAN',
                'DAWN_ENABLE_DESKTOP_GL', 'DAWN_ENABLE_OPENGLES', 'DAWN_ENABLE_NULL',
                'DAWN_ENABLE_SWIFTSHADER', 'DAWN_FETCH_DEPENDENCIES',
                'TINT_BUILD_TESTS', 'TINT_BUILD_CMD_TOOLS', 'TINT_BUILD_BENCHMARKS',
                'TINT_BUILD_FUZZERS', 'TINT_BUILD_SPV_READER', 'TINT_BUILD_SPV_WRITER',
                'TINT_BUILD_GLSL_WRITER', 'TINT_BUILD_HLSL_WRITER', 'TINT_BUILD_IR_BINARY']
    flags.extend('-D' + name + '=OFF' for name in disabled)
    subprocess.run(['cmake', '-S', str(tools / 'dawn'), '-B', str(build), *flags], check=True)
    subprocess.run(['cmake', '--build', str(build), '--target', 'webgpu_dawn', '-j', '2'], check=True)
elif max(path.stat().st_mtime for path in metal_patch_inputs) > (build / 'src/dawn/native/libwebgpu_dawn.a').stat().st_mtime:
    subprocess.run(['cmake', '--build', str(build), '--target', 'webgpu_dawn', '-j', '2'], check=True)
subprocess.run(['python3', 'Scripts/generate_gpu_bridge.py'], cwd=root, check=True)
print('Native Metal renderer dependencies ready.')
