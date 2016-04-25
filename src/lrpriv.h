/*===========================================================================*
 *                                                                           *
 *  lrpriv.h    Standard header file for private LIBERO use                  *
 *                                                                           *
 *  Written:    92/10/25  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/06/08                                                     *
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

#ifndef _PRIV_INCLUDED                  /*  Allow multiple inclusions        */
#define _PRIV_INCLUDED

#include "lrglib.h"                     /*  Include general library protos   */
#include "lroptn.h"                     /*  Command-line options protos      */

#define LIBERO_VERSION  "2.30"          /*  Current version of product       */
#define MESSAGE_FILE    "lrmesg.txt"    /*  All error messages               */

/*  Limits of buffers and other symbolic definitions                         */

#define LR_SYMBOLMAX          32000     /*  Max size of symbol table         */
#define LR_STATEMAX            1000     /*  Maximum states in dialog         */
#define LR_EVENTMAX            1000     /*  Maximum events in dialog         */
#define LR_VECTORMAX           1000     /*  Maximum vectors in dialog        */
#define LR_MODULEMAX           1000     /*  Maximum modules in dialog        */
#define LR_FILENAMEMAX          128     /*  Maximum length of a filename     */
#define LR_NULL_STATE            -1     /*  Used for next state number       */
#define LR_HEADER_WIDTH          79     /*  Default header line width        */
#define LR_RUNAWAY_LIMIT       1000     /*  Default :do run-away limit       */
#define LR_ROW_WIDTH             60     /*  Amount of $row on one line       */

/*  Error and information messages in lrmesg.txt                             */

#define MSG_ANY                     0   /*  If no other message shown        */
#define MSG_HELP                 9000   /*  Help text starts here            */

/*  Messages shown by lr.c                                                   */
#define MSG_INVALID_FILENAME       10   /*  Filename argument is lousy       */
#define MSG_DIALOG_FILE_NF         11   /*  Can't find specified .l file     */
#define MSG_PROCESSING             12   /*  Processing file...               */
#define MSG_ABORTED                13   /*  If process is interrupted        */
#define MSG_INTERRUPTED            14   /*    or dies with an error          */
#define MSG_CANCELLED              15   /*                                   */
#define MSG_SEGMENT_ERROR          16   /*                                   */
#define MSG_TRACE_FILE_ERROR       17   /*  Can't create trace file          */
#define MSG_DIALOG_EMPTY           18   /*  Dialog not complete              */

/*  Messages shown by lrload.c                                               */
#define MSG_PLUSD_NAME_ERROR       20   /*  Doesn't match /etk rules         */
#define MSG_ERROR_ON_INPUT         21   /*  Bad input stream                 */
#define MSG_UNEXPECTED_TOKEN       22   /*  Token in wrong place             */
#define MSG_INVALID_TOKEN          23   /*  Garbage in input                 */
#define MSG_BAD_EVENT_NUMBER       24   /*  Event number error        -TBI-  */
#define MSG_BAD_STATE_NAME         25   /*  State name missing ':'           */
#define MSG_INV_STATE_NAME         26   /*  Invalid state name               */
#define MSG_INV_EVENT_NUMBER       27   /*  Invalid event number             */
#define MSG_INV_EVENT_NAME         28   /*  Reserved event name              */
#define MSG_SSTATE_LOOP            29   /*  Circular superstates             */
#define MSG_SSTATE_NF              30   /*  Superstate not defined           */

/*  Messages shown by lrsort.c                                               */
#define MSG_NEXT_STATE_NF          40   /*  Next state unknown in dialog     */
#define MSG_NEXT_STATE_NFG         41   /*  Next state not allowed           */
#define MSG_DUPLICATE_STATE        42   /*  Same state defined twice         */
#define MSG_STATS                  43   /*  Dialog statistics                */
#define MSG_SYM_USED               44   /*  Symbol table % used              */

/*  Messages shown by lrcode.c                                               */
#define MSG_IDLE_MODULE           100   /*  Module is no longer used         */
#define MSG_DELETE_ERROR          101   /*  Can't delete .bak file           */
#define MSG_WRITE_ERROR           102   /*  Can't write to output stream     */
#define MSG_EXTEND_ERROR          103   /*  Can't write to extend stream     */
#define MSG_SCHEMA_FILE_NF        104   /*  Can't find schema file           */
#define MSG_SOURCE_FILE_NF        105   /*  Source file not built            */
#define MSG_OPTION_SYNTAX         106   /*  /option not valid                */
#define MSG_NAME_TOO_LONG         107   /*  /style=cobol & name too long     */
#define MSG_BUILDING_STUB         108   /*  Building stub                    */

#define MSG_INV_SCHEMA_ACTION     110   /*  Bad :action                      */
#define MSG_INV_VARIABLE          111   /*  Bad $variable                    */
#define MSG_INV_OPEN_STRING       112   /*  String token missing final "     */
#define MSG_INV_SUBSTRING         113   /*  Bad substring specification      */
#define MSG_INV_INTERNAL          114   /*  :internal name missing           */

#define MSG_OUTPUT_IGNORED        120   /*  input-output actions             */
#define MSG_FILENAME_MISSING      121   /*                                   */
#define MSG_INPUT_FILE_NF         122   /*                                   */
#define MSG_CLOSE_IGNORED         123   /*                                   */
#define MSG_TARGET_MISSING        124   /*                                   */
#define MSG_COPY_FAILED           125   /*                                   */
#define MSG_RENAME_FAILED         126   /*                                   */

#define MSG_DO_BLOCK_SYNTAX       130   /*  :do action                       */
#define MSG_DO_BLOCK_OPEN         131   /*                                   */
#define MSG_DO_BLOCK_UNDERFLOW    132   /*                                   */
#define MSG_DO_BLOCK_MISMATCH     133   /*                                   */
#define MSG_DO_BLOCK_REENTRANT    134   /*                                   */
#define MSG_DO_BLOCK_OVERLAP      135   /*                                   */
#define MSG_DO_BLOCK_RUNAWAY      136   /*                                   */

#define MSG_IF_BLOCK_OPEN         140   /*  :if action                       */
#define MSG_IF_BLOCK_UNDERFLOW    141   /*                                   */
#define MSG_IF_BLOCK_OVERLAP      142   /*                                   */

#define MSG_DECLARE_SYNTAX        150   /*  :declare action                  */
#define MSG_DECLARE_EXISTS        151   /*                                   */
#define MSG_DECLARE_INV_EXPR      152   /*                                   */
#define MSG_DECLARE_STANDARD      153   /*                                   */

#define MSG_SET_SYNTAX            160   /*  :set action                      */
#define MSG_SET_INV_VARIABLE      161   /*                                   */
#define MSG_SET_INV_EXPR          162   /*                                   */
#define MSG_SET_VAR_RDONLY        163   /*                                   */
#define MSG_SET_OVERFLOW          164   /*                                   */

#define MSG_PUSH_SYNTAX           170   /*  :push action                     */

#define MSG_POP_SYNTAX            180   /*  :pop action                      */
#define MSG_POP_INV_VARIABLE      181   /*                                   */

/*  Message codes from lrcalc.c                                              */
#define MSG_CALC_INVALID_TOKEN    900   /*    + MSG_CALC_ERROR               */
#define MSG_CALC_TOKEN_EXPECTED   901
#define MSG_CALC_NO_LEFT_PAR      902
#define MSG_CALC_NO_RIGHT_PAR     903
#define MSG_CALC_OPERAND_OVER     904
#define MSG_CALC_OPERATOR_OVER    905
#define MSG_CALC_INTERNAL_ERROR   919

/*  Message codes from lreval.c                                              */
#define MSG_EVAL_INVALID_TOKEN    920   /*    + MSG_EVAL_ERROR               */
#define MSG_EVAL_TOKEN_EXPECTED   921
#define MSG_EVAL_QUOTE_MISSING    922
#define MSG_EVAL_ACTION_EXPECTED  923
#define MSG_EVAL_EXPR_EXPECTED    924
#define MSG_EVAL_STRING_EXPECTED  925
#define MSG_EVAL_EITHER_EXPECTED  926
#define MSG_EVAL_RELATOR_EXPECTED 927
#define MSG_EVAL_INV_RELATOR      928
#define MSG_EVAL_RELATOR_IGNORED  929
#define MSG_EVAL_INTERNAL_ERROR   939

#define MSG_OUT_OF_MEMORY        1000   /*  Messages shown by lrlibr.c       */
#define MSG_SYS_SYMBOL_FULL      1001   /*  Compile-time limit               */
#define MSG_SYS_VECTORS_FULL     1002   /*  Compile-time limit               */


/*  Data types and definitions used by LIBERO programs                       */

struct _lrnode {                        /*  Node in state diagram            */
    char *name,                         /*  Item name in symbol table        */
         type;                          /*  Item type: s, e, m, or n         */
    dbyte number;                       /*  Item number                      */
    struct _lrnode
         *next,                         /*  Pointer to next sibling          */
         *child;                        /*  Pointer to first child           */
};

struct _lrstat {                        /*  Statistics about dialog          */
    int  states,                        /*    Number of states               */
         events,                        /*    Number of events               */
         modules,                       /*    Number of modules              */
         defaults,                      /*    Defaults state, if any         */
         maxaction;                     /*    Max actions for one event      */
    char **snames,                      /*    Table of state names           */
         **enames,                      /*    Table of event names           */
         **mnames,                      /*    Table of module names          */
         *source;                       /*    Name of source file            */
    Bool crlf;                          /*    File is in MS-DOS CRLF format  */
};

typedef struct _lrnode lrnode;          /*  Define data types for structs    */
typedef struct _lrstat lrstat;

/*  Functions in lrlibr.c                                                    */

Bool    ValidName           (char *name);
Bool    ReservedName        (char *name);
char   *TrueName            (char *name);
char   *CleanName           (char *name);
char   *ExternalName        (char *name);
void   *Check               (void *ptr);
dbyte   GetSymNumber        (char *name);
void    PutSymNumber        (char *name, dbyte number);
lrnode *NamedState          (lrnode *head, char *name);
lrnode *DeleteState         (lrnode *head, lrnode *state);

/*  Functions in other source files                                          */

int    lr_load_dialog       (lrnode *head, lrstat *stats, char *filename);
int    lr_sort_dialog       (lrnode *head, lrstat *stats);
int    lr_generate_code     (lrnode *head, lrstat *stats, char *filename);
int    lr_generate_skel     (lrnode *head, lrstat *stats, char *filename);
int    lr_generate_stubs    (lrnode *head, lrstat *stats, char *filename);
int    lr_free_memory       (lrnode *head, lrstat *stats);
void   lr_free_states       (lrnode *head);
void   lr_dump_dialog       (lrnode *head, lrstat *stats);
int    lr_calculate         (char *expression, long *result, int *error_posn);
int    lr_evaluate          (char *condition,  Bool *result, int *error_posn,
                             lrstat *stats);

/*  Command-line options specific to this application                        */
/*  If you change this list check $opt_xxxx definitions in lrcode.c          */

#define OPT_ANIMATE             option_list [ 0]
#define OPT_AUTHOR              option_list [ 1]
#define OPT_CHECK               option_list [ 2]
#define OPT_COMPRESS            option_list [ 3]
#define OPT_DEFAULTS            option_list [ 4]
#define OPT_DPATH               option_list [ 5]
#define OPT_GENERATE            option_list [ 6]
#define OPT_HELP                option_list [ 7]
#define OPT_IDLE                option_list [ 8]
#define OPT_INITIAL             option_list [ 9]
#define OPT_OPTION              option_list [10]
#define OPT_PATH                option_list [11]
#define OPT_PLUSD               option_list [12]
#define OPT_PRETTY              option_list [13]
#define OPT_SCHEMA              option_list [14]
#define OPT_SETTINGS            option_list [15]
#define OPT_SORT                option_list [16]
#define OPT_SOURCE              option_list [17]
#define OPT_STATS               option_list [18]
#define OPT_STUBS               option_list [19]
#define OPT_STYLE               option_list [20]
#define OPT_TRACE               option_list [21]
#define OPT_MAX                              22

#ifdef INCLUDE_OPTIONS                  /*  Should be set in lroptn.c        */
option option_list [OPT_MAX] = {
    { "animate",  '?', 0, NULL, NULL },
    { "author",   '=', 0, NULL, NULL },
    { "check",    '?', 0, NULL, NULL },
    { "compress", '?', 0, NULL, NULL },
    { "defaults", '=', 0, NULL, NULL },
    { "dpath",    '=', 0, NULL, NULL },
    { "generate", '?', 0, NULL, NULL },
    { "help",     '?', 0, NULL, NULL },
    { "idle",     '?', 0, NULL, NULL },
    { "initial",  '=', 0, NULL, NULL },
    { "option",   '+', 0, NULL, NULL },
    { "path",     '=', 0, NULL, NULL },
    { "plusd",    '?', 0, NULL, NULL },
    { "pretty",   '?', 0, NULL, NULL },
    { "schema",   '=', 0, NULL, NULL },
    { "settings", '?', 0, NULL, NULL },
    { "sort",     '?', 0, NULL, NULL },
    { "source",   '=', 0, NULL, NULL },
    { "stats",    '?', 0, NULL, NULL },
    { "stubs",    '?', 0, NULL, NULL },
    { "style",    '=', 0, NULL,
      "plain\0caps\0title\0headline\0normal\0cobol\0" },
    { "trace",    '?', 0, NULL, NULL }
};
#else
    extern option option_list [OPT_MAX];
#endif

/*  Redefine print functions under Windows                                   */
#if (defined (WINDOWS))
#   define puts         W_puts          /*  These functions are supplied     */
#   define printf       W_printf        /*  in the Windows driver code       */
#   define fprintf      W_fprintf       /*  for LR/Windows (LWPROC.C).       */
#   define exit         W_exit          /*                                   */
#   define stdout       &_iob [1]       /*  These are normally not defined   */
#   define stderr       &_iob [2]       /*  in Windows DLL programs (MSVC).  */
    int  W_puts    (const char *string);
    int  W_printf  (const char *format, ...);
    int  W_fprintf (FILE *stream, const char *format, ...);
    void W_exit    (int exit_code);
#endif

#endif                                  /*  Included                         */
