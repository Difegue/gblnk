gblnk difeguemod - for transferring images between your gameboy camera and your PC
Original software (c) Chris McCormick (http://freecode.com/projects/gblnk)

Rewritten to allow batch processing and not depend on the allegro library.
More convenient, runs on any GNU/Linux variant, and a good 70KBs lighter. Now the .bmps obtained weigh more than the actual program!

The old allegro code from Chris is included in oldfiles. 
Since the 0.2 has disappeared from his website and was only available on archive.org (god bless), it's probably a good idea to keep it here.

The old files madcatz.c and madcatz.txt are also included, in case you want to understand how the cable works. 
(I still don't understand it fully myself （´・ω・｀） )

I hope this program will help all the poor lads who can't afford cartridge dumpers like me get more GBCamera photos out there.

~Difegue~

(sugoi@cock.li)

Usage:
From a Terminal:
./gblnk filename1 filename2 filename3 ...
For every filename specified, gblnk will wait for you to initiate printing from your gameboy camera.

protip: bmptoppm < file.bmp | cjpeg -q 90 > file.jpg to convert the bmps into a more web-friendly format. 
If I feel courageous enough, I'll add in native jpeg/png export. (bmp isn't really convenient in 2014.)
