/*===========================================================================*
 *                                                                           *
 *  lrsort.c    Sort and renumber dialog description                         *
 *                                                                           *
 *  Written:    92/10/27  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    96/12/29                                                     *
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


/*  Function prototypes                                                      */

static int  collect_names   (lrnode *head, char type, char **names, int sort);
static int  compare_name    (const void *ptr1, const void *ptr2);
static void find_next_state (lrnode *node, char **names, int statenbr);


/*-----------------------------.
 |  lr_sort_dialog             |
 |-----------------------------`---------------------------------------------.
 |  int lr_sort_dialog (lrnode *head, lrstat *stats);                        |
 |                                                                           |
 |  Description: sorts and renumbers the dialog description at head.  Uses   |
 |  malloc to grab areas for the state, event, and module name lists and     |
 |  attaches these to the stats block.  Each list contains STxxxMAX char *   |
 |  pointers which are set to point into the symbol table.  Each name in     |
 |  the symbol table is given a number after the appropriate name table is   |
 |  sorted.                                                                  |
 |                                                                           |
 |  If OPT_COMPRESS is on, removes any unused states; ie. states not used    |
 |  by other states; ignores first state and defaults state, if any.         |
 |                                                                           |
 |  Returns: 0 if okay, -1 if errors were found.  Sets the various fields    |
 |  in stats (see stpriv.h).                                                 |
 |                                                                           |
 |  Comments: states, events, and modules are numbered from 0 upwards.       |
 `---------------------------------------------------------------------------*/

int lr_sort_dialog (lrnode *listhead, lrstat *stats)
{
    int    feedback = 0,
           actions,                     /*  Nbr actions for this event       */
           scan;                        /*  Index used for scanning          */
    lrnode *state,                      /*  Pointer to state in dialog list  */
           *cmpstate,                   /*  Pointer to state in dialog list  */
           *event,                      /*  Pointer to event                 */
           *nextstate,                  /*  Pointer to next state            */
           *module;                     /*  Pointer to module                */
    Bool   used;                        /*  State used in dialog?            */
    char   *symptr;                     /*  Pointer into symbol table        */

    ASSERT (listhead != NULL);
    ASSERT (stats != NULL);

    stats-> snames = (char **) Check (calloc (LR_STATEMAX,  sizeof (char *)));
    stats-> enames = (char **) Check (calloc (LR_EVENTMAX,  sizeof (char *)));
    stats-> mnames = (char **) Check (calloc (LR_MODULEMAX, sizeof (char *)));

    /*  Remove any unused states from dialog                                 */
    for (state = listhead-> child; state; state = state-> next)
      {
        /*  Ignore first state and defaults state                            */
        if (state != listhead-> child
        &&  strneq (state-> name, OPT_DEFAULTS.value))
          {
            used = FALSE;
            for (cmpstate = listhead-> child; cmpstate && !used;
                 cmpstate = cmpstate-> next)
                if (cmpstate != state)
                    for (event = cmpstate-> child; event && !used;
                         event = event-> next)
                      {
                        nextstate = event-> child;
                        if (streq (nextstate-> name, state-> name))
                            used = TRUE;
                      }
            if (!used && OPT_COMPRESS.flags & OPT_ON)
                DeleteState (listhead, state);
          }
      }

    /*  Collect and sort states, events, and modules                         */
    stats-> states  = collect_names (listhead, 's', stats-> snames, 0);
    stats-> events  = collect_names (listhead, 'e', stats-> enames, 1);
    stats-> modules = collect_names (listhead, 'm', stats-> mnames,
                                     OPT_SORT.flags & OPT_ON? 1: 0);
    stats-> maxaction = 0;

    /*  Find defaults state                                                  */
    stats-> defaults = LR_NULL_STATE;   /*  Assume none defined              */
    for (scan = 0; stats-> snames [scan]; scan++)
        if (streq (stats-> snames [scan], OPT_DEFAULTS.value))
            stats-> defaults = scan;

    /*  Perform a few checks on the dialog                                   */
    /*    - next state cannot be initial state                               */
    /*    - same state does not exist twice                                  */
    for (state = listhead-> child; state; state = state-> next)
      {
        for (cmpstate = state-> next; cmpstate; cmpstate = cmpstate-> next)
            if (streq (state-> name, cmpstate-> name))
                PrintMessage (MSG_DUPLICATE_STATE, state-> name);

        for (event = state-> child; event; event = event-> next)
          {
            nextstate = event-> child;
            if (streq (listhead-> child-> name, nextstate-> name)
            && strneq (listhead-> child-> name,     state-> name))
                PrintMessage (MSG_NEXT_STATE_NFG, nextstate-> name);
          }
      }

    /*  Insert numberings into list of states/events/modules                 */
    for (state = listhead-> child; state; state = state-> next)
      {
        state-> number = GetSymNumber (state-> name);
        for (event = state-> child; event; event = event-> next)
          {
            event-> number = GetSymNumber (event-> name);
            actions   = 0;
            nextstate = event-> child;
            find_next_state (nextstate, stats-> snames, state-> number);
            for (module = nextstate-> next; module; module = module-> next)
              {
                 actions++;
                 module-> number = GetSymNumber (module-> name);
              }
            if (stats-> maxaction < actions)
                stats-> maxaction = actions;
          }
      }
    /*  Report dialog statistics if necessary                                */
    if (OPT_STATS.flags & OPT_ON)
      {
        /*  Find size of symbol table                                        */
        for (symptr = listhead-> name; *symptr; )
            symptr = strchr (symptr + 3, 0) + 1;

        PrintMessage (MSG_STATS, stats-> states,
                                 stats-> events,
                                 stats-> modules);
        PrintMessage (MSG_SYM_USED, (int) (symptr - listhead-> name)
                                           / (LR_SYMBOLMAX / 100));
      }
    return (feedback);
}


static int
collect_names (lrnode *listhead, char type, char **names, int sort)
{
    char
        *typeptr,                       /*  Pointer to symbol type char      */
        *nameptr;                       /*  Pointer to symbol name           */
    dbyte
        item,                           /*  Used to scan table               */
        count;                          /*  Number of items found            */
    byte
        *numbptr;                       /*  Pointer to symbol number         */

    /*  Find all items of desired type in symbol table                       */
    count = 0;
    for (typeptr = listhead-> name; *typeptr; )
      {
        numbptr = (byte *) typeptr + sizeof (type);
        nameptr = (char *) numbptr + sizeof (dbyte);
        if (*typeptr == type && GetSymNumber (nameptr) > 0)
            names [count++] = nameptr;
        typeptr = strchr (nameptr, 0) + 1;
      }
    /*  Sort into alphabetical order                                         */
    if (sort)
        qsort ((void *) names, count, sizeof (names [0]), compare_name);

    /*  Number items from 0 to count-1                                       */
    for (item = 0; item < count; item++)
      {
        nameptr = names [item];         /*  Get pointer to name & number     */
        PutSymNumber (nameptr, item);
      }
    names [count] = NULL;               /*  Store NULL at end of name list   */
    return (count);
}


static int
compare_name (const void *ptr1, const void *ptr2)
{
    return (strcmp (*((char **) ptr1), *((char **) ptr2)));
}


static void
find_next_state (lrnode *node, char **names, int statenbr)
{
    int   count;
    char *nameptr;                      /*  Pointer to symbol name           */

    for (count = 0; ; count++)
      {
        nameptr = names [count];
        if (nameptr == NULL)
          {
            PrintMessage (MSG_NEXT_STATE_NF, node-> name);
            node-> number = (dbyte) statenbr;
            break;
          }
        else
        if (streq (nameptr, node-> name))
          {
            node-> number = GetSymNumber (nameptr);
            break;
          }
      }
}
