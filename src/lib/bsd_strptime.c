// http://cvsweb.netbsd.org/bsdweb.cgi/src/lib/libc/time/strptime.c?rev=HEAD
/*  $NetBSD: strptime.c,v 1.28 2008/04/28 20:23:01 martin Exp $ */  

// Note: source adjust slightly for use in this project
  
/*- 
 * Copyright (c) 1997, 1998, 2005, 2008 The NetBSD Foundation, Inc. 
 * All rights reserved. 
 * 
 * This code was contributed to The NetBSD Foundation by Klaus Klein. 
 * Heavily optimised by David Laight 
 * 
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions 
 * are met: 
 * 1. Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer. 
 * 2. Redistributions in binary form must reproduce the above copyright 
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS 
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR 
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS 
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE. 
 */  

#if (defined _WIN32 || defined _WIN64 || defined WINDOWS)
 
#include <sys/cdefs.h>  
#if defined(LIBC_SCCS) && !defined(lint)  
__RCSID("$NetBSD: strptime.c,v 1.28 2008/04/28 20:23:01 martin Exp $");  
#endif  
 
//#include "namespace.h"  
//#include <sys/localedef.h>  
#include <ctype.h>  
#include <locale.h>  
#include <string.h>  
#include <time.h>  
//#include <tzfile.h>  
 
//#ifdef __weak_alias  
//__weak_alias(strptime,_strptime)  
//#endif  

// ----------------------

#ifndef TM_YEAR_BASE
#define TM_YEAR_BASE   1900
#endif

typedef struct {
    const char *abday[7];
    const char *day[7];
    const char *abmon[12];
    const char *mon[12];
    const char *am_pm[2];
    const char *d_t_fmt;
    const char *d_fmt;
    const char *t_fmt;
    const char *t_fmt_ampm;
} _TimeLocale;
 
static const _TimeLocale _DefaultTimeLocale = 
{
    {
	"Sun","Mon","Tue","Wed","Thu","Fri","Sat",
    },
    {
	"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday",
	"Friday", "Saturday"
    },
    {
	"Jan", "Feb", "Mar", "Apr", "May", "Jun",
	"Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    },
    {
	"January", "February", "March", "April", "May", "June", "July",
	"August", "September", "October", "November", "December"
    },
    {
	"AM", "PM"
    },
    "%a %b %d %H:%M:%S %Y",
    "%m/%d/%y",
    "%H:%M:%S",
    "%I:%M:%S %p"
};

static const _TimeLocale *_CurrentTimeLocale = &_DefaultTimeLocale;


//--------------------------

 
#define _ctloc(x)       (_CurrentTimeLocale->x)  
  
/* 
 * We do not implement alternate representations. However, we always 
 * check whether a given modifier is allowed for a certain conversion. 
 */  
#define ALT_E           0x01  
#define ALT_O           0x02  
#define LEGAL_ALT(x)        { if (alt_format & ~(x)) return NULL; }  
  
static const char gmt[4] = { "GMT" };  
  
static const unsigned char *conv_num(const unsigned char *, int *, unsigned int, unsigned int);  
static const unsigned char *find_string(const unsigned char *, int *, const char * const *,  
    const char * const *, int);  
  
  
char *  
strptime(const char *buf, const char *fmt, struct tm *tm)  
{  
    unsigned char c;  
    const unsigned char *bp;  
    int alt_format, i, split_year = 0;  
    const char *new_fmt;  
  
    bp = (const unsigned char *)buf;  
  
    while (bp != NULL && (c = *fmt++) != '\0') {  
        /* Clear `alternate' modifier prior to new conversion. */  
        alt_format = 0;  
        i = 0;  
  
        /* Eat up white-space. */  
        if (isspace(c)) {  
            while (isspace(*bp))  
                bp++;  
            continue;  
        }  
  
        if (c != '%')  
            goto literal;  
  
  
again:      switch (c = *fmt++) {  
        case '%':   /* "%%" is converted to "%". */  
literal:  
            if (c != *bp++)  
                return NULL;  
            LEGAL_ALT(0);  
            continue;  
  
        /* 
         * "Alternative" modifiers. Just set the appropriate flag 
         * and start over again. 
         */  
        case 'E':   /* "%E?" alternative conversion modifier. */  
            LEGAL_ALT(0);  
            alt_format |= ALT_E;  
            goto again;  
  
        case 'O':   /* "%O?" alternative conversion modifier. */  
            LEGAL_ALT(0);  
            alt_format |= ALT_O;  
            goto again;  
  
        /* 
         * "Complex" conversion rules, implemented through recursion. 
         */  
        case 'c':   /* Date and time, using the locale's format. */  
            new_fmt = _ctloc(d_t_fmt);  
            goto recurse;  
  
        case 'D':   /* The date as "%m/%d/%y". */  
            new_fmt = "%m/%d/%y";  
            LEGAL_ALT(0);  
            goto recurse;  
  
        case 'F':   /* The date as "%Y-%m-%d". */  
            new_fmt = "%Y-%m-%d";  
            LEGAL_ALT(0);  
            goto recurse;  
  
        case 'R':   /* The time as "%H:%M". */  
            new_fmt = "%H:%M";  
            LEGAL_ALT(0);  
            goto recurse;  
  
        case 'r':   /* The time in 12-hour clock representation. */  
            new_fmt =_ctloc(t_fmt_ampm);  
            LEGAL_ALT(0);  
            goto recurse;  
  
        case 'T':   /* The time as "%H:%M:%S". */  
            new_fmt = "%H:%M:%S";  
            LEGAL_ALT(0);  
            goto recurse;  
  
        case 'X':   /* The time, using the locale's format. */  
            new_fmt =_ctloc(t_fmt);  
            goto recurse;  
  
        case 'x':   /* The date, using the locale's format. */  
            new_fmt =_ctloc(d_fmt);  
            recurse:  
            bp = (const unsigned char *)strptime((const char *)bp,  
                                new_fmt, tm);  
            LEGAL_ALT(ALT_E);  
            continue;  
  
        /* 
         * "Elementary" conversion rules. 
         */  
        case 'A':   /* The day of week, using the locale's form. */  
        case 'a':  
            bp = find_string(bp, &tm->tm_wday, _ctloc(day),  
                    _ctloc(abday), 7);  
            LEGAL_ALT(0);  
            continue;  
  
        case 'B':   /* The month, using the locale's form. */  
        case 'b':  
        case 'h':  
            bp = find_string(bp, &tm->tm_mon, _ctloc(mon),  
                    _ctloc(abmon), 12);  
            LEGAL_ALT(0);  
            continue;  
  
        case 'C':   /* The century number. */  
            i = 20;  
            bp = conv_num(bp, &i, 0, 99);  
  
            i = i * 100 - TM_YEAR_BASE;  
            if (split_year)  
                i += tm->tm_year % 100;  
            split_year = 1;  
            tm->tm_year = i;  
            LEGAL_ALT(ALT_E);  
            continue;  
  
        case 'd':   /* The day of month. */  
        case 'e':  
            bp = conv_num(bp, &tm->tm_mday, 1, 31);  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'k':   /* The hour (24-hour clock representation). */  
            LEGAL_ALT(0);  
            /* FALLTHROUGH */  
        case 'H':  
            bp = conv_num(bp, &tm->tm_hour, 0, 23);  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'l':   /* The hour (12-hour clock representation). */  
            LEGAL_ALT(0);  
            /* FALLTHROUGH */  
        case 'I':  
            bp = conv_num(bp, &tm->tm_hour, 1, 12);  
            if (tm->tm_hour == 12)  
                tm->tm_hour = 0;  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'j':   /* The day of year. */  
            i = 1;  
            bp = conv_num(bp, &i, 1, 366);  
            tm->tm_yday = i - 1;  
            LEGAL_ALT(0);  
            continue;  
  
        case 'M':   /* The minute. */  
            bp = conv_num(bp, &tm->tm_min, 0, 59);  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'm':   /* The month. */  
            i = 1;  
            bp = conv_num(bp, &i, 1, 12);  
            tm->tm_mon = i - 1;  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'p':   /* The locale's equivalent of AM/PM. */  
            bp = find_string(bp, &i, _ctloc(am_pm), NULL, 2);  
            if (tm->tm_hour > 11)  
                return NULL;  
            tm->tm_hour += i * 12;  
            LEGAL_ALT(0);  
            continue;  
  
        case 'S':   /* The seconds. */  
            bp = conv_num(bp, &tm->tm_sec, 0, 61);  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'U':   /* The week of year, beginning on sunday. */  
        case 'W':   /* The week of year, beginning on monday. */  
            /* 
             * XXX This is bogus, as we can not assume any valid 
             * information present in the tm structure at this 
             * point to calculate a real value, so just check the 
             * range for now. 
             */  
             bp = conv_num(bp, &i, 0, 53);  
             LEGAL_ALT(ALT_O);  
             continue;  
  
        case 'w':   /* The day of week, beginning on sunday. */  
            bp = conv_num(bp, &tm->tm_wday, 0, 6);  
            LEGAL_ALT(ALT_O);  
            continue;  
  
        case 'Y':   /* The year. */  
            i = TM_YEAR_BASE;   /* just for data sanity... */  
            bp = conv_num(bp, &i, 0, 9999);  
            tm->tm_year = i - TM_YEAR_BASE;  
            LEGAL_ALT(ALT_E);  
            continue;  
  
        case 'y':   /* The year within 100 years of the epoch. */  
            /* LEGAL_ALT(ALT_E | ALT_O); */  
            bp = conv_num(bp, &i, 0, 99);  
  
            if (split_year)  
                /* preserve century */  
                i += (tm->tm_year / 100) * 100;  
            else {  
                split_year = 1;  
                if (i <= 68)  
                    i = i + 2000 - TM_YEAR_BASE;  
                else  
                    i = i + 1900 - TM_YEAR_BASE;  
            }  
            tm->tm_year = i;  
            continue;  
  
        case 'Z':  
            tzset();  
            if (strncmp((const char *)bp, gmt, 3) == 0) {  
                tm->tm_isdst = 0;  
#ifdef TM_GMTOFF  
                tm->TM_GMTOFF = 0;  
#endif  
#ifdef TM_ZONE  
                tm->TM_ZONE = gmt;  
#endif  
                bp += 3;  
            } else {  
                const unsigned char *ep;  
  
                ep = find_string(bp, &i,  
                             (const char * const *)tzname,  
                              NULL, 2);  
                if (ep != NULL) {  
                    tm->tm_isdst = i;  
#ifdef TM_GMTOFF  
                    tm->TM_GMTOFF = -(timezone);  
#endif  
#ifdef TM_ZONE  
                    tm->TM_ZONE = tzname[i];  
#endif  
                }  
                bp = ep;  
            }  
            continue;  
  
        /* 
         * Miscellaneous conversions. 
         */  
        case 'n':   /* Any kind of white-space. */  
        case 't':  
            while (isspace(*bp))  
                bp++;  
            LEGAL_ALT(0);  
            continue;  
  
  
        default:    /* Unknown/unsupported conversion. */  
            return NULL;  
        }  
    }  
  
    return (char*)bp;
}  
  
  
static const unsigned char *  
conv_num(const unsigned char *buf, int *dest, unsigned int llim, unsigned int ulim)  
{  
    unsigned int result = 0;  
    unsigned char ch;  
  
    /* The limit also determines the number of valid digits. */  
    unsigned int rulim = ulim;  
  
    ch = *buf;  
    if (ch < '0' || ch > '9')  
        return NULL;  
  
    do {  
        result *= 10;  
        result += ch - '0';  
        rulim /= 10;  
        ch = *++buf;  
    } while ((result * 10 <= ulim) && rulim && ch >= '0' && ch <= '9');  
  
    if (result < llim || result > ulim)  
        return NULL;  
  
    *dest = result;  
    return buf;  
}  
  
static const unsigned char *  
find_string(const unsigned char *bp, int *tgt, const char * const *n1,  
        const char * const *n2, int c)  
{  
    int i;  
    unsigned int len;  
  
    /* check full name - then abbreviated ones */  
    for (; n1 != NULL; n1 = n2, n2 = NULL) {  
        for (i = 0; i < c; i++, n1++) {  
            len = strlen(*n1);  
            if (strncasecmp(*n1, (const char *)bp, len) == 0) {  
                *tgt = i;  
                return bp + len;  
            }  
        }  
    }  
  
    /* Nothing matched */  
    return NULL;  
}

#endif

