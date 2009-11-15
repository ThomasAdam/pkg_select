/*
 * Copyright (c) 2005
 *      iMil <imil@gcu.info>.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by iMil.
 * 4. Neither the name of the author nor the names of any co-contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY iMil AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL iMil OR THE VOICES IN HIS HEAD
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * $Id: main.c,v 1.52 2009/03/07 18:42:23 imil Exp $ 
 */

#include <sys/cdefs.h>
#ifndef lint
#if 0
static char *rcsid = "$Id: main.c,v 1.52 2009/03/07 18:42:23 imil Exp $";
#else
__RCSID("$Id: main.c,v 1.52 2009/03/07 18:42:23 imil Exp $");
#endif
#endif

#include <stdlib.h>
#include <signal.h>

#include "pkg_select.h"
#include "curses_input.h"

/* reset hl_index and clear screen */
#define DO_RESET 1
#define DONT_RESET 0

static void usage(void);
static void init_curses(void);
static void init_windows(void);
static void finish(int);
static void clear_tree(Etree ***, int);
static void list_win_refresh(void);
static int print_list(Etree **, HL_datas *, char *, int);
static void print_bindings(int);

static int nlines, ncols, top_line, mmenu;
static WINDOW *list_win;

static void
usage()
{
	(void) fprintf(stderr, "%s\n%s\n",
		       "usage: pkg_select [-h] [-b pkgsrcdir] [-K pkg_dbdir] [-c conf file]", 
		       "                  [-l [-m] [-u NetBSD ftp mirror]] [-d url] [-s]");

        exit(1);
}

static void
init_curses()
{
	initscr();
	nonl();
	cbreak();
	noecho();
	curs_set(0);
	init_keymaps();
}

static void
init_windows()
{
	nlines = MAINWIN_LINES;
	ncols = MAINWIN_COLS;
	top_line = 1;
	
	/* create a window 1 char smaller than term */
	list_win = newwin(nlines + 2, ncols, 1, 1);
}

static void
finish(int sig)
{
	/* do something useful with sig so compiler does not 
	   complain on unused variable (FreeBSD) */
	if (sig)
		sig &= 0;
	/* free dir list */
	free_pkgdb();
	/* free "to be installed" package list */
	free_tbi_pkgs();
	/* free conf file members */
	freeconf();
	/* live ftp */
	ftp_stop();
	
	curs_set(1);
	delwin(list_win);
	reset_shell_mode();
	free_keymaps();
	endwin();
	exit(EXIT_SUCCESS);
}

int
toplevel(const char *path)
{
	if (strcmp(path, pkgsrcbase) != 0)
		return(0);
	else
		return(1);
}

/* etree's address is passed so we can NULL **etree */
static void
clear_tree(Etree ***etree, const int where)
{
	if (where == IN_DEPENDS)
		free_nodir_tree(etree);
	else
		free_tree(etree);
}

/* refresh and resize screen (coming from info window) */
static void
list_win_refresh()
{
	wclear(list_win);
	wrefresh(list_win);
	wresize(list_win, nlines + 2, ncols);
}

/* parse directory listing */
static int
print_list(Etree **etree, HL_datas *hl, char *path, const int where)
{
	int i = 0;
	char buf[MAXLEN], *ppath, *item;

	/* don't show path when listing PKGDB */
	if (where == IN_PKGDB)
		draw_box(list_win, INST_PKGS);

	/* etree == NULL, no directories found */
	if (etree == NULL || etree[0] == NULL) {
		/* there was no entries, are we in a pkg dir ? */
		snprintf(buf, MAXLEN, "%s/Makefile", path);
		if ((item = strrchr(path, '/')) == NULL)
			/* should never happen */
			return(IN_DESCR);
		item++;
		
		if (where == IN_FTP || file_exists(buf)) {
			/* remember initial path */
			ppath = path;
			/* switch to information window */
			i = info_win(list_win, item, path);
		}
		/* back from information window */
		/* clean and restore window size */
		list_win_refresh();

		/* return IN_QUIT from info window, quit*/
		if (i == IN_QUIT)
			/* etree == NULL, nothing to free() */
			finish(0);

		if (where == IN_PKGDB)
			/* redraw borders */
			draw_box(list_win, INST_PKGS);
		
		if (where == IN_DEPENDS) {
			if ((ppath = strstr(path, "/..")) != NULL)
				/* dependency */
				*ppath = '\0';
			else
				/* pkgfind */
				strcpy(path, pkgsrcbase);

			draw_box(list_win, path);
		}

		if (where == IN_PKGSRC || where == IN_SYSINST) {
			/* rewind to parent dir */
			if ((ppath = strrchr(path, '/')) != NULL)
				*ppath = '\0';
			
			/* redraw borders */
			draw_box(list_win, path);
		}
		/* jump back to main loop with etree = NULL */
		return(IN_DESCR);

	} /* etree == NULL */

	/* draw the combo list */
	return(combo_list(list_win, etree, hl, path));
}

/* print key shortcuts */
static void
print_bindings(int page)
{
	int spacing, up, down, i;
	static int last_page = 1;

#define MAXCMDPAGES 2

	spacing = strlen(ps_installed.descr);
	up = nlines + 3;
	down = nlines + 4;

	if (page != last_page) {
		/* clear last 2 lines, nicer than a clrscr() */
		for (i = 0; i < ncols; i++) {
			mvprintw(LINES - 2, i, " ");
			mvprintw(LINES - 3, i, " ");
		}
	}

	switch (page) {
	case 1:
		print_kb(ps_enter.icon, ps_enter.descr, up, 2);
		print_kb(ps_back.icon, ps_back.descr, down, 2);
		print_kb(ps_search.icon, ps_search.descr, up, spacing);
		print_kb(ps_quit.icon, ps_quit.descr, down, spacing);
		print_kb(ps_menu.icon, ps_menu.descr, up, spacing * 2);
		print_kb(ps_find.icon, ps_find.descr, down, spacing * 2);
		print_kb(ps_other.icon, ps_other.descr, up, spacing * 3);
		print_kb(ps_installed.icon, ps_installed.descr, 
			 down, spacing * 3);
		break;
	case 2:
		print_kb(ps_update.icon, ps_update.descr, up, 2);
		print_kb(ps_tag.icon, ps_tag.descr, down, 2);
		print_kb(ps_install.icon, ps_install.descr, up, spacing * 1.5);
		print_kb(ps_deinstall.icon, ps_deinstall.descr, down, 
			 spacing * 1.5);
		print_kb(ps_prefs.icon, ps_prefs.descr, down, spacing * 3);
		print_kb(ps_other.icon, ps_other.descr, up, spacing * 3);

		break;
	}
	
	refresh();
	last_page = page;
}

/*
 * non-directory based loop, 
 * we build etree with **list, a char ** flat list
 * comments are built from *path/
 * this is used for pkgfind() and show deps
 */
void
nodir_loop(const char *path, char **list)
{
	Etree **etree;

	if (list != NULL && list[0] != '\0') {
		/* call main loop with tree built from deps */
		etree = get_nodir_tree(path, list);
		if (etree != NULL)
			etree = main_loop(etree, list, path, IN_DEPENDS);
		/* at this point etree should be NULL */
	}
}

Etree **
main_loop(Etree **etree, char **list, const char *basepath, const int where)
{
	int c, tmp, page;
	Etree **etree_sav;
	HL_datas hl;
	char wpath[MAXLEN], tstr[MAXLEN], *p, **pkglist;

	/* init highlight index */
	hl.hl_index = hl.old_index = 0;
	/* save screen props */
	hl.nlines = nlines;
	hl.ncols = ncols;
	hl.top_line = 1;

	/* init working path */
	strcpy(wpath, basepath);

	/* we are called from show-deps, clean display from info window */
	if (where == IN_DEPENDS)
		list_win_refresh();

	/* redraw box */
	draw_box(list_win, wpath);

	/* command page */
	page = 1;

	/* main loop */
	for (;;) {
		WINDOW *popup;

		if (print_list(etree, &hl, wpath, where) == IN_DESCR)
			/* recursive call led to DESCR, return NULL */
			return(NULL);

		/* print bindings */
		print_bindings(page);

		c = wgetch(list_win);
		switch(c) {
			
			/* macro defining up, down, pgup and pgdown */
			BASIC_NAV

		case KEY_RIGHT:
		case KEY_ENTER:
				
			if (where == IN_FTP) {
				/* save etree pointer */
				etree_sav = etree;
				
				snprintf(wpath, MAXLEN, "%s/%s",
					 basepath, hl.hl_entry);

				etree = live_ftp(wpath);
				etree = main_loop(etree, NULL, wpath, IN_FTP);
				/* restore etree */
				etree = etree_sav;
				break;
			}

			if (where == IN_PKGDB) {
				/* point to corresponding pkgdb line */
				p = getpkginfo(hl.hl_entry, PKG_CATEGORY);
				if (p == NULL)
					break;
				snprintf(wpath, MAXLEN, "%s/%s",
					 pkgsrcbase, p);
			}

			if (where == IN_DEPENDS)
				snprintf(wpath, MAXLEN, "%s/%s",
					 basepath, 
					 etree[hl.hl_index]->dep_path);

			if (where == IN_PKGSRC || where == IN_SYSINST)
				snprintf(wpath, MAXLEN, "%s/%s", 
					 basepath, hl.hl_entry);
			
			/* display a wait message */
			popup = mid_info_popup("parsing pkgsrc", PARSE_PKGSRC);
			if (popup != NULL)
				wrefresh(popup);

			/* load new basepath tree */
			clear_tree(&etree, where);
			etree = get_tree(wpath, where);

			/* re-enter main_loop with new tree */
			etree = main_loop(etree, NULL, wpath, where);
			/* back from recursion with NULL etree */
			
			break;
		case KEY_LEFT:
			if (!toplevel(basepath) || where == IN_DEPENDS) {
				clear_tree(&etree, where);
				return(NULL);
			}

			break;
		case 'n':
			tmp= entry_search(etree,1);
			if (tmp >= 0)
				hl.hl_index = tmp;
			break;
		case 'm':
			mmenu = T_TRUE;
			clear_tree(&etree, where);
			return(NULL);
			break;
		case '/':
			tmp = entry_search(etree, 0);
			if (tmp >= 0)
				hl.hl_index = tmp;
			break;
		case 'f':
			/* show popup, p gets result */
			p = getstr_popup("pkgfind", 5, 30, 
					   (LINES / 2) - 2, (COLS / 2) - 15);
			/* show waiting popup */
			popup = mid_info_popup("pkgfind", "searching...");
			wrefresh(popup);
			/* split p into malloc'ed char * pieces */
			pkglist = pkgfind(pkgsrcbase, p, 0);
			/* destroy wait popup */
			clr_del_win(popup);
			XFREE(p);
			if (pkglist == NULL) {
				(void) mid_getch_popup("pkgfind",
						       NO_PKG_FOUND);
				break;
			}
			/* non dir-listing loop */
			nodir_loop(pkgsrcbase, pkglist);
			/* back from pkgfind browsing, free the pkglist */
			free_list(&pkglist);
			break;
		case 'l':
			if (where != IN_PKGDB) {
				/* set basepath to pkgdb_dir */
				strncpy(wpath, pkg_dbdir, 
					strlen(pkg_dbdir) + 1);
				clear_tree(&etree, IN_PKGDB);
				etree = get_tree(wpath, IN_PKGDB);
				/* recurse */
				etree = main_loop(etree, NULL, 
						  wpath, IN_PKGDB);
			}
			break;
		case 't':
			if (!toplevel(basepath)) {
				snprintf(tstr, MAXLEN, "%s/%s",
					 basepath, hl.hl_entry);
				add_pkg(tstr);
			}
			break;
		case 'i':
			clr_allscr(list_win);
			process_many(COMBO_INST);
			/* this overwrites current box, redraw */
			clr_allscr(list_win);
			draw_box(list_win, basepath);
			break;
		case 'd':
			clr_allscr(list_win);
			process_many(COMBO_DEINST);
			/* this overwrites current box, redraw */
			clr_allscr(list_win);
			draw_box(list_win, basepath);
			break;
		case 'u':
			clear_tree(&etree, where);
			cvs_up(basepath);
			/* safety clear */
			break;
		case 'p':
			clear_tree(&etree, where);
			prefs_screen();
			clr_allscr(list_win);
			break;
		case 'o':
			if (page < MAXCMDPAGES)
				page++;
			else
				page = 1;
			break;
		case 'q':
			clear_tree(&etree, where);
			return(NULL);
			break;
		} /* switch */

		/* if we return from DESCR section, etree == NULL */
		if (etree == NULL) {
			/* coming from DESCR, show basepath (parent) */
			if (where == IN_DEPENDS)
				etree = get_nodir_tree(basepath, list);
			else if (where == IN_FTP)
				etree = live_ftp(basepath);
			else
				etree = get_tree(basepath, where);

		}
		/* redraw border */
		draw_box(list_win, basepath);
	} /* for (;;) */
	/* NOTREACHED */
}

int
main(int argc, char *argv[])
{
	Etree **etree;
	char *confpath, *ftp_url, *dl_path, *env_pkg = NULL;
	char basepath[MAXLEN];
	int where, use_live_ftp, read_makefiles;
	int ch, console_output, retmenu;
	WINDOW *popup;

	pkgsrcbase = pkg_dbdir = confpath = ftp_url = dl_path = NULL;
	use_live_ftp = console_output = mmenu = T_FALSE;
	read_makefiles = T_TRUE; /* live_ftp option */

	/* command line handling */
	while ((ch = getopt(argc, argv, "c:b:K:u:d:lmshi")) != -1)
		switch(ch) {
		case 'b':
			pkgsrcbase = optarg;
			break;
		case 'K':
			pkg_dbdir = optarg;
			break;
		case 'c':
			confpath = optarg;
			break;
		case 'l':
			use_live_ftp = 1;
		case 'u':
			XSTRDUP(ftp_url, optarg);
			break;
		case 'm':
			read_makefiles = T_FALSE;
			break;
		case 'd':
			dl_path = optarg;
			break;
		case 's':
			console_output = T_TRUE;
			break;
		case 'i':
			mmenu = T_TRUE;
			break;
		case 'h':
			usage();
			/* NOTREACHED */
		}

        argc -= optind;
        argv += optind;
	/* end of command line handlig */

	/* confpath given to command line or NULL*/
	if (confpath == NULL) {
		conf.confpath = CONFPATH;
	} else
		conf.confpath = confpath;

	loadconf();

	if (pkgsrcbase == NULL) { /* there was no -b flag */
		if (conf.pkgsrcdir != NULL) /* conf file ? */
			pkgsrcbase = conf.pkgsrcdir;
		else
			/* env variable ? */
			if ((pkgsrcbase = getenv("PKGSRCDIR")) == NULL)
				/* default basepath */
				pkgsrcbase = PKGSRCBASE;
	}
	conf.pkgsrcdir = pkgsrcbase;

	if (pkg_dbdir == NULL) { /* no -K flag */
		if (conf.pkg_dbdir != NULL)
			pkg_dbdir = conf.pkg_dbdir;
		else
			if ((pkg_dbdir = getenv("PKG_DBDIR")) == NULL)
				/* default pkg_dbdir */
				pkg_dbdir = PKGDB;
	}
	conf.pkg_dbdir = pkg_dbdir;

	if (console_output == T_TRUE)
		conf.shell_output = T_TRUE;

	/* default basepath */
	strncpy(basepath, pkgsrcbase, MAXLEN);

	signal(SIGINT, finish);

        /* init ncurses */
	init_curses();

	/* set BG color */
	if (has_colors() == TRUE) {
		start_color();
		init_pair(1, FGCOLOR, BGCOLOR);

		bkgd(COLOR_PAIR(1));
		attrset(COLOR_PAIR(1));

		clear();
		refresh();
	}

	/* init different windows */
	init_windows();

	/* set BG color */
	set_colors(list_win);

	/* enable KEY_* */
	keypad(list_win, TRUE);

	popup = mid_info_popup("loading pkgdb", LOAD_PKGDB);
	wrefresh(popup);
	/* load installed package list */
	load_pkgdb();

	clr_del_win(popup);

	/* enter main menu mode */
menuep:
	if (mmenu) {
		clear();
		refresh();
		wclear(list_win);
		wrefresh(list_win);

		retmenu = mainmenu();

		switch(retmenu) {
		case MENU_QUIT:
			finish(0);
			break;
		case MENU_USE_FTP:
			use_live_ftp = T_TRUE;
			break;
		case MENU_USE_PACKAGES:
			if (getenv("PACKAGES") == NULL) {
				env_pkg = 
					mid_getstr_popup("packages path");
				setenv("PACKAGES", env_pkg, 1);
			}
			break;
		}
	}

	/* pkg_select was called with -l, entering live FTP mode */
	if (use_live_ftp) { /* had a -l flag */
		/* ensure basepath is empty */
		memset(basepath, 0, MAXLEN);

		if (read_makefiles == T_FALSE)
			/* dont read makefiles (no comments) */
			conf.live_ftp_read_makefiles = T_FALSE;

		if (ftp_url == NULL) { /* had no -u flag, try conf */
			if (conf.live_ftp_pkgsrc != NULL)
				XSTRDUP(ftp_url, conf.live_ftp_pkgsrc);
			else { /* give mirror list */
				if ((ftp_url = list_mirrors("ftp")) == NULL)
					finish(0);
				snprintf(basepath, MAXLEN,
					 "%s/NetBSD-current/pkgsrc", ftp_url);
				XFREE(ftp_url);
				XSTRDUP(ftp_url, basepath);
			}
		} else {
			int len;
			/* validate url as ftp://host */
			len = strlen(ftp_url) - 1;
			/* flush terminating /'s */
			for (; len > 0 && ftp_url[len] == '/'; len--)
				ftp_url[len] = '\0';
			/* ftp_url has no beginning ftp:// */
			if (strncmp("ftp://", ftp_url, 6) != 0)
				snprintf(basepath, MAXLEN, 
					 "ftp://%s", ftp_url);
		}

		where = IN_FTP;
		/* basepath was not filled by previous snprintf */
		if (basepath[0] == '\0')
			XSTRCPY(basepath, ftp_url);

		conf.live_ftp = ftp_url;
		if ((etree = live_ftp(basepath)) == NULL) {
			warn("error while negociating with %s", basepath);
			finish(0);
		}
		/* set pkgsrc base to ftp base */
		pkgsrcbase = conf.live_ftp;
	} /* end if ftp */ else if (getenv("PACKAGES") != NULL) {
		char **rmout, rmcmd[MIDLEN];

		where = IN_SYSINST;
		/* as of 12 / 2005, this feature allows only local packages
		 * manipulation, but it could quite easily do the same with 
		 * remote packages (see live ftp feature)
		 */
		setenv("PKG_PATH", getenv("PACKAGES"), 1);

		/* used to fill PACKAGES variable in menu mode */
		XFREE(env_pkg);

		if (getenv("PKGSRCDIR") == NULL)
			setenv("PKGSRCDIR", TMPPKGSRCDIR, 1);
		else {
			snprintf(rmcmd, MIDLEN, 
				 WARN_RM_PKGSRC,
				 getenv("PKGSRCDIR"));
			if (mid_getch_popup("warning", rmcmd) != 'y') {
				if (mmenu)
					goto menuep;
				else
					finish(0);
			}
		}
		XSTRCPY(basepath, getenv("PKGSRCDIR"));

		/* remove any existing tmp pkgsrc */
		snprintf(rmcmd, MIDLEN, "%s -rf %s", RM, getenv("PKGSRCDIR"));
		/* we really don't want to cut/paste rm.c... */
		rmout = exec_list(rmcmd, NULL);
		/* we don't need output */
		free_list(&rmout);

		popup = mid_info_popup("message",
				       BUILD_CATS);
		wrefresh(popup);

		if (mkpkgsrc(getenv("PKG_PATH")) < 0)
			err(EXIT_FAILURE, "%s", getenv("PKG_PATH"));

		/* delete please wait popup */
		clr_del_win(popup);

		/* read etree from binaries directory*/
		etree = get_tree(basepath, where);
	} /* end sysinst mode */ else {
		/* was pkgsrc download asked ? */
		if (dl_path != NULL) {
			if (download_pkgsrc(dl_path, basepath) < 0) {
				warnx("pkgsrc download failed");
				finish(0);
			}
		} else
			/* check for pkgsrc presence */
			if (pkgsrc_chk(basepath, 0) < 0) {
				warnx("%s does not exists", basepath);
				/* pkgsrc fetch failed */
				finish(0);
			}
	
		where = IN_PKGSRC;
		/* display a wait message */
		popup = mid_info_popup("parsing pkgsrc", PARSE_PKGSRC);
		wrefresh(popup);

		/* get directory listing */
		etree = get_tree(basepath, where);
		/* destroy wait popup */
		clr_del_win(popup);
	}

	/* enter browser */
	(void) main_loop(etree, NULL, basepath, where);
	if (mmenu)
		/* i know what you're saying, but think about it, 2 choices
		 * 1st: have a big fat while full of conditions, indenting the
		 * code a bit more and so complexifying main function
		 * 2d: goto
		 * got it ?
		 */
		goto menuep;

	finish(0);
	/* NOTREACHED */
	exit(EXIT_SUCCESS);
}
