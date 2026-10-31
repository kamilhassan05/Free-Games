# Free Games — ESP32 Firmware Flasher

A browser-based ESP32 flasher for the **Free Games** project — a website with 21 single-file HTML games served from SPIFFS on an ESP32. Flash it straight from Chrome or Edge. No Arduino IDE, no PlatformIO, no software to install.

Live flasher: `https://kamilhassan05.github.io/Free-Games/`

---

## What's inside

The `factory.bin` is a **single merged flash image**. It already contains:

- bootloader
- partition table
- the Free Games firmware
- the SPIFFS image with `index.html` + all 21 games

So you do **not** need to build or merge anything. Just flash it.

---

## For beginners (just flash it)

1. Open the GitHub Pages URL of this repo in **desktop Chrome or Edge** (not Firefox, not Safari, not mobile).
2. Plug your ESP32 into your computer with a **data** USB cable (not charge-only).
3. If the board does not appear, hold **BOOT**, press **RESET** once, release **BOOT**.
4. Select your board in the dropdown.
5. Click **Flash Free Games Firmware**.
6. Wait for it to finish, then press **RESET**.
7. On your phone or PC, connect to the board's Wi-Fi AP and open `http://192.168.4.1`.

Only the **ESP32-WROOM-32U (4 MB)** is tested right now. Other boards are marked "coming soon" and are disabled until tested on real hardware.

---

## For developers (rebuilding `factory.bin`)

Only needed if you change the sketch or the games. Beginners can ignore this.

1. Open `source/FreeGames/FreeGames.ino` in Arduino IDE.
2. Set **Board: ESP32 Dev Module** and **Flash size: 4 MB**.
3. Upload the sketch.
4. Use **Tools → ESP32 Sketch Data Upload** to upload `source/FreeGames/data/` to SPIFFS.
5. Dump the full flash to one file:
py -m esptool --chip esp32 --port COM4 --baud 115200 read-flash 0 0x400000 factory.bin

text
6. Copy that `factory.bin` into `firmware/esp32-wroom-32u/factory.bin`.
7. Commit and push. GitHub Pages serves the new file automatically.

---

## Tested on

| Board | Chip | Flash | Status |
|---|---|---|---|
| ESP32-WROOM-32U | ESP32-D0WD-V3 | 4 MB | ✅ Tested |
| ESP32-C3 Mini | ESP32-C3 | TBD | ⏳ Not yet |
| ESP32-S3 | ESP32-S3 | TBD | ⏳ Not yet |
| ESP32-S3 Mini | ESP32-S3 | TBD | ⏳ Not yet |

---

## Repo layout
.
├── index.html # flasher page (ESP Web Tools 10.1.0)
├── .nojekyll
├── README.md
├── manifests/ # one manifest per board
│ └── esp32-wroom-32u.json
├── firmware/
│ └── esp32-wroom-32u/
│ └── factory.bin # merged full-flash image
└── source/
└── FreeGames/
├── FreeGames.ino
└── data/ # index.html + games/ (SPIFFS image)

text

---

## Manifest rules

- Paths are **relative to the manifest file** (e.g. `../firmware/esp32-wroom-32u/factory.bin`).
- Offsets are **decimal**.
- `chipFamily` must match the chip (`ESP32`, `ESP32-C3`, `ESP32-S3`).
- This repo uses **one merged `factory.bin` at offset `0`** per board.

Reference offsets inside the merged image (4 MB ESP32 default):

| Part | Offset |
|---|---|
| bootloader | 0x1000 |
| partition table | 0x8000 |
| boot_app0 | 0xe000 |
| app | 0x10000 |
| SPIFFS | 0x290000 |

These are inside the merged file. The manifest still uses a single part at offset `0`.

---

## Offline note

The games load Google Fonts, which fail offline on the board's Wi-Fi, so system fonts are used there. That is expected and fine.

---

## Browser support

| Browser | Works? |
|---|---|
| Chrome 89+ (desktop) | ✅ |
| Edge 89+ (desktop) | ✅ |
| Opera 75+ (desktop) | ✅ |
| Firefox | ❌ |
| Safari | ❌ |
| Mobile browsers | ❌ |

Web Serial API is required.

---

## Troubleshooting

- **No serial port in the browser dialog** — use a data USB cable, try another port, install the CP210x or CH340 driver.
- **Failed to download manifest** — open `manifests/esp32-wroom-32u.json` directly in the browser. If it 404s, check GitHub Pages is enabled and the path matches exactly.
- **Flashing fails or times out** — hold **BOOT**, press **RESET**, release **BOOT**, then retry.
- **404 on firmware files** — confirm `firmware/esp32-wroom-32u/factory.bin` exists and the path in the manifest matches exactly (case-sensitive).
- **Button does nothing** — you are not using Chrome or Edge on desktop.
- **Board boots but no page** — connect to the board's Wi-Fi AP and open `http://192.168.4.1`.

---

## Credits

- Built by **Kamil Hassan**.
- Flashing powered by [ESP Web Tools](https://esphome.github.io/esp-web-tools/).