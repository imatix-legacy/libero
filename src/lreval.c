/*===========================================================================*
 *                                                                           *
 *  lreval.c    Condition evaluation function 'lr_evaluate ()'               *
 *                                                                           *
 *  Written:    95/03/25  Pieter Hintjens <ph@imatix.com>                    *
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

#include "lrevald.d"                    /*  Include dialog data              */

static int  token_posn;                 /*  Offset of last token parsed      */
static int  lposn;                      /*  Offset of left expression        */
static int  rposn;                      /*  Offset of right expression       */
static int  invert;                     /*  1 = invert result of condition   */
static int  feedback;                   /*  0 = okay; else error code        */
static int  column;                     /*  Input line column, 0..79         */
static int  token_len;                  /*  Size of text in token            */
static int  relator;                    /*  Current relator token            */

static char
    token [LINE_MAX + 1],               /*  Current token text               */
    lexpr [LINE_MAX + 1],               /*  Left expression for test         */
    rexpr [LINE_MAX + 1];               /*  Right expression for test        */

static byte
    cmap [256];                         /*  Character classification maps    */

#define OP_EQ            1              /*  Encoded relational operators     */
#define OP_NE            2
#define OP_LT            3
#define OP_LE            4
#define OP_GT            5
#define OP_GE            6

#define CMAP_VALID       1              /*  Normal name or expression token  */
#define CMAP_RELAT       2              /*  Logical relation operator        */
#define CMAP_QUOTE       4              /*  Possible string delimiters       */
                                        /*  Macros for character mapping:    */

#define is_valid(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_VALID) > 0)
#define is_relat(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_RELAT) > 0)
#define is_quote(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_QUOTE) > 0)


/*  Function prototypes                                                      */

static void  init_charmaps         (void);
static void  build_charmap         (byte flag, char *chars);
static void  get_normal_token      (void);
static void  get_relator_token     (void);
static void  get_string_token      (void);
static void  signal_error          (int error_type);


static char   *condition;               /*  Pointer to condition to parse    */
static Bool   *result;                  /*  Result of condition              */
static int    *offset;                  /*  Offset of any error, 0..n-1      */
static lrstat *stats;                   /*  Dialog status block              */

int lr_evaluate (char *p_condition, Bool *p_result, int *p_offset,
                 lrstat *p_stats)
{
    ASSERT (p_condition != NULL);
    ASSERT (p_result    != NULL);
    ASSERT (p_offset    != NULL);
    ASSERT (p_stats     != NULL);

    result    = p_result;               /*  Address parameters to function   */
    condition = p_condition;
    offset    = p_offset;
    stats     = p_stats;

    feedback  = 0;                      /*  Assume no errors                 */

#   include "lrevald.i"                 /*  Do dialog manager                */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    static Bool
        first_time = TRUE;

    *result = FALSE;                    /*  Assume condition is false        */
    *offset = 0;                        /*  Clear error position             */
    column  = 0;                        /*  Move to start of condition       */
    invert  = 0;                        /*  1 when 'not' found               */

    if (first_time)
      {
        init_charmaps ();               /*  Initialise character maps        */
        first_time = FALSE;
      }
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
    build_charmap (CMAP_VALID, "0123456789\\[]_.()+-*/:$");
    build_charmap (CMAP_RELAT, "=<>!");
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


/*****************************   GET NEXT TOKEN   ****************************/

MODULE get_next_token (void)
{
    char
        thisch;

    while (condition [column] == ' ')   /*  Skip leading spaces              */
        column++;

    thisch     = condition [column];
    token_len  = 0;
    token_posn = column;

    if (is_valid (thisch))
      {
        get_normal_token ();
        the_next_event = normal_event;
      }
    else
    if (is_relat (thisch))
      {
        get_relator_token ();
        the_next_event = relator_event;
      }
    else
    if (is_quote (thisch))
      {
        get_string_token ();
        the_next_event = string_event;
      }
    else
    if (thisch == 0)
        the_next_event = finished_event;
    else
        signal_error (MSG_EVAL_INVALID_TOKEN);
}

static void
get_normal_token (void)
{
    char
        thisch;                         /*  Current character                */

    while (is_valid (thisch = condition [column]))
      {
        column++;
        token [token_len++] = thisch;
      }
    token [token_len] = 0;              /*  Terminate token string           */
}

static void
get_relator_token (void)
{
    char
        thisch;                         /*  Current character                */

    while (is_relat (thisch = condition [column]))
      {
        column++;                       /*  Store char and go for next       */
        token [token_len++] = thisch;
      }
    token [token_len] = 0;              /*  Terminate token string           */

    if (streq (token, "=")              /*  Encode relator                   */
    ||  streq (token, "=="))            /*    and signal error               */
        relator = OP_EQ;                /*    if not valid                   */
    else
    if (streq (token, "!=")
    ||  streq (token, "<>"))
        relator = OP_NE;
    else
    if (streq (token, "<"))
        relator = OP_LT;
    else
    if (streq (token, "<="))
        relator = OP_LE;
    else
    if (streq (token, ">"))
        relator = OP_GT;
    else
    if (streq (token, ">="))
        relator = OP_GE;
    else
        signal_error (MSG_EVAL_INV_RELATOR);
}

static void
get_string_token (void)
{
    char
        delim,                          /*  String delimiter " or '          */
        thisch;                         /*  Current character                */

    delim = condition [column++];       /*  Get delimiter character          */
    FOREVER                             /*  Loop until string is complete    */
      {                                 /*    or end of line                 */
        thisch = condition [column++];
        if (thisch == 0)
          {
            signal_error (MSG_EVAL_QUOTE_MISSING);
            break;
          }
        else
        if (thisch == delim)
            break;
        else
        if (thisch == '\\')
          {                             /*  Handle escaped character         */
            thisch = condition [column++];
            if (thisch == 'n')
                thisch = '\n';          /*  Newline                          */
            else
            if (thisch == 't')
                thisch = '\t';          /*  Tab                              */
          }
        token [token_len++] = thisch;
      }
    token [token_len] = 0;              /*  Terminate token string           */
}

static void
signal_error (int error_type)
{
    feedback = error_type;
    raise_exception (exception_event);
}


/************************   CLASSIFY TOKEN AS ACTION   ***********************/

MODULE classify_token_as_action (void)
{
    typedef struct {                    /*  Lookup table structure:          */
        char   *name;                   /*    Action name                    */
        event_t event;                  /*    Corresponding event            */
    } ACTION_LOOKUP;

    /*  Table of actions                                                     */
    /*      arranged alphabetically for cosmetic purposes                    */
    static ACTION_LOOKUP
        lookup [] = {
            { "animate",  animate_event  },
            { "caps",     caps_event     },
            { "check",    check_event    },
            { "cobol",    cobol_event    },
            { "defaults", defaults_event },
            { "event",    event_event    },
            { "exist",    exist_event    },
            { "headline", headline_event },
            { "module",   module_event   },
            { "normal",   normal_event   },
            { "not",      not_event      },
            { "plain",    plain_event    },
            { "state",    state_event    },
            { "test",     test_event     },
            { "title",    title_event    },
            /*  Sentinel marks end of table                                  */
            {  NULL,      other_event    }
        };
    ACTION_LOOKUP
        *lptr;                          /*  Index into lookup table          */

    StrLwr (token);                     /*  Actions names are lower-case     */
    for (lptr = lookup; lptr-> name; lptr++)
        if (streq (lptr-> name, token))
            break;                      /*  Scan till we find match, or end  */

    the_next_event = lptr-> event;      /*    and set appropriate event      */
}


/************************   INVERT CONDITION RESULT   ************************/

MODULE invert_condition_result (void)
{
    invert = 1 - invert;                /*  Flip between 1 and 0             */
}


/***********************   EVALUATE ANIMATION ENABLED   **********************/

MODULE evaluate_animation_enabled (void)
{
    *result = ((OPT_ANIMATE.flags & OPT_ON) != 0);
}


/*************************   EVALUATE CHECK ENABLED   ************************/

MODULE evaluate_check_enabled (void)
{
    *result = ((OPT_CHECK.flags & OPT_ON) != 0);
}


/***********************   EVALUATE DEFAULTS DEFINED   ***********************/

MODULE evaluate_defaults_defined (void)
{
    *result = (stats-> defaults != LR_NULL_STATE);
}


/**************************   EVALUATE FILE EXISTS   *************************/

MODULE evaluate_file_exists (void)
{
    *result = FileExists (token);
}


/*************************   EVALUATE STYLE IS CAPS   ************************/

MODULE evaluate_style_is_caps (void)
{
    *result = (streq (OPT_STYLE.value, "caps"));
}


/************************   EVALUATE STYLE IS COBOL   ************************/

MODULE evaluate_style_is_cobol (void)
{
    *result = (streq (OPT_STYLE.value, "cobol"));
}


/***********************   EVALUATE STYLE IS HEADLINE   **********************/

MODULE evaluate_style_is_headline (void)
{
    *result = (streq (OPT_STYLE.value, "headline"));
}


/************************   EVALUATE STYLE IS PLAIN   ************************/

MODULE evaluate_style_is_plain (void)
{
    *result = (streq (OPT_STYLE.value, "plain"));
}


/************************   EVALUATE STYLE IS TITLE   ************************/

MODULE evaluate_style_is_title (void)
{
    *result = (streq (OPT_STYLE.value, "title"));
}


/************************   EVALUATE STYLE IS NORMAL   ***********************/

MODULE evaluate_style_is_normal (void)
{
    *result = (streq (OPT_STYLE.value, "normal"));
}


/*************************   EVALUATE STATE EXISTS   *************************/

MODULE evaluate_state_exists (void)
{
    int state;

    CleanName (token);                  /*  Get normalised name              */
    puts (token);
    for (state = 0; state < stats-> states; state++)
      {
        puts (stats-> snames [state]);
        if (streq (stats-> snames [state], token))
          {
            *result = TRUE;
            break;
          }
      }
}


/*************************   EVALUATE EVENT EXISTS   *************************/

MODULE evaluate_event_exists (void)
{
    int event;

    CleanName (token);                  /*  Get normalised name              */
    for (event = 0; event < stats-> events; event++)
      {
        if (streq (stats-> enames [event], token))
          {
            *result = TRUE;
            break;
          }
      }
}


/*************************   EVALUATE MODULE EXISTS   ************************/

MODULE evaluate_module_exists (void)
{
    int module;

    CleanName (token);                  /*  Get normalised name              */
    for (module = 0; module < stats-> modules; module++)
      {
        if (streq (stats-> mnames [module], token))
          {
            *result = TRUE;
            break;
          }
      }
}


/************************   INIT LEFT AND RIGHT EXPR   ***********************/

MODULE init_left_and_right_expr (void)
{
    strclr (lexpr);
    strclr (rexpr);
}


/***********************   CONCAT TOKEN TO LEFT EXPR   ***********************/

MODULE concat_token_to_left_expr (void)
{
    strcat (lexpr, token);              /*  Spaces are not significant       */
}


/***********************   CONCAT TOKEN TO RIGHT EXPR   **********************/

MODULE concat_token_to_right_expr (void)
{
    strcat (rexpr, token);              /*  Spaces are not significant       */
}


/************************   SAVE LEFT EXPR POSITION   ************************/

MODULE save_left_expr_position (void)
{
    lposn = token_posn;
}


/************************   SAVE RIGHT EXPR POSITION   ***********************/

MODULE save_right_expr_position (void)
{
    rposn = token_posn;
}


/**********************   EVALUATE NUMERIC COMPARISON   **********************/

MODULE evaluate_numeric_comparison (void)
{
    long lvalue, rvalue;                /*  Expression results               */
    int  feedback,                      /*  Feedback from lr_calculate ()    */
         offset;                        /*  Where is error in expr, if any   */

    feedback = lr_calculate (lexpr, &lvalue, &offset);
    if (feedback)
        token_posn = lposn + offset;    /*  Point to error in left expr      */
    else
      {
        feedback = lr_calculate (rexpr, &rvalue, &offset);
        if (feedback)                   /*  Point to error in right expr     */
            token_posn = rposn + offset;
      }
    if (feedback)                       /*  If error in expr, signal it      */
        signal_error (feedback);
    else                                /*  Else make comparison             */
        switch (relator)
          {
            case OP_EQ: *result = (lvalue == rvalue); break;
            case OP_NE: *result = (lvalue != rvalue); break;
            case OP_LT: *result = (lvalue <  rvalue); break;
            case OP_LE: *result = (lvalue <= rvalue); break;
            case OP_GT: *result = (lvalue >  rvalue); break;
            case OP_GE: *result = (lvalue >= rvalue); break;
            default:    signal_error (MSG_EVAL_INTERNAL_ERROR);
          }
}


/***********************   EVALUATE STRING COMPARISON   **********************/

MODULE evaluate_string_comparison (void)
{
    int
        cmp;

    cmp = strcmp (lexpr, rexpr);

    switch (relator)
      {
        case OP_EQ: *result = (cmp == 0); break;
        case OP_NE: *result = (cmp != 0); break;
        case OP_LT: *result = (cmp <  0); break;
        case OP_LE: *result = (cmp <= 0); break;
        case OP_GT: *result = (cmp >  0); break;
        case OP_GE: *result = (cmp >= 0); break;
        default:    signal_error (MSG_EVAL_INTERNAL_ERROR);
      }
}


/***********************   SIGNAL EXPRESSION EXPECTED   **********************/

MODULE signal_expression_expected (void)
{
    feedback = MSG_EVAL_EXPR_EXPECTED;
}


/*************************   SIGNAL STRING EXPECTED   ************************/

MODULE signal_string_expected (void)
{
    feedback = MSG_EVAL_STRING_EXPECTED;
}


/*********************   SIGNAL EXPR OR STRING EXPECTED   ********************/

MODULE signal_expr_or_string_expected (void)
{
    feedback = MSG_EVAL_EITHER_EXPECTED;
}


/************************   SIGNAL RELATOR EXPECTED   ************************/

MODULE signal_relator_expected (void)
{
    feedback = MSG_EVAL_RELATOR_EXPECTED;
}


/*************************   SIGNAL RELATOR IGNORED   ************************/

MODULE signal_relator_ignored (void)
{
    feedback = MSG_EVAL_RELATOR_IGNORED;
}


/*************************   SIGNAL TOKEN MISSING   **************************/

MODULE signal_token_missing (void)
{
    feedback = MSG_EVAL_TOKEN_EXPECTED;
}


/*************************   SIGNAL ACTION EXPECTED   ************************/

MODULE signal_action_expected (void)
{
    feedback = MSG_EVAL_ACTION_EXPECTED;
}


/***************************   GET EXTERNAL EVENT   **************************/

MODULE get_external_event (void)
{
}


/*************************   TERMINATE THE PROGRAM   *************************/

MODULE terminate_the_program (void)
{
    the_next_event = terminate_event;
    if (feedback)                       /*  If error, return position of     */
        *offset = token_posn;           /*    error in expression line       */
    else
        if (invert)                     /*  Else invert result if needed     */
            *result = ! *result;
}
