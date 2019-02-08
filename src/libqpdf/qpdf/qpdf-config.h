/* libqpdf/qpdf/qpdf-config.h.  Generated from qpdf-config.h.in by configure.  */
/* libqpdf/qpdf/qpdf-config.h.in.  Generated from configure.ac by autoheader.  */

/* Whether to avoid use of HANDLE in Windows */
/* #undef AVOID_WINDOWS_HANDLE */

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if fseeko (and presumably ftello) exists and is declared. */
#define HAVE_FSEEKO 1

/* Define to 1 if you have the `fseeko64' function. */
#ifdef WIN32_
#define HAVE_FSEEKO64 1
#endif

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the `random' function. */
#ifndef _WIN32
#define HAVE_RANDOM 1
#endif

/* Define to 1 (and set RANDOM_DEVICE) if a random device is available */
#define HAVE_RANDOM_DEVICE 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the sub-directory where libtool stores uninstalled libraries. */
#define LT_OBJDIR ".libs/"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT ""

/* Define to the full name of this package. */
#define PACKAGE_NAME "qpdf"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "qpdf 8.4.0"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "qpdf"

/* Define to the home page for this package. */
#define PACKAGE_URL ""

/* Define to the version of this package. */
#define PACKAGE_VERSION "8.4.0"

/* Define to the filename of the random device (and set HAVE_RANDOM_DEVICE) */
#define RANDOM_DEVICE "/dev/urandom"

/* Whether to suppres use of OS-provided secure random numbers */
/* #undef SKIP_OS_SECURE_RANDOM */

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Whether to use insecure random numbers */
/* #undef USE_INSECURE_RANDOM */

/* Enable large inode numbers on Mac OS X 10.5.  */
#ifndef _DARWIN_USE_64_BIT_INODE
# define _DARWIN_USE_64_BIT_INODE 1
#endif

/* Number of bits in a file offset, on hosts where this is settable. */
/* #undef _FILE_OFFSET_BITS */

/* Define to 1 to make fseeko visible on some hosts (e.g. glibc 2.2). */
/* #undef _LARGEFILE_SOURCE */

/* Define for large files, on AIX-style hosts. */
/* #undef _LARGE_FILES */

/* Define for Solaris 2.5.1 so the uint32_t typedef from <sys/synch.h>,
   <pthread.h>, or <semaphore.h> is not used. If the typedef were allowed, the
   #define below would cause a syntax error. */
/* #undef _UINT32_T */

/* Define to the type of an unsigned integer type of width exactly 16 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint16_t */

/* Define to the type of an unsigned integer type of width exactly 32 bits if
   such a type exists and the standard includes do not define it. */
/* #undef uint32_t */
