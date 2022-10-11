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

- cd src\mame

- Compile with `-j9` (CPU cores + 1) and some linker stuff
`make -j9 ARCHOPTS="-fuse-ld=lld" SOURCES=src/mame/ensoniq/esq5505.cpp`


## How to pick this up again when you forget

- MAME source tree lives under `C:\Users\Public\msys64\src\mame`
- Start compiler environment by opening `C:\Users\Public\msys64` folder in Windows, double click on `win32env.bat`
- `cd mame`
- Then compile with: `make -j9 ARCHOPTS="-fuse-ld=lld" SOURCES=src/mame/ensoniq/esq5505.cpp`
- Run with: `./mame.exe vfx -midiin "MIDI4x4" -http`
- Open [http://localhost:8080/esqpanel/FrontPanel.html](http://localhost:8080/esqpanel/FrontPanel.html) in a broswer on the same machine


OR connect to MIDI-OX using loopMIDI
- `./mame.exe vfx -midiin "loopMIDI Port To" -midiout "loopMIDI Port From" -http`

Then, in MIDI-OX, in MIDI Devices window, select:
```
MIDI Inputs:
loopMIDI Port From	To receive SYSEX dumps FROM the virtual VFX
MIDI4x4            	To receive midi from a real midi source

MIDI Outputs:
loopMIDI Port To 	To transmit SYSEX and MIDI notes/CC down to the virtual VFX
```

!! DO NOT FORGET TO ENABLE SYSEX IN THE VFX EMULATION EVERY BOOT UP !!

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



## Reverse engineering the ROM dumps into a single cohesive 12bit PCM file


- 8 bits from LO (U14) + High Nibble from NYB (U16) (pins D0-D3)
- 8 bits from HI (U15) + Low Nibble from NYB (U16) (pins D4-D7)

8 bits is a byte (0x00 through 0xFF)

Nibble is 4 bits (halfbyte)

So to get 12 bit samples from each bank:

@0x00 - Bank 1:  0x00 (from u14) + 0x*9 (from u16) = 0x009
@0x00 - Bank 2:  0x00 (from u15) + 0x9* (from u16) = 0x009

@0x01 - Bank 1:  0x05 (from u14) + 0x*2 (from u16) = 0x052
@0x01 - Bank 2:  0xCA (from u15) + 0xF* (from u16) = 0xCAF

Are those correct or am I ordering them wrong?

```c++
void esq5505_state::init_denib()
{
	uint8_t *pNibbles = (uint8_t *)memregion("nibbles")->base();
	uint8_t *pBS0L = (uint8_t *)memregion("waverom")->base();
	uint8_t *pBS0H = pBS0L + 0x100000;

	init_common();

	// create the 12 bit samples by patching in the nibbles from the nibble ROM
	// low nibbles go with the lower ROM, high nibbles with the upper ROM
	for (int i = 0; i < 0x80000; i++)
	{
		*pBS0L = (*pNibbles & 0x0f) << 4;
		*pBS0H = (*pNibbles & 0xf0);
		pBS0L += 2;
		pBS0H += 2;
		pNibbles++;
	}
}
```

So, the U14 and U15 (which hold 1 byte at each memory offset) are loaded into MAME as 16bit values:

I.e. The low bytes `00 05 09 06` (from u14.bin) end up being loaded into memory as 16 bit words:

```
00 ??
05 ??
09 ??
06 ??
```

And then the low nibbles `?9 ?2 ?8 ?9` (from u16/bin) get shifted left 4, then plopped in behind the existing high byte of the 16 bit value at that address:

```
00 90
05 20
09 80
06 90
```

And that's where we get 12 bit samples... But I'm still not sure which way around they need to be...


0x00 = 144 decimal based on logging from emulator...  which is `0x0090` so that's good!
0x01 = 1312 decimal... which is `0x0520`! So that is absolutely the correct logic for grabbing the data!

I wonder if I should just dump the PCM data directly instead of all this separate magic...

The problem is each Transwave shifts through memory differently.

- OMEGA-X is literally shifting a 1-byte-wide single-cycle sample from 0x000-0x100 (sample 0) to 0x1f00-0x2000 by about 0xff every "slot" of the wave table.
	- Which means each "single cycle waveform" is 255 samples long, where each sample is 12-bits.


After some digging, as modern computers expect 16-bit wave files, convention is to maintain the most significant bits and shove the 12 bit samples into the top 16 bits. And since we're using twos-compliment, the MSB represents sign and we're still good.

I'm going to try writing the first cycle, 0x00-0xff out to a single channel wav file at 16 bit as soon as the code starts seeing bits.

Weirdly, I'll probably need to increment a false, 29 bit accumulator separately from the actual address, so that the address lookup code gets called correctly.

Ooooor, I'll need to record ALL of the addresses being used when a REALLY low note plays and the accumulator is slow.

This code appears to try writing a WAV file (it's used for something else I don't really care about). I think it can be used to write out our wave tables.

```c++
#if ES5506_MAKE_WAVS
	// start the logging once we have a sample rate
	if (m_sample_rate)
	{
		if (!m_wavraw)
			m_wavraw = wav_open("raw.wav", m_sample_rate, 2);
	}
#endif

	// loop until all samples are output
	generate_samples(outputs);

#if ES5506_MAKE_WAVS
	// log the raw data
	if (m_wavraw)
	{
		// determine left/right source data

		s32 *lsrc = m_scratch, *rsrc = m_scratch + length;
		int channel;
		memset(lsrc, 0, sizeof(s32) * length * 2);
		// loop over the output channels
		for (channel = 0; channel < m_channels; channel++)
		{
			s32 *l = outputs[(channel << 1)] + sampindex;
			s32 *r = outputs[(channel << 1) + 1] + sampindex;
			// add the current channel's samples to the WAV data
			for (samp = 0; samp < length; samp++)
			{
				lsrc[samp] += l[samp];
				rsrc[samp] += r[samp];
			}
		}
		wav_add_data_32lr(m_wavraw, lsrc, rsrc, length, 4);
	}
#endif
```

But we wanna do 16 bit, 1 channel. So:

```c++
void * m_wavraw;
s16 buffer[1024];
bool alreadyDumped = false;
if (!alreadyDumped)
{
	alreadyDumped = true;

	// Open the WAV for writing:
	m_wavraw = wav_open("raw.wav", m_sample_rate, 1);

	// Step through the lower 0xff addresses in the ROM and shove them in a buffer:
	for (u16 index = 0x00; index <= 0xff; index++) {
		// Grab the "12bit" sample (actually 16 bit with padded LSB)
		buffer[index] = (s16)read_sample(voice, index);
	}

	// Write the 0xff samples out to the WAV file
	wav_add_data_16(*m_wav_file, buffer, 0xff);

	// Close the WAV file
	wav_close(m_wavraw);
}

```
