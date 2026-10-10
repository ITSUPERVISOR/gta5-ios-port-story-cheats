"""Run GTA's benchmark in the dedicated simulator and retain frame timings."""
import argparse
import csv
import hashlib
import json
import os
import re
from pathlib import Path
import shutil
import statistics
import subprocess
import time

parser = argparse.ArgumentParser()
parser.add_argument('--name', required=True)
parser.add_argument('--reference', action='store_true')
parser.add_argument('--profile', action='store_true')
parser.add_argument('--profile-bridge', action='store_true')
parser.add_argument('--no-capture-world', action='store_true')
parser.add_argument('--frame-reports', action='store_true')
parser.add_argument('--balanced-asset-qos', action='store_true')
parser.add_argument('--reference-texture-uploads', action='store_true')
parser.add_argument('--reference-bc-decoder', action='store_true')
parser.add_argument('--reference-texture-shadows', action='store_true')
parser.add_argument('--reference-fused-texture-uploads', action='store_true')
parser.add_argument('--fast', action='store_true')
parser.add_argument('--reference-passes', action='store_true')
parser.add_argument('--reference-prepared-draws', action='store_true')
parser.add_argument('--reference-proxy-snapshots', action='store_true')
parser.add_argument('--native-warm-dispatch', action='store_true')
parser.add_argument('--reference-warm-dispatch', action='store_true')
parser.add_argument('--native-state-packing', action='store_true')
parser.add_argument('--reference-state-packing', action='store_true')
parser.add_argument('--native-constant-content-cache', action='store_true')
parser.add_argument('--reference-constant-content-cache', action='store_true')
parser.add_argument('--reference-binding-cache', action='store_true')
parser.add_argument('--reference-metal-offsets', action='store_true')
parser.add_argument('--native-usage-stamps', action='store_true')
parser.add_argument('--reference-usage-stamps', action='store_true')
parser.add_argument('--reference-draw-resolver', action='store_true')
parser.add_argument('--reference-resource-usage', action='store_true')
parser.add_argument('--native-resource-usage', action='store_true')
parser.add_argument('--stream-assets', action='store_true')
parser.add_argument('--serialized-asset-fills', action='store_true')
parser.add_argument('--reference-asset-hints', action='store_true')
parser.add_argument('--skip-asset-warm', action='store_true')
parser.add_argument('--asset-latency-ms', type=int, default=0)
parser.add_argument('--cache-tag')
parser.add_argument('--iterations', type=int, choices=range(1,11), default=1)
parser.add_argument('--memory-limit-mb', type=int)
parser.add_argument('--streamer-percentage', type=float)
parser.add_argument('--cores', type=int, choices=range(1,9), default=2)
parser.add_argument('--pass', dest='scene', type=int)
parser.add_argument('--timeout', type=int, default=300)
args = parser.parse_args()
root = Path(__file__).resolve().parents[1]
device = os.environ['SIMULATOR_ID']
bundle = 'io.github.playgta5ios.app'
output = root / 'Build/Benchmarks' / args.name
output.mkdir(parents=True, exist_ok=False)
installed = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, bundle, 'app'], text=True).strip())
built = root / 'Build/Simulator/Build/Products/Release-iphonesimulator/PlayGTA5Native.app'
def digest(path):
    value = hashlib.sha256()
    with path.open('rb') as stream:
        for chunk in iter(lambda: stream.read(1024*1024), b''):
            value.update(chunk)
    return value.hexdigest()
build_ids = {}
for filename in ['PlayGTA5Native', 'wgpu_worker.js', 'gpu-batch.js', 'gpu-runtime.js', 'bootset_native.json', 'bootset_native.summary.json', 'Info.plist']:
    build_ids[filename] = digest(built / filename)
    if digest(installed / filename) != build_ids[filename]:
        raise RuntimeError('The installed simulator app does not match the completed build: ' + filename)
(output / 'build.json').write_text(json.dumps(build_ids, indent=2))
env = dict(os.environ, SIMCTL_CHILD_JSC_useJIT='false',
           SIMCTL_CHILD_JSC_useRegExpJIT='false', SIMCTL_CHILD_PG_BENCHMARK='1',
           SIMCTL_CHILD_PG_ENGINE_CORES=str(args.cores), SIMCTL_CHILD_PG_BENCHMARK_ITERATIONS=str(args.iterations))
if args.memory_limit_mb is not None:
    if not 2048 <= args.memory_limit_mb <= 8192: raise ValueError('Use a simulated memory limit of 2048 to 8192 MiB')
    env['SIMCTL_CHILD_PG_MEMORY_GUARD'] = '1'
    env['SIMCTL_CHILD_PG_STREAM_MEMORY_LIMIT_MB'] = str(args.memory_limit_mb)
if args.streamer_percentage is not None:
    if not .1 <= args.streamer_percentage <= .95: raise ValueError('Streaming percentage must be 0.1 to 0.95')
    env['SIMCTL_CHILD_PG_STREAMER_PERCENTAGE'] = str(args.streamer_percentage)
if args.scene is not None:
    env['SIMCTL_CHILD_PG_BENCHMARK_PASS'] = str(args.scene)
if args.stream_assets:
    tag = args.cache_tag or args.name
    if not re.fullmatch(r'[A-Za-z0-9_-]{1,64}', tag) or not 0 <= args.asset_latency_ms <= 2000:
        raise ValueError('Use a cache tag of 1–64 letters/digits/hyphens/underscores and latency of 0–2000 ms')
    env.update(SIMCTL_CHILD_PG_ASSET_CACHE_TAG=tag,
               SIMCTL_CHILD_PG_ASSET_LATENCY_MS=str(args.asset_latency_ms),
               SIMCTL_CHILD_PG_ASSET_IO_TRACE='1')
reports = root / 'Build/SimulatorData/EngineFiles/userdata/Documents/Rockstar Games/GTA V/Benchmarks'
old_reports = set(reports.glob('*.txt'))
launch = ['xcrun', 'simctl', 'launch', '--terminate-running-process', device,
          bundle, '--boot-engine']
if not args.no_capture_world:
    launch.append('--capture-world')
if args.frame_reports:
    launch.append('--frame-reports')
if args.balanced_asset_qos:
    launch.append('--balanced-asset-qos')
if args.stream_assets:
    launch.append('--stream-assets')
if args.serialized_asset_fills:
    launch.append('--serialized-asset-fills')
if args.reference_asset_hints:
    launch.append('--reference-asset-hints')
if args.skip_asset_warm:
    launch.append('--skip-asset-warm')
if args.reference:
    launch.append('--reference-packing')
if args.profile:
    launch.append('--profile')
if args.profile_bridge:
    launch.append('--profile-bridge')
if args.reference_texture_uploads:
    launch.append('--reference-texture-uploads')
if args.reference_bc_decoder:
    launch.append('--reference-bc-decoder')
if args.reference_texture_shadows:
    launch.append('--reference-texture-shadows')
if args.reference_fused_texture_uploads:
    launch.append('--reference-fused-texture-uploads')
if args.fast:
    launch.append('--fast-renderer')
if args.reference_passes:
    launch.append('--reference-passes')
if args.reference_prepared_draws:
    launch.append('--reference-prepared-draws')
if args.reference_proxy_snapshots:
    launch.append('--reference-proxy-snapshots')
if args.native_warm_dispatch:
    launch.append('--native-warm-dispatch')
if args.reference_warm_dispatch:
    launch.append('--reference-warm-dispatch')
if args.native_state_packing:
    launch.append('--native-state-packing')
if args.reference_state_packing:
    launch.append('--reference-state-packing')
if args.native_constant_content_cache:
    launch.append('--native-constant-content-cache')
if args.reference_constant_content_cache:
    launch.append('--reference-constant-content-cache')
if args.reference_binding_cache:
    launch.append('--reference-binding-cache')
if args.reference_metal_offsets:
    launch.append('--reference-metal-offsets')
if args.native_usage_stamps:
    launch.append('--native-usage-stamps')
if args.reference_usage_stamps:
    launch.append('--reference-usage-stamps')
if args.reference_draw_resolver:
    launch.append('--reference-draw-resolver')
if args.reference_resource_usage:
    launch.append('--reference-resource-usage')
if args.native_resource_usage:
    launch.append('--native-resource-usage')
previous = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, bundle, 'data'], text=True).strip()) / 'Documents/native-port.log'
if previous.exists():
    shutil.copyfile(previous, output / 'previous-run.log')
    previous.write_text('')
launch_wall = time.time()
subprocess.run(launch, env=env, check=True)
container = Path(subprocess.check_output(['xcrun', 'simctl', 'get_app_container', device, bundle, 'data'], text=True).strip())
(root / 'Build/simulator-container.txt').write_text(str(container))
documents = container / 'Documents'
start = time.monotonic()
last_fps = ''
last_dump = 0
status = 'timeout'
log_offset = 0
log_fragment = ''
failure_reason = None
failure_tokens = ['Native trap ', 'Native boot stopped:', 'Native game thread stopped:',
                  'Native renderer loop stopped:', 'Native renderer exception:',
                  '[wgpu] validation error:', '[wgpu] device lost:', 'readback failed:']
failure_tokens += ['Invalid native draw batch.', 'Invalid native prepared draw snapshot.', 'Invalid native warm']
while time.monotonic() - start < args.timeout:
    log_path = documents / 'native-port.log'
    log = ''
    if log_path.exists():
        if log_path.stat().st_size < log_offset:
            log_offset = 0
            log_fragment = ''
        with log_path.open('rb') as stream:
            stream.seek(log_offset)
            log = log_fragment + stream.read().decode(errors='replace')
            log_offset = stream.tell()
        lines = log.splitlines(keepends=True)
        log_fragment = lines.pop() if lines and not lines[-1].endswith('\n') else ''
        log = ''.join(lines)
    fps = [line for line in log.splitlines() if line.startswith('[wgpu] fps:')]
    if fps and fps[-1] != last_fps:
        last_fps = fps[-1]
        print(last_fps, flush=True)
    if time.monotonic() - last_dump >= 20:
        subprocess.run(['xcrun', 'simctl', 'notify_post', device, 'playgta5.metrics'], check=False, capture_output=True)
        last_dump = time.monotonic()
    failure_reason = next((token for token in failure_tokens if token in log), None)
    if failure_reason:
        status = 'failed'
        break
    if 'Game entry point returned' in log or 'Game thread exited with status 0' in log:
        status = 'finished'
        break
    completed_iterations = sum(path.name.startswith('Benchmark-') for path in set(reports.glob('*.txt')) - old_reports)
    if completed_iterations >= args.iterations:
        status = 'finished'
        break
    time.sleep(2)
subprocess.run(['xcrun', 'simctl', 'notify_post', device, 'playgta5.metrics'], check=False, capture_output=True)
if args.profile_bridge or args.profile:
    subprocess.run(['xcrun', 'simctl', 'notify_post', device, 'playgta5.texture-metrics'], check=False, capture_output=True)
    texture_trace = documents / 'native-texture-timings.csv'
    trace_deadline = time.monotonic() + 20
    while time.monotonic() < trace_deadline:
        if texture_trace.exists() and texture_trace.stat().st_mtime >= launch_wall:
            break
        time.sleep(.2)
time.sleep(2)
for filename in ['native-port.log', 'native-metrics.json', 'renderer-frames.csv', 'native-texture-timings.csv']:
    source = documents / filename
    if source.exists() and (filename != 'native-texture-timings.csv' or source.stat().st_mtime >= launch_wall):
        shutil.copyfile(source, output / filename)
if args.stream_assets:
    trace = root / 'Build/SimulatorData/native-asset-timings.csv'
    if trace.exists() and trace.stat().st_mtime >= launch_wall:
        shutil.copyfile(trace, output / trace.name)
capture = documents / 'NativeFrames/optimized-world.png'
if capture.exists() and capture.stat().st_mtime >= launch_wall:
    shutil.copyfile(capture, output / 'optimized-world.png')
new_reports = set(reports.glob('*.txt')) - old_reports
for source in new_reports:
    shutil.copyfile(source, output / source.name)
samples = []
if (output / 'renderer-frames.csv').exists():
    with (output / 'renderer-frames.csv').open() as f:
        samples = [float(row['frame_ms']) for row in csv.DictReader(f) if row['world'] == '1']
summary = {'status': status, 'mode': 'reference' if args.reference else 'compiled',
           'asset_streaming': args.stream_assets,
           'streamer_percentage': args.streamer_percentage,
           'simulated_process_memory_limit_mb': args.memory_limit_mb,
           'concurrent_asset_fills': not args.serialized_asset_fills,
           'queued_asset_hints': not args.reference_asset_hints,
           'asset_latency_ms': args.asset_latency_ms if args.stream_assets else 0,
           'asset_cache_tag': (args.cache_tag or args.name) if args.stream_assets else None,
           'startup_asset_warming': args.stream_assets and not args.skip_asset_warm,
           'scene': args.scene, 'engine_cores': args.cores,
           'requested_benchmark_iterations': args.iterations,
           'completed_benchmark_iterations': sum(p.name.startswith('Benchmark-') for p in new_reports),
           'world_frames': len(samples), 'jit': False,
           'detailed_profile': args.profile,
           'bridge_profile': args.profile_bridge or args.profile,
           'world_capture': not args.no_capture_world,
           'private_frame_reports': args.frame_reports,
           'balanced_asset_qos': args.balanced_asset_qos,
           'borrowed_texture_uploads': not args.reference_texture_uploads,
           'direct_bc_decoder': not args.reference_bc_decoder,
           'native_texture_shadow_rows': not args.reference_texture_shadows,
           'fused_bc_uploads': not args.reference_texture_uploads and not args.reference_fused_texture_uploads,
           'gpu_validation': not args.fast, 'shader_buffer_robustness': True,
           'native_pass_cache': not args.reference and not args.reference_passes,
           'native_prepared_draws': not args.reference and not args.reference_prepared_draws,
           'warm_dispatch': not args.reference_warm_dispatch,
           'fixed_state_packing': not args.reference_state_packing,
           'constant_content_cache': args.native_constant_content_cache and not args.reference_constant_content_cache,
           'prepared_proxies': not args.reference_proxy_snapshots and not args.reference and not args.reference_prepared_draws,
           'binding_cache': 'linear32' if args.reference_binding_cache else 'hash128',
           'metal_buffer_offsets': not args.reference_metal_offsets,
           'cached_usage_stamps': args.native_usage_stamps and not args.reference_usage_stamps and not args.reference and not args.reference_prepared_draws,
           'direct_draw_resolver': not args.reference_draw_resolver and not args.reference and not args.reference_prepared_draws,
           'native_resource_usage': args.native_resource_usage and not args.reference and not args.reference_prepared_draws and not args.reference_resource_usage,
           'hardware': 'Mac iOS simulator; not a phone measurement', 'frame_limit': 0}
retained_log = output / 'native-port.log'
if retained_log.exists():
    contents = retained_log.read_text(errors='replace')
    summary['queued_asset_hints'] = 'Native upcoming asset queue enabled:' in contents
    texture_modes = re.findall(r'Native texture paths: uploads (copied|borrowed); BC decoder (block-copy|direct)(?:; fused BC (on|off); native shadow rows (on|off))?', contents)
    if texture_modes:
        uploads, decoder, fused, shadows = texture_modes[-1]
        summary['borrowed_texture_uploads'] = uploads == 'borrowed'
        summary['direct_bc_decoder'] = decoder == 'direct'
        summary['fused_bc_uploads'] = fused == 'on'
        summary['native_texture_shadow_rows'] = shadows == 'on' 
    actual_modes = re.findall(r'\[wgpu\] native renderer modes: (.+)', contents)
    if actual_modes:
        modes = json.loads(actual_modes[-1])
        summary['renderer_modes'] = modes
        summary['constant_content_cache'] = modes.get('constant_content_cache', False)
        summary['warm_dispatch'] = modes.get('warm_dispatch', False)
        summary['fixed_state_packing'] = modes.get('fixed_state_packing', False)
        summary['native_pass_cache'] = modes['pass_cache']
        summary['native_prepared_draws'] = modes['prepared_draws']
        summary['native_resource_usage'] = modes['resource_usage']
        summary['direct_draw_resolver'] = modes.get('direct_resolver', summary['direct_draw_resolver'])
        summary['cached_usage_stamps'] = modes.get('cached_usage_stamps', False)
        summary['metal_buffer_offsets'] = modes.get('metal_buffer_offsets', False)
        summary['binding_cache'] = modes.get('binding_cache', 'linear32')
        summary['prepared_proxies'] = modes.get('prepared_proxies', False)
    actual_cores = re.findall(r'Native engine worker configuration: (\d+) logical cores', contents)
    if actual_cores:
        summary['engine_cores'] = int(actual_cores[-1])
    footprints = [float(value) for value in re.findall(r'Phone memory: footprint ([0-9.]+) MB', contents)]
    metal_allocations = [float(value) for value in re.findall(r'Native Metal resource allocations: ([0-9.]+) MB', contents)]
    heap_sizes = [float(value) for value in re.findall(r'Native heap grew from [0-9.]+ to ([0-9.]+) MB', contents)]
    draw_counts = re.findall(r'native draw validation: clamped \+(\d+), skipped \+(\d+), issued \+(\d+)', contents)
    prepared_counts = re.findall(r'native prepared draws: hits \+(\d+), misses \+(\d+)', contents)
    warm_counts = re.findall(r'native warm spans: (\d+), packets (\d+), draws (\d+)', contents)
    summary['diagnostics'] = dict(
        shared_vector_memory='arm64' if 'Shared vector memory uses native ARM64' in contents else 'bytewise',
        peak_sampled_footprint_mb=max(footprints, default=None),
        peak_sampled_metal_allocations_mb=max((v for v in metal_allocations if v>0), default=None),
        final_shared_heap_mb=heap_sizes[-1] if heap_sizes else 3328,
        logged_native_clamped_draws=sum(int(row[0]) for row in draw_counts),
        logged_native_skipped_draws=sum(int(row[1]) for row in draw_counts),
        logged_native_issued_draws=sum(int(row[2]) for row in draw_counts),
        logged_prepared_draw_hits=sum(int(row[0]) for row in prepared_counts),
        logged_prepared_draw_misses=sum(int(row[1]) for row in prepared_counts),
        logged_native_warm_spans=sum(int(row[0]) for row in warm_counts),
        logged_native_warm_packets=sum(int(row[1]) for row in warm_counts),
        logged_native_warm_draws=sum(int(row[2]) for row in warm_counts),
        native_memory_guard='Native streaming memory guard enabled' in contents,
        memory_budget_clamps=max([int(v) for v in re.findall(r'Native memory budget guard: ([0-9]+) clamps', contents)],default=0),
        heap_capacity_failures=contents.count('Native heap capacity exceeded:'),
        log_bytes=retained_log.stat().st_size)
    if summary['diagnostics']['heap_capacity_failures']:
        summary['performance_valid'] = False
        summary['performance_warning'] = 'The engine exhausted the shared heap; allocation retries distort performance.'
if failure_reason:
    summary['failure_reason'] = failure_reason
if samples:
    ordered = sorted(samples)
    summary.update(average_fps=1000 / statistics.mean(samples),
                   median_frame_ms=statistics.median(samples),
                   p95_frame_ms=ordered[min(len(ordered)-1, int(len(ordered)*.95))],
                   p99_frame_ms=ordered[min(len(ordered)-1, int(len(ordered)*.99))],
                   frames_over_33_33_ms=sum(t > 1000/30 for t in samples),
                   worst_frame_ms=max(samples))
benchmark_samples = []
pass_samples = {}
# Each iteration exports all prior passes again. Keep the latest copy of each
# numbered pass so multi-iteration averages do not count early scenes twice.
latest_passes = {}
for source in sorted(new_reports):
    match = re.fullmatch(r'FrameTimes-Pass(\d+)-(.+)\.txt', source.name)
    if match:
        latest_passes[int(match[1])] = source
for pass_index in sorted(latest_passes):
    source = latest_passes[pass_index]
    if source.name.startswith('FrameTimes-'):
        current_samples = []
        for line in source.read_text().splitlines()[1:]:
            fields = line.split()
            if len(fields) == 2 and fields[0].isdigit():
                current_samples.append(float(fields[1]))
        benchmark_samples.extend(current_samples)
        if current_samples:
            pass_samples[source.name] = current_samples
if benchmark_samples:
    ordered = sorted(benchmark_samples)
    summary['engine_benchmark'] = dict(frames=len(ordered), average_fps=1000/statistics.mean(ordered),
                                     median_frame_ms=statistics.median(ordered),
                                     p95_frame_ms=ordered[int(len(ordered)*.95)],
                                     p99_frame_ms=ordered[int(len(ordered)*.99)],
                                     frames_over_33_33_ms=sum(t>1000/30 for t in ordered))
    summary['engine_benchmark']['per_pass'] = {}
    for name, values in pass_samples.items():
        ordered = sorted(values)
        summary['engine_benchmark']['per_pass'][name] = dict(
            frames=len(ordered), average_fps=1000/statistics.mean(ordered),
            median_frame_ms=statistics.median(ordered),
            p95_frame_ms=ordered[int(len(ordered)*.95)],
            p99_frame_ms=ordered[int(len(ordered)*.99)],
            frames_over_33_33_ms=sum(t>1000/30 for t in ordered))
(output / 'summary.json').write_text(json.dumps(summary, indent=2))
print(json.dumps(summary, indent=2), flush=True)
if status != 'finished':
    subprocess.run(['xcrun', 'simctl', 'terminate', device, bundle], check=False, capture_output=True)
    raise SystemExit(1)
