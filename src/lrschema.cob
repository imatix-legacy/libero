000100! 								POS00110
000200!  LRSCHEMA.COB - Libero code generation schema for COBOL 	95/03/31
000300! 								95/03/27
000400!  Written:    95/03/27  Pieter Hintjens <ph@imatix.com>		95/03/31
000500!  Revised:    96/12/29						96/04/03
000600! 								95/03/27
000700!  Usage:      Generates single copybook, with extension '.cob'.  95/03/31
000800! 	     By default, generates ANSI 74 COBOL, for a main	95/03/31
000900! 	     program (without linkage).  Accept these options:	95/03/31
001000! 								95/03/31
001100! 	     -opt:model=main   - generate main program (default)95/03/31
001200! 	     -opt:model=called - generate called program	95/06/25
001300! 	     -opt:level=ansi74 - generate ANSI74 code (default) 95/03/31
001400! 	     -opt:level=ansi85 - generate ANSI85 code		95/03/31
001500!              -opt:ext=cbl      - use extension '.cbl' (default) 95/03/31
001600!              -opt:ext=xxx      - use extension '.xxx'           95/03/31
001700!              -opt:console=""   - suffix for DISPLAY verb        95/10/01
001800! 	     -opt:stack_max=n  - subdialog stack size (20)	95/11/02
001900! 	     -opt:template=xxx - template file (TEMPLATE.cob)	95/12/18
002000! 								95/03/27
002100! 	     I recommend that your dialog carries the program	95/03/27
002200!              name followed by 'd', with extension '.l'.         95/03/27
002300! 								95/03/27
002400! 	     Assumes linkage section in $SOURCE\R.$ext. 	95/12/08
002500! 								95/03/27
003200!  FSM Code Generator.  Copyright (c) 1991-97 iMatix.		95/12/18
003300! 								95/03/27
003400!  This program is free software; you can redistribute it and/or	95/03/29
003500!  modify it under the terms of the GNU General Public License as 95/03/29
003600!  published by the Free Software Foundation; either version 2 of 95/03/29
003700!  the License, or (at your option) any later version.		95/03/29
003800! 								95/03/27
003900!  This program is distributed in the hope that it will be useful,95/03/29
004000!  but WITHOUT ANY WARRANTY; without even the implied warranty of 95/03/29
004100!  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the	95/03/29
004200!  GNU General Public License for more details.			95/03/29
004300! 								95/03/27
004400!  You should have received a copy of the GNU General Public	95/03/29
004500!  License along with this program; if not, write to the Free	95/03/29
004600!  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139,	95/03/29
004700!  USA.								95/03/29
004800!=================================================================95/03/27
004900									95/03/31
005000:set array_base = 1						95/12/08
005100:set row_width  = 48		   # If $row is longer, wrap	95/12/08
005200									95/12/08
005300:declare string ext = "cbl"          # works best on Unix         95/12/08
005400:declare string level = "ansi74"     # most portable code         95/12/08
005500:declare string model = "main"       # main program               95/12/08
005600:declare string console = ""         # e.g. 'UPON TERMINAL'       95/12/08
005700:declare string template="TEMPLATE.cob"                           95/12/18
005800:option -style=cobol -noidle					95/12/08
005900									95/12/08
006000!  Check that program name and dialog name are different		95/03/29
006100									95/03/31
006200:if "$source" = "$dialog"                                         95/10/01
006300:    echo "lr E: dialog cannot have same name as source file"     95/04/01
006400:    exit 1							95/04/01
006500:endif								95/03/31
006600									95/03/31
006700!  Generate skeleton program if none already exists		95/03/29
006800									95/03/31
006900:if not exist $SOURCE.cob 					95/04/01
007000:echo "lr I: creating skeleton program $SOURCE.cob..."            95/04/01
007100:output $SOURCE.cob						95/04/01
007200 IDENTIFICATION DIVISION. 					95/03/29
007300 PROGRAM-ID.    $SOURCE.						95/03/29
007400									95/03/29
007500 AUTHOR.	      $AUTHOR.						95/04/01
007600 DATE-WRITTEN.  $DATE						95/03/29
007700	   -REVISED:  $DATE.						95/03/29
007800:include optional $template "<HEADER>" "<END>"                    95/12/18
007900									95/12/18
008000 ENVIRONMENT DIVISION.						95/03/29
008100									95/03/29
008200 CONFIGURATION SECTION.						95/03/29
008300 SOURCE-COMPUTER. PORTABLE.					95/03/29
008400 OBJECT-COMPUTER. PORTABLE.					95/03/29
008500									95/03/29
008600 DATA DIVISION.							95/03/29
008700									95/03/29
008800 WORKING-STORAGE SECTION. 					95/03/29
008900:include optional $template "<DATA>" "<END>"                      95/12/18
009000									95/12/18
009100*DIALOG DATA AND INTERPRETER.					95/03/29
009200 COPY $DIALOG.							95/03/29
009300									95/03/29
009400*******************   INITIALISE THE PROGRAM   *******************95/08/07
009500									95/03/29
009600 INITIALISE-THE-PROGRAM.						95/03/29
009700	   MOVE OK-EVENT TO THE-NEXT-EVENT				95/03/29
009800:include optional $template "<Initialise-The-Program>" "<END>"    95/12/18
009900	   .								95/03/29
010000									95/03/29
010100*********************   GET EXTERNAL EVENT   *********************95/03/31
010200									95/03/31
010300 GET-EXTERNAL-EVENT.						95/03/31
010400:include optional $template "<Get-External-Event>" "<END>"        95/12/18
010500:if $included = 0 						95/12/18
010600	   EXIT 							95/03/31
010700:endif								95/12/18
010800	   .								95/03/31
010900									95/03/31
011000********************   TERMINATE THE PROGRAM   *******************95/08/07
011100									95/03/29
011200 TERMINATE-THE-PROGRAM.						95/03/29
011300	   MOVE TERMINATE-EVENT TO THE-NEXT-EVENT			95/03/29
011400:include optional $template "<Terminate-The-Program>" "<END>"     95/12/18
011500	   .								95/03/29
011600:close								95/04/01
011700:endif								95/04/01
011800:if "$model" = "called"                                           96/04/03
011900:if not exist "$SOURCE\R.$ext"                                    95/12/08
012000:echo "lr I: creating linkage copybook $SOURCE\R.$ext..."         95/12/08
012100:output $SOURCE\R.$ext						95/12/08
012200*    Copybook for calling $SOURCE 				95/12/08
012300* 								95/12/08
012400*    Generated: $date	Libero $version 			95/12/08
012500*    Revised:   $date	$author 				95/12/08
012600* 								95/12/08
012700*    To use:    place operation code in $SOURCE-CONTROL and	95/12/08
012800*               CALL "$SOURCE"                                    95/12/08
012900* 		  USING $SOURCE-CONTROL 			95/12/08
013000* 								95/12/08
013100*    Returns:   If $SOURCE-FEEDBACK = SPACE, there were no	95/12/08
013200* 	      errors.  Else $SOURCE-FEEDBACK indicates the	95/12/08
013300* 	      cause or nature of the error.			95/12/08
013400* 								95/12/08
013500 01  $SOURCE-CONTROL.						95/12/08
013600*CONTENTS 							95/12/08
013700	   02  $SOURCE-OPERATION       PIC X	  VALUE SPACE.		95/12/08
013800	   02  $SOURCE-FEEDBACK        PIC X	  VALUE SPACE.		95/12/08
013900:close								95/12/08
014000:endif								95/12/08
014100:endif								96/04/03
014200									95/03/31
014300:output $DIALOG.$ext						95/03/29
014400:echo "lr I: building $DIALOG.$ext..."                            95/03/31
014500*----------------------------------------------------------------*95/03/28
014600*  $DIALOG.$ext - Libero dialog definitions for $SOURCE	       *95/03/29
014700*  Generated by Libero $version on $fulldate, $time.	       *95/03/29
014800*  Schema file used: $schema				       *95/10/01
014900*----------------------------------------------------------------*95/10/01
015000									95/03/27
015100 01  LR--DIALOG-CONSTANTS.					95/03/29
015200	   02  TERMINATE-EVENT	       PIC S9(3)  COMP VALUE -1.	95/10/01
015300:if check 							95/03/30
015400	   02  LR--NULL-EVENT	       PIC S9(3)  COMP VALUE ZERO.	95/10/01
015500:endif								95/03/30
015600:do event 							95/03/30
015700	   02  $NAME		       PIC S9(3)  COMP VALUE +$number.	95/03/29
015800:enddo								95/03/30
015900	   02  LR--DEFAULTS-STATE      PIC S9(3)  COMP VALUE +$defaults.95/04/01
016000:do state 							95/10/01
016100	   02  LR--STATE-$NAME	       PIC S9(3)  COMP VALUE +$number.	95/05/03
016200:enddo								95/03/30
016300									95/03/27
016400 01  LR--DIALOG-VARIABLES.					95/03/29
016500	   02  LR--EVENT	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
016600	   02  LR--STATE	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
016700	   02  LR--SAVEST	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
016800	   02  LR--INDEX	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
016900	   02  LR--VECPTR	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
017000	   02  LR--MODNBR	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
017100	   02  THE-NEXT-EVENT	       PIC S9(3)  COMP VALUE ZERO.	95/03/29
017200	   02  THE-EXCEPTION-EVENT     PIC S9(3)  COMP VALUE ZERO.	95/03/29
017300	   02  EXCEPTION-RAISED        PIC X	  VALUE SPACE.		95/03/29
017400         88  EXCEPTION-IS-RAISED            VALUE "Y".            95/03/29
017500:if module "Dialog-Call"                                          95/11/02
017600:  if not event "Return"                                          95/11/02
017700:     echo "lr E: you must define the 'Return' event              95/11/02
017800:     exit 1							95/11/02
017900:  endif								95/11/02
018000:  declare int stack_max = 20					95/11/02
018100	   02  LR--STACK-SIZE	       PIC S9(3)  COMP. 		95/11/02
018200	   02  LR--STACK	       PIC S9(3)  COMP			95/11/02
018300						  OCCURS $stack_max.	95/11/04
018400:endif								95/11/04
018500									95/11/04
018600:declare int iw		       # size of item in row		95/03/31
018700:declare int rw		       # size of this row		95/03/31
018800:if $states < 10							95/03/30
018900:  set iw=1							95/03/31
019000:else								95/03/30
019100:if $states < 100 						95/03/31
019200:  set iw=2							95/03/31
019300:else			       #  assume max 999 states :-0	95/03/31
019400:  set iw=3							95/03/31
019500:endif all							95/03/31
019600:set number_fmt = "%ld"                                           95/03/31
019700:set row_first  = "%0$iw\ld"                                      95/03/31
019800:set row_after  = "%0$iw\ld"                                      95/03/31
019900:set number_fmt = "%03ld"                                         95/03/31
020000: 								95/03/30
020100 01  LR--NEXT-STATES.						95/03/29
020200:do nextst							95/03/30
020300:  set rw=$tally * $iw						95/03/31
020400:  if $rw > 12							95/03/31
020500	   02  FILLER		       PIC X($rw) VALUE 		95/03/31
020600         "$row".                                                  95/03/29
020700:  else								95/03/31
020800     02  FILLER                  PIC X($rw) VALUE "$row".         95/03/31
020900:  endif								95/03/31
021000:  do overflow							95/03/31
021100:    set rw=$tally * $iw						95/03/31
021200:    if $rw > 12							95/03/31
021300	   02  FILLER		       PIC X($rw) VALUE 		95/03/31
021400         "$row".                                                  95/03/31
021500:    else 							95/03/31
021600     02  FILLER                  PIC X($rw) VALUE "$row".         95/03/31
021700:    endif							95/03/31
021800:  enddo								95/03/31
021900:enddo								95/03/30
022000 01  FILLER		       REDEFINES  LR--NEXT-STATES.	95/03/29
022100	   02  FILLER				  OCCURS $states TIMES. 95/03/29
022200	       03  LR--NEXTST	       PIC 9($iw) OCCURS $events TIMES. 95/03/31
022300									95/03/29
022400:if $vectors < 10 						95/03/31
022500:  set iw=1							95/03/31
022600:else								95/03/31
022700:if $vectors < 100						95/03/31
022800:  set iw=2							95/03/31
022900:else								95/03/31
023000:  set iw=3							95/03/31
023100:endif all							95/03/31
023200:set number_fmt = "%ld"                                           95/03/31
023300:set row_first  = "%0$iw\ld"                                      95/03/31
023400:set row_after  = "%0$iw\ld"                                      95/03/31
023500:set number_fmt = "%03ld"                                         95/03/31
023600: 								95/03/31
023700 01  LR--ACTIONS. 						95/03/29
023800:do action							95/03/30
023900:  set rw = $tally * $iw						95/03/31
024000:  if $rw > 12							95/03/31
024100	   02  FILLER		       PIC X($rw) VALUE 		95/03/31
024200         "$row".                                                  95/03/31
024300:  else								95/03/31
024400     02  FILLER                  PIC X($rw) VALUE "$row".         95/03/31
024500:  endif								95/03/31
024600:  do overflow							95/03/31
024700:    set rw=$tally * $iw						95/03/31
024800:    if $rw > 12							95/03/31
024900	   02  FILLER		       PIC X($rw) VALUE 		95/03/31
025000         "$row".                                                  95/03/31
025100:    else 							95/03/31
025200     02  FILLER                  PIC X($rw) VALUE "$row".         95/03/31
025300:    endif							95/03/31
025400:  enddo								95/03/31
025500:enddo								95/03/30
025600 01  FILLER		       REDEFINES  LR--ACTIONS.		95/03/29
025700	   02  FILLER				  OCCURS $states TIMES. 95/03/29
025800	       03  LR--ACTION	       PIC 9($iw) OCCURS $events TIMES. 95/03/31
025900									95/03/29
026000 01  LR--OFFSETS. 						95/03/29
026100:do vector							95/04/09
026200	   02  FILLER		       PIC S9(3)  COMP VALUE +$offset.	95/04/09
026300:enddo								95/03/30
026400 01  FILLER		       REDEFINES  LR--OFFSETS.		95/03/29
026500	   02  LR--OFFSET	       PIC S9(3)  OCCURS $vectors COMP. 95/11/04
026600									95/03/29
026700:declare int tblsize = 0	       # total size of table		95/03/31
026800:declare string null						95/03/31
026900:if $modules < 10 						95/03/31
027000:  set iw=1							95/03/31
027100:  set null="0"                                                   95/03/31
027200:else								95/03/31
027300:if $modules < 100						95/03/31
027400:  set iw=2							95/03/31
027500:  set null="00"                                                  95/03/31
027600:else								95/03/31
027700:  set iw=3							95/03/31
027800:  set null="000"                                                 95/03/31
027900:endif all							95/03/31
028000:set number_fmt = "%ld"                                           95/03/31
028100:set row_first  = "%0$iw\ld"                                      95/03/31
028200:set row_after  = "%0$iw\ld"                                      95/03/31
028300:set number_fmt = "%03ld"                                         95/03/31
028400: 								95/03/31
028500 01  LR--MODULES. 						95/03/29
028600:do vector							95/03/30
028700:  set rw = $tally * $iw						95/03/31
028800:  if $rw > 28							95/03/31
028900	   02  FILLER		       PIC X($rw) VALUE 		95/03/31
029000:    if "$row" = ""                                               95/12/11
029100         "$null".                                                 95/12/11
029200:    else 							95/12/11
029300         "$row$null".                                             95/12/11
029400:    endif							95/12/11
029500:  else								95/03/30
029600:    if "$row" = ""                                               95/12/11
029700     02  FILLER  PIC X($rw) VALUE "$null".                        95/12/11
029800:    else 							95/12/11
029900     02  FILLER  PIC X($rw) VALUE "$row$null".                    95/12/11
030000:    endif							95/12/11
030100:  endif								95/03/30
030200:  set tblsize = $tblsize + $tally				95/03/31
030300:enddo								95/03/30
030400 01  FILLER		       REDEFINES  LR--MODULES.		95/03/31
030500	   02  LR--MODULE	       PIC 9($iw) OCCURS $tblsize TIMES.95/11/04
030600									95/03/29
030700:if animate							95/10/01
030800:push $style		       #  Set temporary animation style 95/11/18
030900:option -style=normal						95/11/05
031000 01  LR--MNAMES.							95/10/01
031100:  do module							95/10/01
031200     02  FILLER  PIC X(30) VALUE "$name".                         95/11/04
031300:  enddo								95/10/01
031400 01  FILLER	    REDEFINES  LR--MNAMES.			95/10/01
031500	   02  LR--MNAME    PIC X(30)  OCCURS $modules TIMES.		95/10/01
031600									95/10/01
031700 01  LR--SNAMES.							95/10/01
031800:  do state							95/10/01
031900     02  FILLER  PIC X(30) VALUE "$name".                         95/11/04
032000:  enddo								95/10/01
032100 01  FILLER	    REDEFINES  LR--SNAMES.			95/10/01
032200	   02  LR--SNAME    PIC X(30)  OCCURS $states TIMES.		95/10/01
032300									95/10/01
032400 01  LR--ENAMES.							95/10/01
032500:  do event							95/10/01
032600     02  FILLER  PIC X(30) VALUE "$name".                         95/11/04
032700:  enddo								95/10/01
032800 01  FILLER	    REDEFINES  LR--ENAMES.			95/10/01
032900	   02  LR--ENAME    PIC X(30)  OCCURS $events TIMES.		95/10/02
033000									95/10/01
033100:pop $style							95/11/18
033200:option -style=$style						95/11/18
033300:endif								95/10/01
033400:if "$model" = "main"                                             95/03/31
033500 PROCEDURE DIVISION.						95/03/29
033600:else								95/03/31
033700:if "$model" = "called"                                           95/06/25
033800 LINKAGE SECTION. 						95/03/31
033900									95/03/31
034000 01  PROGRAM-CONTROL.						95/03/31
034100:include "$SOURCE\R.$ext" "*CONTENTS"                             95/04/26
034200									95/03/31
034300 PROCEDURE DIVISION						95/03/31
034400	   USING PROGRAM-CONTROL					95/03/31
034500	   .								95/03/31
034600:else								95/03/31
034700:  echo "lr E: invalid /option - use /opt:model=[main|called]"    95/06/25
034800:  exit 1 							95/03/31
034900:endif all							95/03/31
035000									95/03/29
035100 LR--BEGIN-PROGRAM.						95/03/29
035200	   MOVE  +1  TO LR--STATE					95/11/02
035300:if module "Dialog-Call"                                          95/11/02
035400	   MOVE ZERO TO LR--STACK-SIZE					95/11/02
035500:endif								95/11/02
035600	   PERFORM INITIALISE-THE-PROGRAM				95/03/29
035700	   PERFORM LR--EXECUTE-DIALOG					95/03/29
035800	     UNTIL THE-NEXT-EVENT = TERMINATE-EVENT			95/03/29
035900	   .								95/03/29
036000 LR--END-PROGRAM. 						95/10/01
036100	   EXIT PROGRAM 						95/03/29
036200	   .								95/03/29
036300 LR--STOP-PROGRAM.						95/10/01
036400	   STOP RUN							95/03/29
036500	   .								95/03/29
036600									95/10/01
036700 LR--EXECUTE-DIALOG.						95/03/29
036800	   MOVE THE-NEXT-EVENT TO LR--EVENT				95/03/29
036900:if check 							95/03/29
037000	   IF LR--EVENT > $events OR LR--EVENT < 1			95/03/29
037100         DISPLAY "State " LR--STATE " - event " LR--EVENT         95/04/03
037200                 " is out of range"                               95/10/01
037300:  if "$console" != ""                                            95/10/01
037400		       $console 					95/10/01
037500:  endif								95/10/01
037600	       PERFORM LR--STOP-PROGRAM 				95/11/04
037700	   .								95/03/29
037800:endif								95/03/29
037900	   MOVE LR--STATE			  TO LR--SAVEST 	95/03/29
038000	   MOVE LR--ACTION (LR--STATE, LR--EVENT) TO LR--INDEX		95/03/29
038100:if defaults							95/03/29
038200*    IF NO ACTION FOR THIS EVENT, TRY THE DEFAULTS STATE		95/03/29
038300	   IF LR--INDEX = 0						95/03/29
038400	       MOVE LR--DEFAULTS-STATE		      TO LR--STATE	95/04/01
038500	       MOVE LR--ACTION (LR--STATE, LR--EVENT) TO LR--INDEX	95/03/29
038600	   .								95/03/29
038700:endif								95/03/29
038800:if animate							95/10/01
038900     DISPLAY " "                                                  95/11/05
039000     DISPLAY LR--SNAME (LR--STATE) ":"                            95/10/01
039100:  if "$console" != ""                                            95/10/01
039200		   $console						95/10/01
039300:  endif								95/10/01
039400     DISPLAY "    (--) " LR--ENAME (LR--EVENT)                    95/10/02
039500:  if "$console" != ""                                            95/10/01
039600		   $console						95/10/01
039700:  endif								95/10/01
039800:endif								95/10/01
039900:if check 							95/03/29
040000	   IF LR--INDEX = ZERO						95/03/29
040100         DISPLAY "State " LR--STATE " - event " LR--EVENT         95/04/03
040200                 " is not accepted"                               95/10/01
040300:  if "$console" != ""                                            95/10/01
040400		       $console 					95/10/01
040500:  endif								95/10/01
040600	       PERFORM LR--STOP-PROGRAM 				95/11/04
040700	   .								95/03/29
040800	   MOVE     LR--NULL-EVENT     TO THE-NEXT-EVENT		95/03/31
040900:endif								95/03/29
041000	   MOVE     LR--NULL-EVENT     TO THE-EXCEPTION-EVENT		95/03/29
041100	   MOVE 	SPACE	       TO EXCEPTION-RAISED		95/03/29
041200	   MOVE LR--OFFSET (LR--INDEX) TO LR--VECPTR			95/03/29
041300	   PERFORM LR--EXECUTE-ACTION-VECTOR				95/03/29
041400	     VARYING LR--VECPTR FROM LR--VECPTR BY 1			95/03/29
041500	       UNTIL LR--MODULE (LR--VECPTR) = ZERO			95/03/29
041600		  OR EXCEPTION-IS-RAISED				95/03/29
041700									95/03/29
041800	   IF EXCEPTION-IS-RAISED					95/03/29
041900	       PERFORM LR--GET-EXCEPTION-EVENT				95/03/29
042000	   ELSE 							95/03/29
042100	       MOVE LR--NEXTST (LR--STATE, LR--EVENT) TO LR--STATE	95/03/29
042200	   .								95/03/29
042300:if defaults							95/05/18
042400	   IF LR--STATE = LR--DEFAULTS-STATE				95/05/18
042500	       MOVE LR--SAVEST TO LR--STATE				95/05/18
042600	   .								95/05/18
042700:endif								95/05/18
042800	   IF THE-NEXT-EVENT = LR--NULL-EVENT				95/03/29
042900	       PERFORM GET-EXTERNAL-EVENT				95/03/31
043000:if check 							96/02/03
043100	       IF THE-NEXT-EVENT = LR--NULL-EVENT			95/03/31
043200             DISPLAY "No event set after event " LR--EVENT        95/03/31
043300                     " in state " LR--STATE                       95/10/01
043400:  if "$console" != ""                                            95/10/01
043500			   $console					95/10/01
043600:  endif								95/10/01
043700		   PERFORM LR--STOP-PROGRAM				95/11/04
043800:endif								96/02/03
043900	   .								95/03/29
044000: 								95/03/29
044100:declare int    modto	       # last of group of 10		95/08/07
044200:declare int    modfrom	       # first of group of 10		95/08/07
044300:declare int    modbase	       # last of previous group, or	95/08/07
044400:declare int    modloop	       # loop counter			95/08/07
044500:declare string modelse          # 'else' or spaces               95/08/07
044600:set comma_before="ELSE"                                          95/03/29
044700:set comma_last=""                                                95/03/29
044800									95/03/29
044900 LR--EXECUTE-ACTION-VECTOR.					95/03/29
045000	   MOVE LR--MODULE (LR--VECPTR) TO LR--MODNBR			95/04/26
045100:if animate							95/10/01
045200     DISPLAY "          + " LR--MNAME (LR--MODNBR)                95/10/01
045300:  if "$console" != ""                                            95/10/01
045400		   $console						95/10/01
045500:  endif								95/10/01
045600:endif								95/10/01
045700:set number_fmt = "%02ld"                                         95/03/31
045800:if "$LEVEL" = "ANSI74"                                           95/03/29
045900:if $modules > 10 	       # do gymnastics if > 10 modules	95/03/29
046000:  set modto = $modules						95/03/29
046100:  do while $modto > 10						95/03/29
046200:    set modbase = ($modto - 1) / 10 * 10 			95/03/29
046300:    set modfrom = $modbase + 1					95/03/29
046400	   IF LR--MODNBR > $modbase					95/03/29
046500	       PERFORM LR--EXECUTE-$modfrom-$modto			95/03/29
046600	   ELSE 							95/03/29
046700:    set modto = $modbase 					95/03/29
046800:  enddo								95/03/29
046900	       PERFORM LR--EXECUTE-01-$modto				95/03/29
047000:endif								95/03/29
047100!    Calculate if we need to print a split header 		95/03/29
047200:set modfrom = 1							95/03/29
047300:set modloop = 0							95/03/29
047400:do module							95/03/29
047500:  set modto = $modfrom + 9					95/03/29
047600:  if $modto > $modules						95/03/29
047700:    set modto = $modules 					95/03/29
047800:  endif								95/03/29
047900:  if $modules > 10						95/03/29
048000:    if $modloop = 0						95/03/29
048100	   .								95/03/29
048200									95/03/29
048300 LR--EXECUTE-$modfrom-$modto.					95/03/29
048400:      set modfrom = $modfrom + 10				95/03/29
048500:      set modloop = 10						95/03/29
048600:    endif							95/03/29
048700:    set modloop = $modloop - 1					95/03/29
048800:  endif								95/03/29
048900	   IF LR--MODNBR = $number					95/03/29
049000:  set modelse="$comma"                                           95/03/29
049100:  if $modules > 10						95/03/29
049200:    if $modloop = 0						95/03/29
049300:      set modelse=""                                             95/03/29
049400:    endif							95/03/29
049500:  endif								95/03/29
049600	       PERFORM $NAME				$MODELSE	95/03/29
049700:enddo								95/03/29
049800:else								95/03/29
049900:if "$LEVEL" = "ANSI85"                                           95/03/29
050000	   EVALUATE LR--MODNBR						95/03/29
050100:  do module							95/03/29
050200	       WHEN $number PERFORM $NAME				95/03/29
050300:  enddo								95/03/29
050400	   END-EVALUATE 						95/03/29
050500:else								95/03/29
050600:  echo "lr E: invalid /option - use /opt:level=[ansi74|ansi85]"  95/03/31
050700:  exit 1 							95/03/29
050800:endif all							95/03/29
050900	   .								95/03/29
051000									95/03/29
051100 LR--GET-EXCEPTION-EVENT. 					95/03/29
051200	   IF THE-EXCEPTION-EVENT NOT = LR--NULL-EVENT			95/03/29
051300	       MOVE THE-EXCEPTION-EVENT TO LR--EVENT			95/03/29
051400	   .								95/03/29
051500	   MOVE LR--EVENT TO THE-NEXT-EVENT				95/03/29
051600:if animate							95/10/02
051700     DISPLAY "    (=>) " LR--ENAME (LR--EVENT)                    95/10/02
051800:  if "$console" != ""                                            95/10/02
051900		   $console						95/10/02
052000:  endif								95/10/02
052100:endif								95/10/02
052200	   .								95/03/29
052300									95/03/31
052400:if module "Dialog-Call"                                          95/11/02
052500 DIALOG-CALL.							95/11/02
052600	   IF LR--STACK-SIZE < $stack_max				95/11/02
052700	       ADD 1 TO LR--STACK-SIZE					95/11/05
052800	       MOVE LR--STATE TO LR--STACK (LR--STACK-SIZE)		95/11/02
052900	   ELSE 							95/11/02
053000         DISPLAY "State " LR--STATE " - Dialog-Call overflow"     95/11/02
053100	       PERFORM LR--STOP-PROGRAM 				95/11/04
053200	   .								95/11/02
053300									95/11/02
053400 DIALOG-RETURN.							95/11/02
053500	   IF LR--STACK-SIZE > ZERO					95/11/02
053600	       MOVE LR--STACK (LR--STACK-SIZE) TO LR--STATE		95/11/02
053700	       MOVE	   RETURN-EVENT        TO THE-EXCEPTION-EVENT	95/11/05
053800         MOVE           "YES"            TO EXCEPTION-RAISED      95/11/05
053900	       ADD -1 TO LR--STACK-SIZE 				95/11/05
054000	   ELSE 							95/11/02
054100         DISPLAY "State " LR--STATE " - Dialog-Return underflow"  95/11/02
054200	       PERFORM LR--STOP-PROGRAM 				95/11/04
054300	   .								95/11/02
054400:endif								95/11/02
054500:close								95/03/29
054600!  Generate stubs for all modules not yet defined in source	95/03/29
054700									95/03/31
054800:internal "initialise_the_program"                                95/05/19
054900:internal "get_external_event"                                    95/05/19
055000:set stub_first   = "*"                                           95/11/03
055100:set stub_between = "*"                                           95/11/03
055200:set stub_last    = "*"                                           95/11/03
055300:set stub_width	= 66						95/11/03
055400:set module_line = " %s."                                         95/11/03
055500:do stubs $SOURCE.cob $DIALOG.$ext				95/11/02
055600									95/03/29
055700 $NAME.								95/03/31
055800:include optional $template "<$module_name>" "<END>"              95/12/18
055900:if $included = 0 						95/12/18
056000	   EXIT 							95/12/18
056100:endif								95/12/18
056200	   .								95/03/29
056300:enddo								95/03/29
