MAKE=make

all:
	@echo 'What (emx, unix, bcc2, win32ms, win32mingw, djgpp2)?'

emx:
	$(MAKE) -f fte-emx.mak

unix:
	$(MAKE) -f fte-unix.mak

bcc2:
	$(MAKE) -f fte-bcc2.mak

win32ms:
	nmake /f fte-msvc.mak

win32mingw:
	$(MAKE) -f fte-mngw.mak

djgpp2:
	$(MAKE) -f fte-dj2.mak
