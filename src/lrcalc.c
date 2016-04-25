/*===========================================================================*
 *                                                                           *
 *  lrcalc.c    Expression calculation function 'lr_calculate ()'            *
 *                                                                           *
 *  Written:    91/10/07  Pieter Hintjens <ph@imatix.com>                    *
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

#include "lrcalcd.d"                    /*  Include dialog data              */

#define end_mark_priority   1           /*  Relative priority of tokens      */
#define left_par_priority   2           /*    which may occur in exression   */
#define right_par_priority  3           /*    - higher number means higher   */
#define term_op_priority    4           /*    priority, ie. executed first.  */
#define factor_op_priority  5
#define lowest_op_priority  4
#define end_mark_token      'E'         /*  Indicates end of operator stack  */

typedef struct                          /*  Item on operator stack:          */
  {
    char token;                         /*    Operator token                 */
    int  priority;                      /*    Relative priority              */
  } OPREQ;

typedef struct                          /*  Item on operand stack            */
  {
    long number;                        /*    Value for number               */
  } OPVAL;

#define operator_max 30                 /*  Max size of operator stack       */
static int   operator_ptr;              /*  Current size of operator stack   */
static OPREQ operator_stack [operator_max];

#define operand_max  30                 /*  Max size of operand stack        */
static int   operand_ptr;               /*  Current size of operand stack    */
static OPVAL operand_stack [operand_max];

static long   op_1, op_2;               /*  Operands used in calculations    */
static int    expr_ptr;                 /*  Offset to next char to parse     */
static int    token_posn;               /*  Offset of last token parsed      */
static char   the_token;                /*  Current expression token         */
static char   the_priority;             /*  Priority of current token        */
static long   the_number;               /*  Value of number in expression    */
static char   cur_token;                /*  Token to execute from stack      */

static int    feedback;                 /*  0 = okay; else error code        */

static char  *expr;                     /*  Pointer to expression to parse   */
static long  *result;                   /*  Result of expression             */
static int   *offset;                   /*  Offset of any error, 0..n-1      */

/*  Function prototypes                                                      */

static void collect_number    (void);
static void signal_error      (int error_type);
static void unstack_operator  (void);


int lr_calculate (char *p_expr, long *p_result, int *p_offset)
{
    ASSERT (p_expr   != NULL);
    ASSERT (p_result != NULL);
    ASSERT (p_offset != NULL);

    result   = p_result;                /*  Address parameters to function   */
    expr     = p_expr;
    offset   = p_offset;
    feedback = 0;                       /*  Assume no errors                 */

#   include "lrcalcd.i"                 /*  Do dialog manager                */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    *result      = 0;                   /*  Assume result is zero            */
    expr_ptr     = 0;                   /*  Move to start of expression      */
    operand_ptr  = 0;                   /*  Operand stack holds zero         */
    operator_ptr = 0;                   /*  Operator stack holds end mark    */
    operand_stack  [0].number   = 0;
    operator_stack [0].token    = end_mark_token;
    operator_stack [0].priority = end_mark_priority;

    the_next_event = ok_event;
}


/****************************   GET NEXT TOKEN   *****************************/

MODULE get_next_token (void)
{
    while (expr [expr_ptr] == ' ')      /*  Skip spaces                      */
        expr_ptr++;

    token_posn = expr_ptr;              /*  Save start of this token         */
    the_token  = expr [expr_ptr++];     /*  Get next token                   */
    switch (the_token)
      {
        case '+':
        case '-':
            the_next_event = term_op_event;
            the_priority   = term_op_priority;
            break;

        case '*':
        case '/':
            the_next_event = factor_op_event;
            the_priority   = factor_op_priority;
            break;

        case '(':
            the_next_event = left_par_event;
            the_priority   = left_par_priority;
            break;

        case ')':
            the_next_event = right_par_event;
            the_priority   = right_par_priority;
            break;

        case '\0':
            the_next_event = end_mark_event;
            the_priority   = end_mark_priority;
            break;

        default:
            if (isdigit (the_token))
              {
                the_next_event = number_event;
                collect_number ();
              }
            else
                signal_error (MSG_CALC_INVALID_TOKEN);
      }
}

static void
collect_number (void)
{
    the_number = the_token - '0';
    FOREVER
      {
        the_token = expr [expr_ptr];
        if (isdigit (the_token))
          {
            the_number = the_number * 10 + the_token - '0';
            expr_ptr++;
          }
        else
            break;
      }
}

static void
signal_error (int error_type)
{
    feedback = error_type;
    raise_exception (exception_event);
}


/**************************   ALLOW SIGNED NUMBER   **************************/

MODULE allow_signed_number (void)
{
    char sign = the_token;

    if (isdigit (expr [expr_ptr]))
      {
        the_token = expr [expr_ptr++];
        collect_number ();
        if (sign == '-')
            the_number = 0 - the_number;
        raise_exception (number_event);
      }
}


/***************************   STACK THE NUMBER   ****************************/

MODULE stack_the_number (void)
{
    if (operand_ptr < operand_max - 1)
      {
        operand_ptr++;
        operand_stack [operand_ptr].number = the_number;
      }
    else
        signal_error (MSG_CALC_OPERAND_OVER);
}


/**************************   STACK THE OPERATOR   ***************************/

MODULE stack_the_operator (void)
{
    if (operator_ptr < operator_max - 1)
      {
        operator_ptr++;
        operator_stack [operator_ptr].token    = the_token;
        operator_stack [operator_ptr].priority = the_priority;
      }
    else
        signal_error (MSG_CALC_OPERATOR_OVER);
}


/*************************   UNSTACK GE OPERATORS   **************************/

MODULE unstack_ge_operators (void)
{
    while (operator_stack [operator_ptr].priority >= the_priority)
        unstack_operator ();
}

static void
unstack_operator (void)
{
    cur_token = operator_stack [operator_ptr--].token;
    op_1      = operand_stack  [operand_ptr].number;

    if (cur_token == '+'
    ||  cur_token == '-'
    ||  cur_token == '*'
    ||  cur_token == '/')
      {
        op_2 = op_1;
        op_1 = operand_stack [--operand_ptr].number;
      }
    switch (cur_token)
      {
        case '+':
            op_1 += op_2;
            break;
        case '-':
            op_1 -= op_2;
            break;
        case '*':
            op_1 *= op_2;
            break;
        case '/':
            op_1 /= op_2;
            break;
        case end_mark_token:
            *result = op_1;
            break;
        default:
            signal_error (MSG_CALC_INTERNAL_ERROR);
            break;
      }
    operand_stack [operand_ptr].number = op_1;
}


/*************************   UNSTACK ALL OPERATORS   *************************/

MODULE unstack_all_operators (void)
{
    while (operator_stack [operator_ptr].priority >= lowest_op_priority)
        unstack_operator ();
}


/**************************   UNSTACK IF LEFT PAR   **************************/

MODULE unstack_if_left_par (void)
{
    if (operator_stack [operator_ptr].token == '(')
        operator_ptr--;
    else
        signal_error (MSG_CALC_NO_LEFT_PAR);
}


/**************************   UNSTACK IF END MARK   **************************/

MODULE unstack_if_end_mark (void)
{
    if (operator_stack [operator_ptr].token == end_mark_token)
        unstack_operator ();
    else
        signal_error (MSG_CALC_NO_RIGHT_PAR);
}


/*************************   SIGNAL INVALID TOKEN   **************************/

MODULE signal_invalid_token (void)
{
    feedback = MSG_CALC_INVALID_TOKEN;
}


/*************************   SIGNAL TOKEN MISSING   **************************/

MODULE signal_token_missing (void)
{
    feedback = MSG_CALC_TOKEN_EXPECTED;
}


/***************************   GET EXTERNAL EVENT   **************************/

MODULE get_external_event (void)
{
}


/*************************   TERMINATE THE PROGRAM   *************************/

MODULE terminate_the_program (void)
{
    the_next_event = terminate_event;
    if (feedback)
        *offset = token_posn;
}
