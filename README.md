DCPU-16 custom tools
--------------------

	This is a C++ implementation of the DCPU16 from Mojang's game 0x10c.
	It is based on the 1.7 specification (see doc directory).
	0x10c's website : http://0x10c.com/

Current features
----------------

	- DASM16 assembler
	- LEM1802 emulation (changing palette/border color is not supported yet)
	- Generic Keyboard emulation
	- Generic Clock emulation
	- Instant CPU state display with an overlay by hitting TAB
	- Auto textual memory dump before and after running
	- Font image converter (image 128x32 ==> DASM code)
	- Basic preprocessor (still WIP, only a 1-level #include is supported)

Planned features
----------------

	- Print help in command line
	- N-level #include with infinite inclusion checking
	- More preprocessor commands (#define etc...)
	- More assembly special commands
	- More hardware devices (Custom speakers?)
	- Better support for implemented hardware devices
	- Debug tools / disassembler
	- Games and IDE?

Releases
--------

	Not available for now, I should upload binaries soon.

Compiling / dependencies
------------------------

	SFML 2.0 : http://www.sfml-dev.org/
	
	The project is compiled with GCC/MinGW.
	C++11 is not used yet.
	
	Note: don't include src/dcpu11 to your project if you want to compile,
		this directory contains an outdated version of the emulator for 1.1 specs.

How to use
----------

	In the command line :
	
	dcpu yourFile
		# Will assemble yourFile and run it
		
	dcpu -pp yourFile ouputFile
		# Will perform a preprocessing pass to yourFile
		# and put the result in outputFile.
		# see detailed information about the preprocessor below.
		
	dcpu -cvf yourImage yourDASMFile
		# Converts an font image (128x32 px) into DASM code (dat 0xstuff).
		# fonts are read as white pixels.
		# Supported image formats: BMP, PNG, JPG (not recommended) and a few others.
		
	dcpu
		# Prints an error (should print help later)

==================== More detailed info ========================================

The preprocessor

	#include "file"
	; will copy the content of the given file at this position.
	; Preprocessor commands in the included file are not processed yet.





