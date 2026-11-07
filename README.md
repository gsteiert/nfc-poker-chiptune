# NFC POKER CHIPTUNE
# A noisy poker chip with a message to share

This board is the pinnacle of my beeping businesscard project.  It is a pocketable version in the dimmensions of a real poker chip.  It has an engergy harvesting NFC EEPROM and WCH CH32V003 MCU.  The MCU is connected to an LED and piezo buzzer.  The two ICs are 8 pin SOIC, and the passives are 0603 so it is not too dificult to hand assemble.

3D files are provided for a snap on poker chip frame.  The design is in three pieces to allow two color with no supports to remove.  The three pieces all fit together securely when snapped around the pcb.

The MCU is connected to the NFC EEPROM via I2C and it reads the memory starting at NFC page 0x20 for the encoded tune, so the tune can be written and updated over NFC.

This board uses the NXP NT3H2211 or NT3H2111 for the energy harvesting tag.  I chose this device after [building a battle board to characterize the output power](https://hackaday.io/project/177897-beeping-blinking-business-card-badge-b4/log/221626-course-correction) of this versus the ST25DV.  The NXP part has better voltage regulation and maintains the voltage better under load.  You can see [the results in a hackaday project log](https://hackaday.io/project/177897-beeping-blinking-business-card-badge-b4/log/221646-power-estimates).

## Board Variations

The nfc-coin.kicad_pcb file has a simplified 2 color version of the logo in front silkscreen.
The nfc-coin-enig.kicad_pcb file has a 3 color version of the logo that looks better with ENIG finish and a dark soldermask like is offered by [OSH Park](https://oshpark.com/) 

## Building the code

The MCU firmware is built using an older version of [ch32fun](https://github.com/cnlohr/ch32fun/tree/v0.9)  You need to clone this repo separately and may need to update the relative path in the Makefile.

## NFC Programming

You can format the NFC tag and load a URL that will be shared when tapped using the NXP TagWriter Android app.  (there may be an iPhone version, but I have not used it)  You can also store other data like VCARDs, but iPhones are very particular about which NFC messages they will read and it is safer to use URLs.

The NXP TagWriter app does not work well with binary data, so I recommned the ST25 Android app for writing the binary coded tune.  Be sure to write the binary file to the proper location, writing binary data to the wrong address can lock the tag and make it unusable.  Page 0x20 is address 0x200 in the app.

## Tune Format

The tune format is very simple.
A note is encoded in two bytes consisting of four nibles:

| Bits |  15-12      |  11-8      |  7-4   |  3-0 |
|:-----|:-----------:|:----------:|:-------|:----:|
| Data | Whole Notes | 1/16 Notes | Octave | Note |

Note values are:
C=0x0, C#=0x1, D=0x2, Eb=0x3, 
E=0x4, F=0x5, F#=0x6, G=0x7, 
G#=0x8, A=0x9, Bb=0xA, B=0xB, 
Rest=0xF

Whole Note count value of 0xF is a loop command that will go back to the note index encoded in the lower 8 bits (Octave and Note nibbles).  Add 0xF000 at the end of your tune to get it to repeat.

TBD:  There is no gap between notes, so two consecutive entries of the same note will string together.  A rest between notes should be added to the code.

