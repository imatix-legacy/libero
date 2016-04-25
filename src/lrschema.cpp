!============================================================================*
!                                                                            *
!   lrschema.cpp LIBERO code generation schema for standard C++ programs     *
!                                                                            *
!   Written:    96/05/24  Pieter Hintjens <ph@imatix.com>                    *
!   Revised:    96/12/27                                                     *
!                                                                            *
!   Thanks to:  Pascal Anntonaux <pascal@imatix.com>                         *
!               Mathias Weidner <mathias@weidner.sem.lipsia.de>              *
!                                                                            *
!   Usage:      Generates these files:                                       *
!               $source.cpp     Program source (first time only)             *
!               $source.h       Class definitions (update in pieces)         *
!               $dialog.i       Interpreter code                             *
!                                                                            *
!               Accepts these options:                                       *
!               -opt:stack_max=nnn - size of subdialog stack (20)            *
!               -opt:template=xxxx - template file (template.cpp)            *
!               -opt:class='xxxx'  - class name (C$source)                   *
!                                                                            *
!   FSM Code Generator.  Copyright (c) 1991-97 iMatix.                       *
!                                                                            *
!   This program is free software; you can redistribute it and/or modify     *
!   it under the terms of the GNU General Public License as published by     *
!   the Free Software Foundation; either version 2 of the License, or        *
!   (at your option) any later version.                                      *
!                                                                            *
!   This program is distributed in the hope that it will be useful,          *
!   but WITHOUT ANY WARRANTY; without even the implied warranty of           *
!   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
!   GNU General Public License for more details.                             *
!                                                                            *
!   You should have received a copy of the GNU General Public License        *
!   along with this program; if not, write to the Free Software              *
!   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.                *
!============================================================================*

!   You can use the template file to include project-specific code in the
!   generated code at various points.  This is a (better) alternative to
!   changing the schema in order to add your own project code.
!
!   The template file should contain these sections (each starts with a
!   keyword on a line, and ends with <END> on a line).  These sections may
!   be included in newly-generated code (keywords are case-sensitive):
!
!   <HEADER>                    - program header comment
!   <DATA>                      - data definitions
!   <HEADER_DEFINITIONS>        - additional includes and the like
!   <CLASS_PROTECTED>           - data used by derived classes
!   <CLASS_CONSTANTS>           - array sizes ...
!   <CLASS_INTERFACE>           - class interface
!   <CLASS_PRIVATE>             - private class data/functions
!   <CLASS_PROTECTED>           - data used by derived classes
!   <CLASS_INLINES>             - for class inlines
!   <CONSTRUCTOR>               - default constructor
!   <DESTRUCTOR>                - destructor
!   <START_EXECUTE>             - before dialog gets executed
!   <INTERFACE>                 - additional class interfaces
!   <Initialise-The-Program>    - in Initialise-The-Program
!   <Get-External-Event>        - in Get-External-Event
!   <Terminate-The-Program>     - in Terminate-The-Program
!   <Any-Dialog-Module>         - any dialog module
!
:declare string $template = "template.cpp"
:declare string $class    = "C$source"

:if not exist $source.cpp
:echo "lr I: creating skeleton program $source.cpp..."
:output $source.cpp
/*===========================================================================*
 *                                                                           *
 *  $source.cpp  description...                                              *
 *                                                                           *
 *  Written:    $date       $author                                          *
 *  Revised:    $date                                                        *
 *                                                                           *
:include optional $template "<HEADER>" "<END>"
 *  Skeleton generated by LIBERO $version on $fulldate, $time.               *
 *===========================================================================*/

#include "$source.h"                    //  Include class definitions

:include optional $template "<DATA>" "<END>"


///////////////////////////   C O N T R U C T O R   ///////////////////////////

$class::$class (void)
{
:include optional $template "<CONSTRUCTOR>" "<END>"
}


////////////////////////////   D E S T R U C T O R   //////////////////////////

$class::~$class (void)
{
:include optional $template "<DESTRUCTOR>" "<END>"
}


//////////////////////////////////   M A I N   ////////////////////////////////

int $class::$"execute" (char *$"p_argument")
{
    int
        $"feedback";

:include optional $template "<START_EXECUTE>" "<END>"
#   include "$dialog.i"                 //  Include dialog interpreter
}

:include optional $template "<INTERFACE>" "<END>"


//////////////////////////   INITIALISE THE PROGRAM   /////////////////////////

MODULE $class::$"initialise_the_program" (void)
{
    $"the_next_event" = $"ok_event";
:include optional $template "<Initialise-The-Program>" "<END>"
}


////////////////////////////   GET EXTERNAL EVENT   ///////////////////////////

MODULE $class::$"get_external_event" (void)
{
:include optional $template "<Get-External-Event>" "<END>"
}

:close
:endif

:if not exist $source.h
:output $source.h
//%START DEFINITIONS
//%END DEFINITIONS
//%START CLASS
        //  This is the function that executes the program.  You may want
        //  to change the function name, and you'll probably want to change
        //  the arguments.  The prototype must agree with that in the .cpp.
        int execute (char *argument);   //  Executive
//%END CLASS
:close $source.h
:endif

:output $source.001
:include $source.h "//%START DEFINITIONS" "//%END DEFINITIONS"
:close $source.001
:output $source.002
:include $source.h "//%START CLASS" "//%END CLASS"
:close $source.002

:output $source.h
:echo "lr I: building $source.h..."
/*---------------------------------------------------------------------------*
 *  $source.h - Class definitions for $source.cpp.                           *
 *  Generated by LIBERO $version on $fulldate, $time.                        *
 *  Schema file used: $schema.                                               *
 *  Modify this code ONLY between the %START and %END markers                *
 *---------------------------------------------------------------------------*/

#ifndef _$Source\_INCLUDED              //  Allow multiple inclusions
#define _$Source\_INCLUDED

#include "prelude.h"                    //  Universal header file

:include optional $template "<HEADER_DEFINITIONS>" "<END>"

//  Definitions, classes, types that you need to export
//%START DEFINITIONS
:include $source.001
:delete  $source.001
//%END DEFINITIONS

//  Definitions that we need to run the state machine
#ifndef MODULE
#define MODULE  void                    //  Libero dialog modules
#endif
typedef int event_t;                    //  Type for dialog entity

//- Class definition --------------------------------------------------------

class $class
{
    //  Class constants
:include optional $template "<CLASS_CONSTANTS>" "<END>"
    //  Function prototypes
    public:
        $class      (void);             //  Constructor
       ~$class      (void);             //  Destructor
    //  Functions and data that you want to add to the class
//%START CLASS
:include $source.002
:delete  $source.002
//%END CLASS
:include optional $template "<CLASS_INTERFACE>" "<END>"
    private:
        void   $"raise_exception"         (event_t event);
        MODULE $"initialise_the_program"  (void);
        MODULE $"get_external_event"      (void);
:do module
        MODULE $name                      (void);
:enddo
:include optional $template "<CLASS_PRIVATE>" "<END>"
    //  Variables used by dialog interpreter
    protected:
        event_t
            _LR_event,                  //  Event for state transition
            _LR_state,                  //  Current dialog state
            _LR_savest,                 //  Saved dialog state
            _LR_index,                  //  Index into vector table
            $"the_next_event",          //  Next event from module
            $"the_exception_event";     //  Exception event from module
        word
            *_LR_vecptr;                //  Pointer into vector table
        Bool
            $"exception_raised";        //  TRUE if exception raised
:if module "Dialog-Call"
:  if not event "Return"
:     echo "lr E: you must define the 'Return' event"
:     exit 1
:  endif
:  declare int $stack_max = 20
        event_t
            _LR_stack [$stack_max];     //  Subdialog call stack
        int
            _LR_stack_size;             //  Subdialog stack size; 0=empty
:endif
:include optional $template "<CLASS_PROTECTED>" "<END>"
};

:include optional $template "<CLASS_INLINES>" "<END>"

//  The dialog modules are defined as class functions
typedef void ($class::*classfunc) (void);

/*- Symbolic constants and event numbers ------------------------------------*/

:if check
#define _LR_STOP                        0xFFFFL
#define _LR_NULL_EVENT                  -2
:endif
enum {
    $"terminate_event" = -1,
:do event
    $name = $number$comma
:enddo
};

enum {
:if defaults
    _LR_defaults_state = $defaults,
:endif
:do state
    _LR_STATE_$name = $number$comma
:enddo
};

#endif                                  //  If included

:output $dialog.i
:echo "lr I: building $dialog.i..."
/*---------------------------------------------------------------------------*
 *  $dialog.i - LIBERO dialog interpreter for $source.cpp.                   *
 *  Generated by LIBERO $version on $fulldate, $time.                        *
 *---------------------------------------------------------------------------*/

    //- Static areas --------------------------------------------------------

    static word _LR_nextst [][$events] = {
:do nextst
:  if $overflow = 0
        { $row }$comma
:  else
        { $row
:    do overflow
          $row
:    enddo
        }$comma
:  endif
:enddo
    };

    static word _LR_action [][$events] = {
:set array_base=1
:do action
:  if $overflow = 0
        { $row }$comma
:  else
        { $row
:    do overflow
          $row
:    enddo
        }$comma
:  endif
:enddo
:set array_base=0
    };

    static word _LR_vector [][$maxaction + 1] = {
        {0},
:do vector
:  if "$row" = ""
        {_LR_STOP}$comma
:  else
        {$row,_LR_STOP}$comma
:  endif
:enddo
    };

    static classfunc _LR_module [] = {
:do module
        &$class::$name$comma
:enddo
    };

:if animate
:push $style
:option -style=normal
    static char *_LR_mname [$modules] = {
:  do module
        "$name"$comma
:  enddo
    };

    static char *_LR_sname [$states] = {
:  do state
        "$name"$comma
:  enddo
    };

    static char *_LR_ename [$events] = {
:  do event
        "$name"$comma
:  enddo
    };
:pop $style
:option -style=$style
:endif

    //- Dialog interpreter starts here --------------------------------------

    _LR_state = 0;                      //  First state is always zero
:if module "Dialog-Call"
    _LR_stack_size = 0;                 //  Clear subdialog stack
:endif
    $"initialise_the_program" ();
    while ($"the_next_event" != $"terminate_event")
      {
        _LR_event = $"the_next_event";
:if check
        if (_LR_event >= $events || _LR_event < 0)
          {
            cout << "State "  << _LR_state << " - event ";
            cout << _LR_event << " is out of range" << endl;
            break;
          }
:endif
        _LR_savest = _LR_state;
        _LR_index  = _LR_action [_LR_state][_LR_event];
:if defaults
        //  If no action for this event, try the defaults state
        if (_LR_index == 0)
          {
            _LR_state = _LR_defaults_state;
            _LR_index = _LR_action [_LR_state][_LR_event];
          }
:endif
:if animate
        cout << endl << _LR_sname [_LR_state] << ':' << endl;
        cout << "    (--) " << _LR_ename [_LR_event] << endl;
:endif
:if check
        if (_LR_index == 0)
          {
            cout << "State "  << _LR_state << " - event ";
            cout << _LR_event << " is not accepted" << endl;
            break;
          }
        $"the_next_event"       = _LR_NULL_EVENT;
:endif
        $"the_exception_event"  = _LR_NULL_EVENT;
        $"exception_raised"     = FALSE;
        _LR_vecptr = _LR_vector [_LR_index];

        FOREVER
          {
            if ((*_LR_vecptr == _LR_STOP)
            || ($"exception_raised"))
                break;
:if animate
            cout << "          + " << _LR_mname [*_LR_vecptr] << endl;
:endif
            (this->*_LR_module [*_LR_vecptr++]) ();
          }
        if ($"exception_raised")
          {
            if ($"the_exception_event" != _LR_NULL_EVENT)
                _LR_event = $"the_exception_event";
            $"the_next_event" = _LR_event;
:if animate
            cout << "    (=>) " << _LR_ename [_LR_event] << endl;
:endif
          }
        else
            _LR_state = _LR_nextst [_LR_state][_LR_event];

:if defaults
        if (_LR_state == _LR_defaults_state)
            _LR_state = _LR_savest;
:endif
        if ($"the_next_event" == _LR_NULL_EVENT)
          {
            this-> $"get_external_event" ();
:if check
            if ($"the_next_event" == _LR_NULL_EVENT)
              {
                cout << "No event set after event " << _LR_event;
                cout << " in state " << _LR_state << endl;
                break;
              }
:endif
          }
      }
    return ($"feedback");
}

:if module "Dialog-Call"
MODULE $class::$"dialog_call" (void)
{
    if (_LR_stack_size < $stack_max)
        _LR_stack [_LR_stack_size++] = _LR_state;
    else
      {
        cout << "State " << _LR_state << " - Dialog-Call overflow" << endl;
        exit (1);
      }
}

MODULE $class::$"dialog_return" (void)
{
    if (_LR_stack_size > 0)
      {
        _LR_state = _LR_stack [--_LR_stack_size];
        $"raise_exception" ($"return_event");
      }
    else
      {
        cout << "State " << _LR_state << " - Dialog-Return underflow" << endl;
        exit (1);
      }
}

:endif
//- Standard dialog routines ------------------------------------------------

void $class::$"raise_exception" (event_t event)
{
    $"exception_raised" = TRUE;
    if (event >= 0)
        $"the_exception_event" = event;
:close

!   Generate stubs for all modules not yet defined in source file

:internal initialise_the_program
:internal get_external_event
:set module_line="MODULE $class::%s (void)"
:set stub_first="/"
:set stub_between="/"
:set stub_last="/"
:do stubs $source.cpp $dialog.i

MODULE $class::$name (void)
{
:include optional $template "<$module_name>" "<END>"
}

:enddo
