/*===========================================================================*
 *									     *
 *  lrfree.c	free memory areas used by dialog			     *
 *									     *
 *  Written:	92/11/07  Pieter Hintjens <ph@imatix.com>		     *
 *  Revised:	96/12/29						     *
 *									     *
 *  FSM Code Generator.  Copyright (c) 1991-97 iMatix.			     *
 *									     *
 *  This program is free software; you can redistribute it and/or modify     *
 *  it under the terms of the GNU General Public License as published by     *
 *  the Free Software Foundation; either version 2 of the License, or	     *
 *  (at your option) any later version. 				     *
 *									     *
 *  This program is distributed in the hope that it will be useful,	     *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of	     *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	     *
 *  GNU General Public License for more details.			     *
 *									     *
 *  You should have received a copy of the GNU General Public License	     *
 *  along with this program; if not, write to the Free Software 	     *
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.		     *
 *===========================================================================*/

#include "prelude.h"                    /*  Public definitions               */
#include "lrpriv.h"                     /*  Private definitions              */


/*-----------------------------.
 |  lr_free_memory	       |
 |-----------------------------`---------------------------------------------.
 |  int lr_free_memory (lrnode *head, lrstat *stats);			     |
 |									     |
 |  Description:							     |
 |									     |
 |									     |
 |  Returns: 0 if okay, -1 if errors were found.			     |
 |									     |
 |  Comments:								     |
 |									     |
 `---------------------------------------------------------------------------*/

int lr_free_memory (lrnode *listhead, lrstat *stats)
{
    lrnode *state;			/*  Pointer to state in dialog list  */

    int feedback = 0;

    ASSERT (stats-> mnames != NULL);
    ASSERT (stats-> enames != NULL);
    ASSERT (stats-> snames != NULL);
    ASSERT (stats-> source != NULL);

    free (stats-> mnames);		/*  Drop list of modules	     */
    free (stats-> enames);		/*    and list of events	     */
    free (stats-> snames);		/*    and list of states	     */
    free (stats-> source);		/*    and source file name	     */

    state = listhead-> child;		/*  Free main states list	     */
    while (state)
	state = DeleteState (listhead, state);

    free (listhead-> name);		/*  Free symbol table		     */
    return (feedback);
}
