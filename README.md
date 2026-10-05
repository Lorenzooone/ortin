# Ortin: IS-NITRO management program

![GS-ORTIN-MANAGER](res/icons/256x256/gs-ortin-manager.png)

## What is ortin?

Ortin is a utility based on reverse-engineered documentation for the
Intelligent Systems IS-NITRO-EMULATOR development kit for Nintendo DS.

Portions of Ortin are based on [NitroDriver](https://github.com/Dirbaio/NitroDriver)
by @Dirbaio.

## Features

* Set AV mode on all IS-NITRO-EMULATOR systems, including those that don't
  have the Video option enabled.
* Load a Nintendo DS ROM image. Both encrypted and decrypted ROM images are
  supported. (Decrypted ROM images are re-encrypted on the fly.)
* Boot Game Boy Advance cartridges by enabling Slot 2 and resetting the
  system.
* Dump ISNE firmware as well as DS Firmware and BIOSes.
* Launch cartridges inserted in Slot-1.

## TODO

* Dumping Slot-1 and Slot-2 cartridges on the PC side.
* Graphical interface with fancy ROM loader.
* Implement more of IS-NITRO-DEBUGGER's functionality.

## Notes

* IS-NITRO-EMULATOR systems cannot boot from Slot-1 cards directly, at least
  not without modifying the DS firmware;
  instead they can boot cartridges inserted in Slot-1 by using slot1launch
  and switching off Slot-1 emulation.
  This can be used to launch retail cartridges while still having
  the debug monitor active for development and debugging.
  Link to the included slot1launch:
  https://github.com/Lorenzooone/Simple-DS-Slot-1-Launcher/releases
* When emulating Slot-1, a ROM image must be loaded onto the EMULATOR board,
  which usually has 256 MB RAM. This makes it impossible to load 512 MB games.
  While it is possible to install more RAM and to edit an ISNE's configuration
  to accept the extra RAM (up to 1 GB), currently there is no known way to
  have the cartridge emulator access the extra RAM, despite it being
  accessible from the PC side. So even with more than 256 MBs of RAM installed,
  512 MB games might look like they loaded, but they will likely crash once
  they try accessing data past the 256 MB limit (they can still be used
  without issue by loading them from cartridges using slot1launch).
* IS-NITRO-EMULATOR does *not* emulate Slot-1 save memory. Most games will
  show an error message if the save memory is not present. To work around
  this, you will need to insert a Slot-1 card with a matching save memory chip
  before loading the ROM image.
* The RAM of the ISNE must be: SODIMM SDRAM 144 pin PC133 133 MHz.
* IS-NITRO-EMULATOR systems do not forward certain cartridge operations to the
  cartridge slot. This means that most Slot-1 flashcarts will not work.
  Not only that, but games using NAND saves (like WarioWare Do It Yourself)
  will not save properly.

## References

* [The NSMB Hacking Domain: Nintendo DS dev hardware! IS-NITRO-EMULATOR & co.](https://nsmbhd.net/thread/4438-nintendo-ds-dev-hardware-is-nitro-emulator-and-co/)
* [GBATEK](https://problemkaputt.de/gbatek.htm) Game Boy Advance and Nintendo DS documentation
* [NitroDriver](https://github.com/Dirbaio/NitroDriver) by @Dirbaio
