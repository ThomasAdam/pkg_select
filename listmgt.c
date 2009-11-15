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
 * $Id: listmgt.c,v 1.4 2005/12/11 17:11:33 imil Exp $ 
 */

#include "pkg_select.h"

int
find_value(Etree **etree, char *pattern)
{
	int i;

	for (i = 0; etree[i] != NULL; i++)
		if (strncmp(etree[i]->comment, pattern, strlen(pattern)) == 0)
			return(i);

	return(-1);
}

static int
find_entry(Etree **etree, char *pattern, int skip)
{
	int i;

	for (i = 0; etree[i] != NULL; i++)
		if (((strstr(etree[i]->entry, pattern) != NULL) ||
		     (strstr(etree[i]->comment, pattern) != NULL)) && 
		     skip-- == 0) 
			return(i);

	return(-1);
}

int
entry_search(Etree **etree, int cont)
{
	int i;
	static int skippy = 0;
	static char *key = NULL;

	i = -1;

	if (cont) {
		/* this is a next search (by fab) */
		if (key != NULL) {
			skippy++;
			i = find_entry(etree, key, skippy);
		}
	} else {
		if (key) {
			XFREE(key);
			key=NULL;
		}
		skippy=0;
		key = getstr_popup("search", 5, 30, 
				   (LINES / 2) - 2, (COLS / 2) - 15);
		i = find_entry(etree, key, skippy);
	}

	return(i);
}

Etree **
build_tree_from_list(const char **list)
{
	int i, count, len;
	char *p;
	Etree **etree;

	for (i = 0; list[i] != NULL; i++);
	count = i;

	XMALLOC(etree, (count + 1) * sizeof(Etree *));

	for (i = 0; i < count; i++) {
		XMALLOC(etree[i], sizeof(Etree));
		
		etree[i]->dep_path = NULL;
		XSTRDUP(etree[i]->entry, list[i]);
		/* get rid of \n's */
		trimcr(etree[i]->entry);

		if ((p = strchr(etree[i]->entry, ':')) != NULL) {
			*p = '\0';
			p++;
			
			XSTRDUP(etree[i]->comment, p);
			len = strlen(etree[i]->comment) - 1;
			/* strip any trailing  / */
			if ((etree[i]->comment)[len] == '/')
				(etree[i]->comment)[len] = '\0';

		} else {
			/* there were no comment, invert */
			etree[i]->comment = etree[i]->entry;
			etree[i]->entry = NULL;
		}
	}
	etree[i] = NULL;

	return(etree);
}
