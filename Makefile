PROG=	pkg_select
SRCS=	main.c parsedirs.c tools.c file.c more.c listmgt.c \
		pkg_adm.c pkg_info.c curses_helpers.c makefile.c pkgsrc.c \
		install_many.c conf.c ftpio.c prefs.c live.c \
		curses_input.c sysinst.c menus.c

WARNS=	3

CFLAGS+=	-g -I.
CFLAGS+=	-DPKGSRCBASE=\"/usr/pkgsrc\" -DPKGDB=\"/var/db/pkg\"
CFLAGS+=	-DMAKE=\"/usr/bin/make\"
CFLAGS+=	-DCONFPATH=\"/usr/pkg/etc/pkg_select.conf\"
CFLAGS+=	-DMIRRORS=\"/usr/pkg/share/pkg_select\"
CFLAGS+=	-DBGCOLOR=COLOR_BLUE -DFGCOLOR=COLOR_WHITE


LDADD=	-lcurses
DPADD=	${LIBCURSES}

RELDATE!=	date +%Y%m%d
RELDIR=		../${PROG}-${RELDATE}

release:
	make clean
	mkdir -p ${RELDIR}
	cp -r * ${RELDIR}

.include <bsd.prog.mk>
