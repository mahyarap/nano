/* $Id$ */
/**************************************************************************
 *   utils.c                                                              *
 *                                                                        *
 *   Copyright (C) 1999-2004 Chris Allegretta                             *
 *   This program is free software; you can redistribute it and/or modify *
 *   it under the terms of the GNU General Public License as published by *
 *   the Free Software Foundation; either version 2, or (at your option)  *
 *   any later version.                                                   *
 *                                                                        *
 *   This program is distributed in the hope that it will be useful,      *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of       *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the        *
 *   GNU General Public License for more details.                         *
 *                                                                        *
 *   You should have received a copy of the GNU General Public License    *
 *   along with this program; if not, write to the Free Software          *
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.            *
 *                                                                        *
 **************************************************************************/

#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "proto.h"
#include "nano.h"

#ifdef HAVE_REGEX_H
#ifdef BROKEN_REGEXEC
#undef regexec
int regexec_safe(const regex_t *preg, const char *string, size_t nmatch,
	regmatch_t pmatch[], int eflags)
{
    if (string != NULL && *string != '\0')
	return regexec(preg, string, nmatch, pmatch, eflags);
    return REG_NOMATCH;
}
#define regexec(preg, string, nmatch, pmatch, eflags) regexec_safe(preg, string, nmatch, pmatch, eflags)
#endif /* BROKEN_REGEXEC */

/* Assume that string will be found by regexec() if the REG_NOTBOL and
 * REG_NOTEOL glags are not set. */
int regexp_bol_or_eol(const regex_t *preg, const char *string)
{
    return (regexec(preg, string, 0, NULL, REG_NOTBOL | REG_NOTEOL) ==
	REG_NOMATCH);
}
#endif /* HAVE_REGEX_H */

#ifndef HAVE_ISBLANK
/* This function is equivalent to isblank(). */
int is_blank_char(int c)
{
    return (c == '\t' || c == ' ');
}
#endif

/* This function is equivalent to iscntrl(), except in that it also
 * handles control characters with their high bits set. */
int is_cntrl_char(int c)
{
    return (-128 <= c && c < -96) || (0 <= c && c < 32) ||
	(127 <= c && c < 160);
}

int num_of_digits(int n)
{
    int i = 1;

    if (n < 0)
	n = -n;

    while (n > 10) {
	n /= 10;
	i++;
    }

    return i;
}

/* Fix the memory allocation for a string. */
void align(char **strp)
{
    assert(strp != NULL);
    if (*strp != NULL)
	*strp = charealloc(*strp, strlen(*strp) + 1);
}

/* Null a string at a certain index and align it. */
void null_at(char **data, size_t index)
{
    assert(data != NULL);
    *data = charealloc(*data, index + 1);
    (*data)[index] = '\0';
}

/* For non-null-terminated lines.  A line, by definition, shouldn't
 * normally have newlines in it, so encode its nulls as newlines. */
void unsunder(char *str, size_t true_len)
{
    assert(str != NULL);
    for (; true_len > 0; true_len--, str++) {
	if (*str == '\0')
	    *str = '\n';
    }
}

/* For non-null-terminated lines.  A line, by definition, shouldn't
 * normally have newlines in it, so decode its newlines into nulls. */
void sunder(char *str)
{
    assert(str != NULL);
    for (; *str != '\0'; str++) {
	if (*str == '\n')
	    *str = '\0';
    }
}

#ifndef HAVE_STRCASECMP
/* This function is equivalent to strcasecmp(). */
int nstricmp(const char *s1, const char *s2)
{
    assert(s1 != NULL && s2 != NULL);

    for (; *s1 != '\0' && *s2 != '\0'; s1++, s2++) {
	if (tolower(*s1) != tolower(*s2))
	    break;
    }

    return (tolower(*s1) - tolower(*s2));
}
#endif

#ifndef HAVE_STRNCASECMP
/* This function is equivalent to strncasecmp(). */
int nstrnicmp(const char *s1, const char *s2, size_t n)
{
    assert(s1 != NULL && s2 != NULL);

    for (; n > 0 && *s1 != '\0' && *s2 != '\0'; n--, s1++, s2++) {
	if (tolower(*s1) != tolower(*s2))
	    break;
    }

    if (n > 0)
	return (tolower(*s1) - tolower(*s2));
    else if (n == 0)
	return 0;
    else
	return -1;
}
#endif

#ifndef HAVE_STRCASESTR
/* This function is equivalent to strcasestr().  It was adapted from
 * mutt's mutt_stristr() function. */
const char *nstristr(const char *haystack, const char *needle)
{
    assert(haystack != NULL && needle != NULL);

    for (; *haystack != '\0'; haystack++) {
	const char *p = haystack;
	const char *q = needle;

	for (; tolower(*p) == tolower(*q) && *q != '\0'; p++, q++)
	    ;

	if (*q == '\0')
	    return haystack;
    }

    return NULL;
}
#endif

/* None of this is needed if we're using NANO_SMALL! */
#ifndef NANO_SMALL
const char *revstrstr(const char *haystack, const char *needle, const
	char *rev_start)
{
    for (; rev_start >= haystack; rev_start--) {
	const char *r, *q;

	for (r = rev_start, q = needle ; *q == *r && *q != '\0'; r++, q++)
	    ;
	if (*q == '\0')
	    return rev_start;
    }
    return NULL;
}

const char *revstristr(const char *haystack, const char *needle, const
	char *rev_start)
{
    for (; rev_start >= haystack; rev_start--) {
	const char *r = rev_start, *q = needle;

	for (; (tolower(*q) == tolower(*r)) && (*q != '\0') ; r++, q++)
	    ;
	if (*q == '\0')
	    return rev_start;
    }
    return NULL;
}
#endif /* !NANO_SMALL */

/* If we are searching backwards, we will find the last match that
 * starts no later than start.  Otherwise we find the first match
 * starting no earlier than start.  If we are doing a regexp search, we
 * fill in the global variable regmatches with at most 9 subexpression
 * matches.  Also, all .rm_so elements are relative to the start of the
 * whole match, so regmatches[0].rm_so == 0. */
const char *strstrwrapper(const char *haystack, const char *needle,
	const char *start)
{
    /* start can be 1 character before the start or after the end of the
     * line.  In either case, we just say there is no match found. */
    if ((start > haystack && *(start - 1) == '\0') || start < haystack)
	return NULL;
    assert(haystack != NULL && needle != NULL && start != NULL);
#ifdef HAVE_REGEX_H
    if (ISSET(USE_REGEXP)) {
#ifndef NANO_SMALL
	if (ISSET(REVERSE_SEARCH)) {
	    if (regexec(&search_regexp, haystack, 1, regmatches, 0) == 0
		&& haystack + regmatches[0].rm_so <= start) {
		const char *retval = haystack + regmatches[0].rm_so;

		/* Search forward until there is no more match. */
		while (regexec(&search_regexp, retval + 1, 1, regmatches,
			REG_NOTBOL) == 0 && retval + 1 +
			regmatches[0].rm_so <= start)
		    retval += 1 + regmatches[0].rm_so;
		/* Finally, put the subexpression matches in global
		 * variable regmatches.  The REG_NOTBOL flag doesn't
		 * matter now. */
		regexec(&search_regexp, retval, 10, regmatches, 0);
		return retval;
	    }
	} else
#endif /* !NANO_SMALL */
	if (regexec(&search_regexp, start, 10, regmatches,
		start > haystack ? REG_NOTBOL : 0) == 0) {
	    const char *retval = start + regmatches[0].rm_so;

	    regexec(&search_regexp, retval, 10, regmatches, 0);
	    return retval;
	}
	return NULL;
    }
#endif /* HAVE_REGEX_H */
#if !defined(DISABLE_SPELLER) || !defined(NANO_SMALL)
    if (ISSET(CASE_SENSITIVE)) {
#ifndef NANO_SMALL
	if (ISSET(REVERSE_SEARCH))
	    return revstrstr(haystack, needle, start);
	else
#endif
	    return strstr(start, needle);
    }
#endif /* !DISABLE_SPELLER || !NANO_SMALL */
#ifndef NANO_SMALL
    else if (ISSET(REVERSE_SEARCH))
	return revstristr(haystack, needle, start);
#endif
    return strcasestr(start, needle);
}

/* This is a wrapper for the perror() function.  The wrapper takes care
 * of ncurses, calls perror (which writes to stderr), then refreshes the
 * screen.  Note that nperror() causes the window to flicker once. */
void nperror(const char *s)
{
    /* leave ncurses mode, go to the terminal */
    if (endwin() != ERR) {
	perror(s);		/* print the error */
	total_refresh();	/* return to ncurses and repaint */
    }
}

/* Thanks BG, many ppl have been asking for this... */
void *nmalloc(size_t howmuch)
{
    void *r = malloc(howmuch);

    if (r == NULL && howmuch != 0)
	die(_("nano is out of memory!"));

    return r;
}

void *nrealloc(void *ptr, size_t howmuch)
{
    void *r = realloc(ptr, howmuch);

    if (r == NULL && howmuch != 0)
	die(_("nano is out of memory!"));

    return r;
}

/* Copy one malloc()ed string to another pointer.  Should be used as:
 * dest = mallocstrcpy(dest, src); */
char *mallocstrcpy(char *dest, const char *src)
{
    if (src == NULL)
	src = "";

    if (src != dest)
	free(dest);

    dest = charalloc(strlen(src) + 1);
    strcpy(dest, src);

    return dest;
}

/* Append a new magic-line to filebot. */
void new_magicline(void)
{
    filebot->next = (filestruct *)nmalloc(sizeof(filestruct));
    filebot->next->data = charalloc(1);
    filebot->next->data[0] = '\0';
    filebot->next->prev = filebot;
    filebot->next->next = NULL;
    filebot->next->lineno = filebot->lineno + 1;
    filebot = filebot->next;
    totlines++;
    totsize++;
}

#ifndef NANO_SMALL
/* Set top_x and bot_x to the top and bottom x-coordinates of the mark,
 * respectively, based on the locations of top and bot. */
void mark_order(const filestruct **top, size_t *top_x,
		const filestruct **bot, size_t *bot_x)
{
    assert(top != NULL && top_x != NULL && bot != NULL && bot_x != NULL);
    if ((current->lineno == mark_beginbuf->lineno && current_x > mark_beginx)
	|| current->lineno > mark_beginbuf->lineno) {
	*top = mark_beginbuf;
	*top_x = mark_beginx;
	*bot = current;
	*bot_x = current_x;
    } else {
	*bot = mark_beginbuf;
	*bot_x = mark_beginx;
	*top = current;
	*top_x = current_x;
    }
}
#endif

#ifndef DISABLE_TABCOMP
/*
 * Routine to see if a text string is matched by a wildcard pattern.
 * Returns TRUE if the text is matched, or FALSE if it is not matched
 * or if the pattern is invalid.
 *  *		matches zero or more characters
 *  ?		matches a single character
 *  [abc]	matches 'a', 'b' or 'c'
 *  \c		quotes character c
 * Adapted from code written by Ingo Wilken, and
 * then taken from sash, Copyright (c) 1999 by David I. Bell
 * Permission is granted to use, distribute, or modify this source,
 * provided that this copyright notice remains intact.
 * Permission to distribute this code under the GPL has been granted.
 */
int check_wildcard_match(const char *text, const char *pattern)
{
    const char *retrypat;
    const char *retrytext;
    int ch;
    int found;
    int len;

    retrypat = NULL;
    retrytext = NULL;

    while (*text != '\0' || *pattern != '\0') {
	ch = *pattern++;

	switch (ch) {
	case '*':
	    retrypat = pattern;
	    retrytext = text;
	    break;

	case '[':
	    found = FALSE;

	    while ((ch = *pattern++) != ']') {
		if (ch == '\\')
		    ch = *pattern++;

		if (ch == '\0')
		    return FALSE;

		if (*text == ch)
		    found = TRUE;
	    }
	    len = strlen(text);
	    if (found == FALSE && len != 0) {
		return FALSE;
	    }
	    if (found == TRUE) {
		if (strlen(pattern) == 0 && len == 1) {
		    return TRUE;
		}
		if (len != 0) {
		    text++;
		    continue;
		}
	    }

	    /* fall into next case */

	case '?':
	    if (*text++ == '\0')
		return FALSE;

	    break;

	case '\\':
	    ch = *pattern++;

	    if (ch == '\0')
		return FALSE;

	    /* fall into next case */

	default:
	    if (*text == ch) {
		if (*text != '\0')
		    text++;
		break;
	    }

	    if (*text != '\0') {
		pattern = retrypat;
		text = ++retrytext;
		break;
	    }

	    return FALSE;
	}

	if (pattern == NULL)
	    return FALSE;
    }

    return TRUE;
}
#endif
