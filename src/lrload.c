/*===========================================================================*
 *                                                                           *
 *  lrload.c    loads dialog description into memory                         *
 *                                                                           *
 *  Written:    92/10/25  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/08/03                                                     *
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

#include "prelude.h"                    /*  Public definitions               */
#include "lrpriv.h"                     /*  Private definitions              */

#include "lrloadd.d"                    /*  Include dialog data              */

/*-----------------------------.
 |  lr_load_dialog             |
 |-----------------------------`---------------------------------------------.
 |  int lr_load_dialog (lrnode *head, char *filename);                       |
 |                                                                           |
 |  Description: opens and parses the dialog file specified by filename.     |
 |  Builds a multi-dimensional linked list of states, events, next states    |
 |  and modules onto the head node.                                          |
 |                                                                           |
 |  Returns: size of dialog file in nodes, -1 means file was not found or    |
 |  contained syntax errors.  This function displays any error messages as   |
 |  necessary.                                                               |
 |                                                                           |
 |  Structure of linked list:                                                |
 |                                                                           |
 |      head.name--> symbol-table                                            |
 |        |                                                                  |
 |        state -- state -- state -- state -- state                          |
 |                   |                                                       |
 |                 event -- event -- event                                   |
 |                            |                                              |
 |                        next-state -- module -- module -- module           |
 |                                                                           |
 |  Each node has a 'next' pointer shown as '--' and a 'child' pointer shown |
 |  as '|'.  The head.name points to a symbol table of LR_SYMBOLMAX size     |
 |  which holds the names of states, etc.  lr_load_dialog allocates memory   |
 |  for the symbol table.                                                    |
 |                                                                           |
 |  A separate head holds superstates; this is deleted after resolution of   |
 |  the superstates.                                                         |
 |                                                                           |
 |  The symbol table holds entries of this form: [type][number][name]        |
 |  where type is a character 's', 'e', or 'm', number is an integer which   |
 |  is initially set to zero, and name is a null-terminated string holding   |
 |  the name of the state, event, or module.  The table is initially set to  |
 |  all zeroes; an entry followed by zero indicates the end of the table.    |
 `---------------------------------------------------------------------------*/

/*  Global variables used in this source file only                           */

static int
    feedback,                           /*  Size of dialog in nodes          */
    event_number,                       /*  Number for event                 */
    char_nbr,                           /*  Current read position in line    */
    line_nbr;                           /*  Input line nbr from file         */

static char
    *filename,                          /*  Name of dialog file              */
    token       [LINE_MAX + 1],         /*  Token from input stream          */
    expect      [LINE_MAX + 1],         /*  Expected token or definition     */
    srcline     [LINE_MAX + 1],         /*  Current source line              */
    pretty_line [LINE_MAX + 1],         /*  Output source line               */
    pretty_file [LR_FILENAMEMAX + 1],   /*  Prettified file name             */
    fullname    [LR_FILENAMEMAX + 1],   /*  Dialog file with ext. & path     */
    *statename,                         /*  Logical name of current state    */
    *maxname;                           /*  Pointer to end of symbol table   */

static lrstat
    *stats;                             /*  Pointer to stats block           */

static lrnode
    *listhead,                          /*  Head of dialog list structure    */
    *last_state,                        /*  Pointer to last state node       */
    *cur_state,                         /*  Pointer to this state node       */
    *cur_event,                         /*  Pointer to this event node       */
    *cur_module;                        /*  Pointer to this module node      */

static FILE
    *input,                             /*  Input file stream                */
    *pretty;                            /*  Pretty-printed output            */

#define SS_OPEN             0           /*  Flags for superstate refs        */
#define SS_BUSY             1
#define SS_DONE             2
typedef struct SS_REF_tag               /*  Superstate reference list        */
  {
    struct SS_REF_tag
           *next;                       /*  Next superstate reference        */
    lrnode *state;                      /*  Pointer to state node            */
    char   *ssname;                     /*  Name of superstate               */
    byte    flag;                       /*  Processing flag                  */
  } SS_REF;

static SS_REF *ss_head,                 /*  Reference list head              */
              *ss_tail;                 /*    and tail                       */


/*  Character classification tables and macros                               */

static byte
    cmap [256];                         /*  Character classification tables  */

#define CMAP_VALID       1              /*  Normal name token                */
#define CMAP_QUOTE       2              /*  Possible string delimiters       */
                                        /*  Macros for character mapping:    */
#define is_valid(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_VALID) > 0)
#define is_quote(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_QUOTE) > 0)


/*  Function prototypes                                                      */

static void    init_charmaps      (void);
static void    build_charmap      (byte flag, char *chars);
static lrnode *alloc_node         (char type, lrnode *parent, lrnode *sibling);
static char   *resolve_symbol     (char *name, char type);
static void    syntax_error       (int exception, int msgid, ...);
static void    resolve_superstate (SS_REF *refnode);
static void    inherit_events     (lrnode *state, lrnode *super_state);
static void    duplicate_event    (lrnode *state, lrnode *event, char *name);
static void    remove_event       (lrnode *state, lrnode *event);


int lr_load_dialog (lrnode *p_listhead, lrstat *p_stats, char *p_filename)
{
    feedback = 0;                       /*  Reset return feedback            */
    filename = p_filename;              /*  Copy addresses of parameters     */
    stats    = p_stats;
    listhead = p_listhead;

#   include "lrloadd.i"                 /*  Execute dialog manager           */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    listhead-> next  =                  /*  Set list head pointers to null   */
    listhead-> child =
    last_state       = NULL;            /*  Set other pointers to null       */

    line_nbr = 0;                       /*  Nothing read from file yet       */

    /*  Allocate symbol table from memory                                    */
    listhead-> name = (char *) Check (calloc (LR_SYMBOLMAX, sizeof (char)));
    maxname = listhead-> name + LR_SYMBOLMAX;

    /*  Initialise superstate structures                                     */
    ss_head = (SS_REF *) Check (malloc (sizeof (SS_REF)));
    ss_tail = ss_head;
    ss_head-> next = NULL;

    init_charmaps ();                   /*  Initialise character maps        */

    the_next_event = ok_event;
}


/*  -------------------------------------------------------------------------
 *  init_charmaps
 *
 *  Initialise character map bit tables.  These are used to speed-up
 *  token recognition and collection.
 */

static void
init_charmaps (void)
{
    memset (cmap, 0, sizeof (cmap));    /*  Clear all bitmaps                */

    /*  Map fixed character sets to various bitmaps                          */
    build_charmap (CMAP_VALID, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_VALID, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_VALID, "0123456789");
    build_charmap (CMAP_VALID, "_-()<>-+$:");
    build_charmap (CMAP_QUOTE, "\"'");
}


/*  -------------------------------------------------------------------------
 *  build_charmap
 *
 *  Encode character string and flag into character map table.  Flag should
 *  be a 1-bit value from 1 to 128 (character map is 8 bits wide).
 */

static void
build_charmap (byte flag, char *string)
{
    for (; *string; string++)
        cmap [(byte) *string] |= flag;
}


/***************************   OPEN DIALOG FILE   ****************************/

MODULE open_dialog_file (void)
{
    char
        *located;                       /*  Filename on dpath                */

    /*  Look for dialog file on -dpath or -path unless fully specified       */
    if (strchr (filename, PATHEND))
        strcpy (fullname, filename);
    else
      {
        located = FileWhere ('r', OPT_DPATH.value?
                                  OPT_DPATH.value: OPT_PATH.value,
                                  filename, NULL);
        if (located)
            strcpy (fullname, located);
        else
            strcpy (fullname, filename);
      }
    FileCrLf = FALSE;                   /*  Reset file CRLF state            */
    input = FileOpen (fullname, 'r');
    if (input == NULL)
      {
        PrintMessage (MSG_DIALOG_FILE_NF, filename);
        raise_exception (error_event);
      }
    char_nbr    = 0;                    /*  Clear input line                 */
    srcline [0] = 0;

    sprintf (pretty_file, "lr%ld.tmp", TimeNow ());
    if (OPT_PRETTY.flags & OPT_ON)
        pretty = FileOpen (pretty_file, 'w');
    else
        pretty = NULL;
}


/****************************   GET NEXT TOKEN   *****************************/

MODULE get_next_token (void)
{
    int  size;                          /*  Size of token                    */
    char thisch;                        /*  Next char in token               */
    char first, last;                   /*  Token first & last chars         */

    FOREVER
      {
        if (srcline [char_nbr] == 0)    /*  If we need input, read a line    */
            if (FileRead (input, srcline))
              {
                char_nbr = 0;
                line_nbr++;
              }
            else
              {
                the_next_event = no_more_event;
                return;                 /*  If we hit end-of-file, end       */
              }

        /*  Skip spaces before token                                         */
        while (srcline [char_nbr] == ' ')
            char_nbr++;

        thisch = srcline [char_nbr];
        if (thisch == 0)                /*  If we need input, loop           */
            continue;

        /*  If line is a Libero option or comment, collect complete line     */
        if (thisch == '/'
        || (thisch == '-' && srcline [char_nbr + 1] != '>')
        ||  thisch == '!')
          {
            strcpy (token, srcline + char_nbr);
            char_nbr = strlen (srcline);
            break;
          }
        else
        if (is_valid (thisch))
          {
            size = 0;
            while (is_valid (thisch = srcline [char_nbr]))
              {
                char_nbr++;
                token [size++] = thisch;
              }
            token [size] = 0;           /*  Terminate token string           */
            break;
          }
        else
          {
            token [0] = thisch;
            token [1] = 0;
            syntax_error (error_event, MSG_INVALID_TOKEN, token, expect);
          }
      }
    ASSERT (strlen (token) > 0);
    first = token [0];
    last  = strlast (token);

    if (streq (token, ":"))
        the_next_event = state_name_term_event;
    else
    if (first == '(' && last == ')')
        the_next_event = event_number_event;
    else
    if (streq (token, "->"))
        the_next_event = next_pointer_event;
    else
    if (streq (token, "+"))
        the_next_event = module_flag_event;
    else
    if (first == '/' || first == '-')
        the_next_event = option_event;  /*  Option string                    */
    else
    if (first == '<' && last == '>')
      {
        StrClose (token, 0);            /*  Get rid of < and >               */
        strlast (token) = 0;
        if (ValidName (token))
            the_next_event = superstate_ref_event;
        else
            syntax_error (error_event, MSG_INV_STATE_NAME, token);
      }
    else
    if (first == '!')
        the_next_event = comment_event;
    else
    if (last == ':')
      {
        strlast (token) = 0;
        if (ValidName (token))
            the_next_event = state_name_event;
        else
            syntax_error (error_event, MSG_INV_STATE_NAME, token);
      }
    else
    if (ValidName (token) || ReservedName (token))
        the_next_event = item_name_event;
    else
        syntax_error (error_event, MSG_INVALID_TOKEN, token, expect);
}


local
syntax_error (int exception, int msgid, ...)
{
    static char
        error_text [LINE_MAX + 1];      /*  Formatted error message          */
    static int
        prev_line = 0;

    va_list argptr;                     /*  Argument list pointer            */

    if (line_nbr != prev_line)
      {
        fprintf (stderr, "%s %d: %s\n", filename, line_nbr, srcline);
        prev_line = line_nbr;
      }
    va_start (argptr, msgid);           /*  Start variable arguments list    */
    vsprintf (error_text, MessageText (msgid), argptr);
    va_end   (argptr);                  /*  End variable arguments list      */

    fprintf (stderr, "%s\n", error_text);
    if (exception)                      /*  Raise exception if error also    */
        raise_exception (exception);
}


/*************************   WARNING ON STATE NAME   *************************/

MODULE warning_on_state_name (void)
{
    syntax_error (0, MSG_BAD_STATE_NAME, token);
}


/*************************   PROCESS COMMENT VALUE   *************************/

MODULE process_comment_value (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
        FileWrite (pretty, token);
}


/**************************   PROCESS OPTION VALUE   *************************/

MODULE process_option_value (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
        FileWrite (pretty, token);
}


/***************************   ATTACH NEW STATE   ****************************/

MODULE attach_new_state (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        FileWrite (pretty, "");
        sprintf (pretty_line, "%s:", token);
      }

    last_state = alloc_node ('s', listhead, last_state);
    cur_state  = last_state;
    cur_event  = NULL;
    statename  = cur_state-> name;
}


/*-----------------------------.
 |  alloc_node                 |
 |-----------------------------`---------------------------------------------.
 |  lrnode *alloc_node (char type, lrnode *parent, lrnode *sibling)          |
 |                                                                           |
 |  Description: allocates one dialog node from memory, sets initial values  |
 |  and attaches it to the linked list in an appropriate place.  A node has  |
 |  these values:                                                            |
 |                                                                           |
 |      child pointer   - set to NULL                                        |
 |      next pointer    - set to NULL                                        |
 |      type            - single character indicator, passed by caller       |
 |      name            - pointer to name in symbol table                    |
 |                                                                           |
 |  This function uses the current contents of 'token' as the node name.  It |
 |  stores this value in the symbol table, or reuses a previous name if such |
 |  already exists in the symbol table.                                      |
 |                                                                           |
 |  The node is attached either as the child of the parent, or as the next   |
 |  sibling.  This choice depends on whether 'sibling' is null or not.       |
 |                                                                           |
 |  Returns: a pointer to the newly-create node, or null if some error such  |
 |  as insufficient memory occurred.  'parent' can never be null.            |
 `---------------------------------------------------------------------------*/

static lrnode *alloc_node (char type, lrnode *parent, lrnode *sibling)
{
    lrnode *newnode;

    ASSERT (parent != NULL);
    newnode = (lrnode *) Check (malloc (sizeof (lrnode)));
    feedback++;                         /*  Add 1 to returned dialog size    */
    newnode-> child = NULL;             /*  Set node pointers to NULL        */
    newnode-> next  = NULL;
    newnode-> type  = type;             /*  Set node type                    */
    newnode-> name  = resolve_symbol (CleanName (token), type);

    if (sibling)                        /*  Attach to parent or sibling      */
        sibling-> next = newnode;
    else
        parent-> child = newnode;

    return (newnode);
}


/*-----------------------------.
 |  resolve_symbol             |
 |-----------------------------`---------------------------------------------.
 |  char *resolve_symbol (char *name, type)                                  |
 |                                                                           |
 |  Description: searches for a name of the appropriate type in the symbol   |
 |  table and inserts it if necessary.  Each entry in the symbol table is    |
 |  composed of a type (1 char), number (dbyte), and name (string).  The     |
 |  last entry in the table is followed by a zero.                           |
 |                                                                           |
 |  Returns: a pointer to the newly-established name, or null if the symbol  |
 |  table is full.                                                           |
 `---------------------------------------------------------------------------*/

static char *resolve_symbol (char *name, char type)
{
    char *typeptr,                      /*  Pointer to symbol type           */
         *nameptr,                      /*  Pointer to symbol name           */
          typechr;                      /*  Last type examined in table      */
    byte *numbptr;                      /*  Pointer to symbol number         */

    /*  Scan symbol table for matching type and name, or end                 */
    typeptr = listhead-> name;
    FOREVER
      {
        numbptr = (byte *) typeptr + sizeof (char);
        nameptr = (char *) numbptr + sizeof (dbyte);
        typechr = *typeptr;
        if (typechr == 0)
            break;                      /*  At end of table, not found       */
        else
        if (typechr == type)
            if (streq (nameptr, name))
                break;                  /*  Found in table                   */

        typeptr = strchr (nameptr, 0) + 1;
      }
    if (typechr == 0)                   /*  Insert new name                  */
      {
        if (nameptr + strlen (name) + sizeof (dbyte) > maxname)
          {
            PrintMessage (MSG_SYS_SYMBOL_FULL);
            raise_exception (error_event);
            return (NULL);
          }
        else
          {
            *typeptr = type;            /*  Store symbol type and name       */
            strcpy (nameptr, name);
            PutSymNumber (nameptr, 0);  /*  Set symbol number to zero        */
          }
      }
    /*  Increment usage count                                                */
    PutSymNumber (nameptr, (dbyte) (GetSymNumber (nameptr) + 1));
    return (nameptr);
}


/*************************   ATTACH USES SUPERSTATE   ************************/

MODULE attach_uses_superstate (void)
{
    SS_REF *newnode;

    /*  Pretty-print if necessary                                            */
    if (pretty)
        XStrCat (pretty_line, " <", token, ">", NULL);

    newnode = (SS_REF *) Check (malloc (sizeof (SS_REF)));
    newnode-> next   = NULL;
    newnode-> state  = cur_state;
    newnode-> ssname = StrDup (CleanName (token));
    newnode-> flag   = SS_OPEN;

    ss_tail-> next = newnode;           /*  Attach to reference list         */
    ss_tail        = newnode;           /*    and update ss_tail             */
}


/**************************   STORE EVENT NUMBER   ***************************/

MODULE store_event_number (void)
{
    if (streq (token, "(--)"))
        event_number = 0;
    else
    if (strlen (token) != 4)
      {
        syntax_error (0, MSG_INV_EVENT_NUMBER, token, expect);
        event_number = 0;
      }
    else
      {
        event_number = (token [2] - '0') * 10 + (token [3] - '0');
        if (event_number == 0)
            syntax_error (0, MSG_INV_EVENT_NUMBER, token, expect);
      }
}


/***************************   ATTACH NEW EVENT   ****************************/

MODULE attach_new_event (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        if (cur_event == NULL)
            FileWrite (pretty, pretty_line);

        if (event_number)
            sprintf (pretty_line, "    (%02d) %-35s", event_number, token);
        else
            sprintf (pretty_line, "    (--) %-35s", token);
      }

    cur_event  = alloc_node ('e', cur_state, cur_event);
    cur_module = NULL;
    if (streq (token, "terminate")
    ||  streq (token, "the_next")
    ||  streq (token, "the_exception"))
        syntax_error (error_event, MSG_INV_EVENT_NAME, token);
}


/***************************   ATTACH NEXT STATE   ***************************/

MODULE attach_next_state (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        XStrCat (pretty_line, "-> ", token, NULL);
        FileWrite (pretty, pretty_line);
      }

    cur_module = alloc_node ('n', cur_event, NULL);
}


/*************************   ATTACH SAME NEXT STATE   ************************/

MODULE attach_same_next_state (void)
{
    static char
        saved_token [LINE_MAX];

    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        XStrCat (pretty_line, "->", NULL);
        FileWrite (pretty, pretty_line);
      }

    strcpy (saved_token, token);
    strcpy (token, statename);          /*  Next state is current            */
    cur_module = alloc_node ('n', cur_event, NULL);
    strcpy (token, saved_token);
}


/***************************   ATTACH NEW MODULE   ***************************/

MODULE attach_new_module (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        sprintf (pretty_line, "          + %s", token);
        FileWrite (pretty, pretty_line);
      }

    cur_module = alloc_node ('m', cur_event, cur_module);
}


/**************************   ATTACH EMPTY MODULE   **************************/

MODULE attach_empty_module (void)
{
    /*  Pretty-print if necessary                                            */
    if (pretty)
      {
        sprintf (pretty_line, "          +");
        FileWrite (pretty, pretty_line);
      }
}


/***************************   EXPECT STATE NAME   ***************************/

MODULE expect_state_name (void)
{
    strcpy (expect, "statename:");
}


/************************   EXPECT STATE DEFINITION   ************************/

MODULE expect_state_definition (void)
{
    strcpy (expect, "(--) event");
}


/***************************   EXPECT EVENT NAME   ***************************/

MODULE expect_event_name (void)
{
    strcpy (expect, "eventname");
}


/************************   EXPECT EVENT DEFINITION   ************************/

MODULE expect_event_definition (void)
{
    strcpy (expect, "+ module");
}


/**************************   EXPECT NEXT POINTER   **************************/

MODULE expect_next_pointer (void)
{
    strcpy (expect, "->");
}


/**************************   EXPECT MODULE NAME   ***************************/

MODULE expect_module_name (void)
{
    strcpy (expect, "modulename");
}


/************************   EXPAND MACRO CONSTRUCTS   ************************/

MODULE expand_macro_constructs (void)
{
    SS_REF *refnode;
    lrnode *state,
           *event,
           *check;                      /*  Look at events                   */
    char   *symptr,                     /*  Pointer into symbol table        */
           *nameptr;                    /*  Pointer to symbol name           */
    Bool   found;

    /*  Resolve superstate references                                        */
    for (refnode = ss_head-> next; refnode; refnode = refnode-> next)
        resolve_superstate (refnode);

    /*  Resolve $other event references                                      */
    for (state = listhead-> child; state; state = state-> next)
        for (event = state-> child; event; event = event-> next)
            if (streq (event-> name, "$other"))
              {
                /*  Find all events not handled in state, and add them       */
                for (symptr = listhead-> name; *symptr; )
                  {
                    nameptr = symptr + sizeof (char) + sizeof (dbyte);
                    if (*symptr == 'e')
                      {
                        found = FALSE;
                        for (check = state-> child; check; check = check-> next)
                            if (streq (check-> name, nameptr))
                              {
                                found = TRUE;
                                break;
                              }
                        if (!found)
                            duplicate_event (state, event, nameptr);
                      }
                    symptr = strchr (nameptr, 0) + 1;
                  }
                remove_event (state, event);
              }
}


static void
resolve_superstate (SS_REF *refnode)
{
    SS_REF *ssnext;                     /*  Superstate itself in ref list    */
    lrnode *ssnode;                     /*  Superstate node pointer          */

    if (refnode-> flag == SS_BUSY)
      {
        PrintMessage (MSG_SSTATE_LOOP, refnode-> state-> name);
        raise_exception (error_event);
      }
    else                                /*  Ignore if SS_DONE                */
    if (refnode-> flag == SS_OPEN)
      {
        refnode-> flag = SS_BUSY;
        /*  Resolve super-states recursively, then current super-state       */
        if ((ssnode = NamedState (listhead, refnode-> ssname)) != NULL)
          {
            for (ssnext = ss_head-> next; ssnext; ssnext = ssnext-> next)
                if (ssnext-> state == ssnode)
                    resolve_superstate (ssnext);

            inherit_events (refnode-> state, ssnode);
          }
        else
            PrintMessage (MSG_SSTATE_NF, refnode-> ssname);
      }
    refnode-> flag = SS_DONE;
}


/*  -------------------------------------------------------------------------
 *  inherit_events
 *
 *  Copies each event from the super_state to the target unless the event is
 *  already defined in the target.  When the next state name is equal to the
 *  super_state name, it is changed to the target name.
 */

static void
inherit_events (lrnode *target, lrnode *super_state)
{
    lrnode
        *super_event,                   /*  Superstate event node            */
        *super_module,                  /*  Superstate module node           */
        *event,                         /*  Event in target state            */
        *module;                        /*  Module in target state/event     */

    for (super_event = super_state-> child;
         super_event;
         super_event = super_event-> next)
      {
        event = NULL;                   /*  Look for event in state          */
        for (event = target-> child; event; event = event-> next)
            if (event-> name == super_event-> name
            ||  event-> next == NULL)
                break;

        /*  If not found, add event and all modules                          */
        if (event-> name != super_event-> name)
          {
            module = NULL;
            strcpy (token, super_event-> name);
            event = alloc_node ('e', target, event);
            for (super_module = super_event-> child;
                 super_module;
                 super_module = super_module-> next)
              {
                strcpy (token, super_module-> name);
                if (streq (token, super_state-> name))
                    strcpy (token, target-> name);
                module = alloc_node (super_module-> type, event, module);
              }
          }
      }
}


/*  Duplicate event branch, using specified event name                       */

static void
duplicate_event (lrnode *state, lrnode *event, char *name)
{
    lrnode
        *old_event,
        *old_child,
        *new_event,
        *new_child;

    /*  Find last event in state, and add after that one                     */
    for (old_event = event; old_event-> next; old_event = old_event-> next);

    /*  Create event node                                                    */
    strcpy (token, name);
    new_event = alloc_node ('e', state, old_event);

    /*  Copy event child nodes                                               */
    new_child = NULL;
    for (old_child = event-> child; old_child; old_child = old_child-> next)
      {
        strcpy (token, old_child-> name);
        new_child = alloc_node (old_child-> type, new_event, new_child);
      }
}


static void
remove_event (lrnode *state, lrnode *event)
{
    lrnode
        *next,
        *child,
        *scanev;

    /*  Free all event child nodes                                           */
    for (child = event-> child; child; )
      {
        /*  Decrement usage count and delete node                            */
        PutSymNumber (child-> name, (dbyte) (GetSymNumber (child-> name) - 1));
        next = child-> next;
        free (child);
        child = next;
        feedback--;                     /*  Dialog is one node smaller       */
      }

    /*  Patch-up other event links in state                                  */
    next = event-> next;
    if (state-> child == event)         /*  If first event, update state     */
        state-> child = next;           /*    to point to sibling            */
    else
        for (scanev = state-> child; scanev; scanev = scanev-> next)
            if (scanev-> next == event) /*  Link previous event to next      */
              {                         /*    if there is any previous       */
                scanev-> next = next;
                break;
              }

    /*  Decrement usage count and delete node                                */
    PutSymNumber (event-> name, (dbyte) (GetSymNumber (event-> name) - 1));
    free (event);                       /*  Free event node                  */
    feedback--;                         /*  Dialog is one node smaller       */
}


/************************   BUILD SOURCE FILE NAME   *************************/

MODULE build_source_file_name (void)
{
    stats-> source = (char *) Check (malloc (LR_FILENAMEMAX + 1));
    if (OPT_SOURCE.flags & OPT_ON)
        strcpy (stats-> source, OPT_SOURCE.value);
    else
      {
        strcpy (stats-> source, filename);
        StripExtension (stats-> source);
        /*  If '-plusd' specified, enforce name + 'd' rule for dialogs       */
        if (OPT_PLUSD.flags && OPT_ON)
          {
            if (strlast (stats-> source) == 'd')
                strlast (stats-> source) = 0;
            else
              {
                PrintMessage (MSG_PLUSD_NAME_ERROR, filename);
                raise_exception (error_event);
              }
          }
      }
    DefaultExtension (stats-> source, stats-> source, "c");
}


/***********************   DISPLAY UNEXPECTED TOKEN   ************************/

MODULE display_unexpected_token (void)
{
    syntax_error (0, MSG_UNEXPECTED_TOKEN, token, expect);
}


/*************************   RETURN ERROR FEEDBACK   *************************/

MODULE return_error_feedback (void)
{
    feedback = -1;
}


/***************************   GET EXTERNAL EVENT   **************************/

MODULE get_external_event (void)
{
}


/***************************   CLOSE DIALOG FILE   ***************************/

MODULE close_dialog_file (void)
{
    SS_REF *refnode, *next;

    /*  Free superstate ref list                                             */
    for (refnode = ss_head-> next; refnode; )
      {
        next = refnode-> next;
        free (refnode-> ssname);
        free (refnode);
        refnode = next;
      }
    free (ss_head);                     /*  Free superstate list head        */
    FileClose (input);
}


/*************************   TERMINATE THE PROGRAM    ************************/

MODULE terminate_the_program (void)
{
    /*  The FileCrLf flag indicates whether a CR was found in the dialog     */
    /*  file (being the last file read).  This flag is used when we generate */
    /*  output so that the output matches the dialog.  This is useful when   */
    /*  we run Libero on a system but on a remote disk that has a different  */
    /*  line-end convention.                                                 */

    stats-> crlf = FileCrLf;            /*  Get state of this global flag    */

    /*  Finish-up pretty-printing if that's what we're doing                 */
    if (pretty && feedback != -1)
      {
        FileClose (pretty);
        FixedExtension (token, filename, "bak");
        remove (token);                 /*  Kill any existing .bak file      */
        rename (filename, token);       /*  Now rename old dialog file       */
        rename (pretty_file, filename);
      }
    the_next_event = terminate_event;
}
