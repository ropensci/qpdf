/* options */
/* Whether to avoid use of HANDLE in Windows */
/* #undef AVOID_WINDOWS_HANDLE */
#define DEFAULT_CRYPTO "native"
/* #undef USE_CRYPTO_GNUTLS */
#define USE_CRYPTO_NATIVE 1
/* #undef USE_CRYPTO_OPENSSL */
/* Whether to use insecure random numbers */
/* #undef USE_INSECURE_RANDOM */
#define SKIP_OS_SECURE_RANDOM 1
/* #undef ZOPFLI */

/* large file support -- may be needed for 32-bit systems */
/* #undef _FILE_OFFSET_BITS */

/* headers files */
#define HAVE_INTTYPES_H 1
#define HAVE_STDINT_H 1

/* OS functions and symbols */
#define HAVE_EXTERN_LONG_TIMEZONE 1
#define HAVE_FSEEKO 1
#ifdef WIN32_
#define HAVE_FSEEKO64 1
#endif
#define HAVE_LOCALTIME_R 1
#ifndef _WIN32
#define HAVE_RANDOM 1
#endif
#define HAVE_TM_GMTOFF 1
/* #undef HAVE_MALLOC_INFO */
/* #undef HAVE_OPEN_MEMSTREAM */

/* bytes in the size_t type */
/* #undef SIZEOF_SIZE_T */
