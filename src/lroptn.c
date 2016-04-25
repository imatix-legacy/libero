/*===========================================================================*
 *                                                                           *
 *  lroptn.c    parse option definition, and other option functions          *
 *                                                                           *
 *  Written:    92/11/08  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    96/12/27                                                     *
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

#define  INCLUDE_OPTIONS                /*  Include options block in lrpriv  */
#include "prelude.h"                    /*  Public definitions               */
#include "lrpriv.h"                     /*  Private definitions              */


/*  Global data used in this source file only                                */

static char
    workarea [LINE_MAX + 1];            /*  Used to parse options            */

static option
    option_save [OPT_MAX];


/*-----------------------------.
 |  lr_parse_option_file       |
 |-----------------------------`---------------------------------------------.
 |  int lr_parse_option_file (char *filename);                               |
 |                                                                           |
 |  Description: opens the file specified and reads options from that file.  |
 |  Options must be on a line starting with / or -, and be separated by      |
 |  spaces.  In the case of dialog files, options must come before the first |
 |  state definition, before or after the comment header.                    |
 |                                                                           |
 |  Returns: 0 if okay, -1 if errors occurred.                               |
 |                                                                           |
 |  Comments: if the option file was not found, a message is printed only in |
 |  the case that the options file was specified using the -initial switch.  |
 `---------------------------------------------------------------------------*/

int lr_parse_option_file (char *filename)
{
    FILE
        *input;                         /*  Input file stream                */
    char
        *options_file,                  /*  Full name of options file        */
        *first;                         /*  First char on line               */

    ASSERT (filename != NULL);
    options_file = FileWhere ('r', OPT_DPATH.value? OPT_DPATH.value:
                                   OPT_PATH.value, filename, NULL);
    if (options_file == NULL)
      {
        if (OPT_INITIAL.flags & OPT_ON)
            fprintf (stderr, "Options file '%s' not found\n", filename);
        return (-1);                    /*  File not found                   */
      }
    input = FileOpen (options_file, 'r');

    ASSERT (input != NULL);
    while (FileRead (input, workarea))
      {
        /*  We support the archaic '/' in files, where it is harmless        */
        if (*workarea == '-' || *workarea == '/')
          {
            *workarea = '-';            /*  Filter-out archaic '/' here      */
            lr_parse_option_line (workarea);
          }
        else
          {
            first = workarea;
            while (*first == ' ')
                first++;
            if (!(*first == '!' || *first == 0))
                break;
          }
      }
    FileClose (input);
    return (0);
}


void lr_parse_option_line (char *options_line)
{
    static char
        line [LINE_MAX + 1];            /*  Copy of options line             */

    char *token, *next;
    int in_string = 0;

    ASSERT (options_line != NULL);
    strcpy (line, options_line);
    for (token = line; *token; )
      {
        while (*token == ' ')
            token++;
        if (*token == '!'               /*  Comment - ignore rest of line    */
        ||  *token == 0)                /*  End of line - quit               */
            break;

        if (*token != '-')
          {
            fprintf (stderr, "Bad option name '%s'\n", token);
            break;
          }
        for (next = token; *next; next++)
          {
            if (*next == '\'' || *next == '"')
                in_string = 1 - in_string;
            else
            if (*next == ' ' && !in_string)
              {
                *next = 0;
                next++;                 /*  Bump to start of next            */
                break;
              }
          }
        lr_parse_option (token);
        token = next;
    }
}


int lr_parse_option (char *optname)
{
    static char
        line     [LINE_MAX + 1],        /*  Copy of options line             */
        optvalue [LINE_MAX + 1];        /*  Copy of option value             */

    int     optnbr,                     /*  Index into option_list           */
            length;                     /*  Length of option name            */
    char   *name,                       /*  Pointer to option name           */
           *value,                      /*  Pointer to option value if any   */
           *check,                      /*  Points into value checklist      */
           *scan;                       /*  Scan and parse value string      */
    Bool    negate;                     /*  If TRUE, had '-NoXyz'            */
    option *optptr;                     /*  Pointer to option in list        */
    int     in_string = 0;

    strcpy (line, optname + 1);         /*  Name starts past option ind.     */
    for (value = line; *value; value++) /*  Cut name:value or name=value     */
        if (*value == '=' || *value == ':')
          {
            *value++ = 0;               /*  Value is after separator         */
            break;
          }

    name = line;
    StrLwr (name);                      /*  Get name into lower-case         */
    if (name [0] == 'n' && name [1] == 'o')
      {
        name += 2;
        negate = TRUE;
      }
    else
        negate = FALSE;

    length = strlen (name);
    if (length < 3)
      {
        fprintf (stderr, "Option name '%s' is too short\n", name);
        return (-1);
      }
    for (optnbr = 0; optnbr < OPT_MAX; optnbr++)
      {
        optptr = &option_list [optnbr];
        if (strncmp (optptr-> name, name, length) == 0)
            break;
      }
    if (optnbr == OPT_MAX)
      {
        fprintf (stderr, "Bad option name '%s'\n", name);
        return (-1);
      }
    if (negate)
        if (optptr-> type == '?')
            optptr-> flags = OPT_OFF;
        else
            fprintf (stderr, "'-no%s' is meaningless\n", name);
    else
        optptr-> flags = OPT_ON;

    if (optptr-> type == '='            /*  New value for option             */
    ||  optptr-> type == '+')           /*  Added value for option           */
      {
        if (*value == 0)                /*  Value string is empty            */
            value = NULL;
        else                            /*  Else pick-up value string        */
          {
            /*  Convert to lowercase except within quotes                    */
            for (scan = value; *scan; scan++)
              {
                if (*scan == '\'' || *scan == '"')
                    in_string = 1 - in_string;
                else
                if (!in_string)
                    *scan = (char) tolower (*scan);
              }
            /*  If value starts and ends with a quote or apost, remove them  */
            if ((*value == '\'' || *value == '"')
            && (strlast (value) == *value))
              {
                StrClose (value, 0);    /*  Remove first and last quote      */
                strlast (value) = 0;
              }
            /*  If switch accepts fixed set of values, check we fit one      */
            for (check = optptr-> check; check; )
              {
                if (streq (value, check))
                    break;
                check = strchr (check, 0) + 1;
                if (*check == 0)
                  {
                    fprintf (stderr, "Bad option value '%s'\n", value);
                    return (-1);
                  }
              }
          }
        if (optptr-> type == '='        /*  '=' = replace existing value     */
        ||  optptr-> value == NULL)
          {
            if (optptr-> value)         /*  Deallocate value if any          */
                free (optptr-> value);
            optptr-> value = StrDup (value);
          }
        else                            /*  '+' = Append this value          */
          {                             /*    with ',' delimiter             */
            strcpy (optvalue, optptr-> value);
            strcat (optvalue, ",");
            strcat (optvalue, value);
            free (optptr-> value);
            optptr-> value = StrDup (optvalue);
          }
      }
    else
    if (*value)                         /*  ? options can only take          */
      {                                 /*    yes/no/true/false values       */
        StrLwr (value);                 /*  Get name into lower-case         */
        if (streq (value, "yes") || streq (value, "true"))
            optptr-> flags = OPT_ON;
        else
        if (streq (value, "no") || streq (value, "false"))
            optptr-> flags = OPT_OFF;
        else
            fprintf (stderr, "Unknown value for '-%s' ignored\n", name);
      }
    return (0);
}


void lr_reset_options (void)
{
    int     optnbr;                     /*  Option number in list            */
    option *optptr;                     /*  Pointer to option in list        */

    for (optnbr = 0; optnbr < OPT_MAX; optnbr++)
      {
        optptr = &option_list [optnbr];
        optptr-> flags = 0;             /*  Option is OFF                    */
        if (optptr-> value)
          {
            free (optptr-> value);      /*  Release any memory used for      */
            optptr-> value = NULL;      /*    option value.                  */
          }
      }
}


void lr_push_options (void)
{
    int optnbr;

    memcpy ((void *) option_save,
            (void *) option_list, sizeof (option) * OPT_MAX);
    for (optnbr = 0; optnbr < OPT_MAX; optnbr++)
        if (option_list [optnbr].value)
            option_save [optnbr].value = StrDup (option_list [optnbr].value);
}


void lr_pop_options (void)
{
    int optnbr;

    for (optnbr = 0; optnbr < OPT_MAX; optnbr++)
        if (option_list [optnbr].value)
            free (option_list [optnbr].value);

    memcpy ((void *) option_list,
            (void *) option_save, sizeof (option) * OPT_MAX);
}


void lr_show_options (char *filename)
{
    int optnbr;                         /*  Option number in list            */
    int length;                         /*  Length of formatted line         */
    option *optptr;                     /*  Pointer to option in list        */

    if (OPT_SETTINGS.flags & OPT_ON)
      {
        printf ("  Option settings for %s:\n", filename);
        for (optnbr = 0; optnbr < OPT_MAX; optnbr++)
          {
            optptr = &option_list [optnbr];
            sprintf (workarea, "    %s", optptr-> name);
            length = strlen (workarea);
            length = ((length / 2) + 1) * 2;
            while (length < 20)         /*  Format name . . . . . . .        */
              {
                strcat (workarea, " .");
                workarea [length] = 0;
                length += 2;
              }
            strcat (workarea, ": ");
            if (optptr-> type == '?')
                strcat (workarea, (optptr-> flags & OPT_ON? "Yes": "No"));
            else
                strcat (workarea, (optptr-> value? optptr-> value: "-"));

            puts (workarea);
          }
    }
}
