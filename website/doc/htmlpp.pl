#! /usr/bin/perl
#============================================================================*
#                                                                            *
#   htmlpp.pl   HTML pre-processor 1.2                                       *
#                                                                            *
#   Written:    96/03/27   Pieter Hintjens <ph@imatix.com>                   *
#   Revised:    96/04/09                                                     *
#                                                                            *
#   96/04/06    .define symbol = <expression>                                *
#               .echo [-] <text>                                             *
#               .if <expression> /.else / .endif                             *
#   96/04/08    $(*xxx) non-link if symbol is empty                          *
#   96/04/09    Added $(PASS), $(BASE), $(EXT)                               *
#               Improved .page command                                       *
#                                                                            *
#   Copyright:  (c) 1996 Pieter Hintjens                                     *
#                                                                            *
#   This program is free software; you can redistribute it and/or modify     *
#   it under the terms of the GNU General Public License as published by     *
#   the Free Software Foundation; either version 2 of the License, or        *
#   (at your option) any later version.                                      *
#                                                                            *
#   This program is distributed in the hope that it will be useful,          *
#   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
#   GNU General Public License for more details.                             *
#                                                                            *
#   You should have received a copy of the GNU General Public License        *
#   along with this program; if not, write to the Free Software              *
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
#============================================================================*

require 'htmlpp.d';                     #   Include dialog interpreter


##########################   INITIALISE THE PROGRAM   #########################

sub initialise_the_program
{
    print "htmlpp 1.2 - by Pieter Hintjens\n";

    if ($#ARGV == 0) {                  #   Exactly 1 argument in @ARGV[0]?
        $main_document = @ARGV [0];
        $the_next_event = $ok_event;
    } else {
        print "syntax: htmlpp <filename>\n";
        $the_next_event = $error_event;
    }
}


#########################   INITIALISE PROGRAM DATA   #########################

sub initialise_program_data
{
    #   These are the preprocessor keywords that we recognise
    $keyword {"block"}   = $block_event;
    $keyword {"build"}   = $build_event;
    $keyword {"echo"}    = $echo_event;
    $keyword {"else"}    = $else_event;
    $keyword {"end"}     = $end_event;
    $keyword {"endif"}   = $endif_event;
    $keyword {"define"}  = $define_event;
    $keyword {"if"}      = $if_event;
    $keyword {"ignore"}  = $ignore_event;
    $keyword {"include"} = $include_event;
    $keyword {"page"}    = $page_event;

    #   Prepare date and time symbol strings
    ($sec, $min, $hour, $day, $month, $year) = localtime;
    $symbols {"DATE"} = sprintf ("%02d/%02d/%02d", $year, $month + 1, $sec);
    $symbols {"TIME"} = sprintf ("%2d:%02d:%02d", $hour, $min, $sec);

    #   Prepare other built-in symbols
    $symbols {"PASS"} = 0;              #   Pass 0 = scan, pass 1 = output
    $symbols {"BASE"} = "doc";          #   For default page filenames
    $symbols {"EXT"}  = "htm";          #   For default page filenames

    #   Clear document structure tables
    undef @toc_title;                   #   Table of contents titles
    undef @toc_file;                    #   Table of contents filenames
    undef @toc_level;                   #   Table of contents levels

    undef @page_list;                   #   Clear page name table
    undef @page_title;                  #   Clear page title table
}


#########################   LOAD ANCHOR DEFINITIONS   #########################

sub load_anchor_definitions
{
    undef %anchors;                     #   Clear assoc. array in any case
    if (open (ANCHOR, "anchor.def")) {
        while (<ANCHOR>) {
            next if /^#/;               #   Skip comments
            chop;
            /(\S+)\s+(\S+)/;            #   Break $_ into name and value
            $anchors {$1} = $2;         #     and load into assoc. array
        }
        close (ANCHOR);
    }
}


#########################   SAVE ANCHOR DEFINITIONS   #########################

sub save_anchor_definitions
{
    local ($key, $value);

    if (open (ANCHOR, ">anchor.def")) {
        print ANCHOR "#  Anchor definitions - created by htmlpp\n";
        print ANCHOR "#  Delete this file to reset all anchors\n";
        while (($key, $value) = each %anchors) {
            printf (ANCHOR "%-20s %s\n", $key, $value);
        }
        close (ANCHOR);
    } else {
        print "htmlpp E: can't create anchor.def: $!";
        &raise_exception ($exception_event);
    }
}


############################   OPEN MAIN DOCUMENT   ###########################

sub open_main_document
{
    $lines_read    = 0;                 #   Nothing processed so far
    $header_nbr    = 0;                 #   Header numbering for TOC
    $page_nbr      = 0;                 #   Index into @page_list
    $ignore_header = 0;
    $ignore_pages  = 0;
    $ignore_level  = 99;
    $sequencer     = "";                #   Reset $(INC) symbol

    undef %already_seen;                #   Reset .include handling

    $document = $main_document;
    &open_the_document;
}


############################   OPEN THE DOCUMENT   ############################

sub open_the_document
{
    #   We use an indirect filehandle, whose name is the document name.
    #   To read from the file, we use <$document>
    #
    if (open ($document, $document)) {
        $file_is_open {$document} = 1;  #   Keep track of open documents
    } else {
        print "htmlpp E: ($document $.) can't open $document: $!";
        &raise_exception ($exception_event);
    }
}


##########################   GET NEXT DOCUMENT LINE   #########################

sub get_next_document_line
{
    if ($_ = <$document>) {             #   Get next line of input
        chop;                           #   Remove trailing newline
        $lines_read++;                  #   Count the line
                                        #   Warn if line > 79 chars long
        length > 79 && print "htmlpp E: ($document $.) line > 79 chars\n";

        if (/^$/) {                     #   Blank lines
            $the_next_event = $blank_line_event;
        } elsif (/^\.-/) {              #   Comments
            $the_next_event = $comment_event;
        } elsif (/^\./) {               #   Line starts with a dot
            /^\.(\w+)/;                 #   Get word after dot
            if (defined ($keyword {$1})) {
                $the_next_event = $keyword {$1};
            } else {
                &syntax_error;
            }
        } else {
            $the_next_event = $body_text_event;
        }
    } else {
        $the_next_event = $finished_event;
    }
}

sub syntax_error {
    print "$_\n";
    print "htmlpp E: ($document $.) syntax error\n";
    &raise_exception ($exception_event);
}


#########################   STORE SYMBOL DEFINITION   #########################

sub store_symbol_definition
{
    #   .define symbol value    -- define or redefine symbol
    #   .define symbol ""       -- define symbol as empty string
    #   .define symbol = expr   -- evaluate as Perl expression
    #   .define symbol          -- undefine symbol
    #
    #   Symbol names can consist of letters, digits, -._
    #
    #   The order of the following tests is important, as we need
    #   to treat the special cases first:
    #
    if (/^\.\w+\s+([A-Za-z0-9-\._]+)\s+""/) {
        $symbols {$1} = "";             #   .define symbol ""
    } elsif (/^\.\w+\s+([A-Za-z0-9-\._]+)\s+=\s+(.+)/) {
        $symbols {$1} = eval ($2);      #   .define symbol = expr
    } elsif (/^\.\w+\s+([A-Za-z0-9-\._]+)\s+(.+)/) {
        $symbols {$1} = $2;             #   .define symbol value
    } elsif (/^\.\w+\s+([A-Za-z0-9-\._]+)/) {
        undef $symbols {$1};            #   .define symbol
    } else {
        &syntax_error;                  #   something else
    }
}


##########################   EXPAND SYMBOLS IN LINE   #########################

sub expand_symbols_in_line
{
    #   Expands symbols in $_ variable
    #
    #   Repeatedly expand symbols like this (and in this order):
    #   $(xxx)            - value of variable
    #   $(*xxx)           - create link: <A HREF="xxx">xxx</A>
    #   $(*xxx="label")   - create link: <A HREF="xxx">label</A>
    #   $(*xxx=label)     - create link: <A HREF="xxx">label</A>
    #
    #   Note that the entire symbol must be on one line; if the symbol or
    #   its label is broken over two lines it won't be expanded.  After we
    #   expand symbols, we drop trailing whitespace on the line.  The link
    #   symbols $(*...) omit the <A> and </A> tags if the symbol value is
    #   empty.
    #
    for (;;) {
        if (/\$\(([A-Za-z0-9-_\.]+)\)/) {
            $_ = $`.&valueof ($1).$';
        } elsif (/\$\(\*([A-Za-z0-9-_\.]+)\)/) {
            $_ = $`.&make_link ($1, $1).$';
        } elsif (/\$\(\*([A-Za-z0-9-_\.]+)="([^"]*)"\)/) {
            $_ = $`.&make_link ($1, $2).$';
        } elsif (/\$\(\*([A-Za-z0-9-_\.]+)=([^)]*)\)/) {
            $_ = $`.&make_link ($1, $2).$';
        } else {
            last;
        }
    }
    chop while (/\s$/);                 #   Remove trailing whitespace
}

#   Subroutine returns the value of the specified symbol; it issues a
#   warning message and returns 'UNDEF' if the symbol is not defined.
#
sub valueof {
    local ($symbol_name) = "@_";        #   Argument is symbol name

    defined ($symbols {$symbol_name}) && return $symbols {$symbol_name};
    $symbol_name eq "INC"             && return sprintf ("%s", $sequencer++);
    defined ($anchors {$symbol_name}) && return
        $anchors {$symbol_name} eq $cur_file? $symbol_name:
        $anchors {$symbol_name}."#".$symbol_name;

    print "$_\n";
    print "htmlpp E: ($document $.) undefined symbol \"$symbol_name\"\n";
    $symbols {$symbol_name} = "UNDEF";
    return $symbols {$symbol_name};
}

#   Subroutine returns a formatted link between <A> and </A>: the first
#   argument is the symbol to translate; the second is the label for the
#   link.  If the symbol has an empty value, the <A> and </A> tags are
#   omitted.
#
sub make_link {
    local ($symbol, $label) = @_;
    $symbol = &valueof ($symbol);
    return $symbol? "<A HREF=\"$symbol\">$label</A>": $label;
}


##########################   TAKE INCLUDE FILE NAME   #########################

sub take_include_file_name
{
    #   .include filename       -- include file first time only
    #   .include filename!      -- include file in any case
    #
    #   Get filename after .include, maybe followed by !
    #
    if (/^\.\w+\s+([^\s!]+)(!)?/) {
        if ($file_is_open {$1}) {
            print "$_\n";
            print "htmlpp E: ($document $.) $1 is already open";
            &raise_exception ($exception_event);
        };
        #   If include file already seen and not forced by !, skip it
        if ($already_seen {$1} && $2 ne "!") {
            &raise_exception ($comment_event);
        } else {
            #   Save current document name and switch to new document
            push (@document_stack, $document);
            $document = $1;
            $already_seen {$1} = 1;
        }
    } else {
        &syntax_error;
    }
}


##########################   HANDLE IGNORE COMMAND   ##########################

sub handle_ignore_command
{
    #   .ignore header    - ignore next header
    #   .ignore header n  - ignore headers Hn and greater
    #   .ignore pages     - ignore all future .page commands
    #
    if (/^\.ignore\sheader$/) {
        $ignore_header = 1;
    } elsif (/^\.ignore\sheader\s([0-9]+)$/) {
        $ignore_level = $1;
    } elsif (/^\.ignore\spages$/) {
        $ignore_pages = 1;
    } else {
        &syntax_error;
    }
}


##########################   CHECK IF IGNORE PAGES   ##########################

sub check_if_ignore_pages
{
    $ignore_pages && &raise_exception ($ignore_pages_event);
}


#########################   COLLECT TITLE IF PRESENT   ########################

sub collect_title_if_present
{
    #   If the line contains a value between header tags, get that value
    #   and add it to the @toc table.  We don't check that the tags are
    #   correct, and we don't handle multiple titles on the same line.
    #   The /.../i operator compares without regard for case.

    /<H([1-9])>(.*)<\/H[1-9]>/i && do {
        unless ($ignore_header || $1 >= $ignore_level) {
            push (@toc_level, $1);      #   Store header level 1..9
            push (@toc_title, $2);      #   Store header title text
            push (@toc_file, $cur_page);
        }
        $ignore_header = 0;
    };
}


#########################   COLLECT PAGE INFORMATION   ########################

sub collect_page_information
{
    #   .page [<filename> =] ["]<title>["]
    #
    if (&parse_page_command) {
        push (@page_list,  $symbols {"PAGE"});
        push (@page_title, $symbols {"TITLE"});
    }
}


#   Subroutine parses the .page command and sets the symbols PAGE and
#   TITLE appropriately.  The .page command can take various forms:
#
#   .page <filename> = "<title>"    Filename fully specified
#   .page <filename> = <title>      Filename fully specified
#   .page "<title>"                 Filename built from $(BASE)$(INC).$(EXT)
#   .page <title>                   Filename built from $(BASE)$(INC).$(EXT)
#
#   Returns 1 if the .page command was parsed okay, else 0.  Sets $cur_page
#   to the current page filename.

sub parse_page_command {
    if (/^\.\w+\s+(\S+)\s*=\s*"(.*)"/   #   .page <filename> = "title"
    ||  /^\.\w+\s+(\S+)\s*=\s*(.*)/) {  #   .page <filename> = title
        $cur_page = $1;                 #   Keep current output filename
        $symbols {"PAGE"}  = $cur_page;
        $symbols {"TITLE"} = $2;
        1;                              #   Command parsed okay
    } elsif (/^\.\w+\s+"(.*)"/          #   .page "title"
    ||       /^\.\w+\s+(.*)/ ) {        #   .page title
        $cur_page = $symbols {"BASE"}.$sequencer++.".".$symbols {"EXT"};
        $symbols {"PAGE"}  = $cur_page;
        $symbols {"TITLE"} = $1;
        1;                              #   Command parsed okay
    } else {
        &syntax_error;
        0;                              #
    }
}


###########################   OPEN NEW OUTPUT PAGE   ##########################

sub open_new_output_page
{
    #   .page [<filename> =] ["]<title>["]
    #
    &parse_page_command;
    if (open (OUTPUT, ">$cur_page")) {
        print "htmlpp I: creating $cur_page...\n";

        #   Get symbols for first/last/previous/next pages
        #   $(...PAGE) is name of file, for HREF
        #   $(...TITLE) is name of file, for description
        $symbols {"FIRST_PAGE"}  = $page_list  [0];
        $symbols {"FIRST_TITLE"} = $page_title [0];
        $symbols {"LAST_PAGE"}   = $page_list  [@page_list - 1];
        $symbols {"LAST_TITLE"}  = $page_title [@page_title - 1];
        if ($page_nbr < @page_list - 1) {
            $symbols {"NEXT_PAGE"}  = $page_list  [$page_nbr + 1];
            $symbols {"NEXT_TITLE"} = $page_title [$page_nbr + 1];
        } else {
            $symbols {"NEXT_PAGE"}  = "";
            $symbols {"NEXT_TITLE"} = "";
        }
        if ($page_nbr > 0) {
            $symbols {"PREV_PAGE"}  = $page_list  [$page_nbr - 1];
            $symbols {"PREV_TITLE"} = $page_title [$page_nbr - 1];
        } else {
            $symbols {"PREV_PAGE"}  = "";
            $symbols {"PREV_TITLE"} = "";
        }
        $page_nbr++;
    } else {
        print "htmlpp E: ($document $.) can't create $cur_page: $!";
        &raise_exception ($exception_event);
    }
}


##########################   PARSE PAGE TITLE ONLY   ##########################

sub parse_page_title_only
{
    #   .page [<filename> =] ["]<title>["]
    #
    local ($old_page) = $cur_page;      #   Save current page name
    &parse_page_command;                #   Parse .page command
                                        #     and restore old page name
    $symbols {"PAGE"} = $cur_page = $old_page;
}


#########################   ANCHOR TITLE IF PRESENT   #########################

sub anchor_title_if_present
{
    #   If the line contains a value between header tags, add an anchor
    #   tag so that the table of contents can refer to the header.
    #
    /<H([1-9])>(.*)<\/H[1-9]>/i && do {
        unless ($ignore_header || $ignore_level <= $1) {
            $_ = $`."<H$1><A NAME=\"TOC".++$header_nbr."\"></A>$2</H$1>".$';
        }
        $ignore_header = 0;
    };
}


#########################   COPY LINE TO OUTPUT PAGE   ########################

sub copy_line_to_output_page
{
    print OUTPUT "$_\n";
}


##########################   OUTPUT HEADER FOR PAGE   #########################

sub output_header_for_page
{
    &output_block (*header);
}

sub output_block {
    local (*the_block) = @_;            #   Get reference to argument
    local ($saved_line) = $_;           #   We manipulate $_
    local ($line);                      #   Index into block array

    for ($line = 0; $line < @the_block; $line++) {
        $_ = $the_block [$line];
        &expand_symbols_in_line;
        &copy_line_to_output_page;
    }
    $_ = $saved_line;
}


##########################   OUTPUT FOOTER FOR PAGE   #########################

sub output_footer_for_page
{
    &output_block (*footer);
}


##########################   CLEAR SPECIFIED BLOCK   ##########################

sub clear_specified_block
{
    #   .block <name>
    #
    /^\.\w+\s+(\S+)/;                   #   Get name after .block
    CLEAR: {
        $1 eq "header"    && do { *cur_block = *header;    last CLEAR; };
        $1 eq "footer"    && do { *cur_block = *footer;    last CLEAR; };
        $1 eq "toc_open"  && do { *cur_block = *toc_open;  last CLEAR; };
        $1 eq "toc_entry" && do { *cur_block = *toc_entry; last CLEAR; };
        $1 eq "toc_close" && do { *cur_block = *toc_close; last CLEAR; };
        $1 eq "dir_open"  && do { *cur_block = *dir_open;  last CLEAR; };
        $1 eq "dir_entry" && do { *cur_block = *dir_entry; last CLEAR; };
        $1 eq "dir_close" && do { *cur_block = *dir_close; last CLEAR; };
        $1 eq "index"     && do { *cur_block = *index;     last CLEAR; };
        $1 eq "anchor"    && do { *cur_block = *anchor;    last CLEAR; };
        &syntax_error;
    }
    undef @cur_block unless $exception_raised;
}


############################   ADD LINE TO BLOCK   ############################

sub add_line_to_block
{
    push (@cur_block, $_);
}


##########################   BUILD SPECIFIED TABLE   ##########################

sub build_specified_table
{
    #   .build <block_name> <arguments>
    #
    /^\.\w+\s+(\S+)/;                   #   Get name after .build
    BUILD: {
        $1 eq "toc"    && do { &build_table_of_contents; last BUILD; };
        $1 eq "dir"    && do { &build_directory_listing; last BUILD; };
        $1 eq "index"  && do { &build_document_index;    last BUILD; };
        $1 eq "anchor" && do { &build_anchor_definition; last BUILD; };
        &syntax_error;
    }
}

#   .build toc
#
#   Build table of contents for document, using @toc_open, @toc_entry and
#   @toc_close blocks.

sub build_table_of_contents {
    local ($line);                      #   Index into @toc tables
    local ($level);                     #   Current indentation level
    local ($level_base);                #   Minimum indentation level
    local ($header_nbr);                #   Generate header anchors
    local ($reference);                 #   HREF for toc entry

    $level =
    $level_base = $toc_level [0] - 1;
    $ignore_pages && return;            #   No TOC if we're ignoring pages
    for ($line = 0; $line < @toc_title; $line++) {
        #   Close old level in TOC (with @toc_close) if necessary
        while ($toc_level [$line] < $level) {
            &output_block (*toc_close);
            $level--;
        }
        #   Open new level in TOC (with @toc_open) if necessary
        while ($toc_level [$line] > $level) {
            &output_block (*toc_open);
            $level++;
        }
        $reference = $toc_file [$line] eq $cur_page? "": $toc_file [$line];
        $reference .= "#TOC".++$header_nbr;

        #   Update the symbols used to build the table of contents
        $symbols {"TOC_HREF"}  = $reference;
        $symbols {"TOC_TITLE"} = $toc_title [$line];

        &output_block (*toc_entry);
    }
    while ($level > $level_base) {
        &output_block (*toc_close);
        $level--;
    }
}

#   .build dir <directory> [<filename>...]
#
#   Build one or more lines of directory listing, using @dir_open, @dir_entry,
#   and @dir_close blocks.  The filename(s) can be complete names (no path),
#   or regular expressions.  If no filenames are supplied, the entire
#   directory is read.  Assumes $(LOCAL) in front of the directory name;
#   places $(SERVER) in front of the HREF name.

sub build_directory_listing {
    local ($dir);                       #   Directory name
    local ($local_dir);                 #   LOCAL directory name
    local ($files);                     #   List of files, or empty
    local (@filelist);                  #   List of file specifications

    #   Check that $(LOCAL) and $(SERVER) are defined
    unless (defined ($symbols {"LOCAL"})) {
        print "htmlpp E: ($document $.) .define LOCAL is required\n";
        &raise_exception ($exception_event);
    }
    unless (defined ($symbols {"SERVER"})) {
        print "htmlpp E: ($document $.) .define SERVER is required\n";
        &raise_exception ($exception_event);
    }

    #   .build dir <directory> [<filename>...]
    #   Get directory name, and optional list of files from .build command
    ($dir, $files) = /^\.build\s+dir\s+(\S+)\s*(.*)/;
    $files = "*" if $files eq "";       #   If nothing specified, assume *
    $dir   =~ s/\\/\//g;                #   Replace any \ by /

    foreach (split (/\s/, $files)) {    #   Flag each file specified
        if (/^"(.*)"$/) {               #   Quoted filename
            $_ = $1;                    #     may be regular expression
        } else {                        #   Convert normal wildcards
            s/\./\\./g;                 #   . becomes \.
            s/\+/\\+/g;                 #   + becomes \+
            s/\?/./g;                   #   ? becomes .
            s/\*/.*/g;                  #   * becomes .*
        }
        push (@filelist, $_);
    };

    #   Stick $(LOCAL) in front of directory name
    $local_dir = $symbols {"LOCAL"}.$dir;

    #   Process the directory
    if (opendir (DIR, $local_dir)) {
        #   Process each file in the directory except "." and ".."
        &output_block (*dir_open);
        foreach (grep (!/^\.\.?$/, readdir (DIR))) {
            #   Look for file or pattern in @filelist (may get slow!)
            foreach $files (@filelist) {
                if (/$files/) {         #   If we have a match, process $_
                    &build_dir_entry ("$local_dir/$_",
                                       $symbols {"SERVER"}."$dir/$_");
                    last;
                }
            }
        }
        &output_block (*dir_close);
        closedir (DIR);
    } else {
        print "htmlpp E: ($document $.) can't read directory $local_dir\n";
        &raise_exception ($exception_event);
    }
}

sub build_dir_entry {
    local ($lname, $sname) = @_;        #   Get local and server filenames
    local ($ext) = $lname =~ /(\..*$)/; #   Find extension in filename
    $ext =~ tr/A-Z/a-z/;                #     and convert to lowercase
    $ext = ".NONE" if $ext eq "";       #   If no extension, use .NONE
    local (@stats) = stat ("$lname");   #   [7] = file size, [9] = time
    local ($size) = @stats [7];
    local ($sec, $min, $hour, $day, $mon, $year) = localtime (@stats [9]);

    #   Populate symbols and generate @dir_entry block
    $symbols {"DIR_HREF"} = $sname;
    $symbols {"DIR_NAME"} = sprintf ("%-13s", $_);
    $symbols {"DIR_EXT"}  = $ext;
    $symbols {"DIR_SIZE"} = sprintf ("%8d", $size);
    $symbols {"DIR_DATE"} = sprintf ("%2d/%02d/%02d", $year, $mon + 1, $day);
    $symbols {"DIR_TIME"} = sprintf ("%2d:%02d:%02d", $hour, $min, $sec);
    &output_block (*dir_entry);
}

#   .build index
#
#   Build index for document using @index_entry block.  The index lists
#   all pages in the document; a kind of summarised table of contents.
#   Since htmlpp does not (yet) allow nested blocks, you can't build an
#   index inside the page footer, which is a shame, but there you are.

sub build_document_index {
    local ($line);                      #   Index into tables

    $ignore_pages && return;            #   No index if we're ignoring pages
    for ($line = 0; $line < @page_list; $line++) {
        #   Update the symbols used to build the index
        $symbols {"INDEX_PAGE"}  = $page_list  [$line];
        $symbols {"INDEX_TITLE"} = $page_title [$line];
        &output_block (*index);
    }
}

#   .build anchor <anchor_name>
#
#   Create an anchor definition at the specified point in the document,
#   and (re)define the anchor variable appropriately.

sub build_anchor_definition {
    if (/.build\sanchor\s(\S+)/) {      #   Get name after ".build anchor"
        $symbols {"ANCHOR"} = $1;
        $anchors {$1} = $cur_page;
        &output_block (*anchor);
    } else {
        &syntax_error;
    }
}


##########################   SKIP IF BLOCK IF FALSE   #########################

sub skip_if_block_if_false
{
    #   .if <expression>
    #
    if (/^\.\w+\s+(.+)/) {              #   Get expression into $1
        $if_level++;                    #   We started a new .if block
        &skip_conditional_block ("else|endif") unless eval ($1);
    } else {
        &syntax_error;
    }
}

#   We skip input from the document until we close the current block.  If
#   the current block started at an .if, it ends with an .else or an
#   .endif.  If the current block started at an .else, it ends with an
#   .endif only.  We count down $if_level if we find an .endif.  Note that
#   the whole .if block must be in the same file.

sub skip_conditional_block {
    local ($endif) = @_;                #   Get closing token(s)
    local ($level) = 1;                 #   Current nesting level
    local ($line)  = $.;                #   Current input line number

    while (<$document>) {               #   Get next line of input
        $lines_read++;                  #   Count the line
        $level++ if /^\.if/;            #   We found the end of the local
        $level-- if /^\.$endif/;        #     block when $level is zero
        $if_level-- if /^\.endif/;      #   .endif closes the .if block
        last if $level == 0;            #   End of local block
    }
    #   If we ran-out of input, bitch a little
    if ($level > 0) {
        print "htmlpp E: ($document $line) .endif missing\n";
        &raise_exception ($exception_event);
    }
}


##########################   SKIP ELSE BLOCK ALWAYS   #########################

sub skip_else_block_always
{
    &skip_conditional_block ("endif");
}


##########################   CLOSE IF BLOCK IF OPEN   #########################

sub close_if_block_if_open
{
    if (--$if_level < 0) {
        print "htmlpp E: ($document $line) .endif not expected\n";
        &raise_exception ($exception_event);
    }
}


###########################   ECHO TEXT TO CONSOLE   ##########################

sub echo_text_to_console
{
    #   .echo [-] <text>
    #   .echo [-] "<text>"
    #   .echo [-] '<text>'
    #
    /^\.\w+\s+(-\s+)?(.*)/;             #   Parse .echo command
    local ($newline) = $1 eq ""? "\n": "";
    $_ = $2;                            #   Get text after .echo [-]
    $_ = $2 if /^(["'])(.*)\1$/;        #   Remove " or ' if any
    print "$_$newline";                 #   Print text + $newline
}


#########################   SIGNAL COLLECTING TITLES   ########################

sub signal_collecting_titles
{
    print "htmlpp I: collecting table of contents...\n";
}


#########################   SIGNAL FORMATTING PAGES   #########################

sub signal_formatting_pages
{
    print "htmlpp I: formatting document pages...\n";
}


########################   SIGNAL BUILD NOT EXPECTED   ########################

sub signal_build_not_expected
{
    print "htmlpp I: ($document $.) .build ignored before .page\n";
}


#########################   SIGNAL END BLOCK MISSING   ########################

sub signal_end_block_missing
{
    print "htmlpp E: ($document $.) .end required to close .block\n";
}


#########################   SIGNAL BODY TEXT SKIPPED   ########################

sub signal_body_text_skipped
{
    #   Only print the first such message, otherwise the common error of
    #   missing-out a .page causes a whole lot of similar messages.

    $skip_warning == 1 || do {
        print "htmlpp W: ($document $.) HTML text skipped; no open page\n";
        $skip_warning = 1;
    };
}


#########################   SIGNAL END NOT EXPECTED   #########################

sub signal_end_not_expected
{
    print "htmlpp W: ($document $.) .end not expected here - ignored\n";
}


########################   SIGNAL DOCUMENT PROCESSED   ########################

sub signal_document_processed
{
    print "htmlpp I: $lines_read lines processed\n";
}


############################   CLOSE THE DOCUMENT   ###########################

sub close_the_document
{
    #   Close current document, and see if we can unstack a level
    #
    close ($document);
    undef $file_is_open {$document};

    $document = pop (@document_stack);
    $document eq "" || &raise_exception ($continue_event);

    $symbols {"PASS"}++;                #   End of this pass
}


############################   GET EXTERNAL EVENT   ###########################

sub get_external_event
{
}


##########################   TERMINATE THE PROGRAM    #########################

sub terminate_the_program
{
    $the_next_event = $terminate_event;
}
