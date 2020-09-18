# SpriteViewer
A viewer to see sprites inside raw binary files in several image formats. Initially created to be able to see old ZX spectrum games graphics assets.

Advice: to extract the ZX spectrum raw memory in mame, run the debugger with -debug option and execute this command to dump 0x10000 bytes (64KB):
save mem.dmp,0,10000
