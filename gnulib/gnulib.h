#ifndef GNULIB_H_
#define GNULIB_H_

#include <stdlib.h>
#include <stdarg.h>
#include <limits.h>

#define _GL_EXTERN_INLINE extern inline
#define _GL_INLINE inline
#define _GL_INLINE_HEADER_BEGIN
#define _GL_INLINE_HEADER_END

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#ifndef LOCK_SH
/* Operations for the 'flock' call (same as Linux kernel constants).  */
# define LOCK_SH 1       /* Shared lock.  */
# define LOCK_EX 2       /* Exclusive lock.  */
# define LOCK_UN 8       /* Unlock.  */

/* Can be OR'd in to one of the above.  */
# define LOCK_NB 4       /* Don't block when locking.  */
#endif

#  ifndef ENOMSG
#   define ENOMSG    122
#   define GNULIB_defined_ENOMSG 1
#  endif

#  ifndef EIDRM
#   define EIDRM     111
#   define GNULIB_defined_EIDRM 1
#  endif

#  ifndef ENOLINK
#   define ENOLINK   121
#   define GNULIB_defined_ENOLINK 1
#  endif

#  ifndef EPROTO
#   define EPROTO    134
#   define GNULIB_defined_EPROTO 1
#  endif

#  ifndef EBADMSG
#   define EBADMSG   104
#   define GNULIB_defined_EBADMSG 1
#  endif

#  ifndef EOVERFLOW
#   define EOVERFLOW 132
#   define GNULIB_defined_EOVERFLOW 1
#  endif

#  ifndef ENOTSUP
#   define ENOTSUP   129
#   define GNULIB_defined_ENOTSUP 1
#  endif

#  ifndef ENETRESET
#   define ENETRESET 117
#   define GNULIB_defined_ENETRESET 1
#  endif

#  ifndef ECONNABORTED
#   define ECONNABORTED 106
#   define GNULIB_defined_ECONNABORTED 1
#  endif

#  ifndef ECANCELED
#   define ECANCELED 105
#   define GNULIB_defined_ECANCELED 1
#  endif

#  ifndef EOWNERDEAD
#   define EOWNERDEAD 133
#   define GNULIB_defined_EOWNERDEAD 1
#  endif

#  ifndef ENOTRECOVERABLE
#   define ENOTRECOVERABLE 127
#   define GNULIB_defined_ENOTRECOVERABLE 1
#  endif

#  ifndef EINPROGRESS
#   define EINPROGRESS     112
#   define EALREADY        103
#   define ENOTSOCK        128
#   define EDESTADDRREQ    109
#   define EMSGSIZE        115
#   define EPROTOTYPE      136
#   define ENOPROTOOPT     123
#   define EPROTONOSUPPORT 135
#   define EOPNOTSUPP      130
#   define EAFNOSUPPORT    102
#   define EADDRINUSE      100
#   define EADDRNOTAVAIL   101
#   define ENETDOWN        116
#   define ENETUNREACH     118
#   define ECONNRESET      108
#   define ENOBUFS         119
#   define EISCONN         113
#   define ENOTCONN        126
#   define ETIMEDOUT       138
#   define ECONNREFUSED    107
#   define ELOOP           114
#   define EHOSTUNREACH    110
#   define EWOULDBLOCK     140
#   define GNULIB_defined_ESOCK 1
#  endif

#  ifndef ETXTBSY
#   define ETXTBSY         139
#   define ENODATA         120  /* not required by POSIX */
#   define ENOSR           124  /* not required by POSIX */
#   define ENOSTR          125  /* not required by POSIX */
#   define ETIME           137  /* not required by POSIX */
#   define EOTHER          131  /* not required by POSIX */
#   define GNULIB_defined_ESTREAMS 1
#  endif

/* These are intentionally the same values as the WSA* error numbers, defined
   in <winsock2.h>.  */
#  define ESOCKTNOSUPPORT 10044  /* not required by POSIX */
#  define EPFNOSUPPORT    10046  /* not required by POSIX */
#  define ESHUTDOWN       10058  /* not required by POSIX */
#  define ETOOMANYREFS    10059  /* not required by POSIX */
#  define EHOSTDOWN       10064  /* not required by POSIX */
#  define EPROCLIM        10067  /* not required by POSIX */
#  define EUSERS          10068  /* not required by POSIX */
#  define EDQUOT          10069
#  define ESTALE          10070
#  define EREMOTE         10071  /* not required by POSIX */
#  define GNULIB_defined_EWINSOCK 1

#define DCHAR_T char
#define FCHAR_T char

int fsync (int fd);

int flock (int fd, int operation);

char * strsep (char **stringp, const char *delim);

int asprintf (char **resultp, const char *format, ...);

int vasprintf (char **resultp, const char *format, va_list args);

DCHAR_T *
vasnprintf (DCHAR_T *resultbuf, size_t *lengthp,
            const FCHAR_T *format, va_list args);

void *alloca (size_t size);

char *realpath(const char *path, char resolved_path[PATH_MAX]);

#endif