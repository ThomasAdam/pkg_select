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
 * $Id: pkg_info.c,v 1.23 2009/03/07 21:29:40 imil Exp $ 
 */

#include "pkg_select.h"

static int show_tarfile(WINDOW *, const char *);
static void syspkg_install(WINDOW *, const char *);
static void syspkg_deinstall(WINDOW *, const char *);
static int show_pkgfile(WINDOW *, char *, const char *);
static void pkg_popup(WINDOW *, char *, char *, 
		      const char *, const char *, const char *);
static void pkg_depends(char *);

/* system archives */

static int
show_tarfile(WINDOW *win, const char *file)
{
	int ret;
	char **tarlist, buf[MIDLEN];
	WINDOW *popup;

	set_pkg_path(conf.pkg_path);
	snprintf(buf, MIDLEN, "%s ztf %s/%s.tgz", 
		 TAR_CMD, getenv("PKG_PATH"), file);

	popup = mid_info_popup(file, SYSPKG_LIST);
	wrefresh(popup);

	if ((tarlist = exec_list(buf, NULL)) == NULL)
		return(-1);

	clr_del_win(popup);

	ret = more_list(win, tarlist, LINES - 2, COLS - 2, 1, 1);

	free_list(&tarlist);
	return(ret);
}

static void
syspkg_install(WINDOW *win, const char *file)
{
	char rep;
	char **tarlist, buf[MIDLEN];
	WINDOW *popup;

	rep = mid_getch_popup(file, SYSPKG_INSTALL);
	if (rep != 'y')
		return;

	/* CWD to / */
	if (getenv("ROOTDIR") == NULL)
		setenv("ROOTDIR", ROOTDIR, 1);
	chdir(getenv("ROOTDIR"));

	snprintf(buf, MIDLEN, "%s zxfp %s/%s.tgz", 
		 TAR_CMD, getenv("PKG_PATH"), file);

	popup = mid_info_popup(file, SYSPKG_EXTRACT);
	wrefresh(popup);

	if ((tarlist = exec_list(buf, NULL)) == NULL)
		free_list(&tarlist);

	clr_del_win(popup);
}

static void
syspkg_deinstall(WINDOW *win, const char *file)
{
	int i;
	char rep;
	char **tarlist, **rmcmd, buf[MIDLEN];
	WINDOW *popup;

	rep = mid_getch_popup(file, SYSPKG_DEINSTALL);
	if (rep != 'y')
		return;

	/* CWD to / */
	if (getenv("ROOTDIR") == NULL)
		setenv("ROOTDIR", ROOTDIR, 1);
	chdir(getenv("ROOTDIR"));

	snprintf(buf, MIDLEN, "%s ztf %s/%s.tgz", 
		 TAR_CMD, getenv("PKG_PATH"), file);

	popup = mid_info_popup(file, SYSPKG_RM);
	wrefresh(popup);

	if ((tarlist = exec_list(buf, NULL)) != NULL) {
		for (i = 0; tarlist[i] != NULL; i++) {
			snprintf(buf, MAXLEN, "%s -f %s 2> /dev/null",
				 RM, tarlist[i]);
			rmcmd = exec_list(buf, NULL);
			/* unused output */
			free_list(&rmcmd);
		}
		free_list(&tarlist);
	}

	clr_del_win(popup);
}

/* classic packages */

static int
show_pkgfile(WINDOW *win, char *path, const char *file)
{
	char buf[MAXLEN];

	if (is_ftpurl(path)) {
		char **ftpfile = NULL, rc = 0;

		/* build full path */
		snprintf(buf, MAXLEN, "%s/", path);
		if (ftp_info_start(buf) < 0)
			return(-1);

		if ((ftpfile = ftp_loadfile("./", file)) != NULL) {
			rc = more_list(win, ftpfile, 
				       LINES - 2, COLS - 2, 1, 1);
			freefile(ftpfile);
		}

		ftp_stop();
		return(rc);
	}

	snprintf(buf, MAXLEN, "%s/%s", path, file);
	/* if 'q' was pressed, return 0 so we quit */
	return(more_file(win, buf, LINES - 2, COLS - 2, 1, 1));
}

static void
pkg_popup(WINDOW *win, char *path, char *pkg, 
	  const char *label, const char *msg, const char *action)
{
	int len;
	char rep;

	len = strlen(msg) + 4;
	rep = getch_popup(label, msg, 
			  5, len, 
			  (LINES / 2) - 2, (COLS / 2) - (len / 2));

	if (rep != 'n') {
		/* recursive pkg_delete */
		if (*action == 'd' && rep == 'r')
			pkg_tool("delete", pkg, "-vr", WAIT_KEY);

		/* make action: source install / upgrade */
		if (*action == 'i' || *action == 'u')
			pkgsrc_make(action, path, WAIT_KEY);

		/* pkg_(add|delete) */
		if (*action == 'a' || *action == 'd') {
			if (*action == 'a')
				/* adding a package, set PKG_PATH */
				set_pkg_path(conf.pkg_path);
			pkg_tool(action, pkg, "-v", WAIT_KEY);
		} /* answer was yes */

		wclear(win);
		wrefresh(win);
	}
}

static void
pkg_depends(char *path)
{
	int len;
	char **list;
	WINDOW *popup;
	char buf[MAXLEN];

	snprintf(buf, MAXLEN, "%s dependencies", PLEASE_WAIT_CALC);
	len = strlen(buf) + 4;

	popup = info_popup("information", buf, 
			   5, len, 
			  (LINES / 2) - 2, (COLS / 2) - (len / 2));
	wrefresh(popup);

	snprintf(buf, MAXLEN, "cd %s && %s show-depends", path, MAKE);
	
	list = exec_list(buf, NULL);
	/* enter main_loop */
	nodir_loop(path, list);

	free_list(&list);
	delwin(popup);
}

int
info_win(WINDOW *win, char *pkg, char *path)
{
	int ret, c, line_index, msv, len, in_ftp;
	struct cf *file;
	char buf[MAXLEN], *homepage, *comment, *vmakefile, *syspkg,
		*distname, *version, *maintainer, *p;
	WINDOW *popup;

	snprintf(buf, MAXLEN, "%s/MESSAGE", path);
	
	/* info window needs all screen */
	wresize(win, LINES - 2, COLS - 2);
	
	for (;;) {
		/* load Makefile* */
		if (strncmp(path, "ftp://", 6) == 0) {
			in_ftp = 1;
			if (ftp_info_start(path) < 0)
				return(0);
			else
				file = ftp_loadcf("./", "Makefile");

			/* clean path for display */
			p = &path[6];
			if ((p = strchr(p, '/')) != NULL &&
			    (p = strchr(p, '.')) != NULL)
				*p = '\0';

			if ((p = strrchr(pkg, '/')) != NULL)
				*p = '\0';

			ftp_stop();
		} else {
			in_ftp = 0;
			file = load_makefile(path, FULL);
		}
		line_index = 2;

		/* redraw box and title */
		draw_box(win, path);

		mvwprintw(win, line_index, 2, PKG_ADMIN_INFO);
		wattroff(win, A_BOLD);
		line_index += 2;

		/* get values from Makefile slist */
		distname = getval(file, "DISTNAME=");
		homepage = getval(file, "HOMEPAGE=");
		maintainer = getval(file, "MAINTAINER=");
		/* real pkgsrc Makefile or generated using binary */
		vmakefile = getval(file, "VIRTUAL_MAKEFILE=");
		syspkg = getval(file, "SYSTEM_PKG=");

		comment = getcomment(file, (const char *)path);
		
		msv = 0;
		/* one or more members could not be read from Makefile 
		 * use make show-var to fill all of them
		 */
		if (!in_ftp && (distname == NULL || comment == NULL)) {
			len = strlen(SEARCH_INFOS) + 4;

			/* this cound take a moment, warn user */
			popup = info_popup("information", 
					   SEARCH_INFOS, 
					   5, len, 
					   (LINES / 2) - 2, 
					   (COLS / 2) - (len / 2));
			wrefresh(popup);

			distname = show_first_var(path, "DISTNAME");
			homepage = show_first_var(path, "HOMEPAGE");
			comment = show_first_var(path, "COMMENT");
			maintainer = show_first_var(path, "MAINTAINER=");
			/* set make show-var variable */
			msv = 1;

			delwin(popup);
		}

		if (distname == NULL) {
			wprint_kb(win, "package:", "none", line_index, 2);
		} else
			wprint_kb(win, "distname:", distname, line_index, 2);
		line_index++;

		if (syspkg == NULL) {
			if ((version = getpkginfo(pkg, PKG_VERSION)) != NULL)
				wprint_kb(win, "installed:", 
					  version, line_index, 2);
			else
				wprint_kb(win, "installed:", "none", 
					  line_index, 2);
			line_index++;
		}

		if (homepage != NULL) {
			wprint_kb(win, "homepage:", homepage, line_index, 2);
			line_index++;
		}

		if (maintainer != NULL) {
			wprint_kb(win, "maintainer:", maintainer, 
				  line_index, 2);
			line_index++;
		}

		if (comment == NULL)
			wprint_kb(win, "comment:", "none", line_index, 2);
		else {
			/* 7 == strlen("comment:") + 2 borders + 
			   "..." + 2 spaces */
			cut_str(comment, COLS - 17);
			wprint_kb(win, "comment:", comment, line_index, 2);
		}
		line_index += 2;
		
		wprint_kb(win, "[m]", "show package description", 
			  line_index, 2);
		line_index++;

		if (file_exists(buf)) {
			wprint_kb(win, "[v]", 
				  "view package message", line_index, 2);
			line_index++;
		}
		wprint_kb(win, "[p]", "view package files list", 
			  line_index, 2);
		line_index++;

		if (!in_ftp && vmakefile == NULL) {
			wprint_kb(win, "[e]", "show package dependancies",
				  line_index, 2);
			line_index += 2;

			wprint_kb(win, "[i]", "build and install package",
				  line_index, 2);
			line_index++;
			wprint_kb(win, "[u]", "build and upgrade package",
				  line_index, 2);
			line_index++;
		}

		wprint_kb(win, "[b]", "install binary package", line_index, 2);
		line_index++;

		wprint_kb(win, "[d]", "de-install package", line_index, 2);

		wprint_kb(win, "[<-]", "back", LINES - BOTTOM_KB, 2);
		wprint_kb(win, "[q]", "quit", LINES - BOTTOM_KB, 14);
	
		/* we got infos from make show-var, must free */
		if (msv) {
			XFREE(distname);
			XFREE(homepage);
			XFREE(comment);
			XFREE(maintainer);
		}
	
		c = wgetch(win);
		switch (c) {
		case 'm':
		case KEY_RIGHT:
			ret = show_pkgfile(win, path, "DESCR");
			if (!ret)
				/* quit was asked */
				return(ret);
			break;
		case 'v':
			ret = show_pkgfile(win, path, "MESSAGE");
			if (!ret)
				/* quit was asked */
				return(ret);
			break;
		case 'p':
			if (syspkg != NULL)
				ret = show_tarfile(win, pkg);
			else
				ret = show_pkgfile(win, path, "PLIST");
			if (!ret)
				/* quit was asked */
				return(ret);
			break;
		case 'i':
			pkg_popup(win, path, pkg, 
				  "Install package", 
				  "build and install package ? [Y/n]",
				  "install");
			break;
		case 'u':
			pkg_popup(win, path, pkg, 
				  "Upgrade package", 
				  "build and upgrade package ? [Y/n]",
				  "update");
			break;
		case 'd':
			if (syspkg != NULL)
				syspkg_deinstall(win, pkg);
			else
				pkg_popup(win, path, pkg, 
					  "De-install package", 
					  "de-install package ? [Y/n/r(ecursive)]",
					  "delete");
			break;
		case 'b':
			if (syspkg != NULL)
				syspkg_install(win, pkg);
			else
				pkg_popup(win, path, pkg, 
					  "Install package",
					  "install binary package ? [Y/n]",
					  "add");
			break;
		case 'e':
			pkg_depends(path);
			/* back from main_loop, resize */
			wresize(win, LINES - 2, COLS - 2);

			break;
		case 'q':
			/* if we are on a depend list, we want a clean exit */
			freecf(file);
			return(IN_QUIT);

			break;
		case KEY_LEFT:
			freecf(file);
			return(1);
			break;
		}

	}
	/* NOTREACHED */
	return(1);
}
