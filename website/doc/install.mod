#=============================================================================
#
#   install.mod - module file for install.l
<help>
#
#   install     Rebuilds and installs Libero on UNIX systems
#   Syntax:     Korn shell
#
#   Written:    95/03/10  Pieter Hintjens
#   Revised:    95/12/12  Pieter Hintjens
#
#   Requires:   awk, egrep, cut, echo, test, cc, cp.
#
#   This script builds the 'lr' executable, and installs the necessary
#   files into a specified bin directory.  If the script fails for any
#   reason - eg. incompatible shell - then you can build Libero by hand
#   using these commands:
#
#   $ cc lr.c lr????.c -o lr
#   $ cp lr lrmesg.txt lr.ini lrschema.* lrskelet.* <directory>
#
#   The sources must be compiled in ANSI mode.
#
#   Author:     Pieter A. Hintjens
#               Pijlstraat 9
#               2060 Antwerpen, Belgium
#               ph@mymail.com
#               (+323) 231.5277
#
#   FSM Code Generator.  Copyright (c) 1991-95 Pieter A. Hintjens.
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
<end>
#=============================================================================

#   Put definitions here that will go at the start of the script
<data>

CCNAME="cc"                             #   Default C compiler name
#CCNAME="gcc"                           #   If using GNU C

CATALOG=install.cat                     #   List of files in product
DEFAULTDIR=/usr/local/bin               #   Default install directory
VERSION=2.10                            #   Product version

<end>

<code>
##########################   INITIALISE THE PROGRAM   #########################

function initialise_the_program
{
    echo ""
    echo "Libero installation script                       Version $VERSION"
    echo "-------------------------------------------------------------"
    echo "This script installs or de-installs Libero on your UNIX system."
    echo "If you are going to install into a system binary directory you"
    echo "will need write-access to the system directory.  You can also"
    echo "run Libero from the current directory."

    if test -f $CATALOG; then
        the_next_event=$ok_event
    else
        echo "install: $CATALOG not found - cannot continue"
        the_next_event=$error_event
    fi
}


##########################   WHAT SYSTEM ARE WE ON   ##########################

function what_system_are_we_on
{
    #   Detect UNIX system type
    UTYPE=""
    if [ -s /usr/bin/uname       ]; then UTYPE=`/usr/bin/uname`; fi
    if [ -s /bin/uname           ]; then UTYPE=`/bin/uname`; fi

    if [ -s /usr/apollo/bin      ];               then UTYPE=APOLLO; fi
    if [ -s /usr/bin/ncrm        ];               then UTYPE=NCR;    fi
    if [ -s /usr/bin/swconfig    ];               then UTYPE=SCO;    fi
    if [ -s /usr/lib/NextStep/software_version ]; then UTYPE=NEXT;   fi

    if   [ "$UTYPE" = "AIX"        ]; then
        UTYPE=aix
    elif [ "$UTYPE" = "APOLLO"     ]; then
        UTYPE=apollo
    elif [ "$UTYPE" = "A/UX"       ]; then
        UTYPE=apple
    elif [ "$UTYPE" = "BSD/OS"     ]; then
        UTYPE=bsdos
    elif [ "$UTYPE" = "HP-UX"      ]; then
        UTYPE=hpux
    elif [ "$UTYPE" = "IRIX"       ]; then
        UTYPE=irix
    elif [ "$UTYPE" = "Linux"      ]; then
        UTYPE=linux
    elif [ "$UTYPE" = "NCR"        ]; then
        UTYPE=ncr
    elif [ "$UTYPE" = "NetBSD"     ]; then
        UTYPE=netbsd
    elif [ "$UTYPE" = "NEXT"       ]; then
        UTYPE=next
    elif [ "$UTYPE" = "OSF1"       ]; then
        UTYPE=osf1
    elif [ "$UTYPE" = "SCO"        ]; then
        UTYPE=sco
    elif [ "$UTYPE" = "SMP_DC.OSx" ]; then
        UTYPE=pyramid
    elif [ "$UTYPE" = "SunOS"      ]; then
        UTYPE=sunos
    elif [ "$UTYPE" = "ULTRIX"     ]; then
        UTYPE=ultrix
    else
        UTYPE=unix
    fi

    #   Set compiler options according to UNIX system type
    if   [ "$CCNAME" = "gcc"  ]; then
        CCOPTS="-O2 -Wall -pedantic"
    elif [ "$UTYPE" = "aix"   ]; then
        CCOPTS="-O2"
    elif [ "$UTYPE" = "linux" ]; then
        CCOPTS="-O2 -Wall -pedantic"
    elif [ "$UTYPE" = "hpux"  ]; then
        CCOPTS="-O -Aa"
    elif [ "$UTYPE" = "sunos" ]; then
        CCOPTS="-O -vc -Xa"
    else
        CCOPTS=""
    fi

    echo ""
    echo "UNIX system=\"$UTYPE\", compiler=\"$CCNAME\", options=\"$CCOPTS\""
    echo "-------------------------------------------------------------"
}


#########################   WHAT PACKAGE DO WE HAVE   #########################

function what_package_do_we_have
{
    if test -f lr.c; then
        the_next_event=$source_event
    else
        the_next_event=$binary_event
    fi
}


########################   GET SOURCE INSTALL ACTION   ########################

function get_source_install_action
{
    echo ""
    echo "Choose an action:"
    echo "  (b)uild Libero and optionally install"
    echo "  (d)e-install Libero"
    echo "  (c)lean-up current directory"
    echo "  (q)uit"
    echo ""
    echo "Choice:"
    read ACTION

    if   test "$ACTION" = "b"; then
        the_next_event=$build_event
    elif test "$ACTION" = "d"; then
        the_next_event=$delete_event
    elif test "$ACTION" = "c"; then
        the_next_event=$cleanup_event
    elif test "$ACTION" = "q"; then
        the_next_event=$quit_event
    else
        the_next_event=$error_event
    fi
}


########################   GET BINARY INSTALL ACTION   ########################

function get_binary_install_action
{
    echo ""
    echo "Choose an action:"
    echo "  (i)nstall Libero in target directory"
    echo "  (d)e-install Libero"
    echo "  (q)uit"
    echo ""
    echo "Choice:"
    read ACTION

    if   test "$ACTION" = "i"; then
        the_next_event=$install_event
    elif test "$ACTION" = "d"; then
        the_next_event=$delete_event
    elif test "$ACTION" = "q"; then
        the_next_event=$quit_event
    else
        the_next_event=$error_event
    fi
}


##########################   CHECK ALL FILES EXIST   ##########################

function check_all_files_exist
{
    echo "Checking for required files..."
    for FILE in `awk '/^&.*@m/ {print $2}' $CATALOG`; do
        if test ! -f $FILE; then
            echo "install: missing $FILE - " \
                  `egrep "& $FILE " $CATALOG | cut -f2 -d%`
            raise_exception $error_event
        fi
    done
}


#########################   COMPILE ALL SUBROUTINES   #########################

function compile_all_subroutines
{
    rm -f *.o
    for FILE in `awk '/^&.*@c/ {print $2}' $CATALOG`; do
        echo "Compiling $FILE..."
        $CCNAME -c $CCOPTS $FILE
    done
}


############################   LINK MAIN PROGRAMS   ###########################

function link_main_programs
{
    for FILE in `awk '/^&.*@l/ {print $2}' $CATALOG`; do
        echo "Compiling and linking $FILE..."
        test -f $FILE.o && rm $FILE.o
        $CCNAME -o $FILE $CCOPTS $FILE.c *.o
    done
}


############################   GET DIRECTORY NAME   ###########################

function get_directory_name
{
    echo "Enter the install directory:"
    echo "  (Enter)$DEFAULTDIR"
    echo "  (.)$PWD"
    echo "  (q)uit"
    echo ""
    echo "Choice:"
    read INSTALLDIR

    if test "$INSTALLDIR" = ""; then
        INSTALLDIR=$DEFAULTDIR
        the_next_event=$ok_event

    elif test "$INSTALLDIR" = "."; then
        echo "install: leaving files in current directory"
        the_next_event=$current_event

    elif test "$INSTALLDIR" = "q"; then
        the_next_event=$quit_event

    elif test -d "$INSTALLDIR"; then
        the_next_event=$ok_event

    else
        echo "install: $INSTALLDIR is not a directory"
        the_next_event=$error_event
    fi
}


#########################   CHECK DIRECTORY WRITABLE   ########################

function check_directory_writable
{
    if test ! -w "$INSTALLDIR"; then
        echo "install: you do not have write access for $INSTALLDIR"
        raise_exception $error_event
    fi
}


############################   CLEAN UP DIRECTORY   ###########################

function clean_up_directory
{
    for FILE in `awk '/^&.*@d/ {print $2}' $CATALOG`; do
        echo "Deleting $FILE: " \
             `egrep "& $FILE " $CATALOG | cut -f2 -d%`
        test -f $FILE && rm -f $FILE
    done
}


##########################   INSTALL PRODUCT FILES   ##########################

function install_product_files
{
    echo "Installing files into $INSTALLDIR..."
    for FILE in `awk '/^&.*@i/ {print $2}' $CATALOG`; do
        echo "$FILE: " \
             `egrep "& $FILE " $CATALOG | cut -f2 -d%`
        cp $FILE $INSTALLDIR
        chmod a+r $FILE
    done

    for FILE in `awk '/^&.*@x/ {print $2}' $CATALOG`; do
        echo "$FILE: " \
             `egrep "& $FILE " $CATALOG | cut -f2 -d%`
        cp $FILE $INSTALLDIR
        chmod a+rx $FILE
    done
}


###########################   DELETE PRODUCT FILES   ##########################

function delete_product_files
{
    echo "Deleting files from $INSTALLDIR..."
    for FILE in `awk '/^&.*@[ix]/ {print $2}' $CATALOG`; do
        if test ! -f $INSTALLDIR/$FILE; then
            echo "install: $INSTALLDIR/$FILE does not exist - skipping"
        else
            echo "$FILE: " \
                 `egrep "& $FILE " $CATALOG | cut -f2 -d%`
            rm -f $INSTALLDIR/$FILE
        fi
    done
}


##########################   RETURN ERROR FEEDBACK   ##########################

function return_error_feedback
{
    echo "install: aborted"
    echo ""
    feedback=1
}


############################   GET EXTERNAL EVENT   ###########################

function get_external_event
{
    return
}


##########################   TERMINATE THE PROGRAM    #########################

function terminate_the_program
{
    the_next_event=$terminate_event
}
