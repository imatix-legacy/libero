/*===========================================================================*
 *                                                                           *
 *  lrcode.c    Generates dialog code from dialog description + schema       *
 *                                                                           *
 *  Written:    92/11/01  Pieter Hintjens <ph@imatix.com>                    *
 *  Revised:    97/07/03                                                     *
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
#include "lrsymb.h"                     /*  Symbol table handling            */
#include "lrpriv.h"                     /*  Private definitions              */

#include "lrcoded.d"                    /*  Include dialog data              */


/*  Handling for long $row values                                            */

typedef struct ROWT_tag {
    char   symbol [LINE_MAX + 1];       /*  Contents of row part             */
    long   tally;                       /*  Number of items in row part      */
    struct ROWT_tag
           *next;                       /*  Next row part, if any            */
} ROWT;


/*  Handling for :do ... :enddo blocks                                       */

typedef struct DO_BLOCK_tag {
    long   start_pos;                   /*  Position of line after :do       */
    long   end_pos;                     /*  Position of line after :enddo    */
    function
           handler;                     /*  Function that handles action     */
    dbyte  start_line;                  /*  Number of line after :do         */
    dbyte  end_line;                    /*  Number of line after :enddo      */
    dbyte  iteration;                   /*  Number of times through loop     */
    char   srcline      [LINE_MAX + 1]; /*  :do action line before parsing   */
    char   curline      [LINE_MAX + 1]; /*  :do action line after parsing    */
    char   name_symbol  [LINE_MAX + 1]; /*  $name symbol for do_block        */
    char   comma_symbol [LINE_MAX + 1]; /*  $comma symbol for do_block       */
    long   number_symbol;               /*  $number symbol for do_block      */
    long   offset_symbol;               /*  $offset symbol for do_block      */
    ROWT   row;                         /*  $row symbol for do_block         */
    int    char_nbr;                    /*  Parse index into curline         */
    struct DO_BLOCK_tag
           *parent;                     /*  Parent do_block, if any          */
} DO_BLOCK;

static DO_BLOCK
    do_block;                           /*  Current do_block                 */

static int
    do_block_level;                     /*  Current DO level; 0 = none       */

local handler_for_do_action   (void);
local handler_for_do_event    (void);
local handler_for_do_module   (void);
local handler_for_do_nextst   (void);
local handler_for_do_state    (void);
local handler_for_do_vector   (void);
local handler_for_do_overflow (void);
local handler_for_do_while    (void);
local handler_for_do_stubs    (void);

struct {
    char    *name;                      /*  Name of do action                */
    function handler;                   /*  Function that handles do action  */
    } do_handler [] = {
        { "action",   handler_for_do_action  },
        { "event",    handler_for_do_event   },
        { "module",   handler_for_do_module  },
        { "nextst",   handler_for_do_nextst  },
        { "state",    handler_for_do_state   },
        { "vector",   handler_for_do_vector  },
        { "overflow", handler_for_do_overflow},
        { "while",    handler_for_do_while   },
        { "stubs",    handler_for_do_stubs   },
        { NULL, 0 }
    };


/*  Handling for :if ... :endif blocks                                       */

typedef struct IF_BLOCK_tag {
    long   start_pos;                   /*  Position of line after :if       */
    long   else_pos;                    /*  Position of line after :else     */
    long   end_pos;                     /*  Position of line after :endif    */
    dbyte  start_line;                  /*  Number of line after :if         */
    dbyte  else_line;                   /*  Number of line after :else       */
    dbyte  end_line;                    /*  Number of line after :endif      */
    char   srcline [LINE_MAX + 1];      /*  :if action line before parsing   */
    char   curline [LINE_MAX + 1];      /*  :if action line after parsing    */
    int    char_nbr;                    /*  Parse index into curline         */
    struct IF_BLOCK_tag
           *parent;                     /*  Parent if_block, if any          */
} IF_BLOCK;

static IF_BLOCK
    if_block;                           /*  Current if_block                 */

static int
    if_block_level;                     /*  Current IF level; 0 = none       */


/*  Character classification tables and macros                               */

static byte
    cmap [256];                         /*  Character classification tables  */

#define CMAP_VALID       1              /*  Normal name token                */
#define CMAP_QUOTE       2              /*  Possible string delimiters       */
#define CMAP_FNAME       4              /*  File name string                 */
                                        /*  Macros for character mapping:    */
#define is_valid(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_VALID) > 0)
#define is_quote(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_QUOTE) > 0)
#define is_fname(ch)     ((byte) (cmap [(byte) (ch)] & CMAP_FNAME) > 0)


/*  Global variables used in this source file only                           */

static int
    feedback,                           /*  Feedback to calling program      */
    array_base,                         /*  Current value of array_base      */
    vectorcnt,                          /*  Number of action vectors         */
    name_case,                          /*  Case upper/lower of last token   */
    char_nbr,                           /*  Current read position in line    */
    schema_format;                      /*  Schema language format if known  */

static size_t
    substr_from,                        /*  Substring specification          */
    substr_size;                        /*    from :substr action            */

static dbyte
    line_nbr,                           /*  Input line nbr from schema file  */
    *transit,                           /*  Points to event list for state   */
    *vectors,                           /*  Points to list of action vectors */
    *actions;                           /*  Current action vector            */

static char
    *filename,                          /*  Name of dialog file              */
    curline     [LINE_MAX + 1],         /*  Current line from schema         */
    srcline     [LINE_MAX + 1],         /*  Schema line before processing    */
    token       [LINE_MAX + 1],         /*  Token from current line          */
    local_file  [LR_FILENAMEMAX + 1],   /*  Name of schema, without path     */
    dialog_name [LR_FILENAMEMAX + 1],   /*  Name of dialog file              */
    source_name [LR_FILENAMEMAX + 1];   /*  Name of source file              */

static lrnode
    *listhead,                          /*  Head of dialog list structure    */
    *do_loop_state,                     /*  Current state in :do state       */
    *do_loop_event,                     /*  Current event in :do event       */
    *do_loop_module;                    /*  Current module in :do event      */

static lrstat
    *stats;                             /*  Pointer to stats block           */

static FILE
    *schema,                            /*  Input file stream                */
    *output_stream = NULL;              /*  Generated code output file       */

static ROWT
    *row_overflow;                      /*  First overflow row, if any       */

static sym_tab
    *var_tab,                           /*  User-defined variables go here   */
    *int_tab;                           /*  Internal module names            */

static symbol
    *std_array_base,                    /*  Base for generated tables        */
    *std_author,                        /*  Value of -author option          */
    *std_cob_comment,                   /*  Comment text for COBOL output    */
    *std_comma_before,                  /*  Value for $comma if not last     */
    *std_comma_last,                    /*  Value for $comma when last       */
    *std_comment_ind,                   /*  Comment indicator in schema      */
    *std_defaults,                      /*  Defaults state number            */
    *std_included,                      /*  Number of lines :included        */
    *std_module_line,                   /*  Format of module header line     */
    *std_module_name,                   /*  Original module name             */
    *std_next_state,                    /*  Name of next state               */
    *std_number_fmt,                    /*  Format for $number field         */
    *std_out_count,                     /*  Output line number               */
    *std_overflow,                      /*  Overflow rows, 0=no 1=yes        */
    *std_row_after,                     /*  Format of further items in $row  */
    *std_row_clean,                     /*  Clean-up overflow rows, 1=yes    */
    *std_row_first,                     /*  Format of first item in $row     */
    *std_row_null,                      /*  Value for 'hole' in $row         */
    *std_row_width,                     /*  Max. space for $row in source    */
    *std_run_away,                      /*  Do loop run_away counter         */
    *std_schema,                        /*  Name of schema file              */
    *std_stub_before,                   /*  Line before stub header          */
    *std_stub_between,                  /*  For body of stub header          */
    *std_stub_first,                    /*  At start of stub header          */
    *std_stub_last,                     /*  At end of stub header            */
    *std_stub_width,                    /*  Width of stub line               */
    *std_style,                         /*  Current style setting            */
    *std_vectors;                       /*  Number of action vectors         */

/*  Function prototypes                                                      */

static void    init_standard_values    (void);
static void    refresh_symbol_pointers (void);
static void    init_do_block_values    (DO_BLOCK *do_block);
static void    init_do_block_symbols   (void);
static void    init_if_block_values    (IF_BLOCK *do_block);
static void    init_charmaps           (void);
static void    build_charmap           (byte flag, char *chars);
static void    init_vectors            (void);
static event_t parse_schema_line       (void);
static void    preformat_line_if_reqd  (char *curline);
static void    syntax_error            (int exception, int msgid, ...);
static void    remove_comment          (char *line);
static char   *get_next_token          (int type, int case_convert);
static void    get_number_token        (void);
static void    get_name_token          (int case_convert);
static void    get_string_token        (void);
static void    get_filename_token      (void);
static void    skip_spaces             (void);
static void    expand_symbols          (char *line);
static void    recalc_standard_values  (void);
static int     extract_token           (char *line, int case_convert);
static void    fix_quoted_strings      (char *line, char from, char to);
static int     lookup_do_handler       (void);
static void    insert_string_symbol    (char *line, char *symbol);
static void    insert_numeric_symbol   (char *line, long int symbol);
static void    insert_variable_value   (char *line, char *name);
static void    get_action_filename     (char *action);
static int     eval_condition          (char *line);
static void    store_var_value         (symbol *var_sym, char *var_value);
static void    check_option_value      (symbol *var_sym);
static void    check_name_length       (char *name, char *type);
static FILE   *open_extend             (char *filename);
static void    write_output_line       (char *line);
static void    clear_row_parts         (void);
static char   *row_format              (int row_index);
static int     row_field               (int row_value);
static ROWT   *cat_token_to_row        (ROWT *row, char *token);
static void    build_stub_header       (char *name);
static void    prtitle                 (int width);
static int     sym_dump                (symbol *sp,...);


/*  Constants                                                                */

#define NULL_POS          -1L           /*  Null position for file pointers  */

#define WANT_ANY            0           /*  Options for get_next_token       */
#define WANT_STRING         1           /*                                   */
#define WANT_FILENAME       2           /*                                   */
#define WANT_NAME           3           /*                                   */

#define CV_NO               0           /*  Options for get_next_token       */
#define CV_LC               1           /*          and extract_token        */
#define CV_UC               2
                                        /*  Values for schema_format         */
#define LANG_NONE           0           /*    not defined yet                */
#define LANG_ANY            1           /*    schema is 0..255 free text     */
#define LANG_COBOL          2           /*    schema is 80-col COBOL         */

#define COBOL_WIDTH         80          /*  Width of COBOL source line       */
#define COBOL_MARGIN_A      6           /*  6-digit line number              */
#define COBOL_COMMENT       72          /*  8-character comment              */

/*  Entry point to function                                                  */

int lr_generate_code (lrnode *p_listhead,
                      lrstat *p_stats,
                      char   *p_filename)
{
    feedback = 0;                       /*  Initialise return feedback       */
    listhead = p_listhead;              /*  Copy parameters into local space */
    stats    = p_stats;
    filename = p_filename;

#   include "lrcoded.i"                 /*  Execute dialog manager           */
}


/*************************   INITIALISE THE PROGRAM   ************************/

MODULE initialise_the_program (void)
{
    if (OPT_GENERATE.flags == 0)        /*  If -nogen specified, exit        */
        the_next_event = finished_event;
    else
      {                                 /*  Initialise global variables:     */
        line_nbr       = 0;             /*    nothing read from file yet     */
        do_block_level = 0;             /*    no :do blocks yet              */
        if_block_level = 0;             /*    no :if blocks yet              */
        schema_format  = LANG_NONE;     /*    nothing read yet               */
        substr_from    = 0;             /*  No substring specification       */
        substr_size    = 0;
        row_overflow   = NULL;          /*  No row symbol parts yet          */
        do_loop_state  = NULL;          /*  Current state in :do state       */
        do_loop_event  = NULL;          /*  Current event in :do event       */
        do_loop_module = NULL;          /*  Current module in :do event      */
        StripExtension (strcpy (dialog_name, filename));
        StripExtension (strcpy (source_name, stats-> source));
#if (NAMEFOLD == TRUE)                  /*  If names not case sensitive,     */
        StrLwr (dialog_name);           /*    put into lower-case            */
        StrLwr (source_name);
#endif

        the_next_event = ok_event;
      }
}


/**************************   ALLOC SYMBOL TABLE   ***************************/

MODULE alloc_symbol_table (void)
{
    int_tab = sym_create_table ();      /*  Create module table              */
    var_tab = sym_create_table ();      /*  Create symbol table              */
    init_standard_values ();            /*    and load standard variables    */

    /*  Allocate memory blocks                                               */
    transit = (dbyte *) Check (malloc (LR_EVENTMAX * sizeof (dbyte)));
    actions = (dbyte *) Check (malloc (sizeof (dbyte)
                                       * (stats-> maxaction + 1)));
    vectors = (dbyte *) Check (malloc (LR_VECTORMAX * sizeof (dbyte)
                                       * (stats-> maxaction + 1)));
    init_do_block_values (&do_block);
    init_if_block_values (&if_block);
    init_charmaps ();                   /*  Initialise character maps        */
    init_vectors ();                    /*  Calculate vectors                */
}


/*  Initialise default standard values                                       */

static void
init_standard_values (void)
{
    symbol
        *var_sym;

    /*  Create and initialise all standard variables                         */
    /*   - these are read-only                                               */

    var_sym = sym_create (var_tab, "author", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (OPT_AUTHOR.value);
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "defaults", SYM_TYPE_INT);
    var_sym-> ivalue = stats-> defaults;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "date", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (DtoS (DateNow (), "YY/MM/DD", token));
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "dialog", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (dialog_name);
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "events", SYM_TYPE_INT);
    var_sym-> ivalue = stats-> events;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "fulldate", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (DtoS (DateNow (), "DD MMM, CCYY", token));
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "fulltime", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (TtoS (TimeNow (), "HH:MM:SS", token));
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "included", SYM_TYPE_INT);
    var_sym-> ivalue = 0;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "maxaction", SYM_TYPE_INT);
    var_sym-> ivalue = stats-> maxaction;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "module_name", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup("");
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "modules", SYM_TYPE_INT);
    var_sym-> ivalue = stats-> modules;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "next_state", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("");
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "out_count", SYM_TYPE_INT);
    var_sym-> ivalue = 0;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "overflow", SYM_TYPE_INT);
    var_sym-> ivalue = 0;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "schema", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (OPT_SCHEMA.value);
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "source", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (source_name);
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "states", SYM_TYPE_INT);
    var_sym-> ivalue = stats-> states;
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "style", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (OPT_STYLE.value);
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "time", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (TtoS (TimeNow (), "HH:MM", token));
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "vectors", SYM_TYPE_INT);
    var_sym-> ivalue = 0;               /*  Not yet known                    */
    var_sym-> flags = SYM_FLAG_RDONLY;

    var_sym = sym_create (var_tab, "version", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (LIBERO_VERSION);
    var_sym-> flags = SYM_FLAG_RDONLY;

    /*   - these are read-write                                              */

    var_sym = sym_create (var_tab, "array_base", SYM_TYPE_INT);
    var_sym-> ivalue = 0;

    var_sym = sym_create (var_tab, "cob_comment", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (DtoS (DateNow (), "YY/MM/DD", token));

    var_sym = sym_create (var_tab, "comma_before", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (",");

    var_sym = sym_create (var_tab, "comma_last", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("");

    var_sym = sym_create (var_tab, "comment_ind", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("#");

    var_sym = sym_create (var_tab, "module_line", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("MODULE %s (void)");

    var_sym = sym_create (var_tab, "number_fmt", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("%ld");

    var_sym = sym_create (var_tab, "row_after", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup (",%ld");

    var_sym = sym_create (var_tab, "row_clean", SYM_TYPE_INT);
    var_sym-> ivalue = 0;

    var_sym = sym_create (var_tab, "row_first", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("%ld");

    var_sym = sym_create (var_tab, "row_null", SYM_TYPE_INT);
    var_sym-> ivalue = 0;

    var_sym = sym_create (var_tab, "row_width", SYM_TYPE_INT);
    var_sym-> ivalue = LR_ROW_WIDTH;

    var_sym = sym_create (var_tab, "run_away", SYM_TYPE_INT);
    var_sym-> ivalue = LR_RUNAWAY_LIMIT;

    var_sym = sym_create (var_tab, "stub_before", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("");

    var_sym = sym_create (var_tab, "stub_between", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("*");

    var_sym = sym_create (var_tab, "stub_first", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("/");

    var_sym = sym_create (var_tab, "stub_last", SYM_TYPE_STRING);
    var_sym-> svalue = StrDup ("/");

    var_sym = sym_create (var_tab, "stub_width", SYM_TYPE_INT);
    var_sym-> ivalue = LR_HEADER_WIDTH;

    refresh_symbol_pointers ();
}

/*  -------------------------------------------------------------------------
 *  refresh_symbol_pointers
 *
 *  Updates local pointers std_xxx to address current instance of each
 *  variable used in this source.  Must be called when symbols are created
 *  and after a :pop or :push.
 */

static void
refresh_symbol_pointers (void)
{
    std_array_base   = sym_lookup (var_tab, "array_base");
    std_author       = sym_lookup (var_tab, "author");
    std_cob_comment  = sym_lookup (var_tab, "cob_comment");
    std_comma_before = sym_lookup (var_tab, "comma_before");
    std_comma_last   = sym_lookup (var_tab, "comma_last");
    std_comment_ind  = sym_lookup (var_tab, "comment_ind");
    std_defaults     = sym_lookup (var_tab, "defaults");
    std_included     = sym_lookup (var_tab, "included");
    std_module_line  = sym_lookup (var_tab, "module_line");
    std_module_name  = sym_lookup (var_tab, "module_name");
    std_next_state   = sym_lookup (var_tab, "next_state");
    std_number_fmt   = sym_lookup (var_tab, "number_fmt");
    std_out_count    = sym_lookup (var_tab, "out_count");
    std_overflow     = sym_lookup (var_tab, "overflow");
    std_row_after    = sym_lookup (var_tab, "row_after");
    std_row_clean    = sym_lookup (var_tab, "row_clean");
    std_row_first    = sym_lookup (var_tab, "row_first");
    std_row_null     = sym_lookup (var_tab, "row_null");
    std_row_width    = sym_lookup (var_tab, "row_width");
    std_run_away     = sym_lookup (var_tab, "run_away");
    std_schema       = sym_lookup (var_tab, "schema");
    std_stub_before  = sym_lookup (var_tab, "stub_before");
    std_stub_between = sym_lookup (var_tab, "stub_between");
    std_stub_first   = sym_lookup (var_tab, "stub_first");
    std_stub_last    = sym_lookup (var_tab, "stub_last");
    std_stub_width   = sym_lookup (var_tab, "stub_width");
    std_style        = sym_lookup (var_tab, "style");
    std_vectors      = sym_lookup (var_tab, "vectors");
}


static void
init_do_block_values (DO_BLOCK *block)
{
     block-> parent           = NULL;   /*  Initialise all values to zero    */
     block-> start_pos        = NULL_POS;
     block-> end_pos          = NULL_POS;
     block-> start_line       = 0;
     block-> end_line         = 0;
     block-> iteration        = 0;
     block-> number_symbol    = 0;
     block-> offset_symbol    = 0;
     block-> char_nbr         = 0;
     block-> row.tally        = 0;
     block-> row.symbol   [0] = 0;      /*  And all strings to empty         */
     block-> curline      [0] = 0;
     block-> name_symbol  [0] = 0;
     block-> comma_symbol [0] = 0;
}

static void
init_if_block_values (IF_BLOCK *block)
{
     block-> parent        = NULL;      /*  Initialise all values to zero    */
     block-> start_pos     = NULL_POS;
     block-> else_pos      = NULL_POS;
     block-> end_pos       = NULL_POS;
     block-> start_line    = 0;
     block-> else_line     = 0;
     block-> end_line      = 0;
     block-> char_nbr      = 0;
    *block-> curline       = 0;         /*  And all strings to empty         */
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
    build_charmap (CMAP_VALID, "_\\/:[]$%-.<>");
    build_charmap (CMAP_FNAME, "abcdefghijklmnopqrstuvwxyz");
    build_charmap (CMAP_FNAME, "ABCDEFGHIJKLMNOPQRSTUVWXYZ");
    build_charmap (CMAP_FNAME, "0123456789");
    build_charmap (CMAP_FNAME, "_\\/:[]$%-.<>");
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

/*  -------------------------------------------------------------------------
 *  init_vectors
 *
 *  Builds the internal tables used by :do nextst and :do action.
 */

static void
init_vectors (void)
{
    lrnode *state,                      /*  Pointer to state in dialog list  */
           *event,                      /*  Pointer to event                 */
           *nextstate,                  /*  Pointer to next state node       */
           *module;
    int     modnbr,
            vecnbr,
            vecsize;                    /*  Size of vector in bytes          */
    dbyte  *nextvec;                    /*  Pointer to next vector in table  */

    vectorcnt = 0;                      /*  Have no vectors so far           */
    vecsize = (stats-> maxaction + 1) * sizeof (dbyte);

    /*  Scan through each state in the dialog                                */
    for (state = listhead-> child; state; state = state-> next)
      {
        memset ((void *) transit, 0, sizeof (dbyte) * LR_EVENTMAX);

        /*  Scan through each event in the state                             */
        for (event = state-> child; event; event = event-> next)
          {
            memset ((void *) actions, 0, vecsize);
            nextstate = event-> child;
            modnbr = 0;

            /*  Scan through each module in the event                        */
            for (module = nextstate-> next; module; module = module-> next)
                actions [modnbr++] = module-> number;

            actions [modnbr] = 0xFFFFUL;
            nextvec = vectors + stats-> maxaction + 1;
            for (vecnbr = 0; vecnbr < vectorcnt; vecnbr++)
              {
                if (memcmp ((void *) actions, (void *) nextvec, vecsize) == 0)
                    break;
                nextvec += stats-> maxaction + 1;
              }
            if (vecnbr == vectorcnt)
              {
                if (++vectorcnt > LR_VECTORMAX)
                  {
                    PrintMessage (MSG_SYS_VECTORS_FULL);
                    raise_exception (error_event);
                    return;
                  }
                memcpy ((void *) nextvec, (void *) actions, vecsize);
              }
            transit [event-> number] = (dbyte) (vecnbr + array_base);
          }
      }
    std_vectors-> ivalue = vectorcnt;
}



/***************************   OPEN SCHEMA FILE   ****************************/

MODULE open_schema_file (void)
{
    char *fullname;

    fullname = FileWhere ('r', OPT_PATH.value, OPT_SCHEMA.value, NULL);
    if (fullname == NULL)
      {
        PrintMessage (MSG_SCHEMA_FILE_NF, OPT_SCHEMA.value);
        raise_exception (error_event);
      }
    else
      {
        schema = FileOpen (fullname, 'r');
        strcpy (local_file, fullname);
        StripFilePath (local_file);
      }
}


/*************************   GET NEXT SCHEMA LINE   **************************/

MODULE get_next_schema_line (void)
{
    the_next_event = parse_schema_line ();
}


/*  -------------------------------------------------------------------------
 *  parse_schema_line
 *
 *  Reads from schema into curline, increments line_nbr if not finished.
 *  Returns event value indicating type of line:
 *
 *      finished_event      - end-of-file; nothing more to process
 *      comment_event       - comment line; starting with !, or ':' alone
 *      source_line_event   - line should be copied to output_stream
 *      error_event         - syntax error; exception is raised
 *      <other>             - from action_lookup table
 *
 *  When source_format is LANG_COBOL, first six and last 8 columns (if any)
 *  are removed from curline.
 *
 *  Positions char_nbr to the start of a source line, and after the action
 *  name if the line starts with ':'.  When curline [char_nbr] == 0, there
 *  is no more data in the line.  The original line is saved in srcline.
 *
 *  In an action line contains the character '#', the line is truncated at
 *  that point ('#' can be changed to any string by :set comment_ind=)
 */

static event_t
parse_schema_line (void)
{
    typedef struct {                    /*  Lookup table structure:          */
        char   *name;                   /*    Schema action name             */
        event_t event;                  /*    Corresponding event            */
    } ACTION_LOOKUP;

    static ACTION_LOOKUP
        lookup [] = {                   /*  Table of schema :actions         */
            { "close",    close_event    },
            { "copy",     copy_event     },
            { "declare",  declare_event  },
            { "delete",   delete_event   },
            { "do",       do_event       },
            { "echo",     echo_event     },
            { "enddo",    enddo_event    },
            { "endif",    endif_event    },
            { "else",     else_event     },
            { "exit",     exit_event     },
            { "extend",   extend_event   },
            { "if",       if_event       },
            { "include",  include_event  },
            { "internal", internal_event },
            { "output",   output_event   },
            { "option",   option_event   },
            { "pop",      pop_event      },
            { "push",     push_event     },
            { "rename",   rename_event   },
            { "set",      set_event      },
            { "substr",   substr_event   },
            {  NULL,      error_event    },
        };
    ACTION_LOOKUP
        *lptr;                          /*  Index into lookup table          */
    event_t
        line_event;                     /*  Returned event for line          */

    if (FileRead (schema, curline) == FALSE)
        line_event = finished_event;
    else
      {
        line_nbr++;
        char_nbr = 0;                   /*  Start at beginning of line       */
        preformat_line_if_reqd (curline);
        strcpy (srcline, curline);      /*  Save original line in srcline    */

        if (curline [0] == '!')
            line_event = comment_event;
        else
        if (curline [0] != ':')
            line_event = source_line_event;
        else
          {
            char_nbr++;                 /*  Skip over colon                  */
            if (strstr (curline, std_comment_ind-> svalue))
                remove_comment (curline);   /*  Kill comment, if any         */

            get_next_token (WANT_NAME, CV_LC);

            /*  Find entry in table                                          */
            for (lptr = lookup; lptr-> name; lptr++)
                if (streq (lptr-> name, token))
                    break;              /*  Scan till we find match, or end  */

            line_event = lptr-> event;
            if (lptr-> name == NULL)
                if (*token == 0)        /*  ':' comes on line by itself      */
                    line_event = comment_event;
                else
                    syntax_error (error_event, MSG_INV_SCHEMA_ACTION, token);
          }
      }
    return (line_event);
}


/*  -------------------------------------------------------------------------
 *  preformat_line_if_reqd
 *
 *  Does any pre-processing specific to the current language.  If this is
 *  not defined, tries to determine the schema language format from the
 *  structure of the specified line.  Sets schema_format to:
 *
 *    LANG_COBOL  - line is COBOL_WIDTH columns wide, and characters 1 to 6
 *                  are numeric.  Schema text then starts in column 7.
 *    LANG_ANY    - line is 0 to 255 columns wide, and free-format.
 */

static void
preformat_line_if_reqd (char *line)
{
    if (schema_format == LANG_NONE)     /*  Check first time only            */
      {
        if (strlen (line) == COBOL_WIDTH
        && isdigit (line [0]) && isdigit (line [1]) && isdigit (line [2])
        && isdigit (line [3]) && isdigit (line [4]) && isdigit (line [5]))
            schema_format = LANG_COBOL;
        else
            schema_format = LANG_ANY;
      }
    if (schema_format == LANG_COBOL)
      {
        line [COBOL_COMMENT] = 0;       /*  Cut-out columns 7 to 66          */
        memmove (line, line + COBOL_MARGIN_A,
               strlen (line + COBOL_MARGIN_A) + 1);
      }
}


/*  -------------------------------------------------------------------------
 *  syntax_error
 *
 *  Displays the current schema line, in the form:  schema.c 123: xxxxxxxx
 *  Then retrieves a specified message text from the message file and
 *  formats it using the supplied arguments.  If the exception argument is
 *  not zero, also raises the specified exception event.
 */

static void
syntax_error (int exception, int msgid, ...)
{
    static char
        error_text [LINE_MAX + 1];      /*  Formatted error message          */
    static dbyte
        prev_line = 0;

    va_list argptr;                     /*  Argument list pointer            */

    StrCrop (srcline);                  /*  Remove any trailing spaces       */
    if (line_nbr != prev_line)          /*    from source line               */
      {
        fprintf (stderr, "%s %d: %s\n", local_file, line_nbr, srcline);
        prev_line = line_nbr;           /*  Only show first in series        */
      }
    va_start (argptr, msgid);           /*  Start variable arguments list    */
    vsprintf (error_text, MessageText (msgid), argptr);
    va_end (argptr);                    /*  End variable arguments list      */

    fprintf (stderr, "%s\n", error_text);
    if (exception)                      /*  Raise exception if error also    */
        raise_exception (exception);
}


/*  -------------------------------------------------------------------------
 *  remove_comment
 *
 *  A comment is any text starting with the string in std_comment_ind.
 *  If the string is inside quotes, it is ignored.  The comment ends at
 *  the end of the line.
 */

static void
remove_comment (char *line)
{
    char *scan;
    int  in_string = 0;

    for (scan = line; *scan; scan++)
      {
        if (is_quote (*scan))
            in_string = 1 - in_string;  /*  Toggle string in-out switch      */
        else
        if (*scan == std_comment_ind-> svalue [0]
        && !in_string
        && strstr (scan, std_comment_ind-> svalue) == scan)
          {
            *scan = 0;                  /*  Kill line at start of comment    */
            break;
          }
      }
}


/*  -------------------------------------------------------------------------
 *  get_next_token
 *
 *  Parses next 'token' from 'curline' at 'char_nbr'.  A token is delimited
 *  by spaces, and consists of a sequence of characters as follows:
 *
 *  string    delimited by " or '; token contains contents of string.
 *
 *  number    sequence of digits, delimited by non-digit.
 *
 *  name      upper or lowercase name, starts with letter, and contains any
 *            further mixture of letters, digits, and underscores.  Depending
 *            on 'convert' option, returned lowercase or uppercase, or as-is.
 *            May start with '$'.
 *
 *  filename  as name, but allows additional set of characters that normally
 *            end a name.  For instance, \ and / are permitted in filenames.
 *            Filenames are converted to uppercase if system requires it
 *            (see #define NAMEFOLD in lrpriv.h).
 *
 *  If type is WANT_ANY, looks for number, name, or string.  If type is
 *  WANT_STRING, looks for a string only.  If type is WANT_FILENAME, looks
 *  for a filename or string only; a string is converted implicitly into a
 *  filename (first delimiter is removed).  If type is WANT_NAME, looks for
 *  a name only.
 *
 *  Returns token.  Updates char_nbr.
 */

static char *
get_next_token (int type, int convert)
{
    char thisch;                        /*  Next char in token               */

    skip_spaces ();                     /*  Skip spaces before token         */
    thisch = curline [char_nbr];        /*  Get first character of token     */

    if (isdigit (thisch)                /*  Collect number token             */
    && (type == WANT_ANY))
        get_number_token ();
    else
    if (isalpha (thisch)                /*  Collect name token               */
    && (type == WANT_ANY
    ||  type == WANT_NAME))
        get_name_token (convert);
    else
    if (is_quote (thisch)               /*  Collect string token             */
    && (type == WANT_ANY
    ||  type == WANT_STRING
    ||  type == WANT_FILENAME))
        get_string_token ();
    else
    if (is_fname (thisch)               /*  Collect filename token           */
    && (type == WANT_FILENAME))
        get_filename_token ();
    else
    if (thisch == '$'                   /*  Collect $xxxx token              */
    && (type == WANT_NAME))
        get_name_token (convert);
    else
        *token = 0;                     /*  If nothing, return empty token   */

#   if (NAMEFOLD == TRUE)               /*  If system requires it,           */
    if (type == WANT_FILENAME)          /*    fold filename to uppercase     */
        StrUpr (token);
#   endif
    return (token);
}

static void
skip_spaces (void)
{
    while (curline [char_nbr] == ' ')
        char_nbr++;
}

static void
get_number_token (void)
{
    int  size;                          /*  Size of token                    */
    char thisch;                        /*  Next char in token               */

    size = 0;                           /*  Clear token to empty             */
    while (isdigit (thisch = curline [char_nbr]))
      {
        char_nbr++;
        token [size++] = thisch;
      }
    token [size] = 0;                   /*  Terminate token string           */
}

static void
get_name_token (int convert)
{
    int  size;                          /*  Size of token                    */
    char thisch;                        /*  Next char in token               */

    size = 0;                           /*  Clear token to empty             */
    while (is_valid (thisch = curline [char_nbr]))
      {
        char_nbr++;
        token [size++] = thisch;
      }
    token [size] = 0;                   /*  Terminate token string           */
    if (convert == CV_LC)               /*  Convert name to lower or upper   */
        StrLwr (token);                 /*    case if so wanted              */
    else
    if (convert == CV_UC)
        StrUpr (token);
}

static void
get_string_token (void)
{
    int  size;                          /*  Size of token                    */
    char thisch;                        /*  Next char in token               */
    char delim;                         /*  String delimiter                 */

    size = 0;                           /*  Clear token to empty             */
    delim = curline [char_nbr++];       /*  Save string delimiter            */
    FOREVER                             /*  Loop until string is complete    */
      {                                 /*    or end of line                 */
        thisch = curline [char_nbr++];
        if (thisch == 0)                /*  Unexpected end of line?          */
          {
            syntax_error (0, MSG_INV_OPEN_STRING, delim);
            break;
          }
        else
        if (thisch == delim)            /*  Normal end of string             */
            break;
        else
            token [size++] = thisch;
      }
    token [size] = 0;                   /*  Terminate token string           */
}

static void
get_filename_token (void)
{
    int  size;                          /*  Size of token                    */
    char thisch;                        /*  Next char in token               */

    size = 0;                           /*  Clear token to empty             */
    while (is_fname (thisch = curline [char_nbr]))
      {
        char_nbr++;
        token [size++] = thisch;
      }
    token [size] = 0;                   /*  Terminate token string           */
}


/***************************   PARSE OPTION VALUE   **************************/
/*
 *  :option -name[=value]
 */

MODULE parse_option_value (void)
{
    expand_symbols (curline);           /*  Expand symbols                   */
    lr_parse_option_line (curline + char_nbr);

    /*  Refresh variables that depend on options                             */
    sym_values (std_author, 0, OPT_AUTHOR.value);
    sym_values (std_schema, 0, OPT_SCHEMA.value);
    sym_values (std_style,  0, OPT_STYLE.value);
}


/*************************   DECLARE USER VARIABLE   *************************/
/*
 *  :declare [int | string] [$]itemname [=expression]
 */

MODULE declare_user_variable (void)
{
    symbol
        *var_sym;                       /*  Pointer into symbol table        */
    byte type;                          /*  Type of variable                 */

    ASSERT (var_tab != NULL);

    get_next_token (WANT_NAME, CV_LC);  /*  Look for int or string           */
    if (streq (token, "int"))
      {                                 /*  Get variable name                */
        get_next_token (WANT_NAME, CV_LC);
        type = SYM_TYPE_INT;
      }
    else
    if (streq (token, "string"))
      {                                 /*  Get variable name                */
        get_next_token (WANT_NAME, CV_LC);
        type = SYM_TYPE_STRING;
      }
    else
        type = SYM_TYPE_EITHER;

    if (*token == 0)                    /*  If no name, give-up              */
        syntax_error (error_event, MSG_DECLARE_SYNTAX);
    else
      {
        if (*token == '$')
            StrClose (token, TRUE);     /*  Remove '$' at in name, if any    */

        var_sym = sym_lookup (var_tab, token);
        if (var_sym)                    /*  Warn if already exists           */
            syntax_error (0, MSG_DECLARE_EXISTS, token);
        else
        if (streq (token, "comma")      /*  Error if we have one of          */
        ||  streq (token, "name")       /*    the standard :do block         */
        ||  streq (token, "number")     /*    variables                      */
        ||  streq (token, "offset")
        ||  streq (token, "tally")
        ||  streq (token, "row"))
          {
            syntax_error (error_event, MSG_DECLARE_STANDARD, token);
            return;
          }
        else                            /*    otherwise create it            */
            var_sym = sym_create (var_tab, token, 0);

        var_sym-> type = type;
        expand_symbols (curline + char_nbr);
        skip_spaces ();                 /*  Skip spaces before '='           */
        if (curline [char_nbr] == '=')  /*    if found, go store value       */
            store_var_value (var_sym, curline + char_nbr + 1);
        else
          {                             /*  If no initialiser, clear values  */
            sym_values (var_sym, 0, NULL);
            if (*(curline + char_nbr))  /*    and if not eoln, complain      */
                syntax_error (0, MSG_DECLARE_INV_EXPR, curline);
          }
        check_option_value (var_sym);   /*  See if command-line value set    */
        recalc_standard_values ();
      }
}


/*  ------------------------------------------------------------------------
 *  expand_symbols
 *
 *  Searches the specified string for $symbols, and replaces any it finds
 *  by the appropriate values.  Follows these rules:
 *
 *      $$ is treated as a single $
 *      $xxxx is expanded, where 'xxxx' is converted to lower case
 *      $"xxxx" is replaced the TrueName for xxxx, following current -style
 *      \ at the end of $name is removed
 *
 *  Prevents floating expansion in strings by replacing all spaces in strings
 *  by 0x01 before scanning, and the opposite afterwards.
 */

static void
expand_symbols (char *line)
{
    char *scan;

    ASSERT (line != token);             /*  token is work variable           */
    if (!strchr (line, '$'))
        return;                         /*  Quick exit if no $'s found       */

    fix_quoted_strings (line, ' ', 1);
    for (scan = line; *scan; scan++)
      {
        if (*scan == '$')
          {
            StrClose (scan, TRUE);      /*  Remove '$' from line             */
            if (*scan != '$')
              {
                extract_token (scan, CV_LC);
                if (streq (token, "comma"))
                    insert_string_symbol (scan, do_block.comma_symbol);
                else
                if (streq (token, "name"))
                    insert_string_symbol (scan, do_block.name_symbol);
                else
                if (streq (token, "number"))
                    insert_numeric_symbol (scan, do_block.number_symbol);
                else
                if (streq (token, "offset"))
                    insert_numeric_symbol (scan, do_block.offset_symbol);
                else
                if (streq (token, "tally"))
                    insert_numeric_symbol (scan, do_block.row.tally);
                else
                if (streq (token, "row"))
                    insert_string_symbol (scan, do_block.row.symbol);
                else
                if (token [0] == '"')
                    insert_string_symbol (scan, TrueName (token + 1));
                else
                    insert_variable_value (scan, token);
              }
          }
      }
    fix_quoted_strings (line, 1, ' ');
    Trace ("%3d> %s", line_nbr, srcline);
    Trace (" ==> %s", curline);
}


/*  -------------------------------------------------------------------------
 *  extract_token
 *
 *  Extracts one token from the start of the specified line, and returns it
 *  in 'token'.  A token is delimited by spaces, and consists of a sequence
 *  of characters as follows:
 *
 *      "..." or '...'  - returns contents of string; token [0] is quote;
 *                        \ is an escape character:
 *
 *                          \n  - converted to newline character
 *                          \t  - converted to tab character
 *                          \ch - (any other char) converted to ch
 *
 *      A-Za-z0-9_-     - returns plain token, converted according to case;
 *                        if followed by '\', the '\' is removed & junked.
 *                        case 0=no conv; 1=tolower; 2=toupper; case of
 *                        first letter (1, 2) is left in global name_case.
 *
 *  The token is taken out of the line, and the remaining text is moved
 *  left to the start of the line.  Uses StrClose with alignment to ensure
 *  that aligned text in the schema is left untouched.
 *
 *  If no valid token is found, sets *token to 0.  Returns size of token,
 *  0 to n.
 */

static int extract_token (char *line, int convert)
{
    int size = 0;                       /*  Size of token                    */

    name_case = 0;                      /*  When CV_UC, insert_string_value  */
    if (is_quote (*line))
      {                                 /*  If quoted string,                */
        *token = *line;                 /*    start token with delimiter     */
        size++;
        StrClose (line, TRUE);
        while (*line != *token)         /*  Skip to closing quote            */
          {                             /*    or end of line                 */
            if (*line == '\\')
              {                         /*  Handle escaped character         */
                StrClose (line, TRUE);
                if (*line == 'n')
                    *line = '\n';       /*  Newline                          */
                else
                if (*line == 't')
                    *line = '\t';       /*  Tab                              */
              }
            if (*line == 0)
                break;
            token [size++] = *line == 1? ' ': *line;
            StrClose (line, TRUE);
          }
        if (*line)                      /*  Remove trailing quote if any     */
            StrClose (line, TRUE);
        else
            syntax_error (0, MSG_INV_OPEN_STRING, *token);
      }
    else                                /*  If plain token                   */
      {                                 /*    copy characters                */
        if (isupper (*line))
            name_case = CV_UC;          /*  Remember case of first char      */

        while (isalpha (*line) || isdigit (*line) || *line == '_')
          {
            if (*line == 0)
                break;
            token [size++] = (char) (convert == CV_LC? tolower (*line):
                                     convert == CV_UC? toupper (*line):
                                     /*  else  */              (*line));
            StrClose (line, TRUE);
          }
        if (*line == '\\')              /*  Remove '\' if at line of name    */
            StrClose (line, TRUE);
      }
    token [size] = 0;                   /*  Terminate token string           */
    return (size);
}

static void
fix_quoted_strings (char *line, char from, char to)
{
    char *scan;
    int  in_string = 0;

    for (scan = line; *scan; scan++)
      {
        if (*scan == '\\')
            scan++;                     /*  Ignore anything after \          */
        else
        if (is_quote (*scan))
            in_string = 1 - in_string;  /*  Toggle string in-out switch      */
        else
        if (*scan == from && in_string)
            *scan = to;                 /*  If in string, fix character      */
      }
}


static void
insert_string_symbol (char *line, char *symbol)
{
    int start;
    if (symbol)                         /*  If null, ignore                  */
      {
        start = 0;
        strcpy (token, symbol);
        if (substr_from > 1)
          {
            if (substr_from < strlen (token))
                start = substr_from - 1;
            if (substr_size)
                token [start + substr_size] = 0;
          }
        if (name_case == CV_UC)         /*  CAPITALISE if necessary          */
            StrUpr (token + start);
        while (token [start])           /*  And insert the token text        */
          {
            StrOpen (line, TRUE);
            *line++ = token [start++];
          }
      }
    substr_from = 0;                    /*  Clear substring specification    */
    substr_size = 0;
}


static void
insert_numeric_symbol (char *line, long int symbol)
{
    sprintf (token, std_number_fmt-> svalue, symbol);
    insert_string_symbol (line, token);
}


static void
insert_variable_value (char *line, char *name)
{
    symbol *var_sym;

    var_sym = sym_lookup (var_tab, name);
    if (var_sym == NULL)
      {
        syntax_error (0, MSG_INV_VARIABLE, name);
        var_sym = sym_create (var_tab, name, 0);
        var_sym-> type = SYM_TYPE_EITHER;
        sym_values (var_sym, 0, NULL);
      }
    if ((var_sym-> type == SYM_TYPE_INT)
    ||  (var_sym-> type == SYM_TYPE_EITHER && var_sym-> svalue == NULL))
        insert_numeric_symbol (line, (int) var_sym-> ivalue);
    else
        insert_string_symbol (line, var_sym-> svalue);
}


/*  ------------------------------------------------------------------------
 *  store_var_value
 *
 *  Parses the value or expression and stores the result in the variable.
 *  If the symbol is a string, stores the value without further analysis.
 *  If the symbol is an int, parses as a numeric expression and stores the
 *  result.  If the symbol is of type EITHER, tries to parse as a numeric
 *  expression - if that fails, stores as a string.
 */

static void
store_var_value (symbol *var_sym, char *value)
{
    int
        calc_feedback,                  /*  Feedback from lr_calculate ()    */
        calc_offset,                    /*  Position of any error            */
        indent;                         /*  For nice error messages          */

    ASSERT (var_sym != NULL);
    ASSERT (value != NULL);

    calc_feedback = lr_calculate (value, &var_sym-> ivalue, &calc_offset);
    if ((var_sym-> type == SYM_TYPE_STRING)
    ||  (var_sym-> type == SYM_TYPE_EITHER && calc_feedback != 0))
      {                                 /*  Parse (string) token fom value   */
        memmove (curline, value, strlen (value) + 1);
        char_nbr = 0;
        get_next_token (WANT_ANY, CV_NO);
        sym_values (var_sym, 0, token);
      }
    else
    if (var_sym-> type == SYM_TYPE_INT)
      {
        if (calc_feedback)              /*  Error in expression              */
          {
            StrCrop (srcline);
            fprintf (stderr, "%s %d: %s\n", local_file, line_nbr, srcline);
            fprintf (stderr, "lr I: %s\nlr I: ", value);
            for (indent = calc_offset; indent; indent--)
                fprintf (stderr, "*");
            fprintf (stderr, "\n%s\n", MessageText (calc_feedback));
          }
      }
}


/*  ------------------------------------------------------------------------
 *  check_option_value
 *
 *  If an /option switch was specified for the variable, uses that value
 *  instead of any declared value.  Syntax: /option:var1="value",var2=expr
 *  Because MS-DOS is fussy about quotes on the command line, accepts values
 *  for strings without quotes.
 */

static void
check_option_value (symbol *var_sym)
{
    static Bool
        options_checked = FALSE;        /*  Check options first time         */
    symbol
        *this_sym;

    if (OPT_OPTION.value)
      {
        strcpy (curline, OPT_OPTION.value);
        char_nbr = 0;                   /*  Prepare to parse option string   */
        FOREVER                         /*  Check all option values          */
          {
            while (curline [char_nbr] == ',')
                char_nbr++;
            if (*get_next_token (WANT_NAME, CV_LC) == 0)
                break;                  /*  Get var name; quit when no more  */

            this_sym = sym_lookup (var_tab, token);
            if (curline [char_nbr] == '=')
              {
                char_nbr++;             /*  Expect '=', then value           */
                get_next_token (WANT_ANY, CV_NO);
              }
            else
              {
                if (!options_checked)   /*  '=' is missing; complain         */
                  {
                    PrintMessage (MSG_OPTION_SYNTAX, token);
                    options_checked = TRUE;
                  }
                *token = 0;             /*  And assume empty token           */
              }
            if (this_sym == var_sym)    /*  Store value for this symbol      */
              {
                if (var_sym-> type == SYM_TYPE_STRING)
                    sprintf (curline, "\"%s\"", token);
                else
                    strcpy (curline, token);

                expand_symbols  (curline);
                store_var_value (var_sym, curline);
                break;                  /*  End options processing           */
              }
          }
      }
}

/*  ------------------------------------------------------------------------
 *  recalc_standard_values
 *
 *  Call this function after changing the value of any variable; it
 *  recalculates any dependent variables.
 */

static void
recalc_standard_values (void)
{
    /*  Adjust defaults state for std_array_base                             */
    std_defaults-> ivalue = stats-> defaults + (int) std_array_base-> ivalue;

    /*  Check various limits                                                 */
    if (std_row_width-> ivalue > LINE_MAX)
      {
        syntax_error (0, MSG_SET_OVERFLOW, "row_width", LINE_MAX);
        std_row_width-> ivalue = LINE_MAX;
      }
}


/***************************   OPEN OUTPUT FILE   ****************************/
/*
 *  :output filename
 */

MODULE open_output_file (void)
{
    if (output_stream)                  /*  Close current output if any      */
        FileClose (output_stream);

    get_action_filename ("output");
    if (!exception_raised)
      {
        output_stream = FileOpen (token, 'w');
        if (output_stream == NULL)
          {
            PrintMessage (MSG_WRITE_ERROR, token);
            raise_exception (error_event);
          }
        else
            /*  Reset line numbering                                         */
            std_out_count-> ivalue = std_array_base-> ivalue;
      }
}

static void
get_action_filename (char *action)
{
    expand_symbols (curline);           /*  Expand symbols in line           */
    get_next_token (WANT_FILENAME, 0);  /*  Collect filename token           */
    if (*token == 0)                    /*    and complain if missing        */
        syntax_error (error_event, MSG_FILENAME_MISSING, action);
}


/***************************   OPEN EXTEND FILE   ****************************/
/*
 *  :extend filename
 */

MODULE open_extend_file (void)
{
    if (output_stream)                  /*  Close current output if any      */
        FileClose (output_stream);

    get_action_filename ("extend");
    if (!exception_raised)
        output_stream = open_extend (token);
}


static FILE *
open_extend (char *filename)
{
    FILE
        *output;

    /*  Prepare file for extending, then open if okay                        */
    if (SafeToExtend (filename))
      {
        output = FileOpen (filename, 'a');
        if (output == NULL)
          {
            PrintMessage (MSG_WRITE_ERROR, filename);
            raise_exception (error_event);
          }
        else
            /*  Reset line numbering                                         */
            std_out_count-> ivalue = std_array_base-> ivalue;
      }
    else
      {
        PrintMessage (MSG_EXTEND_ERROR, filename);
        raise_exception (error_event);
        output = NULL;
      }
    return (output);
}


/*************************   INCLUDE SPECIFIED FILE   ************************/
/*
 *  :include [optional] filename [ starttag [endtag] ]
 */

MODULE include_specified_file (void)
{
    char
        *fullname,                      /*  Fully-specified filename         */
        *include_from,                  /*  Start at this line (optional)    */
        *include_to;                    /*  End at this line (optional)      */
    FILE
        *include_file;                  /*  File we read from                */
    Bool
        include_this;                   /*  If TRUE, want current line       */

    /*  Pick-up filename after action and find file on path                  */
    get_action_filename ("include");
    if (exception_raised)
        return;

    std_included-> ivalue = 0;          /*  Reset no. lines included         */
    fullname = FileWhere ('r', OPT_PATH.value, token, NULL);
    if (streq (StrLwr (token), "optional"))
      {
        get_next_token (WANT_FILENAME, CV_NO);
        fullname = FileWhere ('r', OPT_PATH.value, token, NULL);
        if (fullname == NULL)           /*  File was not found - forget it   */
            return;
      }

    include_from = StrDup (get_next_token (WANT_ANY, CV_NO));
    include_to   = StrDup (get_next_token (WANT_ANY, CV_NO));
    include_this = (Bool) (*include_from == 0);
    include_file = fullname? FileOpen (fullname, 'r'): NULL;

    if (include_file == NULL)
        syntax_error (0, MSG_INPUT_FILE_NF, fullname);
    else
      {
        if (output_stream == NULL)
            syntax_error (0, MSG_OUTPUT_IGNORED);
        else
          {
            while (FileRead (include_file, curline))
              {
                preformat_line_if_reqd (curline);
                StrCrop (curline);
                if (!include_this
                &&  streq (curline, include_from)
                &&                 *include_from)
                    include_this = TRUE;
                else
                if (include_this
                &&  streq (curline, include_to)
                &&                 *include_to)
                    include_this = FALSE;
                else
                if (include_this)
                  {
                    write_output_line (curline);
                    std_included-> ivalue++;
                  }
              }
          }
        FileClose (include_file);
      }
    free (include_to);
    free (include_from);
}

/*  -------------------------------------------------------------------------
 *  write_output_line
 *
 *  Writes a line of output to the output_stream.  Sets the global FileCrLf
 *  to match the dialog's crlf status.  This outputs the line with carriage-
 *  return/linefeed if the dialog was so formatted; else outputs only a
 *  linefeed.  The file must be opened using FileOpen.
 */

static void
write_output_line (char *line)
{
    static char
        output_line [LINE_MAX + 1];

    FileCrLf = stats-> crlf;
    if (schema_format != LANG_COBOL)
        FileWrite (output_stream, line);
    else
      {
        sprintf (output_line, "%04ld00", std_out_count-> ivalue);
        strcat  (output_line, line);
        StrPad  (output_line, ' ', COBOL_COMMENT);
        strcat  (output_line, std_cob_comment-> svalue);
        FileWrite (output_stream, output_line);
      }
    std_out_count-> ivalue++;           /*  Bump line number                 */
}


/**************************   COPY SPECIFIED FILE   **************************/
/*
 *  :copy sourcefile targetfile
 */

MODULE copy_specified_file (void)
{
    char
        *source_name = NULL,            /*  Start at this line (optional)    */
        *target_name = NULL;            /*  End at this line (optional)      */

    get_action_filename ("copy");       /*  Get source file name             */
    if (!exception_raised)
      {                                 /*  Get target name if no errors     */
        source_name = StrDup (token);
        get_next_token (WANT_FILENAME, CV_NO);
        target_name = StrDup (token);
        if (*token == 0)
            syntax_error (error_event, MSG_TARGET_MISSING, "copy");
      }
    if (!exception_raised)              /*  Do copy if no errors             */
      {
        if (!FileExists (source_name))
            syntax_error (error_event, MSG_INPUT_FILE_NF, source_name);
        else
          {
            remove (target_name);       /*  Ensure target is overwritten     */
            if (FileCopy (target_name, source_name, 't') == -1)
                syntax_error (error_event, MSG_COPY_FAILED, target_name);
          }
      }
    if (source_name)
        free (source_name);
    if (target_name)
        free (target_name);
}


/*************************   RENAME SPECIFIED FILE   *************************/
/*
 *  :rename oldname newname
 */

MODULE rename_specified_file (void)
{
    char
        *old_name = NULL,               /*  Start at this line (optional)    */
        *new_name = NULL;               /*  End at this line (optional)      */

    get_action_filename ("rename");     /*  Get old file name                */
    if (!exception_raised)
      {                                 /*  Get new name if no errors        */
        old_name = StrDup (token);
        get_next_token (WANT_FILENAME, CV_NO);
        new_name = StrDup (token);
        if (*token == 0)
            syntax_error (error_event, MSG_TARGET_MISSING, "rename");
      }
    if (!exception_raised)              /*  Do rename if no errors           */
        if (rename (old_name, new_name) == -1)
            syntax_error (error_event, MSG_RENAME_FAILED, old_name);

    if (old_name)
        free (old_name);
    if (new_name)
        free (new_name);
}


/*************************   DELETE SPECIFIED FILE   *************************/
/*
 *  :delete filename
 */

MODULE delete_specified_file (void)
{
    get_action_filename ("delete");
    if (!exception_raised)
        remove (token);
}


/**************************   CHECK OUTPUT OPENED   **************************/

MODULE check_output_opened (void)
{
    if (!output_stream)
        syntax_error (0, MSG_CLOSE_IGNORED);
}


/***************************   CLOSE OUTPUT FILE   ***************************/
/*
 *  :close
 */

MODULE close_output_file (void)
{
    if (output_stream)                  /*  Close current output if any      */
      {
        FileClose (output_stream);
        output_stream = NULL;           /*    and clear output stream        */
      }
}


/*************************   OUTPUT LINE OF SOURCE   *************************/

MODULE output_line_of_source (void)
{
    static dbyte
        last_error_line = 0;            /*  Filter serial messages           */

    if (output_stream)                  /*  Output if possible               */
      {
        expand_symbols    (curline);
        write_output_line (curline);
      }
    else                                /*  Else show terse warning          */
      {
        StrCrop (curline);              /*  Crop trailing spaces             */
        if (strused (curline))          /*    and check if anything's left   */
          {
            if (line_nbr > (dbyte) (last_error_line + 1))
                syntax_error (0, MSG_OUTPUT_IGNORED);
            last_error_line = line_nbr;
          }
      }
}


/************************   WRITE MESSAGE TO STDOUT   ************************/
/*
 *  :echo "message"
 */

MODULE write_message_to_stdout (void)
{
    expand_symbols (curline);           /*  Expand spaces in line            */
    skip_spaces ();                     /*  Skip spaces before token         */

    if (is_quote (*(curline + char_nbr)))
        get_next_token (WANT_STRING, CV_NO);
    else
        strcpy (token, curline + char_nbr);

    fprintf (stdout, "%s\n", token);    /*  Print line to console            */
}


/***********************   PARSE INTERNAL MODULE NAME   **********************/
/*
 *  :internal name_of_module
 */

MODULE parse_internal_module_name (void)
{
    ASSERT (int_tab != NULL);

    expand_symbols (curline);           /*  Expand symbols in line           */
    get_next_token (WANT_ANY, CV_LC);   /*  Get name of module               */
    if (*token)
        sym_intern (int_tab, token, SYM_TYPE_STRING);
    else
        syntax_error (error_event, MSG_INV_INTERNAL);
}


/*************************   ASSIGN VARIABLE VALUE   *************************/
/*
 *  :set [$]name = expression
 */

MODULE assign_variable_value (void)
{
    symbol
        *var_sym;                       /*  Pointer into symbol table        */

    ASSERT (var_tab != NULL);

    get_next_token (WANT_NAME, CV_LC);  /*  Get name of variable             */
    if (*token == 0)                    /*  If no name, give-up              */
        syntax_error (error_event, MSG_SET_SYNTAX);
    else
      {
        if (*token == '$')
            StrClose (token, TRUE);     /*  Remove '$' at in name, if any    */

        var_sym = sym_lookup (var_tab, token);
        if (!var_sym)                   /*  Warn if not defined              */
          {
            syntax_error (0, MSG_SET_INV_VARIABLE, token);
            var_sym = sym_create (var_tab, token, 0);
            var_sym-> type = SYM_TYPE_EITHER;
          }
        if (var_sym-> flags & SYM_FLAG_RDONLY)
            syntax_error (0, MSG_SET_VAR_RDONLY, token);
        else
          {
            expand_symbols (curline + char_nbr);
            skip_spaces ();             /*  Skip spaces before '='           */
            if (curline [char_nbr] == '=')
                store_var_value (var_sym, curline + char_nbr + 1);
            else
              {                         /*  If not found, complain           */
                syntax_error (0, MSG_SET_INV_EXPR);
                sym_values (var_sym, 0, NULL);
              }
          }
        recalc_standard_values ();
      }
}


/**************************   PUSH VARIABLE VALUE   **************************/
/*
 *  :push [$]name [= expression]
 */

MODULE push_variable_value (void)
{
    symbol
        *old_sym,                       /*  Old symbol                       */
        *var_sym;                       /*  We define a new symbol           */

    ASSERT (var_tab != NULL);

    get_next_token (WANT_NAME, CV_LC);  /*  Get name of variable             */
    if (*token == 0)                    /*  If no name, give-up              */
        syntax_error (error_event, MSG_PUSH_SYNTAX);
    else
      {
        if (*token == '$')
            StrClose (token, TRUE);     /*  Remove '$' at in name, if any    */

        old_sym = sym_lookup (var_tab, token);
        var_sym = sym_create (var_tab, token, 0);
        if (!old_sym)                   /*  Warn if not defined              */
          {
            syntax_error (0, MSG_SET_INV_VARIABLE, token);
            var_sym-> type = SYM_TYPE_EITHER;
          }
        else
          {
            var_sym-> type  = old_sym-> type;
            var_sym-> flags = old_sym-> flags;
          }
        expand_symbols (curline + char_nbr);
        skip_spaces ();                 /*  Skip spaces before '='           */
        if (curline [char_nbr] == '=')
          {
            if (var_sym-> flags & SYM_FLAG_RDONLY)
                syntax_error (0, MSG_SET_VAR_RDONLY, token);
            else
                store_var_value (var_sym, curline + char_nbr + 1);
          }
        else
            sym_values (var_sym, 0, NULL);

        refresh_symbol_pointers ();
        recalc_standard_values ();
      }
}


/***************************   POP VARIABLE VALUE   **************************/
/*
 *  :pop [$]name
 */

MODULE pop_variable_value (void)
{
    symbol
        *var_sym;                       /*  Pointer into symbol table        */

    ASSERT (var_tab != NULL);

    get_next_token (WANT_NAME, CV_LC);  /*  Get name of variable             */
    if (*token == 0)                    /*  If no name, give-up              */
        syntax_error (error_event, MSG_POP_SYNTAX);
    else
      {
        if (*token == '$')
            StrClose (token, TRUE);     /*  Remove '$' at in name, if any    */

        var_sym = sym_lookup (var_tab, token);
        if (!var_sym)                   /*  Error if not defined             */
            syntax_error (0, MSG_POP_INV_VARIABLE, token);
        else
          {
            var_sym = sym_delete (var_tab, var_sym);
            refresh_symbol_pointers ();
            recalc_standard_values ();
          }
      }
}


/*********************   PARSE SUBSTRING SPECIFICATION   *********************/
/*
 *  :substr from [size]
 */

MODULE parse_substring_specification (void)
{
    get_next_token (WANT_NAME, CV_NO);
    substr_from = atoi (token);
    get_next_token (WANT_NAME, CV_NO);
    substr_size = atoi (token);

    if (substr_from               > LINE_MAX
    ||  substr_from + substr_size > LINE_MAX)
      {
        syntax_error (0, MSG_INV_SUBSTRING);
        substr_from = substr_size = 0;
      }
}


/*****************************   PARSE DO BLOCK   ****************************/
/*
 *  :do actionname [arguments]
 */

MODULE parse_do_block (void)
{
    int do_level = 1;                   /*  Indentation level of do's        */
    int if_level = 0;                   /*  Embedded if's must be closed     */
    int scan_ptr;                       /*  Pointer into do_handler table    */

    /*  :do has already been removed from line; parse rest to find handler   */
    get_next_token (WANT_NAME, CV_LC);
    scan_ptr = lookup_do_handler ();
    if (exception_raised)
        return;                         /*  Quit if there was a problem      */

    strcpy (do_block.srcline, srcline);
    strcpy (do_block.curline, curline);
    do_block.start_pos  = ftell (schema);
    do_block.start_line = line_nbr;
    do_block.handler    = do_handler [scan_ptr].handler;
    do_block.iteration  = 0;
    do_block.char_nbr   = char_nbr;

    get_next_token (WANT_NAME, CV_LC);

    DisableTrace ();                    /*  Don't trace look-ahead           */
    while (do_level > 0)
      {
        switch (parse_schema_line ())
          {
            case finished_event:
                line_nbr = do_block.start_line;
                strcpy (srcline, do_block.srcline);
                syntax_error (error_event, MSG_DO_BLOCK_OPEN);
                return;
            case do_event:
                do_level++;
                break;
            case enddo_event:
                if (do_level == 1)
                  {
                    do_block.end_pos  = ftell (schema);
                    do_block.end_line = line_nbr;
                  }
                do_level--;
                break;
            case if_event:
                if_level++;
                break;
            case endif_event:
                get_next_token (WANT_NAME, CV_LC);
                if (streq (token, "all"))
                    if_level = 1;       /*  :endif all                       */

                if_level--;             /*  :endif                           */
                break;
          }
      }
    if (TraceFile)
        EnableTrace ();

    /*  Check for mismatched :if / :endif within do loop                     */
    if (if_level > 0)
        syntax_error (error_event, MSG_DO_BLOCK_OVERLAP);

    /*  If anything follows :enddo, it must match the :do                    */
    if (!exception_raised)              /*  If we found :enddo,              */
      {
        get_next_token (WANT_NAME, CV_LC);
        if ((*token)                    /*  If anything follows, check       */
        && (do_block.handler
          != do_handler [lookup_do_handler ()].handler))
            syntax_error (error_event, MSG_DO_BLOCK_MISMATCH, token);
      }
}


/*  Lookup current token in do_handler table; if not found, returns          */
/*  pointer to empty action at end of table and raises the error exception.  */

static int
lookup_do_handler (void)
{
    int scan_ptr;                       /*  Pointer into do_handler table    */

    for (scan_ptr = 0; do_handler [scan_ptr].name; scan_ptr++)
        if (streq (token, do_handler [scan_ptr].name))
            break;

    if (!do_handler [scan_ptr].name)
        syntax_error (error_event, MSG_DO_BLOCK_SYNTAX, token);

    return (scan_ptr);
}


/**************************   OPEN DO BLOCK LEVEL   **************************/

MODULE open_do_block_level (void)
{
    DO_BLOCK *parent;

    if (do_block_level)                 /*  If do_block is active,           */
      {                                 /*    save as parent of new do_block */
        parent = (DO_BLOCK *) Check (malloc (sizeof (DO_BLOCK)));
        memcpy ((void *) parent, (void *) &do_block, sizeof (do_block));
        init_do_block_values (&do_block);
        do_block.parent = parent;
      }
    do_block_level++;
}


/**************************   CLOSE DO BLOCK LEVEL   *************************/

MODULE close_do_block_level (void)
{
    DO_BLOCK *parent;

    if (do_block_level > 1)             /*  If 2 or greater, we have a       */
      {                                 /*    parent do_block on the stack   */
        ASSERT (do_block.parent != NULL);
        parent = do_block.parent;
        memcpy ((void *) &do_block, (void *) parent, sizeof (do_block));
        free (parent);
      }
    do_block_level--;
}


/***********************   CHECK DO IS NOT REENTRANT   ***********************/

MODULE check_do_is_not_reentrant (void)
{
    DO_BLOCK *parent;

    for (parent = do_block.parent; parent; parent= parent-> parent)
        if (parent-> handler == do_block.handler)
            syntax_error (error_event, MSG_DO_BLOCK_REENTRANT);
}


/*************************   CHECK DO BLOCK IS OPEN   ************************/

MODULE check_do_block_is_open (void)
{
    if (do_block_level == 0)
        syntax_error (error_event, MSG_DO_BLOCK_UNDERFLOW);
}


/***************************   TEST DO CONDITIONS   **************************/

MODULE test_do_conditions (void)
{
    if (do_block.iteration == 0)
        init_do_block_symbols ();       /*  First time round                 */
    else
        do_block.number_symbol++;       /*  By default, iteration counter    */

    /*  Get orginal and parsed source lines into srcline and curline         */
    strcpy (curline, do_block.curline);
    strcpy (srcline, do_block.srcline);
    char_nbr = do_block.char_nbr;
    expand_symbols (curline);
    the_next_event = done_event;        /*  Most common case...              */

    if (do_block.iteration < (dbyte) std_run_away-> ivalue)
      {
        do_block.handler ();            /*  Do handler for action            */
        do_block.iteration++;           /*    and count our way around       */
      }
    else
        syntax_error (error_event, MSG_DO_BLOCK_RUNAWAY, do_block.iteration);
}


static void
init_do_block_symbols (void)            /*  Set default empty values         */
{
     array_base = (int) std_array_base-> ivalue;
     do_block.number_symbol    = array_base;
     do_block.offset_symbol    = array_base;
     do_block.row.tally        = 0;     /*  No items in $row yet             */
     do_block.name_symbol  [0] = 0;
     do_block.row.symbol   [0] = 0;
     do_block.comma_symbol [0] = 0;
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_state
 *
 *  :do state
 *
 *  Builds list of states.  Outputs the block for each state.
 *  Within each iteration of the loop, these variables are defined:
 *
 *      $name     - state name, formatted using current /style setting.
 *      $number   - state number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 */

static void
handler_for_do_state (void)
{
    dbyte
        state_nbr;

    if (do_block.iteration == 0)
        do_loop_state = listhead-> child;
    else
        do_loop_state = do_loop_state-> next;

    state_nbr = do_block.iteration;
    if (state_nbr < stats-> states)
      {
        strcpy (do_block.name_symbol,
                TrueName (stats-> snames [state_nbr]));
        strcpy (do_block.comma_symbol,
               (state_nbr < stats-> states - 1? std_comma_before-> svalue:
                                                std_comma_last-> svalue));
        the_next_event = ok_event;
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_event
 *
 *  :do event [local]
 *
 *  Builds list of events.  Outputs the block for each event.  If 'local'
 *  is specified, and in context of :do state loop, only takes events in
 *  the current state.
 *  Within each iteration of the loop, these variables are defined:
 *
 *      $name     - event name, with '_event' appended to end, and formatted
 *                  using -style setting.
 *      $number   - event number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 *      $next_state - next state for event, (only if local)
 */

static void
handler_for_do_event (void)
{
    static Bool
        local_event_list;               /*  Do we want a local event list?   */
    dbyte
        event_nbr = 0;

    if (do_block.iteration == 0)        /*  First time - parse command       */
      {
        /*  Look for 'local' keyword                                         */
        get_next_token (WANT_ANY, CV_LC);
        if (streq (token, "local") && do_loop_state)
          {
            do_loop_event = do_loop_state-> child;
            local_event_list = TRUE;
          }
        else
            local_event_list = FALSE;
      }
    else
    if (local_event_list)
        do_loop_event = do_loop_event-> next;

    if (local_event_list)
      {
        if (do_loop_event)
          {
            event_nbr      = do_loop_event-> number;
            the_next_event = ok_event;
            sym_values (std_next_state, 0,
                TrueName (stats-> snames [do_loop_event-> child-> number]));
            do_loop_module = do_loop_event-> child-> next;
          }
      }
    else
      {
        event_nbr = do_block.iteration;
        if (event_nbr < stats-> events)
            the_next_event = ok_event;
      }
    /*  Generate event code                                                  */
    if (the_next_event == ok_event)
      {
        strcpy (do_block.name_symbol, stats-> enames [event_nbr]);
        strcat (do_block.name_symbol, "_event");
        strcpy (do_block.name_symbol, TrueName (do_block.name_symbol));
        strcpy (do_block.comma_symbol,
               (event_nbr < stats-> events - 1? std_comma_before-> svalue:
                                                std_comma_last-> svalue));
        check_name_length (do_block.name_symbol, "event");
      }
}

static void
check_name_length (char *name, char *type)
{
    if ((streq (OPT_STYLE.value, "cobol"))
    && ((int) strlen (name) > 30))
      {
        PrintMessage (MSG_NAME_TOO_LONG, type, name);
        raise_exception (error_event);
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_module
 *
 *  :do module [local]
 *
 *  Builds list of modules.  Outputs the block for each module.  If 'local'
 *  is specified, and within :do event within :do state, only outputs the
 *  modules for current state/event transition.
 *  Within each iteration of the loop, these variables are defined:
 *
 *      $name     - module name, formatted using current /style setting.
 *      $number   - module number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 */

static void
handler_for_do_module (void)
{
    static Bool
        local_module_list;              /*  Do we want a local module list?  */
    dbyte
        module_nbr = 0;

    if (do_block.iteration == 0)        /*  First time - parse command       */
      {
        /*  Look for 'local' keyword                                         */
        get_next_token (WANT_ANY, CV_LC);
        if (streq (token, "local") && do_loop_event)
          {
            do_loop_module = do_loop_event-> child-> next;
            local_module_list = TRUE;
          }
        else
            local_module_list = FALSE;
      }
    else
    if (local_module_list)
        do_loop_module = do_loop_module-> next;

    /*  Get module number, either local to event, or from loop iteration     */
    if (local_module_list)
      {
        if (do_loop_module)
          {
            module_nbr     = do_loop_module-> number;
            the_next_event = ok_event;
          }
      }
    else
      {
        module_nbr = do_block.iteration;
        if (module_nbr < stats-> modules)
            the_next_event = ok_event;
      }
    /*  Generate module code                                                 */
    if (the_next_event == ok_event)
      {
        strcpy (do_block.name_symbol, TrueName (stats-> mnames [module_nbr]));
        strcpy (do_block.comma_symbol,
              (module_nbr < stats-> modules - 1? std_comma_before-> svalue:
                                                 std_comma_last-> svalue));
        check_name_length (do_block.name_symbol, "module");
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_action
 *
 *  :do action
 *
 *  Builds table containing one row per state, with one item per row for
 *  each event transition.  Each item is the number of an action vector.
 *  The action vectors are built using the :do vector action.
 *  Invalid state/event transitions are marked by the value of row_null.
 *
 *  Within each iteration of the loop, these variables are defined:
 *
 *      $row      - value of row, formatted according to row_first,
 *                  row_after, and row_null.
 *      $number   - state number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 *      $offset   - offset of start of current row, added to array_base.
 *                  The offset is counted up by 1 for each item in a row.
 *      $tally    - number of items in $row
 */

static void
handler_for_do_action (void)
{
    static lrnode *state;               /*  Pointer to state in dialog list  */
    static int     offset,              /*  Offset of vector in output table */
                   vecsize;             /*  Size of vector in bytes          */

    lrnode *event,                      /*  Pointer to event                 */
           *nextstate,                  /*  Pointer to next state node       */
           *module;
    int     eventnbr,
            modnbr,
            vecnbr;
    dbyte  *nextvec;                    /*  Pointer to next vector in table  */
    ROWT   *row;                        /*  Pointer to current row block     */

    if (do_block.iteration == 0)        /*  We're called once per state      */
      {
        state   = listhead-> child;     /*  First time, iteration is zero    */
        offset  = array_base;           /*  Offset of vector in output table */
        vecsize = (stats-> maxaction + 1) * sizeof (dbyte);
      }
    if (state)                          /*  List ends when state is null     */
      {
        do_block.offset_symbol = offset;

        /*  Build-up transition buffer for this state and all events         */
        memset ((void *) transit, 0, sizeof (dbyte) * LR_EVENTMAX);
        for (event = state-> child; event; event = event-> next)
          {
            memset ((void *) actions, 0, vecsize);
            nextstate = event-> child;
            modnbr = 0;
            for (module = nextstate-> next; module; module = module-> next)
                actions [modnbr++] = module-> number;

            actions [modnbr] = 0xFFFFUL;
            nextvec = vectors + stats-> maxaction + 1;
            for (vecnbr = 0; vecnbr < vectorcnt; vecnbr++)
              {
                if (memcmp ((void *) actions, (void *) nextvec, vecsize) == 0)
                    break;
                nextvec += stats-> maxaction + 1;
              }
            /*  Vector MUST already exist; built by init_vectors             */
            ASSERT (vecnbr != vectorcnt);
            transit [event-> number] = (dbyte) (vecnbr + array_base);
          }

        /*  Format one or several row buffers for transition data            */
        clear_row_parts ();
        row = &do_block.row;
        for (eventnbr = 0; eventnbr < stats-> events; eventnbr++)
          {
            offset++;                   /*  Keep running offset              */
            sprintf (token, row_format (eventnbr),
                    (long)  row_field  (transit [eventnbr]));
            row = cat_token_to_row (row, token);
          }
        strcpy (do_block.comma_symbol,
               (state-> next? std_comma_before-> svalue:
                              std_comma_last-> svalue));
        state = state-> next;
        the_next_event = ok_event;
      }
}


/*  ------------------------------------------------------------------------
 *  clear_row_parts
 *
 *  Frees any row parts attached to list at row_overflow.  This function
 *  must be called before any $row symbol is built-up.  Clears the do_block
 *  row.symbol and row.tally.
 */

static void
clear_row_parts (void)
{
    ROWT *next;

    *do_block.row.symbol  = 0;          /*  Start new row in any case        */
    do_block.row.tally    = 0;          /*  Initialise size counter          */
    std_overflow-> ivalue = 0;          /*  No overflow yet                  */

    while (row_overflow)                /*  Free any allocated row parts     */
      {
        next = row_overflow-> next;
        free (row_overflow);
        row_overflow = next;
      }
}


/*  Return format for row index.  If first in row, returns std_row_first,    */
/*  else returns std_row_after.                                              */

static char *
row_format (int row_index)
{
    if (row_index == 0)
        return (std_row_first-> svalue);
    else
        return (std_row_after-> svalue);
}

/*  Return specified value if not 0; if 0 then return standard null value    */

static int
row_field (int row_value)
{
    if (row_value == 0)
        return ((int) std_row_null-> ivalue);
    else
        return (row_value);
}


/*  Append to this row, or allocate new row block if overflow                */
/*  If row_clean is 1, removes any non-numeric data from start of overflow   */
/*  row.                                                                     */

static ROWT *
cat_token_to_row (ROWT *row, char *token)
{
    if ((dbyte) (strlen (row-> symbol) + strlen (token))
      > (dbyte) (std_row_width-> ivalue))
      {
        row-> next = (ROWT *) Check (malloc (sizeof (ROWT)));
        if (row_overflow == NULL)
          {
            std_overflow-> ivalue = 1;
            row_overflow = row-> next;
          }
        row = row-> next;
        row-> next   = NULL;            /*  Initialise values in new block   */
        row-> tally  = 0;
       *row-> symbol = 0;

        if (std_row_clean-> ivalue)     /*  Clean-up overflow row if wanted  */
            while (!isdigit (*token))   /*    by removing punctuation at     */
                StrClose (token, 0);    /*    start of token                 */

      }
    strcat (row-> symbol, token);
    row-> tally++;
    return (row);
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_nextst
 *
 *  :do nextst
 *
 *  Builds table containing one row per state, with one item per row for
 *  each event transition.  Each item is the number of the next state.
 *  Invalid state/event transitions are marked by the value of row_null.
 *
 *  Within each iteration of this loop, these variables are defined:
 *
 *      $row      - value of row, formatted according to row_first,
 *                  row_after, and row_null.
 *      $number   - state number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 *      $offset   - offset of start of current row, added to array_base.
 *                  The offset is counted up by 1 for each item in a row.
 *      $tally    - number of items in $row
 */

static void
handler_for_do_nextst (void)
{
    static lrnode *state;               /*  Pointer to state in dialog list  */
    static int    offset;               /*  Offset of vector in output table */

    lrnode *event;                      /*  Pointer to event                 */
    int    eventnbr;                    /*  Event number 0..n-1              */
    ROWT   *row;                        /*  Pointer to current row block     */

    if (do_block.iteration == 0)        /*  We're called once per state      */
      {
        state  = listhead-> child;      /*  First time, iteration is zero    */
        offset = array_base;            /*  Offset of vector in output table */
      }
    if (state)                          /*  List ends when state is null     */
      {
        do_block.offset_symbol = offset;

        /*  Build-up transition buffer for this state and all events         */

        memset ((void *) transit, 0, sizeof (dbyte) * LR_EVENTMAX);
        for (event = state-> child; event; event = event-> next)
            transit [event-> number] = event-> child-> number + array_base;

        /*  Format one or several row buffers for transition data            */

        clear_row_parts ();
        row = &do_block.row;
        for (eventnbr = 0; eventnbr < stats-> events; eventnbr++)
          {
            offset++;
            sprintf (token, row_format (eventnbr),
                    (long)  row_field  (transit [eventnbr]));

            row = cat_token_to_row (row, token);
          }
        strcpy (do_block.comma_symbol,
               (state-> next? std_comma_before-> svalue:
                              std_comma_last-> svalue));
        state = state-> next;
        the_next_event = ok_event;
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_vector
 *
 *  :do vector
 *
 *  Builds table of action vectors.  An action vector is a list of modules
 *  executed in series for a state/event transition.  The vector table is
 *  built by the :do action block.  Duplicate vectors are collapsed.
 *
 *  Within each iteration of this loop, these variables are defined:
 *
 *      $row      - action vector, formatted according to row_first,
 *                  row_after, and row_null.
 *      $number   - vector number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 *      $offset   - offset of start of current row, added to array_base.
 *                  The offset is counted up by 1 for each item in a row,
 *                  including an assumed terminator value (0xFFFF).
 *      $tally    - the number of items in each row, including one
 *                  terminator value (ie. nbr items in $row + 1).
 */

static void
handler_for_do_vector (void)
{
    static int  offset;                 /*  Offset of vector in output table */
    static dbyte *nextvec;              /*  Pointer to next vector in table  */

    ROWT *row;                          /*  Pointer to current row block     */
    int  vecnbr = do_block.iteration,
         modnbr;

    if (do_block.iteration == 0)        /*  We're called once per state      */
      {
        offset  = array_base;           /*  Offset of vector in output table */
        nextvec = vectors + stats-> maxaction + 1;
      }
    if (vecnbr < vectorcnt)
      {
        do_block.offset_symbol = offset;
        clear_row_parts ();
        do_block.row.tally++;           /*  Add 1 for terminator value       */
        row = &do_block.row;
        for (modnbr = 0; ; modnbr++)
          {
            offset++;
            if (nextvec [modnbr] == 0xFFFFL)
                break;
            sprintf (token, row_format (modnbr),
                    (long)  nextvec [modnbr] + array_base);
            row = cat_token_to_row (row, token);
          }
        nextvec += stats-> maxaction + 1;
        strcpy (do_block.comma_symbol,
               (vecnbr < vectorcnt - 1? std_comma_before-> svalue:
                                        std_comma_last-> svalue));
        the_next_event = ok_event;
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_overflow
 *
 *  :do overflow
 *
 *  Within each iteration of this loop, these variables are defined:
 *
 *      $row      - overflow row, ie. part of main $row that did not fit
 *                  onto first line.
 *      $number   - iteration number 0..n-1, added to array_base
 *      $comma    - comma_before until the last iteration; then comma_last.
 *      $tally    - number of items in $row
 */

static void
handler_for_do_overflow (void)
{
    static ROWT *row;                   /*  Pointer to current row block     */
    if (do_block.iteration == 0)        /*  We're called once per state      */
        row = row_overflow;

    if (row)                            /*  If more overflow, get values     */
      {                                 /*    into do_block symbols          */
        strcpy (do_block.row.symbol, row-> symbol);
        do_block.row.tally = row-> tally;
        row = row-> next;
        the_next_event = ok_event;
        strcpy (do_block.comma_symbol,
               (row? std_comma_before-> svalue: std_comma_last-> svalue));
      }
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_stubs
 *
 *  :do stubs [filename]...
 *
 *  Compares modules defined in $source file (or name specified after stubs
 *  action).  If any are undefined, creates stubs for those modules.
 *  This action can be placed at any point in the schema; I usually put it
 *  at the end.  It does not do any output to the current output file.
 *  If multiple filenames are specified, adds module stubs to first file,
 *  but also searches further files for existing modules.
 */

static void
handler_for_do_stubs (void)
{
    static char
        flags [LR_MODULEMAX],           /*  Indicate whether modules used    */
        stubs [LR_FILENAMEMAX + 1];     /*  Actual name of stubs file        */
    static int
        newmods,                        /*  Number of new modules required   */
        modnbr;                         /*  Used to scan through modules     */
    static symbol
        *intsym;                        /*  Internal module name symbol      */

    char
        *name_ptr,                      /*  Start of name in scanned line    */
        *suffix_ptr,                    /*  Start of suffix in module line   */
        *prefix_ptr;                    /*  Start of prefix in module line   */
    int
        name_size,                      /*  Size of name, if found           */
        prefix_size,                    /*  Size of prefix in module line    */
        suffix_size;                    /*  Size of suffix in module line    */
    FILE
        *input;                         /*  Files that contain modules       */
    Bool
        module_done;                    /*  Did we output something?         */

    if ((OPT_STUBS.flags & OPT_ON) == 0)
        return;                         /*  Ignore if /nostubs used          */

    if (do_block.iteration == 0)        /*  First time - build tables        */
      {
        /*  Find all MODULE functions in input                               */
        get_next_token (WANT_FILENAME, CV_NO);
        if (strnull (token))            /*  If no filename specified,        */
            strcpy (token, stats-> source); /*  use source filename          */
        strcpy (stubs, token);          /*  Save name of file to get stubs   */

        /*  We try to match each line in the file against the pattern in     */
        /*  std_module_line. This consists of [prefix]Name[suffix] where     */
        /*  the prefix and suffix are any text, optional. Name is always     */
        /*  represented by '%s'. Eg. :set module_line = "MODULE %s () {"     */
        /*  Module name must be single word starting at %s in line.          */

        name_ptr = strstr (std_module_line-> svalue, "%s");
        if (name_ptr == NULL)
            name_ptr = std_module_line-> svalue;

        suffix_ptr = name_ptr + 2;
        prefix_ptr = std_module_line-> svalue;
        prefix_size = (int) (name_ptr - std_module_line-> svalue);
        suffix_size = strlen (suffix_ptr);

        /*  Reset module flag array to all zeroes - nothing found yet        */
        memset (flags, 0, LR_MODULEMAX);

        /*  Total number of known modules                                    */
        newmods = stats-> modules + int_tab-> size;
        /*  Process each filename in list (1 or more times through loop)     */
        while (*token != 0)
          {
            input = FileOpen (token, 'r');
            if (input == NULL)
              {
                syntax_error (error_event, MSG_SOURCE_FILE_NF, token);
                return;
              }
            while (FileRead (input, srcline))
              {
                preformat_line_if_reqd (srcline);
                StrCrop (srcline);
                name_size = strlen (srcline) - prefix_size - suffix_size;
                name_ptr = srcline + prefix_size;

                /*  Do prefix and suffix texts match?                        */
                if (strncmp (prefix_ptr, srcline,              prefix_size) == 0
                &&  strncmp (suffix_ptr, name_ptr + name_size, suffix_size) == 0
                && (strchr (name_ptr, ' ') == NULL
                ||  strchr (name_ptr, ' ') >= (name_ptr + name_size)))
                  {
                    /*  Collect token from srcline at calculated posn & size */
                    strncpy (token, name_ptr, name_size);
                    token [name_size] = 0;

                    for (modnbr = 0; modnbr < stats-> modules; modnbr++)
                        if (streq (token, TrueName (stats-> mnames [modnbr]))
                        &&  flags [modnbr] == 0)
                          {
                            flags [modnbr] = 1;
                            newmods--;  /*  One less to do                   */
                            break;
                          }
                    if (modnbr >= stats-> modules)
                      {
                        for (intsym = int_tab-> syms; intsym;
                             intsym = intsym-> next)
                            if (streq (token, TrueName (intsym-> name)))
                              {
                                intsym-> flags = SYM_FLAG_FOUND;
                                newmods--;
                                break;
                              }
                        /*  'Idle' message if module not found anywhere      */
                        if ((intsym == NULL) && OPT_IDLE.flags)
                            PrintMessage (MSG_IDLE_MODULE, token);
                      }
                  }
              }
            FileClose (input);
            get_next_token (WANT_FILENAME, CV_NO);
          }
        /*  Open output stream if we are going to generate new modules       */
        if (newmods > 0)
          {
            output_stream = open_extend (stubs);
            intsym = int_tab-> syms;    /*  We'll do the :internal modules   */
            modnbr = 0;                 /*    and then the dialog modules    */
          }
      }

    /*  We come here each time through the :do stubs line...                 */
    /*  If there is anything to generate, we build the module header         */
    /*  ourselves, then return to the code generator to output the body      */
    /*  text of the module.  Module names come from 2 places: the :internal  */
    /*  symbol table and the dialog mnames table.                            */

    if (newmods > 0)
      {
        /*  Look first through :internal table for an unfound module         */
        module_done = FALSE;
        while (intsym && !module_done)
          {
            if (intsym-> flags != SYM_FLAG_FOUND)
              {
                build_stub_header (intsym-> name);
                module_done = TRUE;
              }
            intsym = intsym-> next;
          }

        /*  Now look through module names from dialog                        */
        while (modnbr < stats-> modules && !module_done)
          {
            if (flags [modnbr] == 0)
              {
                build_stub_header (stats-> mnames [modnbr]);
                module_done = TRUE;
              }
            modnbr++;
          }

        /*  If we output a header, tell the dialog to output the body        */
        if (module_done)
            the_next_event = ok_event;
      }
}


static void
build_stub_header (char *name)
{
    int  repcount;                      /*  Number of stub_between's         */
    char *scan;

    strcpy (do_block.name_symbol, name);
    PrintMessage (MSG_BUILDING_STUB, TrueName (do_block.name_symbol));

    strcpy (token, name);               /*  GET NAME LIKE THIS               */
    StrUpr (token);
    for (scan = token; *scan; scan++)
        if (*scan == '_' || *scan == '-')
            *scan = ' ';

    write_output_line (std_stub_before-> svalue);
    strcpy (curline, std_stub_first-> svalue);
    if (*std_stub_between-> svalue)     /*  If stub_between is not empty     */
      {                                 /*    center module name             */
        repcount = (int) std_stub_width-> ivalue
                 - strlen (std_stub_first-> svalue)
                 - strlen (std_stub_last-> svalue);
        prtitle ((int) (repcount - strlen (token) - 6 + 1) / 2);
        strcat (curline, "   ");
        strcat (curline, token);
        strcat (curline, "   ");
        prtitle ((int) (repcount - strlen (token) - 6) / 2);
      }
    else                                /*  Else print as-is                 */
        strcat (curline, token);

    strcat (curline, std_stub_last-> svalue);
    write_output_line (curline);

    /*  Format $name and $module_name for use in code block                  */
    sym_values (std_module_name, 0, ExternalName (name));
    strcpy (do_block.name_symbol, TrueName (do_block.name_symbol));
}

static void
prtitle (int width)
{
    int outlen = 0;
    int maxlen;

    maxlen = width + strlen (curline);
    while (outlen < width)
      {
        strcat (curline, std_stub_between-> svalue);
        outlen += strlen (std_stub_between-> svalue);
      }
    curline [maxlen] = 0;               /*  Truncate if longer than width    */
}


/*  ------------------------------------------------------------------------
 *  handler_for_do_while
 *
 *  :do while condition
 *
 *  Repeats while condition is true.  The condition is a logical comparison
 *  of the type accepted by :if test ...  The loop is executed 0..n times.
 *
 *  Within each iteration of this loop, these variables are defined:
 *
 *      $number   - loop iteration 0..n-1, added to array_base
 */

static void
handler_for_do_while (void)
{
    if (eval_condition (curline + char_nbr) == 1)
        the_next_event = ok_event;
}


/*  ------------------------------------------------------------------------
 *  eval_condition
 *
 *  Calls lr_evaluate; returns 0 if error, 1 if true, 2 if false.  If error,
 *  displays condition text, error indicator, and error message, and raises
 *  the error_event exception.
 */

static int
eval_condition (char *line)
{
    int
        eval_feedback,                  /*  Feedback from lr_evaluate ()     */
        eval_offset,                    /*  Position of any error            */
        indent;                         /*  For nice error messages          */
    Bool
        eval_result = 0;                /*  Result of comparison             */

    eval_feedback = lr_evaluate (line, &eval_result, &eval_offset, stats);
    if (eval_feedback)                  /*  Error in comparison?             */
      {
        StrCrop (srcline);
        fprintf (stderr, "%s %d: %s\n", local_file, line_nbr, srcline);
        fprintf (stderr, "lr I: %s\nlr I: ", line);
        for (indent = eval_offset + 1; indent; indent--)
            fprintf (stderr, "*");
        fprintf (stderr, "\n%s\n", MessageText (eval_feedback));
        raise_exception (error_event);
        return (0);
      }
    else
    if (eval_result)
        return (1);
    else
        return (2);
}


/*************************   SKIP TO DO BLOCK START   ************************/

MODULE skip_to_do_block_start (void)
{
    fseek (schema, do_block.start_pos, SEEK_SET);
    line_nbr = do_block.start_line;
}


/**************************   SKIP TO DO BLOCK END   *************************/

MODULE skip_to_do_block_end (void)
{
    fseek (schema, do_block.end_pos, SEEK_SET);
    line_nbr = do_block.end_line;
}


/*****************************   PARSE IF BLOCK   ****************************/
/*
 *  :if condition
 */

MODULE parse_if_block (void)
{
    int if_level = 1;                   /*  Indentation level of if's        */
    int do_level = 0;                   /*  Embedded do's must be closed     */

    strcpy (if_block.srcline, srcline); /*  Save original                    */
    strcpy (if_block.curline, curline); /*    and parsed source lines        */
    if_block.start_pos  = ftell (schema);
    if_block.start_line = line_nbr;
    if_block.else_line  = 0;            /*  Zero until :else found           */
    if_block.else_pos   = NULL_POS;
    if_block.char_nbr   = char_nbr;

    DisableTrace ();                    /*  Don't trace look-ahead           */
    while (if_level > 0)                /*  Look for matching :endif         */
      {
        switch (parse_schema_line ())
          {
            case finished_event:
                line_nbr = if_block.start_line;
                strcpy (srcline, if_block.srcline);
                syntax_error (error_event, MSG_IF_BLOCK_OPEN);
                return;
            case if_event:
                if_level++;
                break;
            case else_event:
                if (if_level == 1)
                  {
                    if_block.else_pos  = ftell (schema);
                    if_block.else_line = line_nbr;
                  }
                break;
            case endif_event:
                get_next_token (WANT_NAME, CV_LC);
                if (streq (token, "all"))
                    if_level = 1;       /*  :endif all                       */

                if (if_level == 1)      /*  If at end of block               */
                  {                     /*    then save position             */
                    if_block.end_pos  = ftell (schema);
                    if_block.end_line = line_nbr;
                  }
                if_level--;
                break;
            case do_event:
                do_level++;
                break;
            case enddo_event:
                do_level--;
                break;
          }
      }
    if (TraceFile)
        EnableTrace ();

    /*  Check for mismatched :do / :enddo within if loop                     */
    if (do_level > 0)
        syntax_error (error_event, MSG_IF_BLOCK_OVERLAP);
}


/**************************   OPEN IF BLOCK LEVEL   **************************/

MODULE open_if_block_level (void)
{
    IF_BLOCK *parent;

    if (if_block_level)                 /*  If if_block is active,           */
      {                                 /*    save as parent of new if_block */
        parent = (IF_BLOCK *) Check (malloc (sizeof (IF_BLOCK)));
        memcpy ((void *) parent, (void *) &if_block, sizeof (if_block));
        init_if_block_values (&if_block);
        if_block.parent = parent;
      }
    if_block_level++;
}


/**************************   CLOSE IF BLOCK LEVEL   *************************/

MODULE close_if_block_level (void)
{
    IF_BLOCK *parent;

    if (if_block_level > 1)             /*  If 2 or greater, we have a       */
      {                                 /*    parent if_block on the stack   */
        ASSERT (if_block.parent != NULL);
        parent = if_block.parent;
        memcpy ((void *) &if_block, (void *) parent, sizeof (if_block));
        free (parent);
      }
    if_block_level--;
}


/*************************   CHECK IF BLOCK IS OPEN   ************************/

MODULE check_if_block_is_open (void)
{
    if (if_block_level == 0)
        syntax_error (error_event, MSG_IF_BLOCK_UNDERFLOW);
}


/***************************   TEST IF CONDITIONS   **************************/

MODULE test_if_conditions (void)
{
    if (if_block_level == 0)
        syntax_error (error_event, MSG_IF_BLOCK_UNDERFLOW);
    else
      {
        strcpy (curline, if_block.curline);
        strcpy (srcline, if_block.srcline);
        line_nbr = if_block.start_line;
        char_nbr = if_block.char_nbr;   /*  Get context right for :if line   */

        expand_symbols (curline);
        if (eval_condition (curline + char_nbr) == 1)
            the_next_event = true_event;
        else
            the_next_event = false_event;
      }
}


/*************************   CHECK IF ELSE DEFINED   *************************/

MODULE check_if_else_defined (void)
{
    if (if_block.else_pos != NULL_POS)
        raise_exception (have_else_event);
}


/*************************   SKIP TO IF BLOCK START   ************************/

MODULE skip_to_if_block_start (void)
{
    fseek (schema, if_block.start_pos, SEEK_SET);
    line_nbr = if_block.start_line;
}


/*************************   SKIP TO IF BLOCK ELSE   *************************/

MODULE skip_to_if_block_else (void)
{
    fseek (schema, if_block.else_pos, SEEK_SET);
    line_nbr = if_block.else_line;
}


/**************************   SKIP TO IF BLOCK END   *************************/

MODULE skip_to_if_block_end (void)
{
    fseek (schema, if_block.end_pos, SEEK_SET);
    line_nbr = if_block.end_line;
}


/***************************   CLOSE SCHEMA FILE   ***************************/

MODULE close_schema_file (void)
{
    if (schema)
        FileClose (schema);
}


/**************************   SET EXIT CODE IF ANY   *************************/
/*
 *  :exit [returncode]
 */

MODULE set_exit_code_if_any (void)
{
    get_next_token (WANT_ANY, CV_NO);   /*  Get return code if any           */
    if (*token)                         /*    and convert to number          */
        feedback = atoi (token);        /*    without too much checking      */
}


/************************   TRACE SYMBOLS IF WANTED   ************************/

MODULE trace_symbols_if_wanted (void)
{
    Trace ("\nCode Generator Symbol Table:");
    sym_exec_all (var_tab, sym_dump);
    Trace ("");
    Trace ("\nCode Generator Internal Modules:");
    sym_exec_all (int_tab, sym_dump);
    Trace ("");
}


static int sym_dump (symbol *sp, ...)
{
    Trace ("   %-20s  %s  %4ld  \"%s\"",
           sp-> name,
           sp-> type == SYM_TYPE_INT?    "int":
           sp-> type == SYM_TYPE_STRING? "str":
           /*  else  */                  "any",
           sp-> ivalue,
           sp-> svalue? sp-> svalue: "");
    return (1);
}


/*************************   RETURN ERROR FEEDBACK   *************************/

MODULE return_error_feedback (void)
{
    feedback = -1;
}


/***************************   FREE SYMBOL TABLE   ***************************/

MODULE free_symbol_table (void)
{
    clear_row_parts ();
    free (vectors);                     /*  Release allocated memory blocks  */
    free (actions);
    free (transit);
    sym_delete_table (var_tab);         /*  Delete symbol table & contents   */
    sym_delete_table (int_tab);         /*  Delete internal modules table    */
}


/***************************   GET EXTERNAL EVENT   **************************/

MODULE get_external_event (void)
{
}


/*************************   TERMINATE THE PROGRAM    ************************/

MODULE terminate_the_program (void)
{
    the_next_event = terminate_event;
}
