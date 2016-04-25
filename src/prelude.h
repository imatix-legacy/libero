/*  ----------------------------------------------------------------<Prolog>-
    Name:       prelude.h
    Title:      Universal Header File for C programming
    Package:    Standard Function Library (SFL)

    Written:    93/03/29    Pieter Hintjens <ph@imatix.com>
    Revised:    97/02/24    Pieter Hintjens <ph@imatix.com>

    Version:    1.44
    <TABLE>
    1.4_PH      Released with SFL 1.4
    1.41_PH     Define EXIT_SUCCESS, EXIT_FAILURE if not defined.
    1.42_PH     Define extern char **environ under SunOS.
    1.43_PH     Define TRUE/FALSE with typecase (Bool)
    1.44_PH     That last change was NFG... caused cpp errors
    1.45_PH     Changes for SCO UnixWare
    </TABLE>

    Synopsis:   This header file encapsulates many generally-useful include
                files and defines lots of good stuff.  The intention of this
                header file is to hide the messy #ifdef's that you typically
                need to make real programs compile & run.  To use, specify
                as the first include file in your program.

                The main contributors to this file were:
                <Table>
                PH   Pieter Hintjens    <ph@imatix.com>
                EDM  Ewen McNeill       <ewen@naos.co.nz>
                PA   Pascal Antonnaux   <pascal@imatix.com>
                PS   Peter Seebach      <seebs@solon.com>
                </Table>

    Copyright:  Copyright (c) 1991-1996 iMatix
    License:    This is free software; you can redistribute it and/or modify
                it under the terms of the SFL License Agreement as provided
                in the file LICENSE.TXT.  This software is distributed in
                the hope that it will be useful, but without any warranty.
 ------------------------------------------------------------------</Prolog>-*/

#ifndef _PRELUDE_INCLUDED               /*  Allow multiple inclusions        */
#define _PRELUDE_INCLUDED


/*- Establish the compiler and computer system ------------------------------*/
/*
 *  Defines zero or more of these symbols, for use in any non-portable
 *  code:
 *
 *  __WINDOWS__         Microsoft C/C++ with Windows calls
 *  __MSDOS__           System is MS-DOS (set if __WINDOWS__ set)
 *  __VMS__             System is VAX/VMS or Alpha/OpenVMS
 *  __UNIX__            System is UNIX
 *  __OS2__             System is OS/2
 *  __MAC__             System is Mac/OS
 *  __AMIGA__           System is AmigaDOS
 *
 *  __IS_32BIT__        OS/compiler is 32 bits
 *  __IS_64BIT__        OS/compiler is 64 bits
 *
 *  When __UNIX__ is defined, we also define exactly one of these:
 *
 *  __UTYPE_AUX         Apple AUX
 *  __UTYPE_DECALPHA    Digital UNIX (Alpha)
 *  __UTYPE_IBMAIX      IBM RS/6000 AIX
 *  __UTYPE_HPUX        HP/UX
 *  __UTYPE_LINUX       Linux
 *  __UTYPE_MIPS        MIPS (BSD 4.3/System V mixture)
 *  __UTYPE_NETBSD      NetBSD
 *  __UTYPE_NEXT        NeXT
 *  __UTYPE_SCO         SCO Unix
 *  __UTYPE_SUNOS       SunOS
 *  __UTYPE_SUNSOLARIS  Sun Solaris
 *                      ... these are the ones I know about so far.
 *  __UTYPE_UNIXWARE    SCO UnixWare
 *  __UTYPE_GENERIC     Any other UNIX
 *
 *  When __VMS__ is defined, we may define one or more of these:
 *
 *  __VMS_XOPEN         Supports XOPEN functions
 */

#if (defined (__64BIT__))               /*  EDM 96/05/30                     */
#    define __IS_64BIT__                /*  May have 64-bit OS/compiler      */
#else
#    define __IS_32BIT__                /*  Else assume 32-bit OS/compiler   */
#endif

#if (defined WIN32 || defined (_WIN32))
#   undef __WINDOWS__
#   define __WINDOWS__
#   undef __MSDOS__
#   define __MSDOS__
#endif

#if (defined WINDOWS || defined (_WINDOWS))
#   undef __WINDOWS__
#   define __WINDOWS__
#   undef __MSDOS__
#   define __MSDOS__
#endif

/*  MSDOS               Microsoft C                                          */
/*  _MSC_VER            Microsoft C                                          */
/*  __TURBOC__          Borland Turbo C                                      */
#if (defined (MSDOS) || defined (_MSC_VER) || defined (__TURBOC__))
#   undef __MSDOS__
#   define __MSDOS__
#endif

/*  EDM 96/05/28                                                             */
/*  __OS2__    Triggered by __EMX__ define and __i386__ define to avoid      */
/*             manual definition (eg, makefile) even though __EMX__ and      */
/*             __i386__ can be used on a MSDOS machine as well.  Here        */
/*             the same work is required at present.                         */
#if (defined (__EMX__) && defined (__i386__))
#   undef __OS2__
#   define __OS2__
#endif

/*  VMS                 VAX C (VAX/VMS)                                      */
/*  __VMS               Dec C (Alpha/OpenVMS)                                */
/*  __vax__             gcc                                                  */
#if (defined (VMS) || defined (__VMS) || defined (__vax__))
#   undef __VMS__
#   define __VMS__
#   if (__VMS_VER >= 70000000)
#       define __VMS_XOPEN
#   endif
#endif

/*  Untested                                                                 */
/*  GG  96/07/04                                                             */
#if (defined (_MAC) || defined (MAC_OS) || defined (THINK_C))
#   undef __MAC__
#   define __MAC__
#endif

/*  Untested                                                                 */
/*  PS  96/06/17                                                             */
#if (defined (_AMIGA_))
#   undef __AMIGA__
#   define __AMIGA__
#endif

/*  Try to define a __UTYPE_xxx symbol...                                    */
/*  unix                SunOS at least                                       */
/*  __unix__            gcc                                                  */
/*  _POSIX_SOURCE is various UNIX systems, maybe also VAX/VMS                */
#if (defined (unix) || defined (__unix__) || defined (_POSIX_SOURCE))
#   if (!defined (__VMS__))
#       undef __UNIX__
#       define __UNIX__
#       if (defined (__alpha))          /*  Digital UNIX is 64-bit           */
#           undef  __IS_32BIT__
#           define __IS_64BIT__
#           define __UTYPE_DECALPHA
#       endif
#   endif
#endif

#if (defined (_AUX))
#   define __UTYPE_AUX
#   define __UNIX__
#elif (defined (__hpux))
#   define __UTYPE_HPUX
#   define __UNIX__
#   define _INCLUDE_HPUX_SOURCE
#   define _INCLUDE_XOPEN_SOURCE
#   define _INCLUDE_POSIX_SOURCE
#elif (defined (_AIX) || defined (AIX))
#   define __UTYPE_IBMAIX
#   define __UNIX__
#elif (defined (linux))
#   define __UTYPE_LINUX
#   define __UNIX__
#elif (defined (Mips))
#   define __UTYPE_MIPS
#   define __UNIX__
#elif (defined (NetBSD))
#   define __UTYPE_NETBSD
#   define __UNIX__
#elif (defined (NeXT))
#   define __UTYPE_NEXT
#   define __UNIX__
#elif (defined (sco))
#   define __UTYPE_SCO
#   define __UNIX__
#elif (defined (SUNOS) || defined (SUN) || defined (sun))
#   define __UTYPE_SUNOS
#   define __UNIX__
#elif (defined (SOLARIS))
#   define __UTYPE_SUNSOLARIS
#   define __UNIX__
#elif (defined (UnixWare))
#   define __UTYPE_UNIXWARE
#   define __UNIX__
#elif (defined __UNIX__)
#   define __UTYPE_GENERIC
#endif


/*- Standard ANSI include files ---------------------------------------------*/

#ifdef __cplusplus                      /*  PA 96/05/29                      */
#include <iostream.h>                   /*  A bit of support for C++         */
#endif

#include <ctype.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <float.h>
#include <math.h>
#include <signal.h>
#include <setjmp.h>


/*- System-specific include files -------------------------------------------*/

#if (defined (__MSDOS__))
#   if (defined (__WINDOWS__))          /*  When __WINDOWS__ is defined,     */
#       include <windows.h>             /*    so is __MSDOS__                */
#       include <winsock.h>             /*  May cause trouble on VC 1.x      */
#   endif
#   if (defined (__TURBOC__))
#       include <dir.h>
#       include <alloc.h>               /*  Okay for Turbo C                 */
#   else
#       include <malloc.h>              /*  But will it work for others?     */
#   endif
#   include <dos.h>
#   include <io.h>
#   include <fcntl.h>
#   include <sys\types.h>
#   include <sys\stat.h>
#endif

#if (defined (__UNIX__))
#   include <fcntl.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <dirent.h>
#   include <pwd.h>
#   include <grp.h>
#   include <sys/types.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <sys/file.h>
#   include <sys/wait.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
/*  Specific #include's for UNIX varieties                                   */
#   if (defined (__UTYPE_IBMAIX))
#       include <sys/select.h>
#   endif
#endif

#if (defined (__VMS__))
#   if (!defined (vaxc))
#       include <fcntl.h>               /*  Not provided by Vax C            */
#   endif
#   include <netdb.h>
#   include <unixio.h>
#   include <types.h>
#   include <socket.h>
#   include <dirent.h>
#   include <time.h>
#   include <pwd.h>
#   include <stat.h>
#   include <in.h>
#endif

#if (defined (__OS2__))
/*  Include list for OS/2 updated by EDM 96/12/31
 *  NOTE: sys/types.h must go near the top of the list because some of the
 *        definitions in other include files rely on types defined there.
 */
#   include <sys/types.h>
#   include <fcntl.h>
#   include <malloc.h>
#   include <netdb.h>
#   include <unistd.h>
#   include <dirent.h>
#   include <pwd.h>
#   include <grp.h>
#   include <sys/param.h>
#   include <sys/socket.h>
#   include <sys/select.h>
#   include <sys/time.h>
#   include <sys/stat.h>
#   include <sys/ioctl.h>
#   include <sys/file.h>
#   include <sys/wait.h>
#   include <netinet/in.h>
#   include <arpa/inet.h>
#endif

/*  GG  96/07/04                                                             */
#if (defined (__MAC__))
#   include <console.h>
#endif


/*- Data types --------------------------------------------------------------*/

typedef int             Bool;           /*  Boolean TRUE/FALSE value         */
typedef unsigned char   byte;           /*  Single unsigned byte = 8 bits    */
typedef unsigned short  dbyte;          /*  Double byte = 16 bits            */
typedef unsigned short  word;           /*  Alternative for double-byte      */
typedef unsigned long   dword;          /*  Double word >= 32 bits           */
#if (defined (__IS_32BIT__))
typedef unsigned long   qbyte;          /*  Quad byte = 32 bits              */
#else
typedef unsigned int    qbyte;          /*  Quad byte = 32 bits              */
#endif
typedef void (*function) (void);        /*  Address of simple function       */
#define local static void               /*  Shorthand for local functions    */

typedef struct {                        /*  Memory descriptor                */
    size_t size;                        /*    Size of data part              */
    byte  *data;                        /*    Data part follows here         */
} DESCR;


/*- Check compiler data type sizes ------------------------------------------*/

#if (UCHAR_MAX != 0xFF)
#   error "Cannot compile: must change definition of 'byte'."
#endif
#if (USHRT_MAX != 0xFFFFU)
#   error "Cannot compile: must change definition of 'dbyte'."
#endif
#if (defined (__IS_32BIT__))
#   if (ULONG_MAX != 0xFFFFFFFFUL)
#       error "Cannot compile: must change definition of 'qbyte'."
#   endif
#else
#   if (UINT_MAX != 0xFFFFFFFFU)
#       error "Cannot compile: must change definition of 'qbyte'."
#   endif
#endif


/*- Pseudo-functions --------------------------------------------------------*/

#define FOREVER         for (;;)            /*  FOREVER { ... }              */
#define until(expr)     while (!(expr))     /*  do { ... } until (expr)      */
#define streq(s1,s2)    (!strcmp ((s1), (s2)))
#define strneq(s1,s2)   (strcmp ((s1), (s2)))
#define strused(s)      (*(s) != 0)
#define strnull(s)      (*(s) == 0)
#define strclr(s)       (*(s) = 0)
#define strlast(s)      (s [strlen (s) - 1])
#define strterm(s)      (s [strlen (s)])

#define bit_msk(bit)    (1 << (bit))
#define bit_set(x,bit)  ((x) |=  bit_msk (bit))
#define bit_clr(x,bit)  ((x) &= ~bit_msk (bit))
#define bit_tst(x,bit)  ((x) &   bit_msk (bit))

#define tblsize(x)      (sizeof (x) / sizeof ((x) [0]))
#define tbllast(x)      (x [tblsize (x) - 1])

#if (defined (random))
#   undef random
#   undef randomize
#endif
#if (defined (min))
#   undef min
#   undef max
#endif

#if (defined (__IS_32BIT__))
#define random(num)     (int) ((long) rand () % (num))
#else
#define random(num)     (int) ((int)  rand () % (num))
#endif
#define randomize()     srand ((unsigned) time (NULL))
#define min(a,b)        (((a) < (b))? (a): (b))
#define max(a,b)        (((a) > (b))? (a): (b))


/*- ASSERT ------------------------------------------------------------------*/

#if (defined (DEBUG))
    /*  Define _Assert function here locally                                 */
    local _Assert (char *File, unsigned Line)
    {
        fflush  (stdout);
        fprintf (stderr, "\nAssertion failed: %s, line %u\n", File, Line);
        fflush  (stderr);
        abort   ();
    }
#   define ASSERT(f)    \
        if (f)          \
            ;           \
        else            \
            _Assert (__FILE__, __LINE__)
#else
#   define ASSERT(f)
#endif


/*- Boolean operators and constants -----------------------------------------*/

#if (!defined (TRUE))
#    define TRUE        1               /*  ANSI standard                    */
#    define FALSE       0
#endif


/*- Symbolic constants ------------------------------------------------------*/

#define FORK_ERROR      -1              /*  Return codes from fork()         */
#define FORK_CHILD      0

#if (!defined (LINE_MAX))               /*  Length of line from text file    */
#   define LINE_MAX     255             /*    if not previously #define'd    */
#endif
#if (!defined (PATH_MAX))               /*  Length of path variable          */
#   define PATH_MAX     2048            /*    if not previously #define'd    */
#endif                                  /*  EDM 96/05/28                     */

#if (!defined (EXIT_SUCCESS))           /*  ANSI, and should be in stdlib.h  */
#   define EXIT_SUCCESS 0               /*    but not defined on SunOs with  */
#   define EXIT_FAILURE 1               /*    GCC, sometimes.                */
#endif


/*- System-specific definitions ---------------------------------------------*/

/*  We define the STARTUP macro: this performs any initialisation for the
 *  main function that is system-dependent.  In theory a portable main
 *  program should do this...                                                */

#if (defined (__MAC__))
#define STARTUP         argc = ccommand (&argv);
#else
#define STARTUP
#endif

/*  MSVC 1.x does not define standard signals if doing a Windows program     */

#if (defined (_MSC_VER) && !defined (SIGINT))
#    define SIGINT      2               /*  Ctrl-C sequence                  */
#    define SIGILL      4               /*  Illegal instruction              */
#    define SIGSEGV     11              /*  Segment violation                */
#    define SIGTERM     15              /*  Kill signal                      */
#    define SIGABRT     22              /*  Termination by abort()           */
#endif

/*  UNIX defines sleep() in terms of second; Win32 defines Sleep() in
 *  terms of milliseconds.  We want to be able to use sleep() anywhere.      */

#if (defined (__WINDOWS__))
#   if (defined (WIN32))
#       define sleep(a) Sleep(a*1000)   /*  UNIX sleep() is seconds          */
#   else
#       define sleep(a)                 /*  Do nothing?                      */
#   endif
#endif

/*  On SunOs, the ANSI C compiler costs extra, so many people install gcc
 *  but using the standard non-ANSI C library.  We have to make a few extra
 *  definitions for this case.  (Here we defined just what we needed for
 *  Libero and SMT -- we'll add more code as required.)                      */

#if (defined (__UTYPE_SUNOS) || defined (__UTYPE_SUNSOLARIS))
#   if (!defined (_SIZE_T))             /*  Non-ANSI headers/libraries       */
#       define strerror(n)      sys_errlist [n]
#       define memmove(d,s,l)   bcopy (s,d,l)
        extern char *sys_errlist [];
#   endif
    extern char **environ [];           /*  Not define in include files      */
#endif

/*  SCO Unix and UnixWare need a few tweaks                                  */
#if (defined (__UTYPE_SCO) || defined (__UTYPE_UNIXWARE))
    extern char **environ [];           /*  Not define in include files      */
#endif

/*  We define constants for the way the current system formats filenames;
 *  we assume that the system has some type of path concept.                 */

#if (defined (WIN32))                   /*  Windows 95/NT                    */
#   define PATHSEP      ";"             /*  Separates path components        */
#   define PATHEND      '\\'            /*  Delimits directory and filename  */
#   define PATHFOLD     FALSE           /*  Convert pathvalue to uppercase?  */
#   define NAMEFOLD     FALSE           /*  Convert filenames to uppercase?  */
#elif (defined (__MSDOS__))             /*  16-bit Windows, MS-DOS           */
#   define PATHSEP      ";"
#   define PATHEND      '\\'
#   define PATHFOLD     TRUE
#   define NAMEFOLD     TRUE
#elif (defined (__VMS__))               /*  Digital VMS, OpenVMS             */
#   define PATHSEP      ","
#   define PATHEND      ':'
#   define PATHFOLD     TRUE
#   define NAMEFOLD     TRUE
#elif (defined (__UNIX__))              /*  All UNIXes                       */
#   define PATHSEP      ":"
#   define PATHEND      '/'
#   define PATHFOLD     FALSE
#   define NAMEFOLD     FALSE
#elif (defined (__OS2__))               /*  OS/2 using EMX/GCC               */
#   define PATHSEP      ";"             /*  EDM 96/05/28                     */
#   define PATHEND      '\\'
#   define PATHFOLD     FALSE
#   define NAMEFOLD     FALSE
#elif (defined (__MAC__))               /*  GG  96/07/04                     */
#   define PATHSEP      " "
#   define PATHEND      ':'
#   define PATHFOLD     FALSE
#   define NAMEFOLD     FALSE
#else
#   error "No definitions for PATH constants"
#endif


/*- Capability definitions --------------------------------------------------*/
/*
 *  Defines zero or more of these symbols, for use in any non-portable
 *  code:
 *
 *  DOES_SOCKETS        We can use (at least some) BSD socket functions
 *  DOES_UID            We can use (at least some) uid access functions
 *  DOES_TIMERS         We can use the BSD setitimer functions
 */

#if (defined (AF_INET))
#   define DOES_SOCKETS                 /*  System supports BSD sockets      */
#else
#   undef  DOES_SOCKETS
#   define ntohs(x)     (x)             /*  Needs to be correct for the      */
#   define ntohl(x)     (x)             /*    platform as far as possible    */
#   define htons(x)     (x)
#   define htonl(x)     (x)
#endif

#if (defined (__UNIX__) || defined (__VMS__) || defined (__OS2__))
#   define DOES_UID                     /*  System supports uid functions    */
#else
#   undef  DOES_UID
    typedef int gid_t;                  /*  Group id type                    */
    typedef int uid_t;                  /*  User id type                     */
#endif

#if (defined (ITIMER_REAL))
#   define DOES_TIMERS                  /*  Supports BSD setitimer calls     */
#else
#   undef  DOES_TIMERS
#   if (!defined (SIGALRM))             /*  We can use SIGALRM in code       */
#       define SIGALRM  1               /*    though it may never happen     */
#   endif
#endif


#endif                                  /*  Include PRELUDE.H                */
