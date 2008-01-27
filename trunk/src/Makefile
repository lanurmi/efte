MAKE=make

all:
	@echo 'What (emx, unix, cygwin, bcc2, win32ms, win32bcc5, win32mingw, vag, owos2, ownt)?'

emx:
	$(MAKE) -f fte-emx.mak

unix:
	$(MAKE) -f fte-unix.mak

cygwin:
	$(MAKE) -f fte-cygwin-xf86.mak

bcc2:
	$(MAKE) -f fte-bcc2.mak

win32ms:
	nmake /f fte-msvc.mak

win32mingw:
	$(MAKE) -f fte-mngw.mak

win32bcc5:
	$(MAKE) -f fte-bcc5.mak

djgpp2:
	$(MAKE) -f fte-dj2.mak

vag:
	nmake /f fte-vag.mak

owos2: .SYMBOLIC
	wmake -e -f owconvert.mak
	watconvert
	wmake -e -f fte-owos2.mak

ownt:   .SYMBOLIC
	wmake -e -f owconvert.mak
	watconvert
	wmake -e -f fte-ownt.mak

owclean : .SYMBOLIC
	@rm *.obj
	@rm *.exe
	@rm *.err
	@rm *.lst
	@rm *.map
	@rm def*.h
	@rm *.cnf
	@rm *.mif

clean:
	rm -f core *.o *~ .depend
