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
 *
 * $Id: texts.h,v 1.18 2005/12/16 09:47:33 imil Exp $ 
 */

#ifndef _TEXTS_H_
#define _TEXTS_H_

/* ui phrases */

#define LOAD_PKGDB "please wait while loading package database..."
#define PARSE_PKGSRC "please wait while parsing pkgsrc..."
#define PKG_ADMIN_INFO "Package administration and informations"
#define PLEASE_WAIT_CALC "please wait while calculating"
#define INST_PKGS "installed packages"
#define SEARCH_INFOS "searching infos..."
#define PKGSRC_NOTFOUND "pkgsrc was not found in %s, download it ? [Y/n]"
#define DL_METHOD "Download method: (f)tp  / (c)vs / (d)irectory cp ?"
#define FETCH_FAILED "pkgsrc fetch failed, retry ? [Y/n]"
#define UPDT_FAILED "pkgsrc update failed, retry ? [Y/n]"
#define NEXT ">> Next"
#define PKG_TB_INST "packages to be installed"
#define PKG_TB_DEINST "packages to be deinstalled"
#define REALLY_MASS_DELETE "really delete these packages ? [Y/n/r(ecursive)]"
#define CVS_CONNECT "connecting to cvs server..."
#define CVS_COMPARE "comparing trees, please wait..."
#define WARN_DOWN_URL "download url must be one of ftp://, cvs:// or file://"
#define NO_PKG_FOUND "no package found, press any key to continue"
#define BUILD_CATS "please wait while building categories"
#define SYSPKG_LIST "please wait while retrieving contents of archive"
#define SYSPKG_EXTRACT "please wait while extracting archive to installation media"
#define SYSPKG_RM "please wait while deleting system package"
#define SYSPKG_INSTALL "are you sure you want to install this system package ? [y/N]"
#define SYSPKG_DEINSTALL "are you sure you want to UNinstall this system package ? [y/N]"
#define WARN_RM_PKGSRC "directory %s is about to be deleted, continue ? [y/N]"

/* shortcuts */

typedef const struct Shortcut {
	const char *icon;
	const char *descr;
} Shortcut;

#define SHORTCUT static Shortcut

SHORTCUT ps_enter = { "[ret]", "enter" };
SHORTCUT ps_back = { "[<-]", "back" };
SHORTCUT ps_search = { "[/]", "search" };
SHORTCUT ps_next = { "[n]", "next" };
SHORTCUT ps_menu = { "[m]", "menu" };
SHORTCUT ps_quit = { "[q]", "quit" };
SHORTCUT ps_installed = { "[l]", INST_PKGS };
SHORTCUT ps_up = { "[up]", "scroll up" };
SHORTCUT ps_down = { "[down]", "scroll down" };
SHORTCUT ps_find = { "[f]", "pkgfind" };
SHORTCUT ps_other = { "[o]", "other cmds" };
SHORTCUT ps_tag = { "[t]", "tag / untag" };
SHORTCUT ps_install = { "[i]", "install tagged" };
SHORTCUT ps_deinstall = { "[d]", "de-inst tagged" };
SHORTCUT ps_update = { "[u]", "update pkgsrc" };
SHORTCUT ps_prefs = { "[p]", "preferences" };

#endif /* _TEXTS_H_ */
