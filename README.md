# GTA V on iPhone (build-it-yourself port)

[![Watch the 70-second setup video](docs/video-thumbnail.jpg)](https://github.com/kevinerikjs/gta5-ios-port/releases/download/v1.0/gta5-ios-port-tutorial.mp4)

*Click the image for the 70-second setup video.*

A fan project that runs the WebAssembly build of GTA V natively on iPhone: engine compiled to ARM64, Metal graphics, touch controls that act as a real controller (with the game's own aim assist), gyro aiming, tilt steering and graphics presets.

**This repository contains no game files and no game code.** You need your own copy of the game files; the build turns them into an app on your own build machine, and your phone streams the rest from your own computer. Nothing here downloads game content from anywhere else.

> Not affiliated with or endorsed by Rockstar Games or Take-Two Interactive. Rights holders: contact kevin@aether.ee and the repository will be taken down promptly.

## What you need

- **Your own game files** for web build `8b0b5899ed`: the folder that contains `data/` and `b/8b0b5899ed/` (about 20 GB). The build checks `game.wasm` and stops if it is a different build.
- **A computer that stays on while you play** (Windows, macOS or Linux) to hold those files.
- **[Tailscale](https://tailscale.com)** (free) on that computer and on your iPhone.
- **A free GitHub account** (GitHub builds the app for you, no Mac needed) **or an Apple Silicon Mac** with Xcode (free).
- **A free Apple ID** and **[SideStore](https://sidestore.io) or [AltStore](https://altstore.io)** (or Xcode on a Mac) to install it. No paid Apple subscription is required.
- **An iPhone with 6 GB of RAM or more** (iPhone 13 Pro or newer). It runs with a free Apple ID but is much smoother with a paid developer account (see *Free vs paid*).

## 1. Serve your game files over Tailscale

1. Install Tailscale on your computer and phone and sign in to the same account on both.
2. In the Tailscale admin console, under **DNS**, turn on **MagicDNS** and **HTTPS Certificates**.
3. On the computer, share the game folder (the one containing `data/` and `b/`):
   ```sh
   tailscale serve --bg "/path/to/your/game-files"
   ```
4. Run `tailscale serve status`. It shows your server address, like `https://my-pc.tailnet-name.ts.net`. Open `https://my-pc.tailnet-name.ts.net/data/manifest.json` in Safari on your phone; if you see text, the server works.

Only devices on your own tailnet can reach it. To stop sharing: `tailscale serve --https=443 off`.

## 2. Build your app on GitHub

1. On this repository's page, press **Use this template** and **Create a new repository**, and choose **Private**. The build refuses to run in a public copy, because the finished app contains the game engine from your files.
2. In the Tailscale admin console, open **Settings, Keys, Generate auth key**: turn on **Reusable** (so a re-run works) and **Ephemeral** (the build machine leaves your tailnet when it is done), pick a short expiry such as 7 days, then copy the key.
3. In your new repository: **Settings, Secrets and variables, Actions, New repository secret**. Add two secrets:
   - `TS_AUTHKEY`: the key from step 2.
   - `GAME_SERVER_URL`: your server address from part 1, e.g. `https://my-pc.tailnet-name.ts.net`.
4. Open the **Actions** tab, choose **Build my IPA**, press **Run workflow**. Tick *paid account* only if you have a paid Apple developer account.
5. Wait about 15 minutes. When it is done, open the run and download **GTA5Port-ipa** (a zip with `GTA5Port.ipa` inside).

Cost: private repositories get free GitHub Actions minutes every month (2,000 on the free plan). macOS minutes count ten times, so one 15-minute build uses about 150 of them: roughly ten builds a month for free. GitHub only charges if you raise your spending limit above zero. If your account has an old billing problem, every run fails instantly with *startup_failure*; fix the payment method in GitHub's billing settings.

## 3. Install and play

1. Install SideStore (or AltStore) on your iPhone using its own guide.
2. Unzip the download and open `GTA5Port.ipa` with SideStore/AltStore. Free Apple IDs need the app refreshed every 7 days; SideStore can do that on the phone.
3. Keep Tailscale connected, open the app and press **Play**. Your server is already filled in (tap it to change it). The first launch downloads about 4 GB into the app; later launches start much faster.

## Two ways to build (both work with a free Apple ID)

| | GitHub Actions (above) | Your own Mac |
|---|---|---|
| Needs | a free GitHub account | an Apple Silicon Mac with Xcode (free) |
| Apple account | free Apple ID | free Apple ID |
| Install with | SideStore / AltStore | Xcode directly, or SideStore / AltStore |
| Time | about 15 minutes | about 30 to 60 minutes |

A free Apple ID works, but the app is re-signed every 7 days and gets less memory, so it is noticeably laggier. A paid Apple developer account (99 USD/year) gives a 1-year signature and the full memory budget; see *Free vs paid* below.

### Building on your Mac

Install Xcode from the App Store, then `brew install xcodegen`. In this folder:
```sh
./build_ipa.sh https://my-pc.tailnet-name.ts.net          # free Apple ID
./build_ipa.sh https://my-pc.tailnet-name.ts.net --paid   # paid developer account
```
This makes `GTA5Port.ipa` next to the script; install it with SideStore/AltStore. To install straight from Xcode instead, open `app/PlayGTA5Native.xcodeproj` (created by the script), choose your iPhone as the destination, pick your Apple ID under **Signing & Capabilities** (free accounts appear as *Personal Team*; change the bundle identifier to something unique if Xcode asks), and press **Run**. The server can also be any HTTP(S) server that supports range requests.

## Free vs paid Apple account: smoothness

| | Free Apple ID | Paid Apple Developer account (99 USD/year) |
|---|---|---|
| Works | yes | yes |
| Memory iOS allows the app | normal limit | *Increased Memory Limit* + *Extended Virtual Addressing* |
| Experience | playable but **noticeably laggier**: lighter graphics, more pop-in and hitches while assets reload | **smooth**, higher presets on phones with more RAM |
| Re-sign | every 7 days | once a year |

The game needs a lot of memory. Without the two memory entitlements (only paid accounts can grant them) iOS keeps the app on a much smaller budget, so it falls back to the lightest preset and constantly evicts and reloads assets. Tested on an iPhone 13 Pro Max (6 GB): it runs on a free Apple ID, but clearly smoother with a paid account. Phones with 8 GB or more suffer less.

### Using a paid account for the full-memory version

1. Join the [Apple Developer Program](https://developer.apple.com/programs/) and note your **Team ID** (developer.apple.com, Membership).
2. Register your iPhone: connect it to a Mac running Xcode once, or add its UDID under Certificates, Identifiers & Profiles, Devices.
3. Build with the memory entitlements:
   - **On a Mac (most reliable):**
     ```sh
     ./build_ipa.sh https://my-pc.tailnet-name.ts.net --paid
     ```
     Then open `app/PlayGTA5Native.xcodeproj`. Under **Signing & Capabilities**, pick your paid team, change the bundle identifier to something unique (e.g. `com.yourname.gta5port`), and check that **Increased Memory Limit** and **Extended Virtual Addressing** are listed (add them with **+ Capability** if not). Choose your iPhone and press **Run**.
   - **With GitHub Actions:** run the workflow with **paid account** ticked; the IPA then asks for both entitlements. Install it with a signing tool that uses your paid account (AltStore/SideStore signed in with the paid Apple ID). If the app still feels as laggy as the free build, the tool dropped the entitlements; use the Mac route above.
4. In the app, open **Settings**: the bottom line shows the memory limit iOS granted. With the entitlements it should be well above the free-account value (on a 6 GB phone, about 4,000 MB instead of about 3,000 MB), and higher graphics presets unlock.

## Controls and settings

Settings (gear icon, top centre) has graphics presets and resolution, touch control size/opacity and stick speeds, targeting mode (assisted aim by default), gyro aiming and tilt steering. Bluetooth controllers work too.

## How it works

- The engine (`game.wasm` from your files) is translated to C with [wasm2c](https://github.com/WebAssembly/wabt) and compiled to ARM64 at build time; iOS does not allow compiling code on the phone.
- Graphics go through [Dawn](https://dawn.googlesource.com/dawn) (BSD-3-Clause, prebuilt in this project's releases) on Metal. The renderer script comes from the public [playgta5](https://github.com/shadany7824/playgta5) repository at a pinned commit, with this project's changes applied from `app/Patches/`.
- Touch input feeds the engine's own controller code, so GTA's controller features (assisted aim, analog driving) work.
- Game files stream from your server into a block cache in the app.

## License

The code written for this project is MIT licensed (see `LICENSE`). It does not cover the game, its data, or the third-party files fetched at build time.
