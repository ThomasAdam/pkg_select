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
 * $Id: menus.c,v 1.3 2005/12/16 09:47:33 imil Exp $ 
 */

#include "pkg_select.h"
#include "menus.h" 

static char *mkmenu(const char **, const char *);
static int binmaint(void);
static int pkgsrcmaint(void);

static char *
mkmenu(const char **menu, const char *title)
{
	int i, x, y, h, w;
	char *resp;
	Etree **etree;

	for (i = 0; menu[i] != NULL; i++);

	w = COLS - 10;
	h = i;
	x = (COLS - w) / 2;
	y = (LINES - h) / 2;

	etree = build_tree_from_list(menu);
	resp = combo_select(etree, title, h, w, y - 1, x, 1);

	free_tree(&etree);
	return(resp);
}

static int
binmaint(void)
{
	char *resp;
	int retmenu = MENU_QUIT; /* default action is quit */

	/* resp == NULL -> end */
	while ((resp = mkmenu(bin_menu, "binary packages")) != NULL) {
		switch(resp[0]) {
		case 'r':
			retmenu = MENU_QUIT;
			goto end;
			break;
		case 'p':
			retmenu = MENU_USE_FTP;
			goto end;
			break;
		case 'l':
			retmenu = MENU_USE_PACKAGES;
			goto end;
			break;
		}
	}
	if (resp == NULL)
		retmenu = MENU_QUIT;
end:
	XFREE(resp);
	return(retmenu);
}

int
pkgsrcmaint(void)
{
	char *resp;
	int retmenu = MENU_QUIT; /* default action is quit */

	/* set pkgsrc path */
	if (getenv("PKGSRCDIR") == NULL)
		/* this implies a warning from main when choosing
		 * binary update because it rm -rf ${PKGSRCDIR} 
		 * really not an issue imho
		 */
		setenv("PKGSRCDIR", "/usr/pkgsrc", 1);

	while ((resp = mkmenu(pkgsrc_menu, 
				   "pkgsrc maintenance")) != NULL) {
		switch(resp[0]) {
		case 'r':
			retmenu = MENU_QUIT;
			goto end;
			break;
		case 'd':
			retmenu = MENU_DOWNLOAD_PKGSRC;
			pkgsrc_chk(getenv("PKGSRCDIR"), 1);
			break;
		case 'u':
			retmenu = MENU_UPDATE_PKGSRC;
			cvs_up(getenv("PKGSRCDIR"));
			break;
		}
	}
	if (resp == NULL)
		retmenu = MENU_QUIT;

end:
	XFREE(resp);
	return(retmenu);
}

int
mainmenu(void)
{
	char *resp;
	int retmenu = MENU_QUIT; /* default action is quit */

	/* resp == NULL -> end */
	while ((resp = mkmenu(main_menu, "main menu")) != NULL) {
		switch(resp[0]) {
		case 'q':
			retmenu = MENU_QUIT;
			goto end;
			break;
		case 'c':
			retmenu = MENU_CONT;
			goto end;
			break;
		case 'b':
			/* binary mode goes back to main, we must 
			 * transport menu actions to it
			 */
			if ((retmenu = binmaint()) != MENU_QUIT)
				goto end;
			break;
		case 'p':
			/* pkgsrc maint mode is dealt locally */
			(void) pkgsrcmaint();
			break;

		}
	}

end:
	XFREE(resp);
	return(retmenu);
}
