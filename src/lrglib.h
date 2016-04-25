/*===========================================================================*
 *                                                                           *
 *  lrglib.h    header file for general-purpose library functions            *
 *                                                                           *
 *  Written:    92/10/25  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/03/20                                                     *
 *                                                                           *
 *  FSM Code Generator.  Copyright (c) 1991-97 iMatix.                       *
 *                                                                           *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or        *
 *  (at your option) any later version.                                      *
 *                                                                           *
 *  This program is distributed in the hope that it will be useful,          *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU General Public License for more details.                             *
 *                                                                           *
 *  You should have received a copy of the GNU General Public License        *
 *  along with this program; if not, write to the Free Software              *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
 *===========================================================================*/

#ifndef _GLIB_INCLUDED                  /*  Allow multiple inclusions        */
#define _GLIB_INCLUDED


/*  System-specific definitions                                              */

#if (defined (__MSDOS__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under DOS we can be explict      */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#elif (defined (__VMS__))
#   define FOPEN_READ_TEXT      "r"     /*  Dec C does not like 't' or 'b'   */
#   define FOPEN_READ_BINARY    "r"
#   define FOPEN_WRITE_TEXT     "w"
#   define FOPEN_WRITE_BINARY   "w"
#   define FOPEN_APPEND_TEXT    "a"
#   define FOPEN_APPEND_BINARY  "a"
#elif (defined (__UNIX__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under UNIX we can be explict     */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#elif (defined (__OS2__))
#   define FOPEN_READ_TEXT      "rt"    /*  Under OS/2 we can be explict     */
#   define FOPEN_READ_BINARY    "rb"    /*    and use 't' or 'b' in fopen    */
#   define FOPEN_WRITE_TEXT     "wt"
#   define FOPEN_WRITE_BINARY   "wb"
#   define FOPEN_APPEND_TEXT    "at"
#   define FOPEN_APPEND_BINARY  "ab"
#else
#   error "No definitions for FOPEN constants"
#endif

/*  Type definitions                                                         */

typedef struct {
    int yyyy;
    int mm;
    int dd;
} DATE_T;

typedef struct {
    int hh;
    int mm;
    int ss;
} TIME_T;


/*  Function prototypes                                                      */

#ifdef __cplusplus
extern "C" {
#endif

FILE *FileOpen            (char *filename, char mode);
Bool  FileClose           (FILE *stream);
Bool  FileRead            (FILE *stream, char *string);
char *FileWrite           (FILE *stream, char *string);
int   FileCopy            (char *dest, char *src, char mode);
char *FileWhere           (char mode, char *path, char *name, char *ext);
Bool  FileExists          (char *filename);
Bool  SafeToExtend        (char *filename);
int   DefaultExtension    (char *dest, char *src, char *ext);
int   FixedExtension      (char *dest, char *src, char *ext);
char *StripExtension      (char *name);
char *StripFilePath       (char *name);
DATE_T *DateNow           (void);
TIME_T *TimeNow           (void);
int   JulianDate          (DATE_T *date);
int   LeapYear            (int year);
char *DtoS                (DATE_T *date, char *picture, char *dest);
int   StoD                (DATE_T *date, char *picture, char *source);
char *TtoS                (TIME_T *time, char *picture, char *dest);
int   StoT                (TIME_T *time, char *picture, char *source);
char *StrDup              (char *string);
char *StrSkp              (char *string);
char *StrSet              (char *string, char ch);
char *StrPad              (char *string, char ch, int length);
char *StrLwr              (char *string);
char *StrUpr              (char *string);
char *StrCrop             (char *string);
char *StrOpen             (char *string, Bool align);
char *StrClose            (char *string, Bool align);
char *XStrCat             (char *dest, char *src, ...);
int   LexCmp              (char *string1, char *string2);
int   StrMatch            (char *string1, char *string2);
int   OpenMessageFile     (char *filename);
void  CloseMessageFile    (void);
void  PrintMessage        (int msgid, ...);
char *MessageText         (int msgid);
void  EnableTrace         (void);
void  DisableTrace        (void);
void  SetTraceFile        (char *filename, char mode);
void  Trace               (char *format, ...);

#ifdef __cplusplus
}
#endif


/*  Symbols, macros                                                          */

#define ERROR_ANY       0000            /*  Generic error message            */
#define FILE_NAME_MAX   128             /*  Maximum size of filename         */
#define FILE_DIR_MAX    64              /*  Max size of directory name       */


/*  External variables                                                       */

extern Bool  FileCrLf;                  /*  TRUE or FALSE                    */
extern Bool  TraceState;                /*  TRUE or FALSE                    */
extern FILE *TraceFile;                 /*  Current trace output file        */

#endif
