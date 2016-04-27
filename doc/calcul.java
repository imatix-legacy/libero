/*  ----------------------------------------------------------------<Prolog>-
    Name:       calcul.java
    Title:      Expression evaluator class: calculate expression

    Written:    96/06/26  Pascal Antonnaux <pascal@imatix.com>
    Revised:    96/07/14

    Copyright:  Copyright (c) 1991-1996 iMatix
    This program is free software; you can redistribute it and/or modify it
    under the terms of the GNU General Public License as published by the Free
    Software Foundation; either version 2 of the License, or (at your option)
    any later version. This program is distributed in the hope that it will be
    useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
    Public License for more details.  You should have received a copy of the
    GNU General Public License along with this program; if not, write to the
    Free Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 ------------------------------------------------------------------</Prolog>-*/

import java.awt.*;
import java.applet.*;
import java.net.*;
import java.util.*;
import java.io.*;
import java.lang.*;

public class calcul extends calculi
{
    // Public variables

    public double
        result;                         //  Result of evaluation
    public int
        err_pos,                        //  Position of error
        feedback;                       //  feedback value
    public String
        err_str;                        //  Error string

    // Private variables
    private operator
        operator_stack [];              //  Operator stack
    private operand
        operand_stack  [];              //  Operand stack
    private String
        the_string,                     //  Value of string in expression
        expr;                           //  Expression to parse
    private double
        the_number,                     //  Value of number in expression
        op_1,
        op_2;                           //  Operands used in calculations
    private char
        the_token,                      //  Current expression token
        op_type,                        //  Type S or N of operand
        cur_token;                      //  Token to execute from stack
    private int
        xptr,                           //  Offset to next char to parse
        length,                         //  length of expression
        name_size,                      //  Size of name
        string_size,                    //  Size of string
        the_priority,                   //  Priority of current token
        operator_ptr,                   //  Current size of operator stack
        operand_ptr,                    //  Current size of operand stack
        operator_max = 20,              //  Max size of operator stack
        operand_max  = 20,              //  Max size of operand stack
        token_posn;                     //  Offset of last token parsed
    static int                          //  Definition of return code
        INVALID_TOKEN      = 1,
        TOKEN_EXPECTED     = 2,
        LEFT_PAR_EXPECTED  = 3,
        RIGHT_PAR_EXPECTED = 4,
        QUOTES_EXPECTED    = 5,
        OPERAND_OVERFLOW   = 6,
        OPERATOR_OVERFLOW  = 7,
        NUMBER_EXPECTED    = 8,
        STRING_EXPECTED    = 9,
        UNKNOW_FUNCTION    = 10;

    private static int
        end_mark_priority  = 1,         //  Relative priority of tokens
        left_par_priority  = 2,         //    which may occur in exression
        right_par_priority = 3,         //    - higher number means higher
        term_op_priority   = 4,         //    priority, ie. executed first.
        factor_op_priority = 5,
        power_op_priority  = 6,
        lowest_op_priority = 4;

    private static char
        power_op_token     ='P',        //  Indicates ** on operator stack
        end_mark_token     ='E';        //  Indicates end of operator stack

    static String calc_error [] = {     //  Error string array
        "No errors.",
        "Invalid or unrecognised token.",
        "Unexpected end of expression.",
        "Left parenthesis is missing.",
        "Right parenthesis is missing.",
        "Quotes missing after string.",
        "Too many operands in expression.",
        "Too many levels of parentheses in expression.",
        "Number is required here.",
        "String between quotes is required here.",
        "Unknown function name."
    };


///////////////////////////   C O N T R U C T O R   ///////////////////////////

public calcul ()
{
    int
        index;

    //  Allocate operator and operand stack
    operator_stack = new operator [operator_max];
    operand_stack  = new operand  [operand_max];

    for (index = 0; index < operator_max; index++)
        operator_stack [index] = new operator ();
    for (index = 0; index < operand_max; index++)
        operand_stack [index] = new operand ();

    the_string = new String ();
    expr       = new String ();
}


//////////////////////////////////   M A I N   ////////////////////////////////

public int parse (String expression)
{
    expr = expression;
    execute ();

    if (feedback != 0)
        err_str = calc_error [feedback];

    return (feedback);
}


//////////////////////////   INITIALISE THE PROGRAM   /////////////////////////

public void initialise_the_program ()
{
    the_string   = "";
    feedback     = 0;
    xptr         = 0;               //  Move to start of expression
    result       = 0;               //  Assume result is zero
    err_pos      = 0;               //  Default offset for errors
    operand_ptr  = 0;               //  Operand stack holds zero
    operator_ptr = 0;               //  Operator stack holds end mark
    operand_stack  [0].number   = 0;
    operand_stack  [0].type     = 'N';
    operator_stack [0].token    = end_mark_token;
    operator_stack [0].priority = end_mark_priority;
    length = expr.length ();

    the_next_event = ok_event;
}


////////////////////////////   GET EXTERNAL EVENT   ///////////////////////////

public void get_external_event ()
{
}

//%START MODULE

//////////////////////////////   GET NEXT TOKEN   /////////////////////////////

public void get_next_token ()
{
    //  Skip leading spaces
    while (xptr < length && expr.charAt (xptr) == ' ') xptr++;

    if (xptr >= length)
      {
         the_next_event = end_mark_event;
         the_priority   = end_mark_priority;
         return;
      }
    token_posn = xptr;
    the_token  = expr.charAt (xptr++);

    the_next_event = other_event;
    switch (the_token)
      {
        case '+':
        case '-':
            the_next_event = operator_event;
            the_priority   = term_op_priority;
            break;

        case '*':
            if (xptr < length && expr.charAt (xptr) == '*')
              {
                the_next_event = operator_event;
                the_priority   = power_op_priority;
                the_token      = power_op_token;
                xptr++;
                break;
              }                     //  if single '*', fall through
        case '/':
            the_next_event = operator_event;
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

        case '"':
            if (xptr < length)
              {
                String s;
                string_size = -1;

                the_string  = "";
                do
                  {
                    the_token = expr.charAt (xptr++);
                    if (the_token == '\0')
                      {
                        signal_error (QUOTES_EXPECTED);
                        break;
                      }
                    s = "" + the_token;
                    the_string.concat (s);
                    string_size++;
                  } while (xptr < length && the_token != '"');
              }
            the_next_event = string_event;
            break;

        case '\0':
            the_next_event = end_mark_event;
            the_priority   = end_mark_priority;
            break;

        default:
            if (Character.isDigit (the_token))
              {
                StringBuffer
                    num ;
                char
                    ch;

                the_next_event = number_event;
                num = new StringBuffer ();
                xptr--;

                ch = expr.charAt (xptr++);
                do
                  {
                    num.append (ch);
                    if (xptr < length)
                        ch = expr.charAt (xptr++);
                    else
                        break;
                  } while (Character.isDigit (ch) || ch == '.');

                if (xptr < length || !Character.isDigit (ch))
                    xptr--;

                the_number = (Double.valueOf
                             (new String (num))).doubleValue ();
                if (xptr < length)
                  {
                    the_token = Character.toUpperCase (expr.charAt (xptr));
                    if (the_token == 'K')
                      {
                        the_number *= 1024.0;
                        xptr++;
                      }
                    else
                    if (the_token == 'M')
                      {
                        the_number *= 1048576.0;
                        xptr++;
                      }
                  }
              }
            else
                signal_error (INVALID_TOKEN);
      }
}

private void signal_error (int error)
{
    feedback = error;
    raise_exception (exception_event);
}


///////////////////////////   SIGNAL INVALID TOKEN   //////////////////////////

public void signal_invalid_token ()
{
    feedback = INVALID_TOKEN;
}


///////////////////////////   SIGNAL TOKEN MISSING   //////////////////////////

public void signal_token_missing ()
{
    feedback = TOKEN_EXPECTED;
}


/////////////////////////////   STACK THE NUMBER   ////////////////////////////

public void stack_the_number ()
{
    stack_operand ('N', the_number);
}


private void stack_operand (char type, double value)
{
    if (operand_ptr < operand_max - 1)
      {
        operand_ptr++;
        operand_stack [operand_ptr].number = value;
        operand_stack [operand_ptr].string = "";
        operand_stack [operand_ptr].type   = type;
      }
    else
        signal_error (OPERAND_OVERFLOW);
}

private void stack_operand (char type, String value)
{
    if (operand_ptr < operand_max - 1)
      {
        operand_ptr++;
        operand_stack [operand_ptr].number = 0;
        operand_stack [operand_ptr].string = value;
        operand_stack [operand_ptr].type   = type;
      }
    else
        signal_error (OPERAND_OVERFLOW);
}


////////////////////////////   STACK THE OPERATOR   ///////////////////////////

public void stack_the_operator ()
{
    if (operator_ptr < operator_max - 1)
      {
        operator_ptr++;
        operator_stack [operator_ptr].token    = the_token;
        operator_stack [operator_ptr].priority = the_priority;
      }
    else
        signal_error (OPERATOR_OVERFLOW);
}


/////////////////////////////   STACK THE STRING   ////////////////////////////

public void stack_the_string ()
{
    stack_operand ('S', the_string);
}


//////////////////////////   TERMINATE THE PROGRAM   //////////////////////////

public void terminate_the_program ()
{
    the_next_event = terminate_event;
    if (feedback != 0)
        err_pos = token_posn;
}


//////////////////////////   UNSTACK ALL OPERATORS   //////////////////////////

public void unstack_all_operators ()
{
    while (operator_stack [operator_ptr].priority >= lowest_op_priority)
        unstack_operator ();
}


///////////////////////////   UNSTACK GE OPERATORS   //////////////////////////

public void unstack_ge_operators ()
{
    while (operator_stack [operator_ptr].priority >= the_priority)
        unstack_operator ();
}

private void unstack_operator ()
{
    cur_token = operator_stack [operator_ptr--].token;
    op_1      = operand_stack  [operand_ptr].number;
    op_type   = operand_stack  [operand_ptr].type;
    if (op_type == 'S')
      {
        the_string = operand_stack [operand_ptr].string;
        operand_stack [operand_ptr].string = "";
      }
    if ((cur_token == '+')
    ||  (cur_token == '-')
    ||  (cur_token == '*')
    ||  (cur_token == '/')
    ||  (cur_token == power_op_token))
      {
        op_2 = op_1;
        op_1 = operand_stack [--operand_ptr].number;
        require_number_op ();
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
        default:
           if (cur_token == power_op_token)
                op_1 = Math.pow (op_1, op_2);
           else
           if (cur_token == end_mark_token)
             {
               require_number_op ();
               result = op_1;
             }
           else
               signal_error (UNKNOW_FUNCTION);
           break;
      }
    operand_stack [operand_ptr].number = op_1;
    operand_stack [operand_ptr].type   = 'N';
}

private void require_number_op ()
{
    if (op_type != 'N')
        signal_error (NUMBER_EXPECTED);
}

private void require_string_op ()
{
    if (op_type != 'S')
      {
        signal_error (STRING_EXPECTED);
        the_string = "";
      }
}


///////////////////////////   UNSTACK IF END MARK   ///////////////////////////

public void unstack_if_end_mark ()
{
    if (operator_stack [operator_ptr].token == end_mark_token)
        unstack_operator ();
    else
        signal_error (RIGHT_PAR_EXPECTED);
}


///////////////////////////   UNSTACK IF LEFT PAR   ///////////////////////////

public void unstack_if_left_par ()
{
    if (operator_stack [operator_ptr].token == '(')
        operator_ptr--;
    else
        signal_error (LEFT_PAR_EXPECTED);
}

//%END MODULE
}




