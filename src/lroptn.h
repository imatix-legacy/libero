/*===========================================================================*
 *									     *
 *  lroptn.h	Define prototypes for LROPTN.C				     *
 *									     *
 *  Written:	92/10/25  Pieter Hintjens <ph@imatix.com>		     *
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

#ifndef _OPTN_INCLUDED			/*  Allow multiple inclusions	     */
#define _OPTN_INCLUDED

/*  Type definitions							     */

typedef struct {			/*  Command-line option 	     */
    char    *name;			/*    name of option		     */
    char    type;			/*    ?  Boolean on/off value	     */
					/*    =  Single value		     */
					/*    +  List of values 	     */
    byte    flags;			/*    bit 0 = T/F		     */
    char   *value;			/*    actual value if any	     */
    char   *check;			/*    check list of valid values     */
} option;

#define OPT_OFF                 0       /*  Bit masks for 'flags' field      */
#define OPT_ON			1

/*  Function prototypes 						     */

int    lr_parse_option		(char *option);
void   lr_parse_option_line	(char *line);
int    lr_parse_option_file	(char *filename);
void   lr_reset_options 	(void);
void   lr_show_options		(char *filename);
void   lr_push_options		(void);
void   lr_pop_options		(void);

#endif					/*  Included			     */
