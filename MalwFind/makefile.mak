
rebuild:
#	cd WXP     &&  nmake -f Makefile.mak  rebuild
	cd Win7    &&  nmake -f Makefile.mak  rebuild
	cd Win8    &&  nmake -f Makefile.mak  rebuild
	cd Win10   &&  nmake -f Makefile.mak  rebuild

debug:
#	cd WXP     &&  nmake -f Makefile.mak  debug
	cd Win7    &&  nmake -f Makefile.mak  debug 
	cd Win8    &&  nmake -f Makefile.mak  debug
	cd Win10   &&  nmake -f Makefile.mak  debug
 
export: rebuild