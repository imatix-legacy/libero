/*===========================================================================*
 *                                                                           *
 *  lrlibr.c    Private LR library functions                                 *
 *                                                                           *
 *  Written:    92/10/25  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/01/16                                                     *
 *                                                                           *
 *  Contains:   ValidName           - checks if name is valid                *
 *              TrueName            - returns style-formatted name           *
 *              CleanName           - returns cleaned-up name_like_this      *
 *              ExternalName        - returns external Name-Like-This        *
 *              Check               - check that memory allocation worked    *
 *              GetSymNumber        - return number from symbol table        *
 *              PutSymNumber        - store number into symbol table         *
 *              NamedState          - find state with specified name         *
 *              DeleteState         - delete single state from dialog        *
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

#include "prelude.h"                    /*  Public header file               */
#include "lrpriv.h"                     /*  Private header file              */

static char
    formatted [LR_FILENAMEMAX + 1];

/*-----------------------------.
 |  ValidName                  |
 |-----------------------------`---------------------------------------------.
 |  int ValidName (char *name)                                               |
 |                                                                           |
 |  Description: checks whether name is valid.  Permits only a letter        |
 |  followed by letters, digits, or embedded hyphens.  Ignores the case      |
 |  of letters.                                                              |
 |                                                                           |
 |  Returns: TRUE if valid, else FALSE.                                      |
 `---------------------------------------------------------------------------*/

Bool ValidName (char *name)
{
    int feedback = TRUE;                /*  Assume name is valid             */

    ASSERT (name != NULL);

    if (!isalpha (*name)
    ||  !isalnum (strlast (name)))
        feedback = FALSE;               /*  Starts/ends with non-alpha char  */
    else
    for (;*++name;)                     /*  Scan through rest of name        */
      {
        if (isalnum (*name))            /*  Allow alpha and numeric chars    */
            continue;
        else
        if (*name == '-')
            if (*(name + 1))            /*  Allow '-' only if followed by    */
                continue;               /*    non-null character             */
            else
                feedback = FALSE;
        else
            feedback = FALSE;
      }
    return (feedback);
}


/*-----------------------------.
 |  ReservedName               |
 |-----------------------------`---------------------------------------------.
 |  int ReservedName (char *name)                                            |
 |                                                                           |
 |  Description: returns TRUE if name is a known reserved name.  Reserved    |
 |  names start with '$'.  Ignores the case of letters.                      |
 |                                                                           |
 |  Returns: TRUE if known reserved name, else FALSE.                        |
 `---------------------------------------------------------------------------*/

Bool ReservedName (char *name)
{
    ASSERT (name != NULL);

    if (LexCmp (name, "$other") == 0)
        return (TRUE);
    return (FALSE);
}


/*-----------------------------.
 |  TrueName                   |
 |-----------------------------`---------------------------------------------.
 |  char *TrueName (char *name)                                              |
 |                                                                           |
 |  Description: Accepts name in form "some_name_like_this" and returns it   |
 |  in one of three forms as defined by the current -style setting:          |
 |                                                                           |
 |      -style=plain      some_name_like_this                                |
 |      -style=caps       SomeNameLikeThis                                   |
 |      -style=headline   Some_Name_Like_This                                |
 |      -style=title      Some_name_like_this                                |
 |      -style=normal     Some-Name-Like-This                                |
 |      -style=cobol      SOME-NAME-LIKE-THIS                                |
 |                                                                           |
 |  Returns: pointer to static area that is overwritten with each call.      |
 `---------------------------------------------------------------------------*/

char *TrueName (char *name)
{
    char    ch;                         /*  Current char                     */
    Bool    capital,                    /*  Next char is capitalised?        */
            plain,                      /*  -style=plain?                    */
            title,                      /*  -style=title?                    */
            headline,                   /*  -style=headline?                 */
            caps,                       /*  -style=caps?                     */
            normal,                     /*  -style=normal?                   */
            cobol;                      /*  -style=cobol?                    */
    int     fmtlen;                     /*  Length of name so far            */

    plain    = streq (OPT_STYLE.value, "plain");
    title    = streq (OPT_STYLE.value, "title");
    headline = streq (OPT_STYLE.value, "headline");
    caps     = streq (OPT_STYLE.value, "caps");
    normal   = streq (OPT_STYLE.value, "normal");
    cobol    = streq (OPT_STYLE.value, "cobol");

    capital  = !plain;                  /*  Start with capital unless plain  */
    fmtlen   = 0;

    for (fmtlen = 0; *name; name++)
      {
        ch = *name;
        if (ch == '-' || ch == '_')
          {
            if (title)                  /*  Capital only at start of name    */
                capital = (fmtlen == 0);
            else
            if (headline || caps || normal)
                capital = TRUE;

            if (title || headline || plain)
                formatted [fmtlen++] = '_';
            else
            if (normal || cobol)
                formatted [fmtlen++] = '-';
          }
        else
          {
            if (cobol)                  /*  ALL CAPS for cobol               */
                capital = TRUE;

            formatted [fmtlen++] = (char) (capital? toupper (ch): tolower (ch));
            capital = FALSE;
          }
      }
    formatted [fmtlen] = 0;
    return (formatted);
}


/*-----------------------------.
 |  CleanName                  |
 |-----------------------------`---------------------------------------------.
 |  char *CleanName (char *name)                                             |
 |                                                                           |
 |  Description: cleans up name by converting letters to lowercase and '-'   |
 |  to '_'.  Truncates name at first non-valid character.  Allows '$'.       |
 |                                                                           |
 |  Returns: a pointer to the cleaned-up name.                               |
 `---------------------------------------------------------------------------*/

char *CleanName (char *name)
{
    char *ptr, ch;

    for (ptr = name; *ptr; ptr++)
      {
        ch = *ptr;
        if (isalnum (ch))
            ch = (char) tolower (ch);
        else
        if (ch == '-' || ch == '_')
            ch = '_';
        else
        if (ch != '$')
            ch = 0;
        *ptr = ch;
      }
    return (name);
}


/*-----------------------------.
 |  ExternalName               |
 |-----------------------------`---------------------------------------------.
 |  char *ExternalName (char *name)                                          |
 |                                                                           |
 |  Description: prepares name by converting letters to mixedcase and '_'    |
 |  to '-'.  Truncates name at first non-valid character.                    |
 |                                                                           |
 |  Returns: a pointer to a formatted name in a static area.                 |
 `---------------------------------------------------------------------------*/

char *ExternalName (char *name)
{
    char    ch;                         /*  Current char                     */
    Bool    capital;                    /*  Next char is capitalised?        */
    int     fmtlen;                     /*  Length of name so far            */

    capital = TRUE;                     /*  Start with capital               */
    fmtlen  = 0;
    for (fmtlen = 0; *name; name++)
      {
        ch = *name;
        if (ch == '-' || ch == '_')
          {
            capital = TRUE;
            formatted [fmtlen++] = '-';
          }
        else
          {
            formatted [fmtlen++] = (char) (capital? toupper (ch): tolower (ch));
            capital = FALSE;
          }
      }
    formatted [fmtlen] = 0;
    return (formatted);
}


/*-----------------------------.
 |  Check                      |
 |-----------------------------`---------------------------------------------.
 |  void *Check (void *ptr)                                                  |
 |                                                                           |
 |  Description: checks whether ptr was correctly allocated or not.  If ptr  |
 |  is NULL, displays an error message and exits the program with a return   |
 |  code of EXIT_FAILURE.                                                    |
 |                                                                           |
 |  Returns: ptr if successful.                                              |
 |                                                                           |
 |  Comments: Assumes that out-of-memory is a fatal condition.               |
 `---------------------------------------------------------------------------*/

void *Check (void *ptr)
{
    if (ptr == NULL)
      {
        PrintMessage (MSG_OUT_OF_MEMORY);
        exit (EXIT_FAILURE);
      }
    return (ptr);
}


/*-----------------------------.
 |  GetSymNumber               |
 |-----------------------------`---------------------------------------------.
 |  dbyte GetSymNumber (char *name)                                          |
 |                                                                           |
 |  In the Libero state/event/module tables, each name is preceded in        |
 |  memory by the number for that item.  This function extracts and          |
 |  returns that number.                                                     |
 `---------------------------------------------------------------------------*/

dbyte GetSymNumber (char *name)
{
    byte *numbptr;                      /*  Pointer to symbol number         */

    numbptr = (byte *) name - sizeof (dbyte);
    return (dbyte) ((*numbptr << 8) + *(numbptr + 1));
}


/*-----------------------------.
 |  PutSymNumber               |
 |-----------------------------`---------------------------------------------.
 |  void PutSymNumber (char *name, dbyte number)                             |
 |                                                                           |
 |  In the Libero state/event/module tables, each name is preceded in        |
 |  memory by the number for that item.  This function stores a new value    |
 |  for that number.                                                         |
 `---------------------------------------------------------------------------*/

void PutSymNumber (char *name, dbyte number)
{
    byte *numbptr;                      /*  Pointer to symbol number         */

    numbptr = (byte *) name - sizeof (dbyte);
    *(numbptr)     = (byte) (number >> 8);
    *(numbptr + 1) = (byte) (number & 255);
}


/*-----------------------------.
 |  NamedState                 |
 |-----------------------------`---------------------------------------------.
 |  lrnode *NamedState (lrnode *listhead, char *name)                        |
 |                                                                           |
 |  Looks through the specified list for a state with the specified name.    |
 |  Returns the state node number if found, else null.                       |
 `---------------------------------------------------------------------------*/

lrnode *NamedState (lrnode *listhead, char *name)
{
    lrnode *next;

    for (next = listhead-> child; next; next = next-> next)
      {
        if (streq (next-> name, name))
            return (next);
      }
    return (NULL);
}


/*-----------------------------.
 |  DeleteState                |
 |-----------------------------`---------------------------------------------.
 |  lrnode *DeleteState (lrnode listhead, lrnode *state)                     |
 |                                                                           |
 |  Deletes the specified state, keeping the dialog links correct.  Returns  |
 |  the state's sibling, or zero if it was the last state.  Updates the      |
 |  dialog listhead correctly if it was the first state.                     |
 `---------------------------------------------------------------------------*/

lrnode *DeleteState (lrnode *listhead, lrnode *state)
{
    lrnode *scanst,                     /*  Scan to find previous state      */
           *event,                      /*  Pointer to event                 */
           *module,                     /*  Pointer to module                */
           *next;                       /*  Pointer to next item in list     */

    /*  Delete events, modules, and finally state itself                     */
    for (event = state-> child; event; )
      {
        for (module = event-> child; module; )
          {
            /*  Decrement usage count and delete node                        */
            PutSymNumber (module-> name,
                (dbyte) (GetSymNumber (module-> name) - 1));
            next = module-> next;
            free (module);
            module = next;
          }
        /*  Decrement usage count and delete node                            */
        PutSymNumber (event-> name,
            (dbyte) (GetSymNumber (event-> name) - 1));
        next = event-> next;
        free (event);
        event = next;
      }
    /*  Patch-up other state links in dialog                                 */
    next = state-> next;
    if (listhead-> child == state)      /*  If first state, update listhead  */
        listhead-> child = next;        /*    to point to sibling            */
    else
      {
        for (scanst = listhead-> child; scanst; scanst = scanst-> next)
            if (scanst-> next == state) /*  Link previous state to next      */
              {                         /*    if there is any previous       */
                scanst-> next = next;
                break;
              }
      }
    /*  Decrement usage count and delete node                                */
    PutSymNumber (state-> name, (dbyte) (GetSymNumber (state-> name) - 1));
    free (state);                       /*  Free state node                  */
    return (next);                      /*    and return state sibling       */
}
