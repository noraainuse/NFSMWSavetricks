# NFSMW SaveTricks (`NFSMWSavetricks`)

A clean, lightweight ASI plugin for **Need for Speed: Most Wanted (2005)** that tricks the game into detecting an **NFS: Underground 2** save file for loyalty bonus money and custom starter cash.

Created especially for **Linux / Steam Deck / Proton / Wine** users (where NFSU2 and NFSMW live in isolated virtual `C:` drives), as well as Windows players who want the NFSU2 loyalty bonus without having Underground 2 installed.

---

## 🎯 Features

- **NFSU2 Save Loyalty Trick**: Bypasses the filesystem scan (`%LOCALAPPDATA%\NFS Underground 2`) and tricks the game engine into confirming that an NFSU2 save exists.
- **Authentic Experience**: Displays the original in-game message:
  > *"Thanks for having played Need for Speed Underground 2. We have added some cash to help advance you in your career."*
- **Bonus Money++**: Fully customize the bonus cash awarded (default is vanilla $10,000; increase to $20,000, $40,000, etc. for extra starting capital).
- **Starting Cash Injection**: Optionally inject extra starting career money.
- **Clean & Freestanding**: ~3.5 KB binary, zero bloat, zero background threads, zero CRT dependencies (links only against `kernel32.dll`).
- **Full Widescreen Fix Compatibility**: Seamlessly runs alongside ThirteenAG's Widescreen Fix via `dinput8.dll`.

---

## ⚙️ Configuration (`NFSMWSavetricks.ini`)

```ini
[SaveTricks]
; Trick the game into detecting an NFS Underground 2 save file.
; Triggers the authentic loyalty reward sequence: sets the career reward flag,
; displays the in-game notification popup, and awards the bonus starting money.
; Especially helpful on Linux / Steam Deck (Proton / Wine) where games run in separate
; virtual C: prefixes, or on Windows if you don't have NFSU2 installed.
; 1 = Enabled (Always detect NFSU2 save) [Default]
; 0 = Disabled (Vanilla behavior: checks %LOCALAPPDATA%\NFS Underground 2)
Underground2Save = 1

; Display the original in-game NFSU2 loyalty notification dialog:
; "Thanks for having played Need for Speed Underground 2. We have added some cash to help advance you in your career."
; 1 = Show notification dialog [Default]
; 0 = Silent mode (grants bonus money directly without showing dialog)
ShowLoyaltyDialog = 1

; Amount of bonus cash awarded for the NFSU2 save trick.
; Vanilla game gives $10,000 (starting bank: $30,000 + $10,000 = $40,000).
; Increase this value for "more starting money++" (e.g. 20000, 40000, 70000).
; Default: 10000
BonusCash = 10000

; Additional custom cash added directly to starting career balance.
; Can be combined with BonusCash to start with any amount of money you want.
; Default: 0
StartingCash = 0
```

---

## 🛠️ How It Works (Technical Overview)

Target Executable: **NFSMW v1.3 English (`speed.exe`, 6,029,312 bytes, Entry Point `0x007C4040`)**.

1. **NFSU2 Check Hook (`0x00518B40`)**:
   - In vanilla NFSMW, `0x00518B40` calls `SHGetFolderPathA` to locate `%LOCALAPPDATA%\NFS Underground 2` and uses `FindFirstFileA` / `FindNextFileA` to count save entries.
   - If files exist, it writes `byte [0x0091CB20 + 0x1C] = 1` and calls `0x00518570`.
   - `NFSMWSavetricks` overrides `0x00518B40` to immediately set `byte [ecx + 0x1C] = 1` and jump directly to `0x00518570`.
2. **Loyalty Dialog & Reward (`0x00518570`)**:
   - Pushes language hashes `0xC5E2BEAC` (*"Thanks for having played Need for Speed Underground 2..."*), `0x34DC1BEC`, and `0x417B2601` to `FELoadDialog` (`0x00598A60`).
   - Sets the career reward flag `[ecx + 0x04] |= 0x2`.
   - Dispatches to `0x0056D7C0` with `al = 1` to credit the bonus money.
3. **Bonus Cash Patch (`0x0056D7CF`)**:
   - Modifies the immediate 32-bit integer in `add dword ptr [ecx + 0x0C], 0x2710` to your custom `BonusCash` amount.

---

## 🚀 Building from Source

### Quick Build (Linux / Wine)
```bash
./build.sh
```
*Automatically detects system `i686-w64-mingw32-gcc` or uses the bundled Tiny C Compiler (`tools/tcc/tcc.exe`) via Wine.*

### Using Make
```bash
make
# Install directly to game folder:
make install INSTALL_DIR="$HOME/Games/Nfsmw2005/scripts"
```

---

## 🎮 How to Use

1. Download ThirteenAG's **Widescreen Fix** and **NFSMWSavetricks**.
2. Extract the Widescreen Fix zip file.
3. Move `dinput8.dll` and the `scripts` folder to your game install folder.
4. Place `NFSMWSavetricks.asi` and `NFSMWSavetricks.ini` inside the `scripts/` folder.
5. Settings can be changed in `scripts/NFSMWSavetricks.ini`.
6. Start a new career profile in-game and enjoy your loyalty bonus cash!

---

## ❓ FAQ

**Q: Can I use these patches if I don't have a widescreen display or don't wish to play in widescreen?**  
A: Yes. The Widescreen Fixes (and Generic Fixes) are designed to be used with any aspect ratio or resolution.

**Q: Why doesn't the HUD fit within my screen boundaries?**  
A: You're probably using an aspect ratio that's more narrow than 16:9, like 16:10 or 5:4. Try using HUD Adapter by nlgxzef or simply disable `HUDWidescreenMode`.

**Q: What does the Scaling feature do exactly?**  
A: It modifies the aspect ratio. A lot of people don't realize this, but all of Black Box's "Speed" engine games are stretched by default. The Scaling feature allows you to change this. The "Proportionally Correct" option has no stretching at all, and the "Xbox 360" option is an exact replica of that version. It's all personal preference, in the same way that some people may prefer to play their SNES games in 8:7 instead of 4:3.

**Q: Why isn't ImproveGamepadSupport working for me?**  
A: This feature is designed to work with XInput (Xbox) controllers. DualShock / DualSense owners should use an XInput wrapper, like DS4Windows or Steam's controller support, before attempting to use this feature.

**Q: Can I use Multi-Fix with Generic Fix?**  
A: You can, but it's unnecessary. All features from Multi-Fix are included in Generic Fix.

**Q: Why is it called Generic Fix and not Widescreen Fix?**  
A: Generic Fix was made to fix other issues, because technically ProStreet and Undercover already have widescreen support. The code we added to support other aspect ratios came later, and so it's considered a secondary feature, not a primary one.

**Q: Why is a No-CD fix required?**  
A: Need for Speed games have protections (DRM) that make it difficult to view and modify their code, so No-CD patches are often recommended when using ASI scripts. If you're concerned about security, ask the community on the NFSMods Discord to help you acquire these patches from a trusted source.

**Q: Do I need to have NFS Underground 2 installed or an NFSU2 save on disk?**  
A: No! NFSMWSavetricks tricks the game engine internally at runtime. You don't need Underground 2 installed and you don't need any save files.

**Q: Does this work on Steam Deck and Linux?**  
A: Yes! It was built specifically to solve the Steam Deck / Linux Proton issue where separate Wine prefixes make NFSMW unable to see your NFSU2 save folder.

---

## 🏆 Credits

- **ThirteenAG**: Creator of Widescreen Fixes Pack & Ultimate ASI Loader.
- **lambdan**: For reporting issue #1903 requesting the NFSU2 save trick.
- **EA Black Box**: Creators of Need for Speed: Most Wanted (2005).
