/*===========================================================================*
 *                                                                           *
 *  lrglib.c    Various general-purpose library functions.                   *
 *                                                                           *
 *  Written:    92/10/28  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/08/28                                                     *
 *                                                                           *
 *  File i/o functions:                                                      *
 *              FileOpen            - Open file for input/output             *
 *              FileClose           - Close file                             *
 *              FileRead            - Read line of text from file            *
 *              FileWrite           - Write line of text to file             *
 *              FileCopy            - Copy one file to another               *
 *              FileWhere           - Find file on path symbol               *
 *              FileExists          - Check whether a file exists            *
 *              SafeToExtend        - Ensure that file-extend is safe        *
 *              DefaultExtension    - Add ext to filename if none already    *
 *              FixedExtension      - Add ext to filename in anycase         *
 *              StripExtension      - Remove extension from filename if any  *
 *              StripFilePath       - Remove file path from filename if any  *
 *                                                                           *
 *  String functions:                                                        *
 *              StrDup              - Duplicate string                       *
 *              StrSkp              - Skip leading spaces in string          *
 *              StrSet              - Set string to single character         *
 *              StrPad              - Pad string with character to length    *
 *              StrLwr              - Convert string to lowercase            *
 *              StrUpr              - Convert string to uppercase            *
 *              StrCrop             - Remove trailing spaces from string     *
 *              StrOpen             - Insert blank into string               *
 *              StrClose            - Remove one character from string       *
 *              StrMatch            - Calculate string similarity index      *
 *              XStrCat             - Concatenate multiple strings           *
 *              LexCmp              - Compare two strings, ignoring case     *
 *                                                                           *
 *  Date and time functions:                                                 *
 *              DateNow             - Return system date                     *
 *              TimeNow             - Return system time                     *
 *              JulianDate          - Return Julian date                     *
 *              LeapYear            - Return 1 if leap year, else 0          *
 *              PackDate            - Convert DATE_T to single dbyte         *
 *              PackTime            - Convert TIME_T to single dbyte         *
 *              UnpackDate          - Convert packed date to DATE_T          *
 *              UnpackTime          - Convert packed time to TIME_T          *
 *              DtoS                - Convert date to string                 *
 *              StoD                - Convert string to date                 *
 *              TtoS                - Convert time to string                 *
 *              StoT                - Convert string to time                 *
 *                                                                           *
 *  Error message functions:                                                 *
 *              OpenMessageFile     - Open error message file                *
 *              CloseMessageFile    - Close error message file               *
 *              PrintMessage        - Format, print message to stderr        *
 *              MessageText         - Return message text, unformatted       *
 *                                                                           *
 *  Trace functions:                                                         *
 *              EnableTrace         - Enable trace output                    *
 *              DisableTrace        - Disable trace output                   *
 *              SetTraceFile        - Direct trace output to a file          *
 *              Trace               - Display trace output                   *
 *                                                                           *
 *  Globals:    FileCrLf            - TRUE = CRLF found in input file        *
 *              TraceState          - TRUE = on, FALSE = off (default)       *
 *              TraceFile           - Trace output stream, or NULL           *
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

#include "prelude.h"                    /*  Standard header file             */
#include "lrglib.h"                     /*  Prototypes for library functions */
#include "lrpriv.h"                     /*  Private includes                 */


/*-----------------------------.
 |  FileOpen                   |
 |-----------------------------`---------------------------------------------.
 |  Description: opens the specified file for input or output.  If you use   |
 |  the FileRead / FileWrite functions you must open the file using this     |
 |  function.  This set of functions lets you read files without concern     |
 |  for the line format (CRLF or LF).  Mode should be one of 'r' 'w' 'a'.    |
 |                                                                           |
 |  Returns: a FILE pointer if the file is opened correctly; else NULL.      |
 |  Sets the global variable FileCrLf to FALSE.                              |
 `---------------------------------------------------------------------------*/

Bool FileCrLf = FALSE;                  /*  Initial default                  */

FILE *FileOpen (char *filename, char mode)
{
    ASSERT (filename != NULL);

    FileCrLf = FALSE;
    return (fopen (filename, (mode == 'r'? FOPEN_READ_BINARY:
                              mode == 'w'? FOPEN_WRITE_BINARY:
                              /* else */   FOPEN_APPEND_BINARY)));
}


/*-----------------------------.
 |  FileClose                  |
 |-----------------------------`---------------------------------------------.
 |  Description: closes an open file stream.                                 |
 |  Equivalent to fclose(); supplied here for completeness.                  |
 |                                                                           |
 |  Returns: FALSE if okay; TRUE if there were errors.                       |
 |  Sets the global variable FileCrLf to FALSE.                              |
 `---------------------------------------------------------------------------*/

Bool FileClose (FILE *stream)
{
    ASSERT (stream != NULL);
    return (fclose (stream));
}


/*-----------------------------.
 |  FileRead                   |
 |-----------------------------`---------------------------------------------.
 |  Description: reads a line of text delimited by newline from the stream.  |
 |  The string must be LINE_MAX + 1 long.  Places a null byte in place of    |
 |  the newline character.  Expands tab characters to every 8th column.      |
 |                                                                           |
 |  Returns: TRUE when there is more input waiting; FALSE when the last line |
 |  of the file has been read.                                               |
 |                                                                           |
 |  Sets the global variable FileCrLf to TRUE if a CR was found in the file. |
 |  This variable is by default FALSE.  It is also used by FileWrite.        |
 `---------------------------------------------------------------------------*/

Bool FileRead (FILE *stream, char *string)
{
    int
        ch,                             /*  Character read from file         */
        cnbr;                           /*  Index into returned string       */

    cnbr = 0;                           /*  Start at the beginning...        */
    memset (string, ' ', LINE_MAX);     /*    and prepare entire line        */
    FOREVER
      {
        ch = fgetc (stream);            /*  Get next character from file     */
        if (ch == '\t')                 /*  Jump if tab                      */
            cnbr = ((cnbr >> 3) << 3) + 8;
        else
        if (ch == '\r')                 /*  Found carriage-return            */
            FileCrLf = TRUE;            /*    Set flag and ignore CR         */
        else
        if ((ch == '\n')                /*  Have end of line                 */
        ||  (ch == EOF)                 /*    or end of file                 */
        ||  (ch == 26))                 /*    or MS-DOS Ctrl-Z               */
          {
            string [cnbr] = '\0';       /*  Terminate string                 */
            return (ch == '\n' || cnbr);    /*  and return TRUE/FALSE        */
          }
        else
        if (cnbr < LINE_MAX)
            string [cnbr++] = (char) ch;    /*  Else add char to string      */

        if (cnbr >= LINE_MAX)           /*  Return in any case if line is    */
          {                             /*    too long - the line will be    */
            string [LINE_MAX] = '\0';   /*    cut into pieces                */
            return (TRUE);
          }
      }
}


/*-----------------------------.
 |  FileWrite                  |
 |-----------------------------`---------------------------------------------.
 |  Description: writes string to the output stream.                         |
 |  If the variable FileCrLf is TRUE, adds a carriage-return to the line     |
 |  being written to the output stream.                                      |
 |                                                                           |
 |  Returns: string, or NULL if no data could be written from the stream.    |
 `---------------------------------------------------------------------------*/

char *FileWrite (FILE *stream, char *string)
{
    fputs (string, stream);
    if (FileCrLf)
        fputc ('\r', stream);

    if (fputc ('\n', stream) == EOF)
        return (NULL);
    else
        return (string);
}


/*-----------------------------.
 |  FileCopy                   |
 |-----------------------------`---------------------------------------------.
 |  Description: copies a file called src to one called dest.  The dest file |
 |  may not already exist.  If mode is 'b', copies a binary file; if mode is |
 |  't', copies a text file.                                                 |
 |                                                                           |
 |  Returns: 0 if no problems occurred, else -1 if a system error occurred,  |
 |  or 1 if the dest file already existed.                                   |
 |                                                                           |
 |  In Dec C the mode is ignored.                                            |
 `---------------------------------------------------------------------------*/

int FileCopy (char *dest, char *src, char mode)
{
    FILE *inf, *outf;
    char *buffer,
         openmode [3] = "??";
    size_t chars_read;                  /*  Amount read from stream          */
    int  feedback = 0;

    if (FileExists (dest))
        return (1);                     /*  Cancel: dest already exists      */

#   if (defined (__VMS__))
    openmode [1] = 0;
#   else
    openmode [1] = mode;
#   endif
    openmode [0] = 'r';
    if ((inf = fopen (src, openmode)) == NULL)
        return (-1);                    /*  Input file not found             */

    if ((buffer = malloc (SHRT_MAX)) == NULL)
        feedback = -1;                  /*  Insufficient memory for buffer   */
    else
      {
        openmode [0] = 'w';
        outf = fopen (dest, openmode);
        while ((chars_read = fread (buffer, 1, SHRT_MAX, inf)) != 0)
            if (fwrite (buffer, 1, chars_read, outf) != chars_read)
              {
                feedback = -1;
                break;
              }
        fclose (outf);
        free (buffer);
      }
    fclose (inf);
    return (feedback);
}


/*-----------------------------.
 |  FileExists                 |
 |-----------------------------`---------------------------------------------.
 |  Description: returns TRUE if the file exists, or FALSE if it does not.   |
 `---------------------------------------------------------------------------*/

Bool FileExists (char *filename)
{
    FILE *file;

    file = fopen (filename, "r");
    if (file)
        fclose (file);
    return (file != NULL);
}


/*  Ensure our buffers will be big enough for dir + name + delimiters        */
#if ((LINE_MAX - FILE_NAME_MAX) < (FILE_DIR_MAX + 10))
#   error "Cannot compile; FILE_NAME_MAX is too large."
#endif

static char *BuildNextPath (char *dest, char *path, char *name);

/*  -------------------------------------------------------------------------
 *  FileWhere
 *
 *  Returns a fully-specified filename, by searching along a path and adding
 *  an extension as required.  Input parameters are:
 *
 *  mode:   one of 'r' = read, 'w' = write, 'a' = append, or 's' = static.
 *          This function tries to locate existing files somewhere on the
 *          path.  New files are always created in the current directory.
 *          'static' files are created in the first directory on the path.
 *
 *  path:   only used when mode is 'r', 'a', or 'a'.  If path is NULL or
 *          empty, it is ignored.  Otherwise, path is translated as an
 *          environment variable, and cut into a list of directory names.
 *          The path is cut up as follows:
 *
 *          MS-DOS: directory names separated by ';'.  ;; means current.
 *          OS/2:   directory names separated by ';'.  ;; means current.
 *          Unix:   directory names separated by ':'.  :: means current.
 *          VMS:    directory names separated by ','.  " ", means current.
 *          Other:  single directory name.
 *
 *          When the mode is 'r' or 'a', searches the current directory
 *          before considering the path value.
 *
 *          When the path cannot be translated, and is not null or empty, it
 *          is used as a literal value.
 *
 *  name:   file name with or without extension.  Will be prefixed by the
 *          path if required, and optionally the extension.
 *
 *  ext:    default or mandatory extension.  If ext starts with a dot '.',
 *          it is always used.  Otherwise the extension is used only if the
 *          name does not already have an extension.  If ext is NULL or "",
 *          it is ignored.
 *
 *  The total length of a name including path, name, extension, and any
 *  delimiters is FILE_NAME_MAX.  Names are truncated if too long.
 *  Maximum size of one directory component is FILE_DIR_MAX chars.
 *
 *  All parameters are case-sensitive; the precise effect of this depends on
 *  the system.  On MS-DOS, filenames are always folded to uppercase, but the
 *  path symbol name must be supplied in uppercase correctly.
 *  On UNIX the path name and filenames are both case sensitive.
 *  On VMS, path name and filenames are folded into uppercase.
 *
 *  Returns a pointer to a static character array containing the filename;
 *  if mode is 'r' and the file does not exist, returns NULL.  If the mode is
 *  'w', 'a', or 's', always returns a valid filename.
 */

char *
FileWhere (char mode, char *path, char *name, char *ext)
{
    static char
#       if (PATHFOLD == TRUE)
        path_name [PATH_MAX + 1],       /*  Copy of path symbol              */
#       endif
        work_name [LINE_MAX + 1],       /*  Name plus ext                    */
        full_name [LINE_MAX + 1];       /*  Dir plus name plus ext           */
    char
        *pathptr;                       /*  End of directory in Path         */

    if (ext != NULL && *ext)            /*  Append extension if not null     */
      {                                 /*    to get name + ext into         */
        if (ext [0] == '.')             /*    work_name.                     */
            FixedExtension (work_name, name, ext);
        else
            DefaultExtension (work_name, name, ext);
      }
    else
        strcpy (work_name, name);

#if (NAMEFOLD == TRUE)
    StrUpr (work_name);                 /*  Fold to uppercase if needed      */
#endif

    if (path != NULL && *path)          /*  Get value of path, or NULL       */
      {
        pathptr = getenv (path);        /*  Translate path symbol            */
        if (pathptr == NULL)
            pathptr = path;             /*  If not found, use literally      */
#if (PATHFOLD == TRUE)                  /*  Fold to uppercase if necessary   */
        if (pathptr)
          {
            strcpy (path_name, pathptr);
            pathptr = path_name;
            StrUpr (path_name);
          }
#endif
      }
    else
        pathptr = NULL;

    /*  Take care of 'w' and 's' options first                               */
    if (mode == 'w')                    /*  Create output file locally       */
        return (work_name);

    if (mode == 's')                    /*  Get specific directory name      */
      {
        BuildNextPath (full_name, pathptr, work_name);
        return (full_name);
      }

    if (FileExists (work_name))         /*  Find file in current directory?  */
        return (work_name);             /*  Then return name + ext           */

#   if (defined (__VMS__))
    /*  VMS lets you open a file like this 'path:name'                       */
    if (pathptr)
        sprintf (full_name, "%s:%s", path, work_name);
    else
        strcpy (full_name, work_name);

    if ((mode == 'a')                   /*  Create file locally or find it   */
    ||  (FileExists (full_name)))       /*    using path symbol value        */
        return (full_name);
    else
        return (NULL);                  /*  Not found for reading            */
#   else

    if (!pathptr)                       /*  Now we need a path               */
        return (NULL);                  /*   - if none defined, give up      */

    FOREVER                             /*  Try each path component          */
      {
        pathptr = BuildNextPath (full_name, pathptr, work_name);
        if (FileExists (full_name))
            return (full_name);         /*  Until we find one,               */

        if (*pathptr == '\0')           /*    or we come to the end of       */
          {                             /*    the path                       */
            if (mode == 'r')
                return (NULL);          /*  Input file was not found...      */
            else
                return (full_name);
          }
      }
#   endif
}


static char *
BuildNextPath (char *dest, char *path, char *name)
{
    int
        length;                         /*  length of directory name         */

    length = strcspn (path, PATHSEP);
    strncpy (dest, path, length);
    path += length;                     /*  Bump past path delimiter         */
    if (*path)                          /*    unless we are at the end       */
        path++;                         /*    of the path                    */

    if ((length)
    && (dest [length - 1] != PATHEND))
        dest [length++] = PATHEND;      /*  Add path-to-filename delimiter   */

    dest [length] = '\0';
    strcat (dest, name);
    return (path);
}


/*-----------------------------.
 |  SafeToExtend               |
 |-----------------------------`---------------------------------------------.
 |  Bool SafeToExtend (char *filename)                                       |
 |                                                                           |
 |  Handles system-specific case of extending a file that may not be in a    |
 |  valid state for such an operation.  Returns TRUE if the extend can go    |
 |  ahead; returns FALSE if the extend cannot be permitted.                  |
 |                                                                           |
 |  MS-DOS: if the last byte in the file is Ctrl-Z (26), the file is         |
 |          truncated by 1 position to remove this character.                |
 `---------------------------------------------------------------------------*/

Bool SafeToExtend (char *filename)
{
#if (defined (__MSDOS__))
    int  handle;                        /*  Opened file handle               */
    char endoffile;                     /*  Last character in file           */

    handle = open (filename, O_RDWR + O_BINARY, S_IREAD | S_IWRITE);
    if (handle)                         /*  If not found, ignore             */
      {
        lseek (handle, -1, SEEK_END);
        read  (handle, &endoffile, 1);
        if (endoffile == 26)
            chsize (handle, filelength (handle) - 1);

        close (handle);
      }
#endif
    return (TRUE);
}


/*-----------------------------.
 |  DefaultExtension           |
 |-----------------------------`---------------------------------------------.
 |  Description: copies src to dest and adds ext if necessary.               |
 |                                                                           |
 |  Returns: 0 if the name was okay, -1 if it was invalid.  Does not print   |
 |  any error messages for invalid names.                                    |
 |                                                                           |
 |  Comments: dest must be large enough for fully-formatted filename.  ext   |
 |  can start with or without a point.  If ext is null or empty, exits.      |
 `---------------------------------------------------------------------------*/

int DefaultExtension (char *dest, char *src, char *ext)
{
    int len, i;
    char *ptr;

    if (dest != src)                    /*  Copy src to dest if not same     */
        strcpy (dest, src);

    if (ext != NULL && *ext != 0)
      {
        len = strlen (dest);
        for (i = len - 1, ptr = dest + i; i >= 0; i--, ptr--)
            if (*ptr == '\\' || *ptr == '/' || *ptr == '.')
                break;

        if (i < 0 || *ptr != '.')
          {
            if (*ext != '.')
              {
                dest [len++] = '.';
                dest [len] = '\0';
              }
            strcat (dest + len, ext);
          }
      }
    return (0);
}


/*-----------------------------.
 |  FixedExtension             |
 |-----------------------------`---------------------------------------------.
 |  Description: copies src to dest, removes any existing extension and adds |
 |  ext to dest.  Does not change src.                                       |
 |                                                                           |
 |  Returns: 0 if the name was okay, -1 if it was invalid.  Does not print   |
 |  any error messages for invalid names.                                    |
 |                                                                           |
 |  Comments: dest must be large enough for fully-formatted filename.  ext   |
 |  can start with or without a point.                                       |
 `---------------------------------------------------------------------------*/

int FixedExtension (char *dest, char *src, char *ext)
{
    if (dest != src)                    /*  Copy src to dest if not same     */
        strcpy (dest, src);

    StripExtension (dest);
    return (DefaultExtension (dest, dest, ext));
}


/*-----------------------------.
 |  StripExtension             |
 |-----------------------------`---------------------------------------------.
 |  Description: removes dot and extension from name, if any was present.    |
 |                                                                           |
 |  Returns: name.                                                           |
 `---------------------------------------------------------------------------*/

char *StripExtension (char *name)
{
    char *dot, *slash;

    dot = strrchr (name, '.');          /*  Find dot in name, if any         */
    slash = strrchr (name, '\\');       /*  Find last slash (DOS or Unix)    */
    if (slash == NULL)
        slash = strrchr (name, '/');
    if (dot > slash)
        *dot = 0;                       /*  If we had a dot, truncate name   */

    return (name);
}


/*-----------------------------.
 |  StripFilePath              |
 |-----------------------------`---------------------------------------------.
 |  Description: removes leading path from name, if any was present.         |
 |                                                                           |
 |  Returns: name.                                                           |
 `---------------------------------------------------------------------------*/

char
*StripFilePath (char *name)
{
    char *path_end;

    ASSERT (name != NULL);
    path_end = strrchr (name, PATHEND); /*  Find end of path, if any         */
    if (path_end != NULL)
        memmove (name, path_end + 1, strlen (path_end));
    return (name);
}


/*-----------------------------.
 |  StrDup                     |
 |-----------------------------`---------------------------------------------.
 |  Makes a duplicate of string, obtaining space with a call to malloc().    |
 |  The allocated space is strlen (string) + 1 bytes long.  The caller is    |
 |  responsible for freeing the space allocated by StrDup when it is no      |
 |  longer needed.  Returns a pointer to the allocated string, which holds   |
 |  a copy of the parameter string.  Returns NULL if there was insufficient  |
 |  heap storage available to allocate the string, or if the original string |
 |  was itself NULL.                                                         |
 `---------------------------------------------------------------------------*/

char *StrDup (char *string)
{
    char *copy;

    if (string)
      {
        copy = malloc (strlen (string) + 1);
        if (copy)
            strcpy (copy, string);
      }
    else
        copy = NULL;

    return (copy);
}


/*-----------------------------.
 |  StrSkp                     |
 |-----------------------------`---------------------------------------------.
 |  Description: skips leading spaces in string, and returns first non-blank |
 |  character.  When this is a null, the end of the string was reached.      |
 `---------------------------------------------------------------------------*/

char *StrSkp (char *string)
{
    while (*string == ' ')
        string++;
    return (string);
}


/*-----------------------------.
 |  StrSet                     |
 |-----------------------------`---------------------------------------------.
 |  Description: sets all characters in string up to but not including the   |
 |  final null character to ch.  Returns string.                             |
 `---------------------------------------------------------------------------*/

char *StrSet (char *string, char ch)
{
    char *scan;

    scan = string;
    while (*scan)
        *scan++ = ch;
    return (string);
}


/*-----------------------------.
 |  StrPad                     |
 |-----------------------------`---------------------------------------------.
 |  Description: returns string of length characters, padding with ch or     |
 |  truncating if necessary.  String must be at least length + 1 long.       |
 `---------------------------------------------------------------------------*/

char *StrPad (char *string, char ch, int length)
{
    int cursize;

    cursize = strlen (string);          /*  Get current length of string     */
    while (cursize < length)            /*  Pad until at desired length      */
        string [cursize++] = ch;

    string [cursize++] = '\0';          /*  Add terminating null             */
    return (string);                    /*    and return to caller           */
}


/*-----------------------------.
 |  StrLwr                     |
 |-----------------------------`---------------------------------------------.
 |  Description: converts all alphabetic characters in string to lowercase,  |
 |  stopping at the final null character.  Returns string.  If string is     |
 |  null, returns null.                                                      |
 `---------------------------------------------------------------------------*/

char *StrLwr (char *string)
{
    char *scan;

    if (string)
      {
        scan = string;
        while (*scan)
          {
            *scan = (char) tolower (*scan);
            scan++;
          }
      }
    return (string);
}


/*-----------------------------.
 |  StrUpr                     |
 |-----------------------------`---------------------------------------------.
 |  Description: converts all alphabetic characters in string to uppercase,  |
 |  stopping at the final null character.  Returns string.  If string is     |
 |  null, returns null.                                                      |
 `---------------------------------------------------------------------------*/

char *StrUpr (char *string)
{
    char *scan;

    if (string)
      {
        scan = string;
        while (*scan)
          {
            *scan = (char) toupper (*scan);
            scan++;
          }
      }
    return (string);
}


/*-----------------------------.
 |  StrCrop                    |
 |-----------------------------`---------------------------------------------.
 |  Description: drops trailing whitespace from string by truncating string  |
 |  to the last non-whitespace character.  Returns string.  If string is     |
 |  null, returns null.                                                      |
 `---------------------------------------------------------------------------*/

char *StrCrop (char *string)
{
    char *last;

    if (string)
      {
        last = string + strlen (string);
        while (last > string)
          {
            if (!isspace (*(last - 1)))
                break;
            last--;
          }
        *last = 0;
      }
    return (string);
}


/*-----------------------------.
 |  StrOpen                    |
 |-----------------------------`---------------------------------------------.
 |  Description: inserts a character at string, and places a blank in the    |
 |  gap.  If align is TRUE, makes room by reducing the size of the next gap  |
 |  of 2 or more spaces.  If align is FALSE, extends the size of the string. |
 |  Returns string.                                                          |
 `---------------------------------------------------------------------------*/

char *StrOpen (char *string, Bool align)
{
    char *gap;
    int  length;

    length = strlen (string) + 1;       /*  By default, move string + NULL   */
    if (align)                          /*  If align is TRUE, find gap       */
      {
        gap = strstr (string, "  ");
        if (gap)
            length = (int) (gap - string);
      }
    memmove (string + 1, string, length);
    string [0] = ' ';                   /*  Stick a space into string        */
    return (string);
}


/*-----------------------------.
 |  StrClose                   |
 |-----------------------------`---------------------------------------------.
 |  Description: removes the character at string, and shifts the remainder   |
 |  down by one.  If align is TRUE, only shifts up to the next gap of 2 or   |
 |  more spaces.  Returns string.                                            |
 `---------------------------------------------------------------------------*/

char *StrClose (char *string, Bool align)
{
    char *gap;
    int  length;

    length = strlen (string);           /*  By default, move string + NULL   */
    if (align) {                        /*  If align is TRUE, find gap       */
        gap = strstr (string, "  ");
        if (gap && gap != string)
            length = (int) (gap - string);
    }
    memmove (string, string + 1, length);
    return (string);
}


/*-----------------------------.
 |  StrMatch                   |
 |-----------------------------`---------------------------------------------.
 |  Description: calculates a similarity index for the two strings.  This    |
 |  is a value from 0 to 32767 with higher values indicating a closer match. |
 |  The two strings are compared without regard for case.                    |
 |                                                                           |
 |  The algorithm was designed by Leif Svalgaard (lsvalg@ibm.net).           |
 `---------------------------------------------------------------------------*/

int StrMatch (char *string1, char *string2)
{
    static int
        name_weight [30] = {
            20, 15, 13, 11, 10, 9, 8, 8, 7, 7, 7, 6, 6, 6, 6,
             6,  5,  5,  5,  5, 5, 4, 4, 4, 4, 4, 4, 4, 4, 4
        };
    int
        comp_index,
        name_index,
        start_of_string,
        longest_so_far,
        substring_contribution,
        substring_length,
        compare_length,
        longest_length,
        length_difference,
        name_length,
        char_index,
        similarity_index,
        similarity_weight;
    char
        cur_name_char;

    name_length    = strlen (string1);
    compare_length = strlen (string2);
    if (name_length > compare_length)
      {
        length_difference = name_length - compare_length;
        longest_length    = name_length;
      }
    else
      {
        length_difference = compare_length - name_length;
        longest_length    = compare_length;
      }
    if (compare_length)
      {
        similarity_weight = 0;
        substring_contribution = 0;

        for (char_index = 0; char_index < name_length; char_index++)
          {
            start_of_string = char_index;
            cur_name_char   = (char) tolower (string1 [char_index]);
            longest_so_far  = 0;
            comp_index      = 0;

            while (comp_index < compare_length)
              {
                while ((comp_index < compare_length)
                &&     (tolower (string2 [comp_index]) != cur_name_char))
                    comp_index++;

                substring_length = 0;
                name_index = start_of_string;

                while ((comp_index < compare_length)
                &&     (tolower (string2 [comp_index])
                     == tolower (string1 [name_index])))
                  {
                    if (comp_index == name_index)
                        substring_contribution++;
                    comp_index++;
                    if (name_index < name_length)
                      {
                        name_index++;
                        substring_length++;
                      }
                  }
                substring_contribution += (substring_length + 1) * 3;
                if (longest_so_far < substring_length)
                    longest_so_far = substring_length;
              }
            similarity_weight += (substring_contribution
                                  + longest_so_far + 1) * 2;
            similarity_weight /= name_length + 1;
          }
        similarity_index  = (name_length < 30? name_weight [name_length]: 3)
                          * longest_length;
        similarity_index /= 10;
        similarity_index += 2 * length_difference / longest_length;
        similarity_index  = 100 * similarity_weight / similarity_index;
      }
    else
        similarity_index = 0;

    return (similarity_index);
}

/*-----------------------------.
 |  XStrCat                    |
 |-----------------------------`---------------------------------------------.
 |  Description: concatenates multiple strings into a single result.  Eg.    |
 |  XStrCat (buffer, "A", "B", NULL) stores "AB" in buffer.  Returns dest.   |
 |                                                                           |
 |  [DDJ Nov 1992, p. 155]                                                   |
 `---------------------------------------------------------------------------*/

char *XStrCat (char *dest, char *src, ...)
{
    char
        *feedback = dest;
    va_list
        va;

    while (*dest)                       /*  Find end of dest string          */
        dest++;
    va_start (va, src);
    while (src != NULL) {
        while (*src)
            *dest++ = *src++;
        src = va_arg (va, char *);
    }
    *dest = '\0';                       /*  Append a null character          */
    va_end (va);
    return (feedback);
}


/*-----------------------------.
 |  LexCmp                     |
 |-----------------------------`---------------------------------------------.
 |  Description: performs unsigned comparison of two strings without regard  |
 |  to the case of any letters in the strings.  Returns a value that is      |
 |                                                                           |
 |   < 0  if string1 is less than string2                                    |
 |   == 0 if string1 is equal to string2                                     |
 |   > 0  if string1 is greater than string2                                 |
 `---------------------------------------------------------------------------*/

int LexCmp (char *string1, char *string2)
{
    int cmp;

    do {
        cmp = (byte) tolower (*string1) - (byte) tolower(*string2);
    } while (*string1++ && *string2++ && cmp == 0);
    return (cmp);
}


/*-----------------------------.
 |  DateNow                    |
 |-----------------------------`---------------------------------------------.
 |  Description: returns pointer to DATE_T structure holding current date.   |
 |                                                                           |
 |  Comments: the returned pointer is to a static area which is overwritten  |
 |  with each call to DateNow.  The DATE_T structure is defined as follow:   |
 |                                                                           |
 |      typedef struct {                                                     |
 |          int yyyy;                                                        |
 |          int mm;                                                          |
 |          int dd;                                                          |
 |      } DATE_T;                                                            |
 |                                                                           |
 `---------------------------------------------------------------------------*/

DATE_T *DateNow (void)
{
    time_t        time_secs;
    struct tm    *time_struct;
    static DATE_T date_now;

    time_secs     = time (NULL);
    time_struct   = localtime (&time_secs);
    date_now.yyyy = time_struct-> tm_year + (time_struct-> tm_year > 80? 1900
                                                                       : 2000);
    date_now.mm   = time_struct-> tm_mon  + 1;
    date_now.dd   = time_struct-> tm_mday;
    return (&date_now);
}


/*-----------------------------.
 |  TimeNow                    |
 |-----------------------------`---------------------------------------------.
 |  Description: returns pointer to TIME_T structure holding current time.   |
 |                                                                           |
 |  Comments: the returned pointer is to a static area which is overwritten  |
 |  with each call to TimeNow.  The TIME_T structure is defined as follow:   |
 |                                                                           |
 |      typedef struct {                                                     |
 |          int hh;                                                          |
 |          int mm;                                                          |
 |          int ss;                                                          |
 |      } TIME_T;                                                            |
 |                                                                           |
 `---------------------------------------------------------------------------*/


TIME_T *TimeNow (void)
{
    time_t        time_secs;
    struct tm    *time_struct;
    static TIME_T time_now;

    time_secs   = time (NULL);
    time_struct = localtime (&time_secs);
    time_now.hh = time_struct-> tm_hour;
    time_now.mm = time_struct-> tm_min;
    time_now.ss = time_struct-> tm_sec;
    return (&time_now);
}


/*-----------------------------.
 |  JulianDate                 |
 |-----------------------------`---------------------------------------------.
 |  Description: returns the number of days since 0 January.                 |
 `---------------------------------------------------------------------------*/

int JulianDate (DATE_T *date)
{
    int julian;
    int days [12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};

    julian = days [date-> mm - 1] + date-> dd;
    if (date-> mm > 2)
        julian += LeapYear (date-> yyyy);

    return (julian);
}


/*-----------------------------.
 |  LeapYear                   |
 |-----------------------------`---------------------------------------------.
 |  Description: returns TRUE if the year is a leap year, else FALSE.        |
 |                                                                           |
 |  Comments: handles centuries (1900, 2000, ...) correctly.                 |
 `---------------------------------------------------------------------------*/

int LeapYear (int year)
{
    Bool leap;

    leap = FALSE;
    if (year % 4 == 0)
        leap = TRUE;
        if ((year % 100 == 0)
        &&  (year % 400 != 0))
            leap = FALSE;

    return (leap);
}


/*-----------------------------.
 |  PackDate                   |
 |-----------------------------`---------------------------------------------.
 |  Description: packs the date into a single dbyte (unsigned short). Use    |
 |  this to store a date when memory space is limited.  The date is packed   |
 |  into bitfields, and can be correctly used in comparisons.                |
 |                                                                           |
 |  Returns: a dbyte containing the packed date.                             |
 |                                                                           |
 |  Comments: this packed format is used by MS-DOS to store a file's date.   |
 `---------------------------------------------------------------------------*/

dbyte PackDate (DATE_T *date)
{
    int year;

    year = date-> yyyy - 1980;          /*  Store year as offset since 1980  */
    if (year < 0)
        year += 1900;                   /*  Permit year with century = zero  */

    return (dbyte) ((year << 9) + (date-> mm << 5) + date-> dd);
}


/*-----------------------------.
 |  PackTime                   |
 |-----------------------------`---------------------------------------------.
 |  Description: packs the time into a single dbyte (unsigned short). Use    |
 |  this to store a time when memory space is limited.  The time is packed   |
 |  into bitfields, and can be correctly used in comparisons.                |
 |                                                                           |
 |  Returns: a dbyte containing the packed time.                             |
 |                                                                           |
 |  Comments: this packed format is used by MS-DOS to store a file's time.   |
 |  The seconds are stored with 2-second accuracy, so conversion back to a   |
 |  TIME_T will lose 1 second accuracy in 50% of cases.                      |
 `---------------------------------------------------------------------------*/

dbyte PackTime (TIME_T *time)
{
    return (dbyte) ((time-> hh << 11) + (time-> mm << 5) + (time-> ss >> 1));
}


/*-----------------------------.
 |  UnpackDate                 |
 |-----------------------------`---------------------------------------------.
 |  Description: converts a packed date to a DATE_T structure.               |
 |                                                                           |
 |  Comments: this function reverses PackDate ().                            |
 `---------------------------------------------------------------------------*/

void UnpackDate (DATE_T *date, dbyte packdate)
{
    date-> yyyy = ((dbyte) (packdate & 0xfe00) >> 9) + 1980;
    date-> mm   =  (dbyte) (packdate & 0x1e0) >> 5;
    date-> dd   =           packdate & 0x1f;
}


/*-----------------------------.
 |  UnpackTime                 |
 |-----------------------------`---------------------------------------------.
 |  Description: converts a packed time to a TIME_T structure.               |
 |                                                                           |
 |  Comments: this function reverses PackTime ().                            |
 `---------------------------------------------------------------------------*/

void UnpackTime (TIME_T *time, dbyte packtime)
{
    time-> hh = (dbyte) (packtime & 0xf800) >> 11;
    time-> mm = (dbyte) (packtime & 0x7e0) >> 5;
    time-> ss = (dbyte) (packtime & 0x1f) << 1;
}


/*-----------------------------.
 |  DtoS                       |
 |-----------------------------`---------------------------------------------.
 |  Description: converts date to a string according to picture.  Picture    |
 |  should contain one character for each position in dest, and be composed  |
 |  of some or any of these pieces:                                          |
 |                                                                           |
 |          CC    - century as two digits.                                   |
 |          YY    - year as two digits.                                      |
 |          MM    - month as two digits.                                     |
 |          MMM.. - month as abbreviated or full name.                       |
 |          DD    - day as two digits.                                       |
 |          other - output as date separator                                 |
 |                                                                           |
 |  A leading zero in dest is converted to a space.  If DD follows a space   |
 |  in picture, any leading zero is removed (eg. "MMMMMMMMM DD, CCYY").      |
 |  If date-> year is zero, assumes that the rest of the date is zero, and   |
 |  formats dest as spaces.  Returns dest.                                   |
 `---------------------------------------------------------------------------*/

static int  rlen,                       /*  Length of picture component      */
            dptr;                       /*  Output dest size                 */
static char pchr;                       /*  Current picture char             */

local DtoS_component (int value, char *dest);

char *DtoS (DATE_T *date, char *picture, char *dest)
{
    char *month_name [] = {
        "January",
        "February",
        "March",
        "April",
        "May",
        "June",
        "July",
        "August",
        "September",
        "October",
        "November",
        "December"
    };

    int pptr, plen;                     /*  Pointer and picture length       */
    int mlen;                           /*  Month name length                */
    int month;                          /*  Month 0 - 11                     */

    plen = strlen (picture);
    strcpy (dest, picture);
    StrSet (dest, ' ');

    if (date-> yyyy == 0)               /*  Is date zero?                    */
        return (dest);
    else
    if (date-> yyyy < 100)              /*  No century given?                */
        date-> yyyy += 1900;

    pptr = 0;
    dptr = 0;
    do {
        pchr = picture [pptr];
        for (rlen = 0; picture [pptr] == pchr; rlen++, pptr++);

        switch (pchr) {
            case 'C':
                DtoS_component (date-> yyyy / 100, dest);
                break;

            case 'Y':
                DtoS_component (date-> yyyy % 100, dest);
                break;

            case 'M':
                if (rlen < 3)
                    DtoS_component (date-> mm, dest);
                else {
                    month = date-> mm - 1;
                    mlen = strlen (month_name [month]);
                    if (rlen > mlen)
                        rlen = mlen;
                    memcpy (&dest [dptr], month_name [month], rlen);
                    dptr += rlen;
                }
                break;

            case 'D':
                DtoS_component (date-> dd, dest);
                if ((dptr > 2)
                && (dest [dptr - 3] == ' ')
                && (dest [dptr - 2] == '0')) {
                    dest [dptr - 2] = dest [dptr - 1];
                    dest [--dptr] = ' ';
                }
                break;

            default:
                while (rlen--)
                    dest [dptr++] = pchr;
        }
    } until (pptr >= plen);
    if (dest [0] == '0')                /*  Suppress leading zero,           */
        if ((dest [2] < '0')            /*    if third char in result        */
        ||  (dest [2] > '9'))           /*    is a separator.                */
            dest [0] = ' ';

    dptr = plen - 1;
    while (dest [dptr] == ' ')
        dest [dptr--] = 0;

    return (dest);
}

local DtoS_component (int value, char *dest)
{
    if (rlen == 2) {
        dest [dptr++] = (char) (value / 10 + '0');
        dest [dptr++] = (char) (value % 10 + '0');
    } else
        dest [dptr++] = pchr;
}


/*-----------------------------.
 |  StoD                       |
 |-----------------------------`---------------------------------------------.
 |  Description: converts source to a date according to the picture.  The    |
 |  picture must be one of: "DYM", "YMD", "MDY".  The conversion is relaxed  |
 |  and allows values like: 010195, 1-1-95, 1-Jan-95, 1jan95, 01jan95,       |
 |  1 1 95.  The source string must be null-terminated.                      |
 |                                                                           |
 |  Returns: -1 if the date is invalid or incomplete, 0 if no errors.  If    |
 |  an error occurred, the contents of date are undefined.                   |
 `---------------------------------------------------------------------------*/

int StoD (DATE_T *date, char *picture, char *source)
{
    char *month_name [] =
      {
        "jan", "feb", "mar", "apr", "may", "jun",
        "jul", "aug", "sep", "oct", "nov", "dec"
      };
    int month_days [] =
      {
        31, 29, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
      };

    char
        date_digits   [9],              /*  8 digits of date                 */
        month_letters [4],              /*  3 characters of month            */
        ch,                             /*  Next character in date           */
       *y_ptr,                          /*  Where is year in date?           */
       *m_ptr,                          /*  Where is month in date?          */
       *d_ptr;                          /*  Where is day in date?            */
    int
        digits,                         /*  Number of digits in source       */
        feedback,                       /*  0 if okay, -1 if errors          */
        delimiters,                     /*  Number of delimiters in date     */
        digitseq,                       /*  Number of digits in sequence     */
        count,                          /*  Number of month letters          */
        year,                           /*  Date year value                  */
        month,                          /*  Date month value                 */
        day;                            /*  Date day value                   */
    Bool
        had_month;                      /*  Did we already get a month?      */

    /*  Collect date digits                                                  */
    digits     = 0;                     /*  Nothing collected so far         */
    digitseq   = 0;                     /*  No digits in sequence            */
    feedback   = 0;                     /*  No errors so far                 */
    delimiters = 0;                     /*  We allow up to 2 delimiters      */
    had_month  = FALSE;                 /*  True after 3-letter month seen   */

    while (*source)
      {
        ch = *source++;
        if (isdigit (ch))
          {
            if (digits < 8)
              {
                digitseq++;
                date_digits [digits++] = ch;
              }
            else
                feedback = -1;
          }
        else
          {
            /*  Fill-up to even number of digits                             */
            if (digits > (digits / 2) * 2)
              {
                date_digits [digits] = date_digits [digits - 1];
                date_digits [digits - 1] = '0';
                digits++;
              }
            /*  3 or 5 in a row is not allowed                               */
            if (digitseq == 3 || digitseq == 5)
                feedback = -1;
            digitseq = 0;

            /*  If a letter, try to match against a month                    */
            if (isalpha (ch))
              {
                if (had_month)
                    feedback = -1;
                else
                  {
                    for (count = 0; count < 4; count++)
                      {
                        month_letters [count] = (char) tolower (ch);
                        ch = *source;
                        if (isalpha (ch))
                            source++;
                        else
                          {
                            feedback = -1;
                            break;
                          }
                      }
                    month_letters [3] = 0;
                    for (count = 0; count < 12; count++)
                        if (streq (month_letters, month_name [count]))
                          {
                            count++;
                            date_digits [digits++] = (char) (count / 10 + '0');
                            date_digits [digits++] = (char) (count % 10 + '0');
                          }
                    had_month = TRUE;
                  }
              }
            else
            if (ispunct (ch))
                if (++delimiters > 2)
                    feedback = -1;
          }
      }
    if (streq (picture, "DMY"))
      {
        y_ptr = date_digits + 4;
        m_ptr = date_digits + 2;
        d_ptr = date_digits;
      }
    else
    if (streq (picture, "MDY"))
      {
        y_ptr = date_digits + 4;
        m_ptr = date_digits;
        d_ptr = date_digits + 2;
      }
    else
    if (streq (picture, "YMD"))
      {
        y_ptr = date_digits;
        if (digits == 8)
          {
            m_ptr = date_digits + 4;
            d_ptr = date_digits + 6;
          }
        else
          {
            m_ptr = date_digits + 2;
            d_ptr = date_digits + 4;
          }
      }
    else
        return (-1);                    /*  Error - bad picture              */

    if (digits == 0)                    /*  Return zero date                 */
      {
        year  =
        month =
        day   = 0;
      }
    else
    if (digits == 6 || digits == 8)
      {
        day   = (d_ptr [0] - '0') * 10 + (d_ptr [1] - '0');
        month = (m_ptr [0] - '0') * 10 + (m_ptr [1] - '0');
        year  = (y_ptr [0] - '0') * 10 + (y_ptr [1] - '0');
        if (digits == 8)
            year = (y_ptr [2] - '0') * 10 + (y_ptr [3] - '0') + year * 100;

        if (year < 50)
            year += 2000;
        else
        if (year < 100)
            year += 1900;

        if ((month == 0 || month > 12)
        ||  (day   == 0 || day   > month_days [month - 1])
        ||  (month == 2 && day == 29 && !LeapYear (year)))
            feedback = -1;
      }
    else
        return (-1);                    /*  Error - bad size for date        */

    date-> yyyy = year;
    date-> mm   = month;
    date-> dd   = day;

    return (feedback);
}


/*-----------------------------.
 |  TtoS                       |
 |-----------------------------`---------------------------------------------.
 |  Description: converts time to a string according to picture.  Picture    |
 |  should contain one character for each position in dest, and be composed  |
 |  of some or any of these pieces:                                          |
 |                                                                           |
 |      HH    - hour as two digits, 24-hour clock                            |
 |      II    - hour as two digits, 12-hour clock, leading space.            |
 |      MM    - minute as two digits.                                        |
 |      SS    - seconds as two digits.                                       |
 |      ##    - am/pm indicator.                                             |
 |      other - output as time separator                                     |
 |                                                                           |
 |  A leading zero in dest is converted to a space.  If the hour is zero,    |
 |  the destination is formatted as spaces.  Returns dest.                   |
 `---------------------------------------------------------------------------*/

char *TtoS (TIME_T *time, char *picture, char *dest)
{
    int pptr, plen;                     /*  Pointer and picture length       */
    int hour;

    plen = strlen (picture);
    strcpy (dest, picture);
    StrSet (dest, ' ');

    if (time-> hh == 0)                 /*  Is time zero?                    */
        return (dest);

    pptr = 0;
    dptr = 0;
    do {
        pchr = picture [pptr];
        for (rlen = 0; picture [pptr] == pchr; rlen++, pptr++);

        switch (pchr) {
            case 'H':
                DtoS_component (time-> hh, dest);
                break;

            case 'I':
                hour = (time-> hh - 1) % 12 + 1;
                DtoS_component (hour, dest);
                if (hour < 10)
                    dest [dptr - 2] = ' ';
                break;

            case 'M':
                DtoS_component (time-> mm, dest);
                break;

            case 'S':
                DtoS_component (time-> ss, dest);
                break;

            case '#':
                if (rlen == 2) {
                    if (time-> hh < 12)
                        dest [dptr++] = 'a';
                    else
                        dest [dptr++] = 'p';
                    pchr = 'm';
                }
            default:
                dest [dptr++] = pchr;
        }
    } until (pptr >= plen);

    if (dest [0] == '0')                /*  Suppress leading zero            */
        dest [0] = ' ';

    return (dest);
}


/*-----------------------------.
 |  StoT                       |
 |-----------------------------`---------------------------------------------.
 |  Description: converts source to a time according to the picture.  The    |
 |  picture must follow the rules for TtoS.                                  |
 |                                                                           |
 |  Returns: -1 if the time is invalid or incomplete, 0 if no errors.  If    |
 |  an error occurred, the contents of time are undefined.                   |
 `---------------------------------------------------------------------------*/

int StoT (TIME_T *time, char *picture, char *source)
{
    int  index;                         /*  Index into picture               */
    char *sptr;                         /*  Next char in source              */
    int  hh, mm, ss;                    /*  Local time values                */
    int  digits = 0;                    /*  Number of digits in source       */
    Bool pm_found;

    for (dptr = 0; source [dptr] != '\0'; dptr++)
        if ((source [dptr] >= '0')
        &&  (source [dptr] <= '9'))
            digits++;

    if (digits == 0) {                  /*  Return zero time                 */
        time-> hh = 0;
        time-> mm = 0;
        time-> ss = 0;
        return (0);
    }
    hh = 0;
    mm = 0;
    ss = 0;
    pm_found = FALSE;
    sptr = StrLwr (source);

    if (source [0] == ' ')              /*  Allow leading zero               */
        source [0] = '0';

    for (index = 0; picture [index]; index++)
      {
        switch (picture [index]) {
            case 'H':
            case 'I':
                hh = *sptr++ - '0' + hh * 10;
                break;

            case 'M':
                mm = *sptr++ - '0' + mm * 10;
                break;

            case 'S':
                ss = *sptr++ - '0' + ss * 10;
                break;

            case '#':
                if (strstr (sptr, "pm") != NULL)
                    pm_found = TRUE;
                break;

            default:
                sptr++;                 /*  Skip delimiters                  */
        }
    }

    if (pm_found)
        hh += 12;

    if ((hh > 23)
     || (mm > 59)
     || (ss > 59))
        return (-1);

    time-> hh = hh;
    time-> mm = mm;
    time-> ss = ss;

    return (0);
}


/*-----------------------------.
 |  OpenMessageFile            |
 |-----------------------------`---------------------------------------------.
 |  Description: opens specified error message file for reading.             |
 |                                                                           |
 |  Returns: 0 if the file exists, else -1.                                  |
 |                                                                           |
 |  Comments: Use OpenMessageFile before PrintError/SystemMessage.           |
 |  If you use OpenMessageFile again, it first closes any open message file. |
 `---------------------------------------------------------------------------*/

static FILE *msgfile = NULL;
static char msgline [LINE_MAX + 1];

int OpenMessageFile (char *filename)
{
    int feedback;

    CloseMessageFile ();
    msgfile = FileOpen (filename, 'r');

    if (msgfile)
        feedback = 0;
    else
        feedback = -1;

    return (feedback);
}


/*-----------------------------.
 |  CloseMessageFile           |
 |-----------------------------`---------------------------------------------.
 |  Returns: nothing.                                                        |
 `---------------------------------------------------------------------------*/

void CloseMessageFile (void)
{
    if (msgfile)
      {
        FileClose (msgfile);
        msgfile = NULL;
      }
}


/*-----------------------------.
 |  PrintMessage               |
 |-----------------------------`---------------------------------------------.
 |  Description: scans the message file for a message with the specified id. |
 |  Each line in the message file should start with a four-digit id, then a  |
 |  space, then the message to print.  The message can include format spec-  |
 |  ifiers using '%'.  Values for each format are passed after the msgid.    |
 |                                                                           |
 |  Returns: nothing.                                                        |
 |                                                                           |
 |  Comments: the message file must be sorted by ascending msg id's.  Make   |
 |  sure you call OpenMessageFile () before this function.                   |
 `---------------------------------------------------------------------------*/

local read_msg (int msgid);
static char
    error_text [LINE_MAX + 1];

void PrintMessage (int msgid, ...)
{
    va_list argptr;                     /*  Argument list pointer            */

    read_msg (msgid);                   /*  Retrieve message into msgline    */
    va_start (argptr, msgid);           /*  Start variable arguments list    */
    vsprintf (error_text, msgline, argptr);
    va_end   (argptr);                  /*  End variable arguments list      */
    fprintf  (stderr, "%s\n", error_text);
}


/*-----------------------------.
 |  MessageText                |
 |-----------------------------`---------------------------------------------.
 |  Description: as for PrintMessage, but returns a pointer to the raw       |
 |  message rather than printing it.                                         |
 |                                                                           |
 |  The message text is stored in a static area that is over-written by      |
 |  each call to MessageText().                                              |
 |                                                                           |
 |  If msgid is -1, retrieves the next message sequentially, ignoring any    |
 |  numbering.  This is only valid after previously reading a message.       |
 |  Places "." in the message if no more are found.                          |
 `---------------------------------------------------------------------------*/

char *MessageText (int msgid)
{
    read_msg (msgid);                   /*  Retrieve message into msgline    */
    return (msgline);
}


/*  Read message into msgline, expand '$' if found at end of message         */

local read_msg (int msgid)
{
    static int
        lastid = 32767;                 /*  Last message we read             */

    if (msgfile == NULL)
      {
        sprintf (msgline, "** Message %d not found - file not open **\n",
                          msgid);
        return;                         /*  Message not found in file        */
      }
    if (msgid == -1)                    /*  Get next ignoring numbers?       */
      {
        if (!FileRead (msgfile, msgline))
            strcpy (msgline, "0000 ."); /*  "." signals end of file          */
      }
    else
      {
        if (msgid <= lastid)            /*  If necessary, back to start      */
            rewind (msgfile);

        FOREVER
          {
            if (!FileRead (msgfile, msgline))
              {
                sprintf (msgline, "0000 ** Message %d not found **\n", msgid);
                break;                  /*  Message not found in file        */
              }
            if ((isdigit (*msgline))
            && (atoi (msgline) == msgid))
              {
                lastid = msgid;
                break;
              }
          }
      }
    /*  Remove first four digits of message                                  */
    memmove (msgline, msgline + 4, strlen (msgline) - 3);
    /*  Remove leading space, if any                                         */
    if (msgline [0])
        memmove (msgline, msgline + 1, strlen (msgline));

    /*  Append system message if reqd                                        */
    if (*msgline && strlast (msgline) == '$')
      {
        strlast (msgline) = 0;
        strcat  (msgline, ": ");
        strcat  (msgline, strerror (errno));
      }
    /*  Kill newline at end of line                                          */
    if (*msgline && strlast (msgline) == '\n')
        strlast (msgline) = 0;
}


Bool TraceState = FALSE;                /*  Initial default                  */
FILE *TraceFile = NULL;                 /*  Trace file stream                */

/*-----------------------------.
 |  EnableTrace                |
 |-----------------------------`---------------------------------------------.
 |  void EnableTrace ()                                                      |
 |                                                                           |
 |  Enables tracing.  All calls to Trace () send a text to stdout or to the  |
 |  trace file if defined using SetTraceFile ().                             |
 `---------------------------------------------------------------------------*/

void EnableTrace (void)
{
    TraceState = TRUE;
}


/*-----------------------------.
 |  DisableTrace               |
 |-----------------------------`---------------------------------------------.
 |  void DisableTrace ()                                                     |
 |                                                                           |
 |  Disables tracing.  After a call to DisableTrace (), calls to Trace ()    |
 |  have no effect.                                                          |
 `---------------------------------------------------------------------------*/

void DisableTrace (void)
{
    TraceState = FALSE;
}


/*-----------------------------.
 |  SetTraceFile               |
 |-----------------------------`---------------------------------------------.
 |  void SetTraceFile (char *filename, char mode)                            |
 |                                                                           |
 |  Sends trace output to the specified file.  If SetTraceFile is not used,  |
 |  traces go to the console.  If filename is null, any open trace file is   |
 |  close.  Only one trace file can be open at any time.  If mode is 'a',    |
 |  output is appended to the trace file; if 'w' the trace file is reset at  |
 |  open time.  The caller can check for errors in this function by looking  |
 |  at the value of TraceFile, which is left null if errors occur.           |
 `---------------------------------------------------------------------------*/

void SetTraceFile (char *filename, char mode)
{
    if (TraceFile)
      {
        FileClose (TraceFile);
        TraceFile = NULL;
      }
    if (filename)
        TraceFile = FileOpen (filename, mode);
}


/*-----------------------------.
 |  Trace                      |
 |-----------------------------`---------------------------------------------.
 |  void Trace (char *format, ...)                                           |
 |                                                                           |
 |  If the global variable TraceState is TRUE, this function formats the     |
 |  parameters (using printf () conventions) and sends these to stdout, or   |
 |  the TraceFile if opened using SetTraceFile ().  The trace output is      |
 |  given a newline automatically.                                           |
 `---------------------------------------------------------------------------*/

void Trace (char *format, ...)
{
    va_list argptr;                     /*  Argument list pointer            */

    if (TraceState)
      {
        va_start (argptr, format);      /*  Start variable args processing   */
        vsprintf (error_text, format, argptr);
        va_end (argptr);                /*  End variable args processing     */
        fprintf ((TraceFile? TraceFile: stdout), "%s\n", error_text);
      }
}
