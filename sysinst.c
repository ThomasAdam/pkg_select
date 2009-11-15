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
 * $Id: sysinst.c,v 1.7 2005/12/06 09:33:21 imil Exp $ 
 */

#include "pkg_select.h"
#include "basesets.h"

static char *findpkgval(const char *, char **);
static void mkmakefile(const char *, const char *, char *);
static void mkpkgfile(const char *, 
		      const char *,
		      const char *, 
		      const char, 
		      const char *,
		      const char *);

static int isbaseset(const char *);
static void mkbasesetmakefile(int);

static void
mkbasesetmakefile(int set)
{
	FILE *fp;

	char buf[MIDLEN];

	snprintf(buf, MIDLEN, "system/%s", baseset[set].name);

	/* create category dir */
	mkdir("system", S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	/* create package dir */
	mkdir(buf, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
	/* create Makefile */
	snprintf(buf, MIDLEN, "system/%s/Makefile", baseset[set].name);
	if ((fp = fopen(buf, "w")) == NULL)
		err(EXIT_FAILURE, "fopen");

	fprintf(fp, "DISTNAME=\t%s\n", baseset[set].name);
	fprintf(fp, "COMMENT=\t%s\n", baseset[set].descr);
	fprintf(fp, "VIRTUAL_MAKEFILE=\tyes\n");
	fprintf(fp, "SYSTEM_PKG=\tyes\n");

	fclose(fp);
	/* create DESCR file*/
	snprintf(buf, MIDLEN, "system/%s/DESCR", baseset[set].name);
	if ((fp = fopen(buf, "w")) == NULL)
		err(EXIT_FAILURE, "fopen");
	fprintf(fp, "%s\n", baseset[set].descr);
	fclose(fp);
}

static int
isbaseset(const char *pkg)
{
	int i;
	char *p, buf[MIDLEN];

	strncpy(buf, pkg, strlen(pkg));
	if ((p = strstr(buf, ".tgz")) != NULL)
		*p = '\0';

	for (i = 0; baseset[i].name != NULL; i++)
		if (strncmp(baseset[i].name, buf, 
			    strlen(baseset[i].name)) == 0) {
			return(i);
		}

	return(-1);
}

static char *
findpkgval(const char *search, char **pkginfo)
{
	int i;

	for (i = 0; pkginfo[i] != NULL; i++)
		if (strncmp(pkginfo[i], search, strlen(search)) == 0) {
			char *p;

			p = strchr(pkginfo[i], '=');
			return(++p);
		}

	return(NULL);
}

static void
mkpkgfile(const char *path, 
	  const char *pkgpath, 
	  const char *pkgname, 
	  const char flag, 
	  const char *filename,
	  const char *title)
{
	int i, show_descr = T_FALSE;
	FILE *fp;
	char **pkgi, buf[MIDLEN];

	snprintf(buf, MIDLEN, "%s/pkg_info -%c %s/%s",
		 PKGTOOLS_PATH, flag, path, pkgname);

	pkgi = exec_list(buf, NULL);

	snprintf(buf, MIDLEN, "%s/%s", pkgpath, filename);

	if ((fp = fopen(buf, "w")) == NULL)
		err(EXIT_FAILURE, "fopen");

	for (i = 0; pkgi[i] != NULL; i++) {
		if (show_descr == T_TRUE)
			fprintf(fp, "%s\n", pkgi[i]);
		if (strncmp(pkgi[i], title, strlen(title)))
			show_descr = T_TRUE;
	}
	free_list(&pkgi);
	fclose(fp);

}

static void
mkmakefile(const char *path, const char *pkgpath, char *pkgname)
{
	FILE *fp;
	char **pkgi, *p, *comment, buf[MIDLEN];

	/* get package DESCR */
	mkpkgfile(path, pkgpath, pkgname, 'd', "DESCR", "Description:");

	/* get package PLIST */
	mkpkgfile(path, pkgpath, pkgname, 'L', "PLIST", "Files:");

	/* build false Makefile */
	snprintf(buf, MIDLEN, "%s/Makefile", pkgpath);

	if ((fp = fopen(buf, "w")) == NULL)
		err(EXIT_FAILURE, "fopen");

	/* retreive pkg one-line comment */
	snprintf(buf, MIDLEN, "%s/pkg_info -I %s/%s",
		 PKGTOOLS_PATH, path, pkgname);

	pkgi = exec_list(buf, NULL);
	comment = strchr(*pkgi, ' ');
	*comment++ = '\0';

	/* kick .tgz extension */
	p = strrchr(pkgname, '.');
	*p = '\0';

	fprintf(fp, "DISTNAME=\t%s\n", pkgname);
	fprintf(fp, "COMMENT=\t%s\n", comment);
	fprintf(fp, "VIRTUAL_MAKEFILE=\tyes\n");

	free_list(&pkgi);

	fclose(fp);
}

int
mkpkgsrc(const char *path)
{
	int i, count;
	struct stat sb;
	struct dirent **dlist;
	char *pkgsrcpath, *dir;

	if (stat(path, &sb) < 0)
		return(-1);
	if ((sb.st_mode & S_IFMT) != S_IFDIR)
		return(-1);

	if ((count = scandir(path, &dlist, NULL, NULL)) < 0)
		return(-1);

	pkgsrcpath = getenv("PKGSRCDIR");
	/* create virtual pkgsrc root */
	if (mkdir(pkgsrcpath, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) < 0)
		return(-1);

	for (i = 0; i < count; i++) {
		if (strstr(dlist[i]->d_name, ".tgz") != NULL) {
			int set;
			char **info;
			char cmd[MIDLEN];

			/* cd pkgsrc at each loop */
			chdir(pkgsrcpath);

			/* is archive a base set ? */
			if ((set = isbaseset(dlist[i]->d_name)) > -1) {
				mkbasesetmakefile(set);
				continue;
			}

			snprintf(cmd, MIDLEN, 
				 "%s/pkg_info -B %s/%s 2> /dev/null",
				 PKGTOOLS_PATH, path, dlist[i]->d_name);

			/* dont bother unknown archives */
			if ((info = exec_list(cmd, NULL)) == NULL)
				continue;

			/* mkdir category */
			if ((dir = findpkgval("PKGPATH", info)) != NULL) {
				char *p, pkgpath[MIDLEN];

				/* save PKGPATH */
				strncpy(pkgpath, dir, MIDLEN);
				
				if ((p = strchr(dir, '/')) == NULL)
					continue;

				*p = '\0';
				/* create category dir */
				mkdir(dir, S_IRWXU | S_IRWXG | 
				      S_IROTH | S_IXOTH);
				/* create package dir */
				mkdir(pkgpath, S_IRWXU | S_IRWXG | 
				      S_IROTH | S_IXOTH);
				/* create Makefile */
				mkmakefile(path, pkgpath, dlist[i]->d_name);
			} /* if PKGPATH */
		} /* if tgz */
	} /* for count */
	free(dlist);
	return(0);
}
