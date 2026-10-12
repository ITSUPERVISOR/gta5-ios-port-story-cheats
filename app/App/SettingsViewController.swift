import UIKit

/// Graphics and control options. Before launch every change applies on Play;
/// during play only resolution and controls apply live, the rest on restart.
final class SettingsViewController: UITableViewController {
    private let settings = GameSettings.shared
    private let engineRunning: Bool
    private let bootPreset: GraphicsPreset
    /// The mode the running engine started in (nil before Play).
    private let runningMode: GameMode?
    var resolutionChanged: (() -> Void)?
    var controlsChanged: (() -> Void)?
    var editLayout: (() -> Void)?
    var targetingChanged: (() -> Void)?
    var gameModeChanged: (() -> Void)?

    private enum Section: Int, CaseIterable { case language, mode, graphics, controls, gyro, files, device }
    private let preloader = AssetPreloader.shared
    /// Rows shown in the Game files section, rebuilt from the preloader's phase.
    private enum FileRow { case status, start, pause, resume, cancel, remove }
    private var fileRows: [FileRow] = []
    private var fileStateKey = ""
    private var fileMessage: String?

    init(engineRunning: Bool, runningMode: GameMode? = nil) {
        self.engineRunning = engineRunning
        self.runningMode = engineRunning ? runningMode : nil
        self.bootPreset = GameSettings.shared.graphicsPreset
        super.init(style: .insetGrouped)
        title = AppLanguage.choose("Settings", "設定")
        modalPresentationStyle = .formSheet
        overrideUserInterfaceStyle = .dark
    }
    required init?(coder: NSCoder) { fatalError("init(coder:) has not been implemented") }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.rightBarButtonItem = UIBarButtonItem(title: AppLanguage.choose("Done", "完成"), primaryAction: UIAction { [weak self] _ in self?.dismiss(animated: true) })
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "cell")
        fileRows = makeFileRows()
        NotificationCenter.default.addObserver(self, selector: #selector(preloadChanged), name: .assetPreloadChanged, object: nil)
        preloader.refresh()
        let arguments = ProcessInfo.processInfo.arguments
        if let i = arguments.firstIndex(of: "--settings-section"), i + 1 < arguments.count,
           let section = ["files": Section.files, "graphics": Section.graphics, "resolution": Section.graphics][arguments[i + 1]] {
            let row = arguments[i + 1] == "resolution" ? settings.tier.allowed.count : 0
            DispatchQueue.main.asyncAfter(deadline: .now() + 0.3) {
                self.tableView.scrollToRow(at: IndexPath(row: row, section: section.rawValue), at: .middle, animated: false)
            }
        }
    }

    override func numberOfSections(in tableView: UITableView) -> Int { Section.allCases.count }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch Section(rawValue: section)! {
        case .mode: return GameMode.allCases.count
        case .graphics: return settings.tier.allowed.count + 3
        case .controls: return 8
        case .gyro: return (settings.gyroAim == .off ? 1 : 3) + (settings.tiltSteer == .off ? 1 : 2)
        case .files: return fileRows.count
        case .device: return 3
        case .language: return 1
        }
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        [AppLanguage.choose("Language", "語言"), AppLanguage.choose("Game mode", "遊戲模式"), AppLanguage.choose("Graphics", "畫質"), AppLanguage.choose("Touch controls", "觸控按鍵"), AppLanguage.choose("Gyro", "陀螺儀"), AppLanguage.choose("Game files", "遊戲資料"), AppLanguage.choose("This device", "此裝置")][section]
    }

    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        switch Section(rawValue: section)! {
        case .mode:
            var text = AppLanguage.choose("Story mode saves stay on this phone between launches. Sandbox keeps its own separate files.", "劇情存檔會保留在這支手機；沙盒模式使用獨立的存檔。")
            if let runningMode {
                text = AppLanguage.choose("Changes apply the next time you start the game. ", "變更會在下次啟動遊戲時生效。") + text
                if settings.gameMode != runningMode { text += AppLanguage.choose(" Close and reopen the app to play \(settings.gameMode.title).", " 關閉再重新開啟 App，即可進入 \(settings.gameMode.title)。") }
            }
            return text
        case .graphics:
            var text = AppLanguage.choose("Resolution changes apply right away. ", "解析度變更會立即生效。")
            text += engineRunning ? AppLanguage.choose("Preset and frame rate changes apply the next time you start the game.", "畫質預設與幀率會在下次啟動遊戲時生效。") : AppLanguage.choose("Presets apply when you press Play.", "按下「開始遊戲」後套用畫質預設。")
            if settings.graphicsPreset != bootPreset && engineRunning { text += AppLanguage.choose(" Restart needed for \(settings.graphicsPreset.title).", " 必須重新啟動才能套用「\(settings.graphicsPreset.title)」。") }
            if settings.tier.risky(settings.graphicsPreset) { text += AppLanguage.choose(" Presets above the recommended one can run out of memory or overheat on this phone; if a launch fails, the game falls back automatically.", " 高於建議的畫質可能導致記憶體不足或裝置過熱；若無法啟動，將自動降低設定。") }
            return text
        case .controls:
            return AppLanguage.choose("Controls act as a game controller, so the game's own aim assist and analog steering work. Assisted aim locks onto targets like on a console.", "觸控按鍵會模擬遊戲控制器，因此能使用遊戲原有的輔助瞄準和類比轉向。")
        case .gyro:
            return AppLanguage.choose("Move the phone to fine-tune your aim; turning your body turns the camera. Tilt steering turns the phone like a steering wheel: Assist adds fine control on top of the touch steering, Tilt to steer replaces it. The straight-ahead angle is however you hold the phone when you get in.", "移動手機可微調瞄準。傾斜轉向能把手機當方向盤：「輔助」會配合觸控轉向；「傾斜控制」則改用手機傾斜操作。上車時的持機角度會設定為正前方。")
        case .files: return filesFooter()
        case .device: return nil
        case .language:
            return AppLanguage.choose("Menu language changes now. In-game language is only requested at next launch and may remain English if the WebAssembly game does not support it.", "App 介面語言立即切換；遊戲內語言會在下次啟動時嘗試套用，但網頁遊戲引擎仍可能顯示英文。")
        }
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        cell.selectionStyle = .default
        var content = cell.defaultContentConfiguration()
        content.secondaryTextProperties.color = .secondaryLabel
        let presets = settings.tier.allowed
        switch Section(rawValue: indexPath.section)! {
        case .mode:
            let mode = GameMode.allCases[indexPath.row]
            content.text = mode.title + (mode == runningMode ? AppLanguage.choose(" (playing now)", "（目前遊玩中）") : "")
            content.secondaryText = mode.detail
            cell.accessoryType = settings.gameModeChoice == mode ? .checkmark : .none
        case .graphics:
            if indexPath.row < presets.count {
                let preset = presets[indexPath.row]
                content.text = preset.title + (preset == settings.tier.recommended ? AppLanguage.choose(" (recommended)", "（建議）") : "")
                content.secondaryText = AppLanguage.choose("Detail level, default \(preset.height)p · ", "畫質等級，預設 \(preset.height)p · ") + preset.detail + (settings.tier.risky(preset) ? AppLanguage.choose("\n⚠︎ Heavier than this phone's recommended setting: may run out of memory or overheat.", "\n⚠︎ 高於此裝置的建議設定，可能記憶體不足或過熱。") : "")
                if settings.tier.risky(preset) { content.secondaryTextProperties.color = .systemOrange }
                cell.accessoryType = preset == settings.graphicsPreset ? .checkmark : .none
            } else {
                let row = indexPath.row - presets.count
                cell.selectionStyle = .none
                if row == 0 {
                    content.text = AppLanguage.choose("Resolution", "解析度")
                    content.secondaryText = currentSizeText() + AppLanguage.choose(" · changes right away", " · 立即生效")
                    let button = UIButton(type: .system)
                    button.setTitle(resolutionTitle(settings.renderHeight) + " ▾", for: .normal)
                    button.showsMenuAsPrimaryAction = true
                    button.accessibilityLabel = AppLanguage.choose("Resolution", "解析度")
                    let choices = [0] + GraphicsConfiguration.resolutionChoices
                    button.menu = UIMenu(children: choices.map { height in
                        UIAction(title: resolutionTitle(height), state: height == settings.renderHeight ? .on : .off) { [weak self] _ in
                            guard let self else { return }
                            self.settings.renderHeight = height
                            self.resolutionChanged?()
                            self.reloadFooters()
                        }
                    })
                    button.sizeToFit()
                    cell.accessoryView = button
                } else if row == 1 {
                    content.text = AppLanguage.choose("Fill the screen", "填滿螢幕")
                    content.secondaryText = AppLanguage.choose("Render at the screen's shape instead of stretching 16:9", "依照螢幕比例渲染，不再拉伸 16:9 畫面")
                    cell.accessoryView = toggle(settings.matchScreenShape, label: AppLanguage.choose("Fill the screen", "填滿螢幕")) { [weak self] in
                        self?.settings.matchScreenShape = $0; self?.resolutionChanged?(); self?.tableView.reloadData()
                    }
                } else {
                    content.text = AppLanguage.choose("Frame rate", "幀率")
                    content.secondaryText = AppLanguage.choose("Higher uses more power and heat; 30 FPS is steadiest. Unlocked goes up to 120 on ProMotion iPhones.", "幀率愈高愈耗電、愈容易發熱；30 FPS 最穩定。ProMotion iPhone 解除限制後最高可達 120 FPS。")
                    let button = UIButton(type: .system)
                    button.setTitle(GameSettings.frameRateTitle(settings.frameRateCap) + " ▾", for: .normal)
                    button.showsMenuAsPrimaryAction = true
                    button.accessibilityLabel = AppLanguage.choose("Frame rate", "幀率")
                    button.menu = UIMenu(children: [30, 60, 0].map { cap in
                        UIAction(title: GameSettings.frameRateTitle(cap), state: cap == settings.frameRateCap ? .on : .off) { [weak self] _ in
                            self?.settings.frameRateCap = cap; self?.reloadFooters(); self?.tableView.reloadData()
                        }
                    })
                    button.sizeToFit()
                    cell.accessoryView = button
                }
            }
        case .controls:
            cell.selectionStyle = .none
            switch indexPath.row {
            case 0:
                cell.accessoryView = slider(settings.lookSensitivity, 0.3, 3, label: AppLanguage.choose("Look speed", "鏡頭速度")) { [weak self] in self?.settings.lookSensitivity = $0 }
                content.text = AppLanguage.choose("Camera stick speed", "鏡頭搖桿速度")
            case 5:
                cell.accessoryView = slider(settings.aimSensitivity, 0.3, 4, label: AppLanguage.choose("Aim look speed", "瞄準鏡頭速度")) { [weak self] in self?.settings.aimSensitivity = $0 }
                content.text = AppLanguage.choose("Aim stick speed", "瞄準搖桿速度")
            case 7:
                content.text = AppLanguage.choose("Edit controls layout", "編輯按鍵配置")
                content.secondaryText = ControlsLayout.isCustomized ? AppLanguage.choose("Customized", "已自訂") : AppLanguage.choose("Move, resize or hide buttons", "移動、縮放或隱藏按鍵")
                content.textProperties.color = .systemBlue
            case 1:
                cell.accessoryView = slider(settings.controlScale, 0.7, 1.5, label: AppLanguage.choose("Button size", "按鍵大小")) { [weak self] in self?.settings.controlScale = $0; self?.controlsChanged?() }
                content.text = AppLanguage.choose("Button size", "按鍵大小")
            case 2:
                cell.accessoryView = slider(settings.controlOpacity, 0.2, 1, label: AppLanguage.choose("Button opacity", "按鍵透明度")) { [weak self] in self?.settings.controlOpacity = $0; self?.controlsChanged?() }
                content.text = AppLanguage.choose("Button opacity", "按鍵透明度")
            case 3:
                content.text = AppLanguage.choose("Targeting", "瞄準方式")
                let button = UIButton(configuration: .gray())
                button.configuration?.title = GameSettings.targetingTitles[settings.targetingMode]
                button.menu = UIMenu(children: GameSettings.targetingTitles.enumerated().map { index, title in
                    UIAction(title: title, state: index == settings.targetingMode ? .on : .off) { [weak self] _ in
                        self?.settings.targetingMode = index
                        self?.targetingChanged?()
                        self?.tableView.reloadSections([Section.controls.rawValue], with: .none)
                    }
                })
                button.showsMenuAsPrimaryAction = true
                button.sizeToFit()
                cell.accessoryView = button
            case 6:
                content.text = AppLanguage.choose("Hide when a controller is connected", "連接控制器時隱藏觸控按鍵")
                content.secondaryText = AppLanguage.choose("Touch controls come back as soon as the controller disconnects", "控制器斷線後會自動恢復觸控按鍵")
                cell.accessoryView = toggle(settings.hideTouchWithController, label: AppLanguage.choose("Hide touch controls with a controller", "控制器連線時隱藏觸控按鍵")) { [weak self] in self?.settings.hideTouchWithController = $0 }
            default:
                content.text = AppLanguage.choose("Fire also aims", "開火時自動瞄準")
                content.secondaryText = AppLanguage.choose("Fire locks on first, like the classic mobile GTA games", "開火時先鎖定目標，操作方式類似經典手機版 GTA")
                cell.accessoryView = toggle(settings.fireAlsoAims, label: AppLanguage.choose("Fire also aims", "開火時自動瞄準")) { [weak self] in self?.settings.fireAlsoAims = $0 }
            }
        case .gyro:
            let aimRows = settings.gyroAim == .off ? 1 : 3
            if indexPath.row == aimRows {
                content.text = AppLanguage.choose("Tilt steering", "傾斜轉向")
                let button = UIButton(configuration: .gray())
                button.configuration?.title = settings.tiltSteer.title
                button.menu = UIMenu(children: TiltSteerMode.allCases.map { mode in
                    UIAction(title: mode.title, state: mode == settings.tiltSteer ? .on : .off) { [weak self] _ in
                        self?.settings.tiltSteer = mode; self?.tableView.reloadSections([Section.gyro.rawValue], with: .automatic)
                    }
                })
                button.showsMenuAsPrimaryAction = true
                button.sizeToFit()
                cell.accessoryView = button
                cell.selectionStyle = .none
            } else if indexPath.row == aimRows + 1 {
                content.text = AppLanguage.choose("Full lock at \(Int(settings.tiltFullLock))°", "轉向到底角度：\(Int(settings.tiltFullLock))°")
                cell.accessoryView = slider(settings.tiltFullLock, 10, 45, label: AppLanguage.choose("Tilt for full steering", "完全轉向所需傾斜角度")) { [weak self] in self?.settings.tiltFullLock = $0.rounded() }
                cell.selectionStyle = .none
            } else if indexPath.row == 0 {
                content.text = AppLanguage.choose("Gyro", "陀螺儀")
                content.secondaryText = settings.gyroAim.title
                let menu = UIMenu(children: GyroAimMode.allCases.map { mode in
                    UIAction(title: mode.title, state: mode == settings.gyroAim ? .on : .off) { [weak self] _ in
                        self?.settings.gyroAim = mode; self?.tableView.reloadSections([Section.gyro.rawValue], with: .automatic)
                    }
                })
                let button = UIButton(configuration: .gray())
                button.configuration?.title = settings.gyroAim.title
                button.menu = menu
                button.showsMenuAsPrimaryAction = true
                button.sizeToFit()
                cell.accessoryView = button
                cell.selectionStyle = .none
            } else if indexPath.row == 1 {
                content.text = String(format: AppLanguage.choose("Sensitivity %.1f×", "靈敏度 %.1f×"), settings.gyroSensitivity)
                let slider = UISlider()
                slider.minimumValue = 0.2; slider.maximumValue = 3
                slider.value = Float(settings.gyroSensitivity)
                slider.frame.size.width = 150
                slider.accessibilityLabel = AppLanguage.choose("Gyro sensitivity", "陀螺儀靈敏度")
                slider.addAction(UIAction { [weak self, weak slider] _ in
                    guard let self, let slider else { return }
                    self.settings.gyroSensitivity = (Double(slider.value) * 10).rounded() / 10
                    var updated = cell.defaultContentConfiguration()
                    updated.text = String(format: AppLanguage.choose("Sensitivity %.1f×", "靈敏度 %.1f×"), self.settings.gyroSensitivity)
                    cell.contentConfiguration = updated
                }, for: .valueChanged)
                cell.accessoryView = slider
                cell.selectionStyle = .none
            } else {
                content.text = AppLanguage.choose("Invert vertical", "反轉上下方向")
                cell.accessoryView = toggle(settings.gyroInvertY, label: AppLanguage.choose("Invert vertical gyro", "反轉陀螺儀垂直方向")) { [weak self] in self?.settings.gyroInvertY = $0 }
                cell.selectionStyle = .none
            }
        case .files:
            configureFileCell(cell, &content, fileRows[indexPath.row])
        case .language:
            cell.selectionStyle = .none
            content.text = AppLanguage.choose("App language", "應用程式語言")
            content.secondaryText = AppLanguage.choose("Language for menus and settings", "主選單、設定及按鍵介面語言")
            let button = UIButton(type: .system)
            button.setTitle(AppLanguage.current.name + " ▾", for: .normal)
            button.accessibilityLabel = AppLanguage.choose("App language", "應用程式語言")
            button.showsMenuAsPrimaryAction = true
            button.menu = UIMenu(children: AppLanguage.allCases.map { language in
                UIAction(title: language.name, state: language == AppLanguage.current ? .on : .off) { [weak self] _ in
                    AppLanguage.select(language)
                    self?.title = AppLanguage.choose("Settings", "設定")
                    self?.navigationItem.rightBarButtonItem?.title = AppLanguage.choose("Done", "完成")
                    self?.tableView.reloadData()
                }
            })
            button.sizeToFit()
            cell.accessoryView = button
        case .device:
            if indexPath.row == 0 {
                content.text = settings.tier.summary
                content.textProperties.font = .preferredFont(forTextStyle: .footnote)
                content.textProperties.color = .secondaryLabel
                cell.selectionStyle = .none
            } else if indexPath.row == 1 {
                content.text = AppLanguage.choose("Send log to the developer", "傳送紀錄給開發者")
                content.secondaryText = AppLanguage.choose("One tap. Helps fix crashes and loading problems", "回報閃退與載入問題，協助開發者排查")
                content.textProperties.color = .systemBlue
            } else {
                content.text = AppLanguage.choose("Share log", "分享紀錄")
                content.secondaryText = AppLanguage.choose("Save or send this run's log and the previous one yourself", "儲存或分享本次及前次執行紀錄")
                content.textProperties.color = .systemBlue
            }
        }
        cell.contentConfiguration = content
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        if indexPath.section == Section.device.rawValue, indexPath.row == 1 { sendLog(); return }
        if indexPath.section == Section.device.rawValue, indexPath.row == 2 { shareLogs(from: tableView.cellForRow(at: indexPath)); return }
        if indexPath.section == Section.controls.rawValue, indexPath.row == 7 { editLayout?(); return }
        if indexPath.section == Section.mode.rawValue {
            settings.gameMode = GameMode.allCases[indexPath.row]
            gameModeChanged?()
            tableView.reloadSections([Section.mode.rawValue], with: .none)
            return
        }
        if indexPath.section == Section.files.rawValue { selectFileRow(fileRows[indexPath.row]); return }
        let presets = settings.tier.allowed
        guard indexPath.section == Section.graphics.rawValue, indexPath.row < presets.count else { return }
        settings.graphicsPreset = presets[indexPath.row]
        resolutionChanged?()
        tableView.reloadSections([Section.graphics.rawValue], with: .none)
    }

    // MARK: Game files

    private func makeFileRows() -> [FileRow] {
        let phase = preloader.phase
        let hasFiles = preloader.cachedBytes > 0
        switch phase {
        case .unavailable: return [.status]
        case .idle: return hasFiles ? [.start, .remove] : [.start]
        case .downloading: return [.status, .pause, .cancel]
        case .paused: return [.status, .resume, .cancel] + (hasFiles ? [.remove] : [])
        case .complete: return [.status, .remove]
        }
    }

    @objc private func preloadChanged() {
        let rows = makeFileRows()
        let section = Section.files.rawValue
        // Footer text depends on these; a footer only resizes on a reload.
        let key = "\(rows)|\(preloader.phase)|\(preloader.removing)|\(preloader.lastError ?? "")|\(fileMessage ?? "")|\(preloader.measured)|\(preloader.cachedBytes > 0)"
        if rows != fileRows || key != fileStateKey {
            fileRows = rows
            fileStateKey = key
            UIView.performWithoutAnimation { tableView.reloadSections([section], with: .none) }
            return
        }
        // Same rows: refresh the visible text in place (no flicker).
        for (row, kind) in rows.enumerated() {
            guard let cell = tableView.cellForRow(at: IndexPath(row: row, section: section)) else { continue }
            var content = cell.defaultContentConfiguration()
            content.secondaryTextProperties.color = .secondaryLabel
            configureFileCell(cell, &content, kind)
            cell.contentConfiguration = content
        }
    }

    private func progressText() -> String {
        let percent = Int((preloader.fraction * 100).rounded(.down))
        var text = AppLanguage.choose("\(percent)% · \(AssetPreloader.format(preloader.cachedBytes)) of \(AssetPreloader.format(preloader.totalBytes))", "\(percent)% · \(AssetPreloader.format(preloader.cachedBytes)) / \(AssetPreloader.format(preloader.totalBytes))")
        if preloader.phase == .downloading {
            if let rate = preloader.bytesPerSecond { text += " · \(AssetPreloader.format(UInt64(rate)))/s" }
            if let seconds = preloader.secondsRemaining { text += " · " + AssetPreloader.formatDuration(seconds) }
        }
        return text
    }

    private func configureFileCell(_ cell: UITableViewCell, _ content: inout UIListContentConfiguration, _ row: FileRow) {
        let total = preloader.measured ? AssetPreloader.format(preloader.totalBytes) : AppLanguage.choose("Checking size", "正在檢查大小")
        switch row {
        case .status:
            cell.selectionStyle = .none
            switch preloader.phase {
            case .unavailable(let reason):
                content.text = AppLanguage.choose("Download all game files", "下載完整遊戲資料")
                content.secondaryText = reason
                content.textProperties.color = .secondaryLabel
            case .complete:
                content.text = AppLanguage.choose("All files on device", "遊戲資料已完整下載")
                content.secondaryText = AppLanguage.choose("\(total) · plays without streaming", "\(total) · 無須串流即可遊玩")
                content.image = UIImage(systemName: "checkmark.circle.fill")
                content.imageProperties.tintColor = .systemGreen
            case .downloading:
                content.text = AppLanguage.choose("Downloading game files", "正在下載遊戲資料")
                content.secondaryText = progressText()
                cell.accessoryView = progressView()
            default:
                content.text = preloader.lastError == nil ? AppLanguage.choose("Download paused", "下載已暫停") : AppLanguage.choose("Download stopped", "下載已停止")
                content.secondaryText = progressText()
                cell.accessoryView = progressView()
            }
        case .start:
            content.text = AppLanguage.choose("Download all game files", "下載完整遊戲資料")
            content.textProperties.color = .tintColor
            if preloader.cachedBytes > 0 && preloader.measured {
                content.secondaryText = AppLanguage.choose("\(total) · \(AssetPreloader.format(preloader.cachedBytes)) already on this iPhone", "總共 \(total) · 已下載 \(AssetPreloader.format(preloader.cachedBytes))")
            } else {
                content.secondaryText = total
            }
            content.image = UIImage(systemName: "arrow.down.circle")
        case .pause:
            content.text = AppLanguage.choose("Pause", "暫停")
            content.textProperties.color = .tintColor
            content.image = UIImage(systemName: "pause.circle")
        case .resume:
            content.text = AppLanguage.choose("Resume download", "繼續下載")
            content.textProperties.color = .tintColor
            content.image = UIImage(systemName: "arrow.down.circle")
        case .cancel:
            content.text = AppLanguage.choose("Cancel download", "取消下載")
            content.secondaryText = AppLanguage.choose("Keeps what is already downloaded", "已下載的資料會保留")
            content.textProperties.color = .systemRed
            content.image = UIImage(systemName: "xmark.circle")
            content.imageProperties.tintColor = .systemRed
        case .remove:
            content.text = preloader.removing ? AppLanguage.choose("Removing files…", "正在刪除檔案…") : AppLanguage.choose("Remove downloaded files", "刪除已下載的檔案")
            content.secondaryText = engineRunning ? AppLanguage.choose("Available after restarting the app", "重啟 App 後才能使用") : AppLanguage.choose("Frees \(AssetPreloader.format(preloader.cachedBytes))", "可釋放 \(AssetPreloader.format(preloader.cachedBytes))")
            content.textProperties.color = engineRunning ? .secondaryLabel : .systemRed
            content.image = UIImage(systemName: "trash")
            content.imageProperties.tintColor = engineRunning ? .secondaryLabel : .systemRed
            cell.selectionStyle = engineRunning ? .none : .default
        }
    }

    private func progressView() -> UIProgressView {
        let bar = UIProgressView(progressViewStyle: .default)
        bar.frame.size.width = 110
        bar.progress = Float(preloader.fraction)
        bar.accessibilityLabel = AppLanguage.choose("Download progress", "下載進度")
        return bar
    }

    private func filesFooter() -> String {
        var text = ""
        if let message = fileMessage ?? preloader.lastError { text = message + "\n\n" }
        switch preloader.phase {
        case .unavailable: return text
        case .complete:
            text += AppLanguage.choose("The game reads everything from this iPhone. Removing the files frees the space; the game then streams again.", "遊戲會直接讀取手機上的資料；刪除後可騰出空間，但之後必須重新使用串流。")
        default:
            text += "Saves the whole game on this iPhone (\(preloader.measured ? AssetPreloader.format(preloader.totalBytes) : "about 21 GB")) so it no longer streams from the server. Tailscale must be connected while it downloads. Keep the app open: in the background the download pauses after a short while and continues when you come back. You can play while it downloads."
            if preloader.cachedBytes > 0 && preloader.phase != .downloading {
                text += AppLanguage.choose(" Removing files also clears the startup set; the next Play prepares it again.", " 刪除檔案也會清除啟動所需的快取；下次遊玩時會重新準備。")
            }
        }
        return text
    }

    private func selectFileRow(_ row: FileRow) {
        fileMessage = nil
        switch row {
        case .status: return
        case .start, .resume:
            let start = { [weak self] in
                guard let self else { return }
                if let message = self.preloader.start() { self.showAlert(AppLanguage.choose("Can't download", "無法下載"), message) }
            }
            if row == .start {
                let alert = UIAlertController(title: AppLanguage.choose("Download all game files?", "要下載完整遊戲資料嗎？"),
                    message: AppLanguage.choose("This saves \(AssetPreloader.format(preloader.missingBytes)) more on this iPhone (\(AssetPreloader.format(preloader.availableBytes())) free). Wi-Fi and Tailscale recommended.", "還需要在手機上儲存 \(AssetPreloader.format(preloader.missingBytes))（可用空間 \(AssetPreloader.format(preloader.availableBytes()))）。建議使用 Wi-Fi 和 Tailscale。"),
                    preferredStyle: .alert)
                alert.addAction(UIAlertAction(title: AppLanguage.choose("Cancel", "取消"), style: .cancel))
                alert.addAction(UIAlertAction(title: AppLanguage.choose("Download", "下載"), style: .default) { _ in start() })
                present(alert, animated: true)
            } else { start() }
        case .pause: preloader.pause()
        case .cancel: preloader.cancel()
        case .remove:
            guard !engineRunning else { return }
            let alert = UIAlertController(title: AppLanguage.choose("Remove downloaded files?", "確定刪除已下載的資料？"),
                message: AppLanguage.choose("Frees \(AssetPreloader.format(preloader.cachedBytes)). The game will stream from the server again.", "可釋放 \(AssetPreloader.format(preloader.cachedBytes))，遊戲之後將重新透過伺服器串流。"),
                preferredStyle: .alert)
            alert.addAction(UIAlertAction(title: AppLanguage.choose("Cancel", "取消"), style: .cancel))
            alert.addAction(UIAlertAction(title: AppLanguage.choose("Remove", "刪除"), style: .destructive) { [weak self] _ in
                guard let self else { return }
                self.preloader.removeDownloads { message in
                    if let message { self.showAlert(AppLanguage.choose("Can't remove files", "無法刪除檔案"), message) }
                }
            })
            present(alert, animated: true)
        }
        preloadChanged()
    }

    private func showAlert(_ title: String, _ message: String) {
        let alert = UIAlertController(title: title, message: message, preferredStyle: .alert)
        alert.addAction(UIAlertAction(title: AppLanguage.choose("OK", "確定"), style: .default))
        present(alert, animated: true)
    }

    private func currentSizeText() -> String {
        let screen = view.window?.windowScene?.screen.bounds.size ?? UIScreen.main.bounds.size
        let (w, h) = GraphicsConfiguration.size(for: engineRunning ? bootPreset : settings.graphicsPreset, renderHeight: settings.renderHeight,
                                                screen: screen, matchScreenShape: settings.matchScreenShape)
        return AppLanguage.choose("Renders at \(w)×\(h)", "目前渲染解析度：\(w)×\(h)")
    }
    /// Uploads the logs after the player confirms, with an optional note (its own screen, so the
    /// keyboard never covers Send in landscape).
    private func sendLog() {
        let screen = SendLogViewController()
        screen.finished = { [weak self] title, message in self?.showAlert(title, message) }
        let nav = UINavigationController(rootViewController: screen)
        nav.modalPresentationStyle = .fullScreen
        present(nav, animated: true)
    }
    private func shareLogs(from cell: UITableViewCell?) {
        let documents = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
        let logs = ["native-port.log", "native-port-previous.log"].map { documents.appendingPathComponent($0) }
            .filter { FileManager.default.fileExists(atPath: $0.path) }
        guard !logs.isEmpty else { showAlert(AppLanguage.choose("No log yet", "尚無紀錄"), AppLanguage.choose("Start the game once, then try again.", "請先啟動一次遊戲，再重試。")); return }
        let sheet = UIActivityViewController(activityItems: logs, applicationActivities: nil)
        sheet.popoverPresentationController?.sourceView = cell ?? view
        present(sheet, animated: true)
    }
    private func resolutionTitle(_ height: Int) -> String {
        if height == 0 {
            let screen = view.window?.windowScene?.screen.bounds.size ?? UIScreen.main.bounds.size
            let (_, h) = GraphicsConfiguration.size(for: engineRunning ? bootPreset : settings.graphicsPreset, renderHeight: 0, screen: screen, matchScreenShape: settings.matchScreenShape)
            return AppLanguage.choose("Auto (\(h)p)", "自動（\(h)p）")
        }
        return height == GraphicsConfiguration.nativeHeight ? AppLanguage.choose("Native (\(height)p)", "原生（\(height)p）") : "\(height)p"
    }
    private func reloadFooters() {
        UIView.performWithoutAnimation { tableView.reloadSections([Section.graphics.rawValue], with: .none) }
    }
    private func slider(_ value: Double, _ low: Float, _ high: Float, label: String, changed: @escaping (Double) -> Void) -> UISlider {
        let control = UISlider()
        control.minimumValue = low; control.maximumValue = high
        control.value = Float(value)
        control.frame.size.width = 160
        control.accessibilityLabel = label
        control.addAction(UIAction { [weak control] _ in changed(Double(control?.value ?? 0)) }, for: .valueChanged)
        return control
    }
    private func toggle(_ on: Bool, label: String, changed: @escaping (Bool) -> Void) -> UISwitch {
        let control = UISwitch()
        control.isOn = on
        control.accessibilityLabel = label
        control.addAction(UIAction { [weak control] _ in changed(control?.isOn ?? false) }, for: .valueChanged)
        return control
    }
}
