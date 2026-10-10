import UIKit

/// Graphics and control options. Before launch every change applies on Play;
/// during play only resolution and controls apply live, the rest on restart.
final class SettingsViewController: UITableViewController {
    private let settings = GameSettings.shared
    private let engineRunning: Bool
    private let bootPreset: GraphicsPreset
    var resolutionChanged: (() -> Void)?
    var controlsChanged: (() -> Void)?
    var targetingChanged: (() -> Void)?

    private enum Section: Int, CaseIterable { case graphics, controls, gyro, device }

    init(engineRunning: Bool) {
        self.engineRunning = engineRunning
        self.bootPreset = GameSettings.shared.graphicsPreset
        super.init(style: .insetGrouped)
        title = "Settings"
        modalPresentationStyle = .formSheet
        overrideUserInterfaceStyle = .dark
    }
    required init?(coder: NSCoder) { fatalError("init(coder:) has not been implemented") }

    override func viewDidLoad() {
        super.viewDidLoad()
        navigationItem.rightBarButtonItem = UIBarButtonItem(systemItem: .done, primaryAction: UIAction { [weak self] _ in self?.dismiss(animated: true) })
        tableView.register(UITableViewCell.self, forCellReuseIdentifier: "cell")
    }

    override func numberOfSections(in tableView: UITableView) -> Int { Section.allCases.count }

    override func tableView(_ tableView: UITableView, numberOfRowsInSection section: Int) -> Int {
        switch Section(rawValue: section)! {
        case .graphics: return settings.tier.allowed.count + 3
        case .controls: return 6
        case .gyro: return (settings.gyroAim == .off ? 1 : 3) + (settings.tiltSteer == .off ? 1 : 2)
        case .device: return 1
        }
    }

    override func tableView(_ tableView: UITableView, titleForHeaderInSection section: Int) -> String? {
        ["Graphics", "Touch controls", "Gyro", "This device"][section]
    }

    override func tableView(_ tableView: UITableView, titleForFooterInSection section: Int) -> String? {
        switch Section(rawValue: section)! {
        case .graphics:
            var text = "Resolution changes apply right away. "
            text += engineRunning ? "Preset and frame rate changes apply the next time you start the game." : "Presets apply when you press Play."
            if settings.graphicsPreset != bootPreset && engineRunning { text += " Restart needed for \(settings.graphicsPreset.title)." }
            if settings.tier.risky(settings.graphicsPreset) { text += " Presets above the recommended one can run out of memory or overheat on this phone; if a launch fails, the game falls back automatically." }
            return text
        case .controls:
            return "Controls act as a game controller, so the game's own aim assist and analog steering work. Assisted aim locks onto targets like on a console."
        case .gyro:
            return "Move the phone to fine-tune your aim; turning your body turns the camera. Tilt steering turns the phone like a steering wheel: Assist adds fine control on top of the touch steering, Tilt to steer replaces it. The straight-ahead angle is however you hold the phone when you get in."
        case .device: return nil
        }
    }

    override func tableView(_ tableView: UITableView, cellForRowAt indexPath: IndexPath) -> UITableViewCell {
        let cell = UITableViewCell(style: .subtitle, reuseIdentifier: nil)
        cell.selectionStyle = .default
        var content = cell.defaultContentConfiguration()
        content.secondaryTextProperties.color = .secondaryLabel
        let presets = settings.tier.allowed
        switch Section(rawValue: indexPath.section)! {
        case .graphics:
            if indexPath.row < presets.count {
                let preset = presets[indexPath.row]
                content.text = preset.title + (preset == settings.tier.recommended ? " (recommended)" : "")
                content.secondaryText = "\(preset.height)p · " + preset.detail
                cell.accessoryType = preset == settings.graphicsPreset ? .checkmark : .none
            } else {
                let row = indexPath.row - presets.count
                cell.selectionStyle = .none
                if row == 0 {
                    content.text = "Resolution scale \(Int((settings.resolutionScale * 100).rounded()))%"
                    content.secondaryText = currentSizeText()
                    let slider = UISlider()
                    slider.minimumValue = 0.5; slider.maximumValue = 1
                    slider.value = Float(settings.resolutionScale)
                    slider.frame.size.width = 150
                    slider.accessibilityLabel = "Resolution scale"
                    slider.addAction(UIAction { [weak self, weak slider] _ in
                        guard let self, let slider else { return }
                        self.settings.resolutionScale = (Double(slider.value) * 20).rounded() / 20
                        var updated = cell.defaultContentConfiguration()
                        updated.text = "Resolution scale \(Int((self.settings.resolutionScale * 100).rounded()))%"
                        updated.secondaryText = self.currentSizeText()
                        updated.secondaryTextProperties.color = .secondaryLabel
                        cell.contentConfiguration = updated
                    }, for: .valueChanged)
                    slider.addAction(UIAction { [weak self] _ in self?.resolutionChanged?() }, for: [.touchUpInside, .touchUpOutside])
                    cell.accessoryView = slider
                } else if row == 1 {
                    content.text = "Fill the screen"
                    content.secondaryText = "Render at the screen's shape instead of stretching 16:9"
                    cell.accessoryView = toggle(settings.matchScreenShape, label: "Fill the screen") { [weak self] in
                        self?.settings.matchScreenShape = $0; self?.resolutionChanged?(); self?.tableView.reloadData()
                    }
                } else {
                    content.text = "60 FPS limit"
                    content.secondaryText = "Uses more power and heat; 30 FPS is steadier"
                    cell.accessoryView = toggle(settings.sixtyFPS, label: "60 FPS limit") { [weak self] in
                        self?.settings.sixtyFPS = $0; self?.reloadFooters()
                    }
                }
            }
        case .controls:
            cell.selectionStyle = .none
            switch indexPath.row {
            case 0:
                cell.accessoryView = slider(settings.lookSensitivity, 0.3, 3, label: "Look speed") { [weak self] in self?.settings.lookSensitivity = $0 }
                content.text = "Camera stick speed"
            case 5:
                cell.accessoryView = slider(settings.aimSensitivity, 0.3, 4, label: "Aim look speed") { [weak self] in self?.settings.aimSensitivity = $0 }
                content.text = "Aim stick speed"
            case 1:
                cell.accessoryView = slider(settings.controlScale, 0.7, 1.5, label: "Button size") { [weak self] in self?.settings.controlScale = $0; self?.controlsChanged?() }
                content.text = "Button size"
            case 2:
                cell.accessoryView = slider(settings.controlOpacity, 0.2, 1, label: "Button opacity") { [weak self] in self?.settings.controlOpacity = $0; self?.controlsChanged?() }
                content.text = "Button opacity"
            case 3:
                content.text = "Targeting"
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
            default:
                content.text = "Fire also aims"
                content.secondaryText = "Fire locks on first, like the classic mobile GTA games"
                cell.accessoryView = toggle(settings.fireAlsoAims, label: "Fire also aims") { [weak self] in self?.settings.fireAlsoAims = $0 }
            }
        case .gyro:
            let aimRows = settings.gyroAim == .off ? 1 : 3
            if indexPath.row == aimRows {
                content.text = "Tilt steering"
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
                content.text = "Full lock at \(Int(settings.tiltFullLock))°"
                cell.accessoryView = slider(settings.tiltFullLock, 10, 45, label: "Tilt for full steering") { [weak self] in self?.settings.tiltFullLock = $0.rounded() }
                cell.selectionStyle = .none
            } else if indexPath.row == 0 {
                content.text = "Gyro"
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
                content.text = String(format: "Sensitivity %.1f×", settings.gyroSensitivity)
                let slider = UISlider()
                slider.minimumValue = 0.2; slider.maximumValue = 3
                slider.value = Float(settings.gyroSensitivity)
                slider.frame.size.width = 150
                slider.accessibilityLabel = "Gyro sensitivity"
                slider.addAction(UIAction { [weak self, weak slider] _ in
                    guard let self, let slider else { return }
                    self.settings.gyroSensitivity = (Double(slider.value) * 10).rounded() / 10
                    var updated = cell.defaultContentConfiguration()
                    updated.text = String(format: "Sensitivity %.1f×", self.settings.gyroSensitivity)
                    cell.contentConfiguration = updated
                }, for: .valueChanged)
                cell.accessoryView = slider
                cell.selectionStyle = .none
            } else {
                content.text = "Invert vertical"
                cell.accessoryView = toggle(settings.gyroInvertY, label: "Invert vertical gyro") { [weak self] in self?.settings.gyroInvertY = $0 }
                cell.selectionStyle = .none
            }
        case .device:
            content.text = settings.tier.summary
            content.textProperties.font = .preferredFont(forTextStyle: .footnote)
            content.textProperties.color = .secondaryLabel
            cell.selectionStyle = .none
        }
        cell.contentConfiguration = content
        return cell
    }

    override func tableView(_ tableView: UITableView, didSelectRowAt indexPath: IndexPath) {
        tableView.deselectRow(at: indexPath, animated: true)
        let presets = settings.tier.allowed
        guard indexPath.section == Section.graphics.rawValue, indexPath.row < presets.count else { return }
        settings.graphicsPreset = presets[indexPath.row]
        resolutionChanged?()
        tableView.reloadSections([Section.graphics.rawValue], with: .none)
    }

    private func currentSizeText() -> String {
        let screen = view.window?.windowScene?.screen.bounds.size ?? UIScreen.main.bounds.size
        let (w, h) = GraphicsConfiguration.size(for: engineRunning ? bootPreset : settings.graphicsPreset, scale: settings.resolutionScale,
                                                screen: screen, matchScreenShape: settings.matchScreenShape)
        return "Renders at \(w)×\(h)"
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
