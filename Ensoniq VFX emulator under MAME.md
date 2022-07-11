# Ensoniq VFX emulator under MAME

The Ensoniq VFX is an available emulated machine under MAME. These directions attempt to explain setting up and using the emulated synth.

## Compile and Run under windows

- Download `mysys64-2022-01-12.exe` from https://www.mamedev.org/tools/
- Extract to `C:\Users\Public\msys64`
- Run `msys64/win32env.bat` and close terminal when it tells ya to
- Run `msys64/win32env.bat` again
- Clone MAME from GitHub

```bash
cd src/
git clone git@github.com:JamesHagerman/mame.git
```

- Compile with `-j9` (CPU cores + 1) and some linker stuff
`make -j9 ARCHOPTS="-fuse-ld=lld"`


## Running VFX emulator

- Grab VFX ROMS and put them into `roms/vfx/`
- Create a config with `mame.exe -createconfig`
- Sort out config to not run in fullscreen
- Update config with low latency bits
    - `sound portaudio`
    - `audio_latency 0`
    - `pa_latency 0.1`
- Find a good MIDI intput device connected to your machine `mame.exe listmidi`
- Run VFX emulator: `mame.exe vfx -midiin "MIDI4x4" -lowlatency`

Under Linux, I used `./mame vfx -midiin "MIDI4x4 MIDI 1"`

## Optional web interface

Tested this under Windows

- Run VFX emulator with `-http`:
`mame.exe vfx -midiin "MIDI4x4" -lowlatency -http`
- Open [http://localhost:8080/esqpanel/FrontPanel.html](http://localhost:8080/esqpanel/FrontPanel.html) in a broswer on the same machine



## Proposed redo of keyboard layout of VFX

The intention is to allow the VFX synth engine to be used. Full use of the synth is honestly excessive

These are the basics we know we need:

- Numbers 0-9 should be the 0-9 numbers on the synth
- - and = should be decrement increment data
- qwe and asd  should be the 6 soft buttons
- z   should be the Sounds button
- x   could be the Master button
- c   could be the MIDI/Control button

And these are the buttons we need to operate the Synth engine:
- rtyu   LFO, ENV1, ENV2, ENV3
- fghj   Pitch, Pitch Mod, Filters, Output
- vbnm   Wave, Mod Mixer, Program Control, Effects

- iop    Select Voice, Copy, Write
- kl     Compare, Storage (sure, why not?)


*Note: I don't know how all the keys work in MAME/MESS. Arrow keys and others might be useful for other stuff when I run out*
More details about MAME/MESS keyboards: https://www.ninerpedia.org/wiki/MESS_keyboard_modes

For "Cursor left" we can query for UCHAR_MAMEKEY(LEFT).
Ah, function keys might be available. And inputcode.h has a list of all the codes so that makes it easier to think about.

These are mostly useless buttons for various reasions:
- carts is useless without cartridge rom suppot
- Presets and all of the performance buttons are way too complex and miss the Transwave point of the synth and emulation
    - Volume, Pan, Timbre
    - Key Zone, Transpose, Release
    - Patch select, midi, effects


Stuff I KNow:

	PORT_BIT(0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_A) PORT_CHAR('a') PORT_CHAR('A') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0xaa) PORT_NAME("SOFT TOP CENTER")
	PORT_BIT(0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_S) PORT_CHAR('s') PORT_CHAR('S') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0xab) PORT_NAME("SOFT TOP RIGHT")


    0xa2) PORT_NAME("BUTTON 8")

    	PORT_BIT(0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0xbf) PORT_NAME("DATA INCREMENT")
	PORT_BIT(0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHANGED_MEMBER(DEVICE_SELF, esq5505_state, key_stroke, 0xbe) PORT_NAME("DATA DECREMENT")



