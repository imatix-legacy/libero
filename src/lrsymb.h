/*===========================================================================*
 *									     *
 *  lrsymb.h	Libero symbol-table function prototypes 		     *
 *									     *
 *  Written:	93/12/27  Pieter Hintjens <ph@imatix.com>		     *
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

#ifndef _LRSYMB_INCLUDED		/*  Allow multiple inclusions	     */
#define _LRSYMB_INCLUDED

#define SYM_TYPE_INT	    1		/*  Indicates integer symbol	     */
#define SYM_TYPE_STRING     2		/*  Indicates string symbol	     */
#define SYM_TYPE_EITHER     3		/*  Indicates flexible type	     */

#define SYM_FLAG_RDONLY     1		/*  Indicates read-only variable     */
#define SYM_FLAG_FOUND	    2		/*  Indicates found in source	     */

#define SYM_SIGNATURE	    12345	/*  Indicates valid symbol	     */
#define SYM_HASH_SIZE	    256 	/*  If you change this, also change  */
					/*  sym_hash () 		     */
#define ASSERT_SYM(s)	    ASSERT((s)-> signature == SYM_SIGNATURE)

typedef struct symbol_tag {
    struct  symbol_tag
	   *next, *prev,		/*  Next/prev symbol in scope	     */
	   *h_next, *h_prev;		/*  Next/prev symb in bucket	     */
    char   *name;			/*  Copy of name		     */
    long    ivalue;			/*  Long integer value		     */
    char   *svalue;			/*  String value, or null	     */
    byte    type;			/*  SYM_TYPE_...		     */
    byte    flags;			/*  SYM_FLAG_...		     */
    byte    hash_value; 		/*  Hash bucket #		     */
#   ifdef DEBUG
    int     signature;			/*  Indicates valid symbol	     */
#   endif
} symbol;

typedef struct {
    symbol   *syms;
    symbol   *hash [SYM_HASH_SIZE];
    int      size;
} sym_tab;

typedef int  (*sym_func)(symbol *,...);

sym_tab *sym_create_table  (void);
void	 sym_delete_table  (sym_tab *);
symbol	*sym_lookup	   (const sym_tab *, const char *);
symbol	*sym_intern	   (sym_tab *, const char *, const int);
symbol	*sym_create	   (sym_tab *, const char *, const int);
symbol	*sym_delete	   (sym_tab *, symbol *);
void	 sym_values	   (symbol *, const long ivalue, const char *svalue);
void	 sym_exec_all	   (const sym_tab *, const sym_func,...);

#endif					/*  Include LRSYMB.H		     */
