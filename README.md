# Dev Toolbox

**English** | [Русский](README.ru.md)

**An offline developer toolbox for Windows.** One small `.exe` instead of twenty
browser tabs with online services: format JSON, decode a JWT, test a regex,
generate a QR code and more, with no internet connection and no risk of pasting
sensitive data into someone else's website.

<p align="center">
  <img src="https://raw.githubusercontent.com/Kakadu525/dev-toolbox/main/resources/demo.gif" width="900" alt="Dev Toolbox demo">
</p>

![Platform](https://img.shields.io/badge/platform-Windows-blue)
![Language](https://img.shields.io/badge/language-C%2B%2B20-00599C)
![License](https://img.shields.io/badge/license-MIT-green)
[![Download](https://img.shields.io/github/v/release/Kakadu525/dev-toolbox?label=download)](https://github.com/Kakadu525/dev-toolbox/releases/latest/download/DevToolbox.exe)

**[Download DevToolbox.exe](https://github.com/Kakadu525/dev-toolbox/releases/latest/download/DevToolbox.exe)**:
no installer, just run it.

---

## Why

Developers reach for dozens of small utilities every day: decode a token,
format some JSON, compute a hash, generate a UUID. Usually that means one more
browser tab with an online service, and that approach has three constant
problems:

- **Privacy.** When you paste a token, a piece of config or a production log
  into an unfamiliar site, you don't know what happens to that data on the
  server.
- **No internet, no tool.** Offline, these services simply aren't there.
- **Fragmentation.** Every tool is a new tab, a new interface, new banners and
  ads.

**Dev Toolbox** puts all of it in one application: everything is processed
locally on your machine and nothing is sent anywhere. One `.exe`, no install,
no internet, no tracking.

---

## Features

- 🖥️ **Fully offline**: no tool goes online for its main job. The HTTP Client
  is the exception: it is a network client, but it only connects where you tell
  it to.
- 📦 **A single exe**: the whole interface is embedded in the executable, there
  is nothing to unpack or install.
- ⚡ **Native performance**: written in C++20.
- 🎨 **Dark and light themes**, adjustable accent color and font size.
- 🌐 **English and Russian interface**, switchable in Settings.
- 🔒 **Private by default**: clipboard history lives only in the process
  memory, nothing is written to disk behind your back.

---

## Tools

### Encoding and hashes
| Tool | Description |
|---|---|
| **Base64** | Encode and decode Base64 strings, with length and ratio stats |
| **Hash Calculator** | MD5 and SHA256 hashes of any text |
| **JWT Decoder** | Splits a JSON Web Token into header/payload/signature and pretty-prints it |

### Generators
| Tool | Description |
|---|---|
| **UUID Generator** | Random UUID v4 identifiers |
| **QR Generator** | QR codes with a selectable error correction level, PNG export |

### Formatters
| Tool | Description |
|---|---|
| **JSON Formatter** | Pretty-print and minify JSON, keeps non-ASCII text intact, clear parse errors |
| **XML Formatter** | Format and minify XML documents |
| **YAML Formatter** | Validate YAML and normalize its style |
| **SQL Formatter** | Line breaks and indentation for SQL, including nested subqueries and `CASE WHEN` |

### Text
| Tool | Description |
|---|---|
| **Regex Tester** | Test regular expressions with every match and capture group highlighted |
| **Diff Viewer** | Line-by-line comparison of two texts with colored changes |

### Network
| Tool | Description |
|---|---|
| **cURL Generator** | Builds a ready-to-run `curl` command from the method, headers and body |
| **HTTP Client** | A small client for requests with any method, headers and body; shows status, response headers and body |

### Visual
| Tool | Description |
|---|---|
| **Color Picker** | Convert colors between HEX, RGB and HSL |
| **Image Converter** | Convert images between PNG/JPEG/BMP, resize and set compression quality |

### System
| Tool | Description |
|---|---|
| **Log Viewer** | View text log files with filtering and live tail |
| **Process Explorer** | Running processes sorted by memory/PID/name, process details, end task |
| **Clipboard History** | Clipboard history (text and images), restore any earlier entry |

### Utilities
| Tool | Description |
|---|---|
| **Cron Parser** | Parse cron expressions and list the next run times |
| **Settings** | Theme, accent color, font size and language, remembered between launches |

---

## Install and run

No installation needed. Download `DevToolbox.exe` from
[Releases](https://github.com/Kakadu525/dev-toolbox/releases/latest) and run
it: everything else is inside the file.

> **Administrator rights.** The app asks for elevated rights on start, because
> Process Explorer needs them to view and end system processes.

### Requirements

- Windows 10 version 1809 or newer, or Windows 11
- [Microsoft Edge WebView2 Runtime](https://developer.microsoft.com/microsoft-edge/webview2/),
  usually already installed on current Windows

---

## Build from source

### Requirements

- Visual Studio 2022 or newer with the **Desktop development with C++** workload
- CMake 3.20+
- Git
- An internet connection for the first build (dependencies are downloaded)

### Steps

```powershell
git clone https://github.com/Kakadu525/dev-toolbox.git
cd dev-toolbox
```

Open the folder in Visual Studio with **File → Open → Folder**: the CMake
configuration starts automatically. When it finishes, build with
**Build → Build All**.

Or from the command line:

```powershell
cmake -S . -B build -A x64
cmake --build build --config Release
```

`DevToolbox.exe` ends up in the build folder (`build/Release/`, or
`out/build/x64-Debug/` when built from Visual Studio).

### Libraries

Every dependency is pulled in automatically with CMake `FetchContent`, nothing
has to be installed by hand.

| Library | Used for |
|---|---|
| [Microsoft WebView2](https://developer.microsoft.com/microsoft-edge/webview2/) | Rendering the HTML/CSS/JS interface inside a native window |
| [nlohmann/json](https://github.com/nlohmann/json) | JSON, including the message protocol between C++ and the interface |
| [pugixml](https://github.com/zeux/pugixml) | Parsing and formatting XML |
| [yaml-cpp](https://github.com/jbeder/yaml-cpp) | Parsing and formatting YAML |
| [QR-Code-generator](https://github.com/nayuki/QR-Code-generator) | QR code generation |
| [stb_image / stb_image_write / stb_image_resize2](https://github.com/nothings/stb) | Decoding, encoding and resizing images |
| [miniz](https://github.com/richgel999/miniz) | Packing the interface into an archive embedded in the executable |

The native Windows API does the rest: BCrypt (hashes), WinHTTP (HTTP client),
Toolhelp32Snapshot (Process Explorer), the Clipboard API (clipboard history)
and other system interfaces, with no third-party code where Windows already
provides it.

---

## Architecture

The app is a native core with a web interface:

```
┌──────────────────────────────────────────────────┐
│                    DevToolbox.exe                │
│                                                  │
│    WinAPI window ──▶ WebView2 (UI layer)         │
│                       │ JSON messages            │
│                       ▼                          │
│              ToolRegistry (router)               │
│                       │                          │
│     ┌─────────────────┼──────────────┐           │
│  Base64Tool      JsonTool   ...  19 tools        │
└──────────────────────────────────────────────────┘
```

- **Interface**: HTML/CSS/JS rendered by WebView2 (the Microsoft Edge engine).
- **Logic**: all in C++, every tool implements the shared `ITool` interface.
- **Messaging**: a simple JSON protocol (`{tool, action, payload}` →
  `{status, result}`) over `PostWebMessageAsString`.
- **Assets**: at build time the interface is packed into a zip archive and
  embedded in the executable as a resource, then unpacked to a temp folder on
  start.
- **Translations**: `ui/i18n.js` holds the Russian strings keyed by the English
  text, so a string missing from the dictionary falls back to English.

This keeps all the logic in fast, type-safe C++, while the interface stays easy
to restyle with plain CSS.

---

## Privacy

- No tool sends data to the internet, except the HTTP Client, which only makes
  the requests you explicitly set up.
- Clipboard history is kept only in the process memory and disappears
  completely when the app closes.
- The only file written to disk is `settings.json` with interface settings
  (theme, font, accent color, language) in `%APPDATA%\DevToolbox`; it holds no
  user data.

---

## License

MIT: use, modify and distribute freely.
