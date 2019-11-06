
#ifndef __ERROR_H
#define __ERROR_H

#include <stdarg.h>             /* Variable length args */
#include <syslog.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#define ERR_MAXPROGNAME 32      /* Maximum length of program name */
#define ERR_MAXLINE 4096

/* The program name is truncated if it is longer than ERR_MAXPROGNAME.
 * If this is the case, returns 1. Otherwise returns 0. */
int err_setprogname(const char *progname);

void err_msg(const char *fmt, ...);
void err_ret(const char *fmt, ...);
void err_sys(const char *fmt, ...);
void err_quit(const char *fmt, ...);

extern char err_progname[];

#endif
