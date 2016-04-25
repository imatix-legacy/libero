/*===========================================================================*
 *                                                                           *
 *  calcul.c    Expression calculation functions                             *
 *                                                                           *
 *  Written:    91/10/07  Pieter Hintjens                                    *
 *  Revised:    95/12/07                                                     *
 *                                                                           *
 *===========================================================================*/

#include "prelude.h"
#include "calcul.h"                     /*  Include prototypes               */
#include <math.h>

#include "calculd.d"                    /*  Include dialog data              */

#define end_mark_priority   1           /*  Relative priority of tokens      */
#define left_par_priority   2           /*    which may occur in exression   */
#define right_par_priority  3           /*    - higher number means higher   */
#define term_op_priority    4           /*    priority, ie. executed first.  */
#define factor_op_priority  5
#define power_op_priority   6
#define function_priority   6
#define lowest_op_priority  4

#define power_op_token      'P'         /*  Indicates ** on operator stack   */
#define end_mark_token      'E'         /*  Indicates end of operator stack  */
#define function_token      'F'         /*  Indicates user-defined function  */

#define name_max            10          /*  Max size of function names       */
#define string_max          256         /*  Max size of string parameter     */

typedef struct {                        /*  Item on operator stack:          */
    char token;                         /*    Operator token                 */
    char name [name_max];               /*    Name of function               */
    int  priority;                      /*    Relative priority              */
    } OPREQ;

typedef struct {                        /*  Item on operand stack            */
    double number;                      /*    Value for number               */
    char  *string;                      /*    Value for string               */
    char   type;                        /*    Type: 'N' or 'S'               */
    } OPVAL;

#define operator_max 20                 /*  Max size of operator stack       */
static int   operator_ptr;              /*  Current size of operator stack   */
static OPREQ operator_stack [operator_max];

#define operand_max  20                 /*  Max size of operand stack        */
static int   operand_ptr;               /*  Current size of operand stack    */
static OPVAL operand_stack [operand_max];

static double op_1, op_2;               /*  Operands used in calculations    */
static char  *expr;                     /*  Pointer to expression to parse   */
static int    xptr;                     /*  Offset to next char to parse     */
static int    token_posn;               /*  Offset of last token parsed      */
static char   the_token;                /*  Current expression token         */
static char   op_type;                  /*  Type S or N of operand           */
static char   the_priority;             /*  Priority of current token        */
static double the_number;               /*  Value of number in expression    */
static char   the_name [name_max];      /*  Value of name in expression      */
static char   the_string [string_max];  /*  Value of string in expression    */
static int    name_size;                /*  Size of name                     */
static int    string_size;              /*  Size of string                   */
static char   cur_token;                /*  Token to execute from stack      */
static char   cur_name [name_max];      /*  Function to execute from stack   */

static calc_control *params;            /*  Pointer to control block         */
static int          feedback;           /*  0 if no errors, else error type  */

int calculate (calc_control *control)
{
    params = control;                   /*  Address parameters to function   */
    feedback = 0;                       /*  Assume no errors                 */

    #include "calculd.i"                /*  Do dialog manager                */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    xptr = 0;                           /*  Move to start of expression      */
    params-> result = 0;                /*  Assume result is zero            */
    params-> error_posn = 0;            /*  Default offset for errors        */
    expr = params-> expression;         /*  Get address of expression        */
    operand_ptr  = 0;                   /*  Operand stack holds zero         */
    operator_ptr = 0;                   /*  Operator stack holds end mark    */
    operand_stack  [0].number   = 0;
    operand_stack  [0].type     = 'N';
    operator_stack [0].token    = end_mark_token;
    operator_stack [0].priority = end_mark_priority;

    the_next_event = ok_event;          /*  Set ok                           */
}


/****************************   GET NEXT TOKEN   *****************************/

static void signal_error (int error);

MODULE get_next_token (void)
{
    char *next;

    while (expr [xptr] == ' ') xptr++;
    token_posn = xptr;
    the_token = expr [xptr++];
    the_next_event = other_event;

    switch (the_token) {
        case '+':
        case '-':
            the_next_event = operator_event;
            the_priority   = term_op_priority;
            break;

        case '*':
            if (expr [xptr] == '*') {
                the_next_event = operator_event;
                the_priority   = power_op_priority;
                the_token      = power_op_token;
                xptr++;
                break;
            }                           /*  if single '*', fall through      */
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
            string_size = -1;
            do {
                the_token = expr [xptr++];
                if (the_token == '\0') {
                    signal_error (QUOTES_EXPECTED);
                    break;
                }
                the_string [++string_size] = the_token;
            } until (the_token == '"');
            the_string [string_size] = '\0';
            the_next_event = string_event;
            break;

        case '\0':
            the_next_event = end_mark_event;
            the_priority   = end_mark_priority;
            break;

        default:
            if (isdigit (the_token)) {
                the_next_event = number_event;
                the_number = strtod (expr + xptr - 1, &next);
                xptr = (int) (next - expr);
                the_token = toupper (expr [xptr]);
                if (the_token == 'K') {
                    the_number *= 1024.0;
                    xptr++;
                } else
                if (the_token == 'M') {
                    the_number *= 1048576.0;
                    xptr++;
                }
            } else
            if (isalpha (the_token)) {
                name_size = 0;
                while (isalnum (the_token)) {
                    if (name_size < name_max - 1)
                        the_name [name_size++] = toupper (the_token);
                    the_token = expr [xptr++];
                }
                the_name [name_size] = '\0';
                the_next_event = function_event;
                the_priority   = function_priority;
                the_token      = function_token;
            } else
                signal_error (INVALID_TOKEN);
    }
}

void signal_error (int error)
{
    feedback = error;
    raise_exception (exception_event);
}


/***************************   STACK THE NUMBER   ****************************/

static void stack_operand (char type, double value);

MODULE stack_the_number (void)
{
    stack_operand ('N', the_number);
}

void stack_operand (char type, double value)
{
    if (operand_ptr < operand_max - 1) {
        operand_ptr++;
        operand_stack [operand_ptr].number = value;
        operand_stack [operand_ptr].type   = type;
    } else
        signal_error (OPERAND_OVERFLOW);
}


/***************************   STACK THE STRING   ****************************/

MODULE stack_the_string (void)
{
    stack_operand ('S', 0);
    operand_stack [operand_ptr].string = malloc (string_max);
    strcpy (operand_stack [operand_ptr].string, the_string);
}


/**************************   STACK THE OPERATOR   ***************************/

MODULE stack_the_operator (void)
{
    if (operator_ptr < operator_max - 1) {
        operator_ptr++;
        operator_stack [operator_ptr].token    = the_token;
        operator_stack [operator_ptr].priority = the_priority;
        strcpy (operator_stack [operator_ptr].name, the_name);
    } else
        signal_error (OPERATOR_OVERFLOW);
}


/*************************   UNSTACK GE OPERATORS   **************************/

static void unstack_operator (void);
static void execute_function (void);
static void require_number_op (void);
static void require_string_op (void);

MODULE unstack_ge_operators (void)
{
    while (operator_stack [operator_ptr].priority >= the_priority)
        unstack_operator ();
}

static void unstack_operator (void)
{
    cur_token = operator_stack [operator_ptr].token;
    strcpy (cur_name, operator_stack [operator_ptr--].name);

    op_1    = operand_stack [operand_ptr].number;
    op_type = operand_stack [operand_ptr].type;
    if (op_type == 'S') {
        strcpy (the_string, operand_stack [operand_ptr].string);
        free (operand_stack [operand_ptr].string);
    }
    if ((cur_token == '+')
    OR  (cur_token == '-')
    OR  (cur_token == '*')
    OR  (cur_token == '/')
    OR  (cur_token == power_op_token)) {
        op_2 = op_1;
        op_1 = operand_stack [--operand_ptr].number;
        require_number_op ();
    }
    switch (cur_token) {
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
        case power_op_token:
            op_1 = pow (op_1, op_2);
            break;
        case end_mark_token:
            require_number_op ();
            params-> result = op_1;
            break;
        case function_token:
            execute_function ();
            break;
        default:
            printf ("!Error: Invalid operator %c\n", cur_token);
            exit (0);
    }
    operand_stack [operand_ptr].number = op_1;
    operand_stack [operand_ptr].type   = 'N';
}

static void require_number_op (void)
{
    if (op_type != 'N')
        signal_error (NUMBER_EXPECTED);
}

static void require_string_op (void)
{
    if (op_type != 'S') {
        signal_error (STRING_EXPECTED);
        strcpy (the_string, "");
    }
}

static void execute_function (void)
{
    calc_fct *fptr;                     /*  Pointer to possible functions    */
    int fnbr;                           /*  Function number in list          */
    char ftype;                         /*  Function type                    */

    fptr = params-> flist;
    for (fnbr = 0; ; fnbr++) {
        if (fptr [fnbr].name [0] == '\0') {
            signal_error (UNKNOWN_FUNCTION);
            break;
        }
        if (streq (fptr [fnbr].name, cur_name)) {
            ftype = fptr [fnbr].type;
            if (ftype == 's')
                require_string_op ();
            else
                require_number_op ();

            switch (ftype) {
                case 'i':
                    op_1 = (*fptr [fnbr].fct.i) ((int) op_1);
                    break;
                case 'l':
                    op_1 = (*fptr [fnbr].fct.l) ((long) op_1);
                    break;
                case 'd':
                    op_1 = (*fptr [fnbr].fct.d) (op_1);
                    break;
                case 's':
                    op_1 = (*fptr [fnbr].fct.s) (the_string);
                    break;
                default:
                    printf ("!Error: Invalid function type %c\n", ftype);
                    exit (0);
            }
            break;
        }
    }
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
        signal_error (LEFT_PAR_EXPECTED);
}


/**************************   UNSTACK IF END MARK   **************************/

MODULE unstack_if_end_mark (void)
{
    if (operator_stack [operator_ptr].token == end_mark_token)
        unstack_operator ();
    else
        signal_error (RIGHT_PAR_EXPECTED);
}


/*************************   SIGNAL INVALID TOKEN   **************************/

MODULE signal_invalid_token (void)
{
    feedback = INVALID_TOKEN;
}


/*************************   SIGNAL TOKEN MISSING   **************************/

MODULE signal_token_missing (void)
{
    feedback = TOKEN_EXPECTED;
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
        params-> error_posn = token_posn;
}
