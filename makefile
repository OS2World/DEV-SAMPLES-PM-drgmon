###########################################################################
#                                                                         #
# MAKE FILE FOR DRGDROP.EXE                                               #
#                                                                         #
# NOTES:                                                                  #
#                                                                         #
#  To enable the C Set/2 memory management debugging code, uncomment the  #
#  DEBUGALLOC macro. The debugging info will be sent to DRGDROP.DBG.      #
#                                                                         #
# HISTORY:                                                                #
#                                                                         #
#  07-20-93 - started coding.                                             #
#                                                                         #
#  Rick Fishman                                                           #
#  Code Blazers, Inc.                                                     #
#  4113 Apricot                                                           #
#  Irvine, CA. 92720                                                      #
#  CIS ID: 72251,750                                                      #
#                                                                         #
###########################################################################

#DEBUGALLOC=-D__DEBUG_ALLOC__

BASE=drgdrop

CFLAGS=/Q+ /Ss /W3 /Kbcepr /Gm+ /Gd- /O- $(DEBUGALLOC) /C
LFLAGS=/NOI /NOE /MAP /DE /NOL /A:16 /EXEPACK /BASE:0x10000

.SUFFIXES: .c

.c.obj:
    icc $(CFLAGS) $*.c

OFILES=$(BASE).obj drag.obj debugwin.obj show.obj notebook.obj dlgmisc.obj \
       dlgdragi.obj dlgrmf.obj dlgreply.obj menu.obj

LFILES=$(OFILES:.obj=)

$(BASE).exe: $(OFILES) $(BASE).res $(BASE).def
    link386 $(LFLAGS) $(LFILES),,, os2386, $*
    rc $*.res $@

$(BASE).res: $(BASE).rc $(BASE).h $(BASE).dlg $(BASE).ico makefile
    rc -r $*

$(BASE).obj:  $(BASE).c $(BASE).h makefile
drag.obj:     drag.c $(BASE).h makefile
debugwin.obj: debugwin.c $(BASE).h makefile
show.obj:     show.c $(BASE).h makefile
notebook.obj: notebook.c $(BASE).h makefile
dlgmisc.obj:  dlgmisc.c $(BASE).h makefile
dlgdragi.obj: dlgdragi.c $(BASE).h makefile
dlgrmf.obj:   dlgrmf.c $(BASE).h makefile
dlgreply.obj: dlgreply.c $(BASE).h makefile
menu.obj:     menu.c $(BASE).h makefile

###########################################################################
#                       E N D   O F   S O U R C E                         #
###########################################################################
