MAKE=make

all:
	@echo 'What (unix, win32bcc5, win32mingw)?'

unix:
	$(MAKE) -f fte-unix.mak

win32mingw:
	$(MAKE) -f fte-mngw.mak

win32bcc5:
	$(MAKE) -f fte-bcc5.mak

clean:
	rm -f core *.o *~ .depend
