import Foundation

/// A folder on this iPhone (Files, "On My iPhone") that holds your own copy of the
/// game: the folder that contains data/ (or data/ itself). The app keeps access to
/// it between launches with a bookmark and reads the files directly while you play,
/// so no server is needed.
enum GameFolder {
    private static let bookmarkKey = "gameFolderBookmark"
    private static let nameKey = "gameFolderName"
    private static var accessing: URL?

    static var isChosen: Bool { UserDefaults.standard.data(forKey: bookmarkKey) != nil }
    static var name: String { UserDefaults.standard.string(forKey: nameKey) ?? "folder" }

    static func clear() {
        UserDefaults.standard.removeObject(forKey: bookmarkKey)
        UserDefaults.standard.removeObject(forKey: nameKey)
    }

    /// Checks a folder picked in Files and remembers it. Returns a message if it is not usable.
    static func choose(_ picked: URL) -> String? {
        // Folders picked in Files need security-scoped access; plain paths (tests) don't.
        let scoped = picked.startAccessingSecurityScopedResource()
        defer { if scoped { picked.stopAccessingSecurityScopedResource() } }
        guard FileManager.default.isReadableFile(atPath: picked.path) else { return "The app was not allowed to open that folder." }
        guard let data = dataFolder(in: picked) else {
            return "That folder doesn't contain the game's data folder. Pick the folder that contains data (and b), or the data folder itself."
        }
        if let problem = check(data) { return problem }
        do {
            let bookmark = try picked.bookmarkData(options: [], includingResourceValuesForKeys: nil, relativeTo: nil)
            UserDefaults.standard.set(bookmark, forKey: bookmarkKey)
            UserDefaults.standard.set(picked.lastPathComponent, forKey: nameKey)
            return nil
        } catch {
            return "The app could not remember that folder: \(error.localizedDescription)"
        }
    }

    /// The chosen folder's data/ path, with access granted for the rest of this launch.
    static func open() -> URL? {
        guard let bookmark = UserDefaults.standard.data(forKey: bookmarkKey) else { return nil }
        var stale = false
        guard let root = try? URL(resolvingBookmarkData: bookmark, options: [], relativeTo: nil, bookmarkDataIsStale: &stale) else { return nil }
        if root.startAccessingSecurityScopedResource() { accessing = root }
        guard FileManager.default.isReadableFile(atPath: root.path) else { return nil }
        if stale, let fresh = try? root.bookmarkData(options: [], includingResourceValuesForKeys: nil, relativeTo: nil) {
            UserDefaults.standard.set(fresh, forKey: bookmarkKey)
        }
        return dataFolder(in: root)
    }

    private static func dataFolder(in folder: URL) -> URL? {
        let manager = FileManager.default
        for candidate in [folder.appendingPathComponent("data"), folder] {
            var isDirectory: ObjCBool = false
            let probe = candidate.appendingPathComponent("x64")
            if manager.fileExists(atPath: probe.path, isDirectory: &isDirectory), isDirectory.boolValue { return candidate }
            if manager.fileExists(atPath: candidate.appendingPathComponent("manifest.json").path) { return candidate }
        }
        return nil
    }

    /// Compares a sample of files (the first ones, the biggest ones) with the game's file list.
    private static func check(_ data: URL) -> String? {
        guard let path = EnginePaths.manifest,
              let json = try? JSONSerialization.jsonObject(with: Data(contentsOf: URL(fileURLWithPath: path))) as? [String: Any],
              let files = json["files"] as? [[Any]] else { return "The app's game file list is missing." }
        let entries = files.compactMap { entry -> (String, UInt64)? in
            guard entry.count >= 2, let name = entry[0] as? String, let size = (entry[1] as? NSNumber)?.uint64Value else { return nil }
            return (name, size)
        }
        let sample = Array(entries.prefix(5)) + Array(entries.sorted { $0.1 > $1.1 }.prefix(5))
        for (name, size) in sample {
            let file = data.appendingPathComponent(name)
            if !FileManager.default.fileExists(atPath: file.path) {
                let cloud = file.deletingLastPathComponent().appendingPathComponent("." + file.lastPathComponent + ".icloud")
                if FileManager.default.fileExists(atPath: cloud.path) {
                    return "Some game files are only in iCloud. Move the game folder to On My iPhone and try again."
                }
                return "The folder is missing game files (for example \(name)). Is the download complete?"
            }
            let actual = (try? FileManager.default.attributesOfItem(atPath: file.path)[.size] as? NSNumber)?.uint64Value ?? 0
            if actual != size { return "\(name) has a different size than the game expects. The folder may be incomplete or from another version." }
        }
        return nil
    }
}
