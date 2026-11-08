<div align="center">

# 🎮 Free Games

**A pocket arcade on a $5 microcontroller.**
Play 21 games in your browser, or flash an ESP32 and play them anywhere, even with no internet.

[![Play online](https://img.shields.io/badge/▶_Play_online-no_install-25b896?style=for-the-badge)](https://kamilhassan05.github.io/Free-Games/play/)
[![Flash it](https://img.shields.io/badge/⚡_Flash_an_ESP32-Web_Flasher-3b82f6?style=for-the-badge)](https://kamilhassan05.github.io/Free-Games/)
![Version](https://img.shields.io/badge/version-1.2-ffc93c?style=for-the-badge)

</div>

---

## 🧭 Pick your way to play

| | Play online | Flash an ESP32 |
|---|---|---|
| What you need | Just a browser | An ESP32 board and a USB cable |
| Internet needed? | Yes | **No**, it makes its own Wi-Fi |
| Link | **[Open the games](https://kamilhassan05.github.io/Free-Games/play/)** | **[Open the flasher](https://kamilhassan05.github.io/Free-Games/)** |

## ✨ What is this?

The ESP32 creates its own open Wi-Fi network called **Free Games**. Connect with any phone or laptop and the games open automatically. If they don't, go to `http://192.168.4.1`. The games are stored on the board, so it works with no internet.

- 🎯 **21 original games**: Arcade, Puzzle, Action, Racing, Strategy and Word
- 🎚️ **Easy / Medium / Hard** in every game, with best scores saved on your device
- 🔊 **Sound effects** made in the browser, with a mute button
- 📱 **Phone-friendly** touch controls
- ⚡ **One-click flashing** from the browser

## 🚀 Flash an ESP32

1. Open the **[web flasher](https://kamilhassan05.github.io/Free-Games/)** in desktop **Chrome or Edge**.
2. Plug in your board with a **data** USB cable and pick it in the dropdown.
3. Click **Flash Free Games Firmware**.
4. Press **RESET** when it finishes.
5. Connect to the Wi-Fi **Free Games** (no password).

> **Board not showing up?** Hold **BOOT**, press **RESET** once, release **BOOT**. For the S3 boards, hold **BOOT** while plugging in the USB cable.

You can also download the `.bin` files from **[Releases](../../releases)**.

## ✅ Supported boards

| Board | Chip | Status |
|---|---|---|
| ESP32-WROOM-32U | ESP32 | ✅ Tested |
| ESP32 DevKit V1 | ESP32 | ✅ Tested |
| ESP32-S3 N16R8 | ESP32-S3 | ✅ Tested |
| ESP32-S3 Zero / Super Mini | ESP32-S3 | ✅ Tested |
| ESP32-C3 Super Mini | ESP32-C3 | ⏳ v1.3 |
| ESP8266 / Wemos D1 Mini | ESP8266 | ⏳ v2.1 |

## 🕹️ The games

| Category | Games |
|---|---|
| Arcade | Coin Dodge, Brick Breaker, Whack-a-Mole, Stack Tower, Sky Flap, Snake, Ping Pong, Hop Across |
| Puzzle | Memory Match, Merge Up, Pattern Pulse, Mine Sweeper, Slide Puzzle, Color Clash, Code Breaker |
| Action | Star Blaster, Bolt Runner |
| Racing | Road Rush |
| Strategy | Tic Tac Toe (computer, 2 players or a friend), Connect Four |
| Word | Hangman (with hints) |

## 🛠️ Build it yourself

1. Open `source/FreeGamesAP/FreeGamesAP.ino` in Arduino IDE (keep `RoomData.h` next to it).
2. **ESP32:** board *ESP32 Dev Module*, 4 MB flash.
   **ESP32-S3:** board *ESP32S3 Dev Module*, USB CDC On Boot *Enabled*, flash *4MB*, partition *Default 4MB with spiffs*, PSRAM *Disabled*.
3. Upload the sketch, then the contents of `source/FreeGamesAP/data/` to SPIFFS.
4. Dump the flash into one file (`esp32` or `esp32s3`):

```
py -m esptool --chip esp32 --port COM4 --baud 115200 read-flash 0 0x400000 factory.bin
```

5. Copy it into `firmware/<board>/factory.bin`.

## 📁 Repo layout

```
.
├── index.html                  # web flasher page
├── play/                       # the games, playable online
├── manifests/                  # one flashing recipe per board
├── firmware/
│   ├── esp32-wroom-32u/factory.bin
│   ├── esp32-s3/factory.bin
│   └── esp32-s3-mini/factory.bin
└── source/
    └── FreeGamesAP/            # sketch, RoomData.h and data/ (games)
```

<details>
<summary><b>Troubleshooting</b></summary>

- **No port in the browser dialog:** use a data cable, try another USB port, install the CP210x or CH340 driver.
- **Flashing fails or times out:** hold BOOT, press RESET, release BOOT, then retry.
- **Button does nothing:** Web Serial only works in desktop Chrome, Edge and Opera.
- **Page doesn't open by itself:** open `http://192.168.4.1`. Some phones need you to tap "Sign in to network".
- **Fonts look different on the board:** the games use online fonts and fall back to system fonts offline. That's expected.

</details>

## 🗺️ Roadmap

- [x] **v1.0** Web flasher for ESP32-WROOM-32U and DevKit V1
- [x] **v1.1** Play-online page and Releases
- [x] **v1.2** 8 players, live count, auto-open page, Tic Tac Toe with a friend, ESP32-S3 boards
- [ ] **v1.3** Storage display, upload and delete games with a login, ESP32-C3 support
- [ ] **v2.0** Games sent in by the community, with author credits
- [ ] **v2.1** ESP8266 / Wemos D1 Mini

## 👤 Author

- Made by **Kamil Hassan**.
- Flashing powered by [ESP Web Tools](https://esphome.github.io/esp-web-tools/).
