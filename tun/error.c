

/* Based on the code provided with Stevens "The Sockets Networking API book." */

#include "error.h"

char err_progname[ERR_MAXPROGNAME + 1] = "";
static int err_progname_set = 0;

/* TODO(oral): Consider using syslog if I implement this as a daemon proc. */

static void err_doit(int, int, const char *, va_list);

int
err_setprogname(const char *progname)
{
     err_progname_set = 1;
     return snprintf(err_progname, ERR_MAXPROGNAME, progname) != strlen(progname);
}

/* Nonfatal error unrelated to system call
 * Print message and return. */

void
err_msg(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(0, LOG_INFO, fmt, ap);
     va_end(ap);
}

/* Nonfatal error related to system call
 * Print message and return*/

void
err_ret(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, LOG_INFO, fmt, ap);
     va_end(ap);
}

/* Fatal error related to system call
 * Print message and terminate */

void
err_sys(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(1, LOG_ERR, fmt, ap);
     va_end(ap);
     exit(1);
}

/* Fatal error unrelated to system call
 * Print message and terminate */

void
err_quit(const char *fmt, ...)
{
     va_list ap;
     va_start(ap, fmt);
     err_doit(0, LOG_ERR, fmt, ap);
     va_end(ap);
     exit(1);
}

/* Print message and return to caller
 * Caller specifies "errnoflag" and "level" */

static void
err_doit(int errnoflag, int level, const char *fmt, va_list ap)
{
     int errno_save, n = 0;
     char buf[ERR_MAXLINE + 1];

     if (err_progname_set) {
          snprintf(buf, ERR_MAXLINE, "%s: ", err_progname);
          n = strlen(buf);
     }
     errno_save = errno;
     vsnprintf(buf + n, ERR_MAXLINE - n, fmt, ap);
     n = strlen(buf);
     if (errnoflag)
          snprintf(buf + n, ERR_MAXLINE - n, ": %s", strerror(errno_save));
     strcat(buf, "\n");
     fflush(stdout);            /* in case stdout and stderr are the same */
     fputs(buf, stderr);
     fflush(stderr);
}
