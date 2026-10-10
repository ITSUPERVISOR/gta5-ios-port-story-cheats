import Foundation
import Metal
import UIKit
import os

/// Graphics presets. The engine reads quality settings once at boot; only the
/// resolution can change while it runs.
enum GraphicsPreset: Int, CaseIterable {
    case battery, performance, balanced, quality, ultra

    var title: String { ["Battery", "Performance", "Balanced", "Quality", "Ultra"][rawValue] }
    /// Render height in pixels; width follows the screen's shape.
    var height: Int { [432, 540, 720, 900, 1080][rawValue] }
    var detail: String {
        ["Lowest load, longest play time", "Today's tested setup for 6 GB iPhones", "Sharper image and shadows",
         "Higher detail textures, effects and draw distance", "Highest settings this port supports"][rawValue]
    }

    /// Engine command-line options. Shader quality never goes below 1 (the
    /// low shader set is mostly missing), and tessellation, MSAA and depth of
    /// field stay off because their shaders do not run on Metal here.
    var engineOptions: [String] {
        switch self {
        case .battery, .performance:
            return ["-shaderQuality=1", "-textureQuality=0", "-shadowQuality=-1", "-reflectionQuality=0", "-particleQuality=0",
                    "-grassQuality=0", "-cityDensity=0", "-lodScale=0", "-pedLodBias=0", "-vehicleLodBias=0", "-pedVariety=0", "-vehicleVariety=0"]
        case .balanced:
            return ["-shaderQuality=1", "-textureQuality=0", "-shadowQuality=0", "-reflectionQuality=0", "-particleQuality=0",
                    "-grassQuality=0", "-cityDensity=0.2", "-lodScale=0.2", "-pedLodBias=0", "-vehicleLodBias=0", "-pedVariety=0.2", "-vehicleVariety=0.2"]
        case .quality:
            return ["-shaderQuality=1", "-textureQuality=1", "-shadowQuality=0", "-reflectionQuality=1", "-particleQuality=1",
                    "-grassQuality=1", "-cityDensity=0.4", "-lodScale=0.4", "-pedLodBias=0.2", "-vehicleLodBias=0.2", "-pedVariety=0.4", "-vehicleVariety=0.4"]
        case .ultra:
            return ["-shaderQuality=2", "-textureQuality=1", "-shadowQuality=1", "-reflectionQuality=2", "-particleQuality=1",
                    "-grassQuality=1", "-cityDensity=0.6", "-lodScale=0.6", "-pedLodBias=0.4", "-vehicleLodBias=0.4", "-pedVariety=0.6", "-vehicleVariety=0.6"]
        }
    }
}

/// What this device can afford, measured before the engine starts.
struct DeviceTier {
    let memoryLimitMB: Int
    let physicalMemoryMB: Int
    let model: String
    let supportsBCTextures: Bool

    static func probe() -> DeviceTier {
        var info = utsname(); uname(&info)
        let model = withUnsafeBytes(of: &info.machine) { String(decoding: $0.prefix { $0 != 0 }, as: UTF8.self) }
        let device = MTLCreateSystemDefaultDevice()
        return DeviceTier(memoryLimitMB: Self.processMemoryLimitMB(),
                          physicalMemoryMB: Int(ProcessInfo.processInfo.physicalMemory / 1_048_576),
                          model: model,
                          supportsBCTextures: device?.supportsBCTextureCompression ?? false)
    }

    /// Jetsam limit = memory still available + what the app already uses.
    private static func processMemoryLimitMB() -> Int {
        #if targetEnvironment(simulator)
        // Model the target phone (iPhone 13 Pro Max) unless told otherwise.
        return Int(ProcessInfo.processInfo.environment["PG_SIM_MEMORY_LIMIT_MB"] ?? "") ?? 4096
        #else
        var vm = task_vm_info_data_t()
        var count = mach_msg_type_number_t(MemoryLayout<task_vm_info_data_t>.size / MemoryLayout<integer_t>.size)
        let result = withUnsafeMutablePointer(to: &vm) {
            $0.withMemoryRebound(to: integer_t.self, capacity: Int(count)) { task_info(mach_task_self_, task_flavor_t(TASK_VM_INFO), $0, &count) }
        }
        let footprint = result == KERN_SUCCESS ? Int(vm.phys_footprint) : 0
        return (Int(os_proc_available_memory()) + footprint) / 1_048_576
        #endif
    }

    /// 6 GB phones (about 4.1 GB limit) are already at the edge on Performance.
    var recommended: GraphicsPreset {
        // Under ~3.6 GB (e.g. a 6 GB phone signed with a free Apple ID, which
        // cannot get the extra-memory entitlement) only the lightest preset fits.
        memoryLimitMB < 3600 ? .battery : memoryLimitMB < 5000 ? .performance : memoryLimitMB < 7000 ? .balanced : .quality
    }
    /// Presets that are allowed at all; higher ones are hidden.
    var allowed: [GraphicsPreset] {
        if memoryLimitMB < 3600 { return [.battery, .performance] }
        if memoryLimitMB < 5000 { return [.battery, .performance, .balanced] }
        if memoryLimitMB < 7000 { return [.battery, .performance, .balanced, .quality] }
        return GraphicsPreset.allCases
    }
    /// Presets shown with a warning because they may run out of memory or heat up.
    func risky(_ preset: GraphicsPreset) -> Bool { preset.rawValue > recommended.rawValue }
    var summary: String { "\(model) · \(physicalMemoryMB / 1024) GB · app limit \(memoryLimitMB) MB\(supportsBCTextures ? " · BC textures" : "")" }
}

enum GraphicsConfiguration {
    /// Width/height for a preset and a 0.5...1.0 resolution scale, matching
    /// the screen's shape so the picture is not stretched.
    static func size(for preset: GraphicsPreset, scale: Double, screen: CGSize, matchScreenShape: Bool) -> (Int, Int) {
        let aspect = matchScreenShape && screen.height > 0 ? max(screen.width, screen.height) / min(screen.width, screen.height) : 16.0 / 9.0
        var height = Double(preset.height) * min(max(scale, 0.5), 1.0)
        height = max(height, 360)
        let width = min(height * aspect, 3840)
        func even(_ value: Double) -> Int { Int(value.rounded()) & ~1 }
        return (even(width), even(height))
    }

    /// Pass the chosen settings to the engine. Call before PGStartEngine.
    static func apply(_ settings: GameSettings, screen: CGSize) -> String {
        let preset = settings.graphicsPreset
        let (width, height) = size(for: preset, scale: settings.resolutionScale, screen: screen, matchScreenShape: settings.matchScreenShape)
        let options = preset.engineOptions
        var pointers = options.map { strdup($0) }
        defer { pointers.forEach { free($0) } }
        pointers.withUnsafeMutableBufferPointer { buffer in
            buffer.baseAddress!.withMemoryRebound(to: UnsafePointer<CChar>?.self, capacity: buffer.count) {
                PGConfigureEngine($0, UInt32(buffer.count), Int32(width), Int32(height), settings.sixtyFPS ? 1 : 2)
            }
        }
        return "\(preset.title) \(width)×\(height) at \(settings.sixtyFPS ? 60 : 30) FPS"
    }
}
