000100 IDENTIFICATION DIVISION.                                         POS00535
000200 PROGRAM-ID.    CALCPK.                                           90/01/01
000300                                                                  90/01/01
000400 AUTHOR.        PIETER HINTJENS.                                  90/01/01
000500 DATE-WRITTEN.  89/06/05                                          90/01/01
000600     -REVISED:  95/12/07.                                         95/12/07
000700                                                                  90/01/01
000800 ENVIRONMENT DIVISION.                                            90/01/01
000900                                                                  90/01/01
001000 CONFIGURATION SECTION.                                           90/01/01
001100 SOURCE-COMPUTER. PORTABLE.                                       90/01/01
001200 OBJECT-COMPUTER. PORTABLE.                                       90/01/01
001300                                                                  90/01/01
001400 DATA DIVISION.                                                   90/01/01
001500                                                                  90/01/01
001600 WORKING-STORAGE SECTION.                                         90/01/01
001700                                                                  90/01/01
001800 01  EXPRESSION-PRIORITIES.                                       90/01/01
001900     02  END-MARK-PRIORITY       PIC 9      VALUE 1.              90/01/01
002000     02  LEFT-PAR-PRIORITY       PIC 9      VALUE 2.              90/01/01
002100     02  RIGHT-PAR-PRIORITY      PIC 9      VALUE 3.              90/01/01
002200     02  TERM-OP-PRIORITY        PIC 9      VALUE 4.              90/01/01
002300     02  FACTOR-OP-PRIORITY      PIC 9      VALUE 5.              90/01/01
002400     02  FUNCTION-PRIORITY       PIC 9      VALUE 6.              90/01/01
002500     02  LOWEST-OPR-PRIORITY     PIC 9      VALUE 4.              90/01/01
002600                                                                  90/01/01
002700 01  VARIOUS-CONSTANTS.                                           90/01/01
002800     02  EXPR-SIZE               PIC S9(3)  COMP VALUE +80.       90/01/01
002900     02  WHOLE-PART-SIZE         PIC S9(3)  COMP VALUE  +9.       90/01/01
003000     02  FRACTION-SIZE           PIC S9(3)  COMP VALUE  +9.       90/01/01
003100     02  FUNCTION-SIZE           PIC S9(3)  COMP VALUE  +3.       90/01/01
003200                                                                  90/01/01
003300 01  VARIOUS-NUMBERS.                                             90/01/01
003400     02  INV-LOG-OF-E            PIC S9(9)V9(9)  COMP             90/01/01
003500                                            VALUE +2.302585093.   90/01/01
003600     02  HALF-OF-PI              PIC S9(9)V9(9)  COMP             90/01/01
003700                                            VALUE +1.570796327.   90/01/01
003800     02  ROUND-UP-VALUE          PIC S9(9)V9(9)  COMP             90/01/01
003900                                            VALUE +0.500000000.   90/01/01
004000                                                                  90/01/01
004100 01  VARIOUS-TOKENS.                                              90/01/01
004200     02  FUNCTION-TOKEN          PIC X      VALUE "F".            90/01/01
004300     02  END-MARK-TOKEN          PIC X      VALUE ">".            90/01/01
004400                                                                  90/01/01
004500 01  VARIOUS-INDICES.                                             90/01/01
004600     02  CHAR-NBR                PIC S9(3)  COMP.                 90/01/01
004700     02  DIGIT-NBR               PIC S9(3)  COMP.                 90/01/01
004800     02  EXPR-INDEX              PIC S9(3)  COMP.                 90/01/01
004900     02  TOKEN-POSITION          PIC S9(3)  COMP.                 90/01/01
005000                                                                  90/01/01
005100 01  VARIOUS-VALUES.                                              90/01/01
005200     02  THE-TOKEN               PIC X.                           90/01/01
005300         88  THE-TOKEN-IS-NUMERIC           VALUE "0" THRU "9".   93/02/12
005400         88  THE-TOKEN-IS-ALPHABETIC        VALUE "A" THRU "Z".   93/02/12
005500     02  THE-TOKEN-VALUE         REDEFINES  THE-TOKEN             90/01/01
005600                                 PIC 9.                           90/01/01
005700     02  THE-PRIORITY            PIC 9.                           90/01/01
005800     02  THE-OPERATOR            PIC X.                           90/01/01
005900         88  BINARY-OPERATOR                VALUE IS "+" "-" "*"  90/01/01
006000                                                     "/" "P".     90/01/01
006100     02  THE-NAME.                                                90/01/01
006200         88  VALID-VARIABLE-NAME            VALUE "A" "B" "C"     90/01/01
006300                                                  "D" "E".        90/01/01
006400         88  VALID-FUNCTION-NAME            VALUE "ABS" "NEG"     90/01/01
006500                                                  "RND".          95/10/22
006600         03  THE-NAME-CHAR       PIC X      OCCURS 3 TIMES.       90/01/01
006700                                                                  90/01/01
006800     02  CUR-CHAR                PIC X.                           90/01/01
006900         88  VALID-NAME-CHAR                VALUE "A" THRU "Z".   90/01/01
007000                                                                  90/01/01
007100     02  THE-NUMBER              PIC S9(9)V9(9).                  93/02/12
007200     02  FILLER                  REDEFINES  THE-NUMBER.           90/01/01
007300         03  NUMBER-WHOLE-PART   PIC 9(9).                        90/01/01
007400         03  NUMBER-FRACT-CHAR   PIC X      OCCURS 9 TIMES.       90/01/01
007500                                                                  90/01/01
007600     02  SIGN-OF-NUMBER          PIC X.                           93/02/12
007700     02  COLLECTING-NUMBER       PIC X.                           90/01/01
007800         88  COLLECTING-WHOLE-PART          VALUE "W".            90/01/01
007900         88  COLLECTING-FRACTION            VALUE "F".            90/01/01
008000         88  NUMBER-IS-COLLECTED            VALUE "D".            90/01/01
008100                                                                  90/01/01
008200 01  VARIOUS-OPERANDS.                                            90/01/01
008300     02  OPERAND-1               PIC S9(9)V9(9) COMP.             90/01/01
008400     02  OPERAND-2               PIC S9(9)V9(9) COMP.             90/01/01
008500     02  OPERAND-INTEGER         PIC S9(9)      COMP.             90/01/01
008600                                                                  90/01/01
008700                                                                  90/01/01
008800 01  OPERAND-STACK.                                               90/01/01
008900     02  OPERAND-PTR             PIC S9(3)  COMP.                 90/01/01
009000     02  OPERAND-MAX-PTR         PIC S9(3)  COMP VALUE +20.       90/01/01
009100     02  FILLER                             OCCURS 20 TIMES.      90/01/01
009200         03  STACK-OPERAND       PIC S9(9)V9(9) COMP.             90/01/01
009300                                                                  90/01/01
009400 01  OPERATOR-STACK.                                              90/01/01
009500     02  OPERATOR-PTR            PIC S9(3)  COMP.                 90/01/01
009600     02  OPERATOR-MAX-PTR        PIC S9(3)  COMP VALUE +20.       90/01/01
009700     02  FILLER                             OCCURS 20 TIMES.      90/01/01
009800         03  STACK-OPERATOR      PIC X.                           90/01/01
009900         03  STACK-OP-NAME       PIC X(3).                        90/01/01
010000         03  STACK-PRIORITY      PIC 9.                           90/01/01
010100                                                                  90/01/01
010200*DIALOG MANAGER.                                                  90/01/01
010300 COPY CALCPKD.                                                    90/01/01
010400                                                                  90/01/01
010500*******************   INITIALISE THE PROGRAM   *******************95/04/26
010600                                                                  95/04/26
010700 INITIALISE-THE-PROGRAM.                                          95/04/26
010800     MOVE SPACE TO CALCPK-FEEDBACK                                95/04/26
010900     IF CALCPK-OPERATION = "C"                                    95/04/26
011000         PERFORM SET-OK                                           90/01/01
011100         PERFORM INITIALIZE-THE-PARSER                            90/01/01
011200     ELSE                                                         90/01/01
011300         PERFORM SET-ERROR                                        90/01/01
011400         MOVE "OPERATION" TO CALCPK-FEEDBACK                      95/04/26
011500     .                                                            90/01/01
011600                                                                  90/01/01
011700 SET-OK.                                                          90/01/01
011800     MOVE OK-EVENT TO THE-NEXT-EVENT                              90/01/01
011900     .                                                            90/01/01
012000                                                                  90/01/01
012100 INITIALIZE-THE-PARSER.                                           90/01/01
012200     MOVE      SPACES       TO CALCPK-ERROR-RETURN                90/01/01
012300     MOVE       ZERO        TO CALCPK-ERROR-POSN                  90/01/01
012400                               CALCPK-RESULT                      90/01/01
012500     MOVE        1          TO EXPR-INDEX                         90/01/01
012600     MOVE        1          TO OPERATOR-PTR                       90/01/01
012700     MOVE       ZERO        TO OPERAND-PTR                        90/01/01
012800     MOVE       ZERO        TO STACK-OPERAND     (1)              90/01/01
012900     MOVE END-MARK-PRIORITY TO STACK-PRIORITY    (1)              90/01/01
013000     MOVE  END-MARK-TOKEN   TO STACK-OPERATOR    (1)              90/01/01
013100                               EXPR-CHAR (EXPR-SIZE)              90/01/01
013200     .                                                            90/01/01
013300                                                                  90/01/01
013400 SET-ERROR.                                                       90/01/01
013500     MOVE ERROR-EVENT TO THE-NEXT-EVENT                           90/01/01
013600     .                                                            90/01/01
013700                                                                  90/01/01
013800**********************    GET NEXT TOKEN    **********************90/01/01
013900                                                                  90/01/01
014000 GET-NEXT-TOKEN.                                                  90/01/01
014100     PERFORM SKIP-SPACES                                          90/01/01
014200       VARYING EXPR-INDEX FROM EXPR-INDEX BY 1                    90/01/01
014300         UNTIL EXPR-CHAR (EXPR-INDEX) > SPACE                     90/01/01
014400                                                                  90/01/01
014500     MOVE EXPR-CHAR (EXPR-INDEX) TO THE-TOKEN                     90/01/01
014600     MOVE EXPR-INDEX             TO TOKEN-POSITION                90/01/01
014700     MOVE OTHER-EVENT            TO THE-NEXT-EVENT                90/01/01
014800                                                                  90/01/01
014900     IF THE-TOKEN = "+" OR "-"                                    90/01/01
015000         PERFORM HAVE-TERM-OP                                     90/01/01
015100     ELSE                                                         90/01/01
015200     IF THE-TOKEN = "/" OR "*"                                    95/10/22
015300         PERFORM HAVE-FACTOR-OP                                   90/01/01
015400     ELSE                                                         90/01/01
015500     IF THE-TOKEN = "("                                           90/01/01
015600         PERFORM HAVE-LEFT-PAR                                    90/01/01
015700     ELSE                                                         90/01/01
015800     IF THE-TOKEN = ")"                                           90/01/01
015900         PERFORM HAVE-RIGHT-PAR                                   90/01/01
016000     ELSE                                                         90/01/01
016100     IF THE-TOKEN-IS-NUMERIC                                      93/02/12
016200         PERFORM HAVE-NUMBER                                      90/01/01
016300     ELSE                                                         90/01/01
016400     IF THE-TOKEN-IS-ALPHABETIC                                   93/02/12
016500         PERFORM HAVE-NAMED-ITEM                                  90/01/01
016600     ELSE                                                         90/01/01
016700     IF THE-TOKEN = END-MARK-TOKEN                                90/01/01
016800         PERFORM HAVE-END-MARK                                    90/01/01
016900     ELSE                                                         90/01/01
017000         PERFORM SIGNAL-INVALID-CHAR                              90/01/01
017100     .                                                            90/01/01
017200                                                                  90/01/01
017300 SKIP-SPACES.                                                     90/01/01
017400     EXIT                                                         90/01/01
017500     .                                                            90/01/01
017600                                                                  90/01/01
017700 HAVE-TERM-OP.                                                    90/01/01
017800     MOVE TERM-OP-PRIORITY TO THE-PRIORITY                        90/01/01
017900     MOVE OPERATOR-EVENT   TO THE-NEXT-EVENT                      90/01/01
018000     ADD 1 TO EXPR-INDEX                                          90/01/01
018100     .                                                            90/01/01
018200                                                                  90/01/01
018300 HAVE-FACTOR-OP.                                                  90/01/01
018400     MOVE FACTOR-OP-PRIORITY TO THE-PRIORITY                      90/01/01
018500     MOVE OPERATOR-EVENT     TO THE-NEXT-EVENT                    90/01/01
018600     ADD 1 TO EXPR-INDEX                                          90/01/01
018700     .                                                            90/01/01
018800                                                                  90/01/01
018900 HAVE-LEFT-PAR.                                                   90/01/01
019000     MOVE LEFT-PAR-PRIORITY TO THE-PRIORITY                       90/01/01
019100     MOVE LEFT-PAR-EVENT    TO THE-NEXT-EVENT                     90/01/01
019200     ADD 1 TO EXPR-INDEX                                          90/01/01
019300     .                                                            90/01/01
019400                                                                  90/01/01
019500 HAVE-RIGHT-PAR.                                                  90/01/01
019600     MOVE RIGHT-PAR-PRIORITY TO THE-PRIORITY                      90/01/01
019700     MOVE RIGHT-PAR-EVENT    TO THE-NEXT-EVENT                    90/01/01
019800     ADD 1 TO EXPR-INDEX                                          90/01/01
019900     .                                                            90/01/01
020000                                                                  90/01/01
020100 HAVE-NUMBER.                                                     90/01/01
020200     MOVE OPERAND-EVENT TO THE-NEXT-EVENT                         90/01/01
020300     MOVE     ZERO      TO THE-NUMBER                             90/01/01
020400                           DIGIT-NBR                              90/01/01
020500     MOVE "WHOLE PART"  TO COLLECTING-NUMBER                      90/01/01
020600     PERFORM COLLECT-THE-NUMBER                                   90/01/01
020700       UNTIL NUMBER-IS-COLLECTED                                  90/01/01
020800     .                                                            90/01/01
020900                                                                  90/01/01
021000 COLLECT-THE-NUMBER.                                              90/01/01
021100     IF COLLECTING-WHOLE-PART                                     90/01/01
021200         PERFORM GET-TOKEN-IN-WHOLE-PART                          90/01/01
021300     ELSE                                                         90/01/01
021400     IF COLLECTING-FRACTION                                       90/01/01
021500         PERFORM GET-TOKEN-IN-FRACTION                            90/01/01
021600     .                                                            90/01/01
021700     MOVE EXPR-CHAR (EXPR-INDEX) TO THE-TOKEN                     90/01/01
021800     .                                                            90/01/01
021900                                                                  90/01/01
022000 GET-TOKEN-IN-WHOLE-PART.                                         90/01/01
022100     IF THE-TOKEN-IS-NUMERIC                                      93/02/12
022200         ADD 1 TO DIGIT-NBR                                       90/01/01
022300         PERFORM PICK-UP-WHOLE-PART-DIGIT                         90/01/01
022400     ELSE                                                         90/01/01
022500     IF THE-TOKEN = CALCPK-POINT-CHAR                             90/01/01
022600         MOVE "FRACTION" TO COLLECTING-NUMBER                     90/01/01
022700         MOVE    ZERO    TO DIGIT-NBR                             90/01/01
022800         ADD 1 TO EXPR-INDEX                                      90/01/01
022900     ELSE                                                         90/01/01
023000         MOVE "DONE" TO COLLECTING-NUMBER                         90/01/01
023100     .                                                            90/01/01
023200                                                                  90/01/01
023300 PICK-UP-WHOLE-PART-DIGIT.                                        90/01/01
023400     IF DIGIT-NBR > WHOLE-PART-SIZE                               90/01/01
023500         MOVE "WF WHOLE PART OF NUMBER TOO LARGE"                 90/01/01
023600                     TO CALCPK-ERROR-RETURN                       90/01/01
023700         MOVE "DONE" TO COLLECTING-NUMBER                         90/01/01
023800         PERFORM SIGNAL-OVERFLOW-ERROR                            90/01/01
023900     ELSE                                                         90/01/01
024000         COMPUTE NUMBER-WHOLE-PART                                90/01/01
024100               = NUMBER-WHOLE-PART * 10 + THE-TOKEN-VALUE         90/01/01
024200         ADD 1 TO EXPR-INDEX                                      90/01/01
024300     .                                                            90/01/01
024400                                                                  90/01/01
024500 SIGNAL-OVERFLOW-ERROR.                                           90/01/01
024600     MOVE "FULL"     TO CALCPK-FEEDBACK                           95/04/26
024700     MOVE EXPR-INDEX TO CALCPK-ERROR-POSN                         90/01/01
024800     PERFORM RAISE-EXCEPTION                                      90/01/01
024900     .                                                            90/01/01
025000                                                                  90/01/01
025100 RAISE-EXCEPTION.                                                 90/01/01
025200     MOVE      "YES"      TO EXCEPTION-RAISED                     90/01/01
025300     MOVE EXCEPTION-EVENT TO THE-EXCEPTION-EVENT                  90/01/01
025400     .                                                            90/01/01
025500                                                                  90/01/01
025600 GET-TOKEN-IN-FRACTION.                                           90/01/01
025700     IF THE-TOKEN-IS-NUMERIC                                      93/02/12
025800         ADD 1 TO DIGIT-NBR                                       90/01/01
025900         PERFORM PICK-UP-FRACTION-DIGIT                           90/01/01
026000     ELSE                                                         90/01/01
026100         MOVE "DONE" TO COLLECTING-NUMBER                         90/01/01
026200     .                                                            90/01/01
026300                                                                  90/01/01
026400 PICK-UP-FRACTION-DIGIT.                                          90/01/01
026500     IF DIGIT-NBR > FRACTION-SIZE                                 90/01/01
026600         MOVE "FF FRACTION OF NUMBER TOO LARGE"                   90/01/01
026700                     TO CALCPK-ERROR-RETURN                       90/01/01
026800         MOVE "DONE" TO COLLECTING-NUMBER                         90/01/01
026900         PERFORM SIGNAL-OVERFLOW-ERROR                            90/01/01
027000     ELSE                                                         90/01/01
027100         MOVE THE-TOKEN TO NUMBER-FRACT-CHAR (DIGIT-NBR)          90/01/01
027200         ADD 1 TO EXPR-INDEX                                      90/01/01
027300     .                                                            90/01/01
027400                                                                  90/01/01
027500 HAVE-NAMED-ITEM.                                                 90/01/01
027600     MOVE SPACES TO THE-NAME                                      90/01/01
027700     PERFORM PICK-UP-THE-NAME-CHAR                                90/01/01
027800       VARYING CHAR-NBR FROM 1 BY 1                               90/01/01
027900         UNTIL CHAR-NBR > FUNCTION-SIZE                           90/01/01
028000                                                                  90/01/01
028100     IF VALID-VARIABLE-NAME                                       90/01/01
028200         PERFORM HAVE-VARIABLE                                    90/01/01
028300     ELSE                                                         90/01/01
028400     IF VALID-FUNCTION-NAME                                       90/01/01
028500         PERFORM HAVE-FUNCTION                                    90/01/01
028600     ELSE                                                         90/01/01
028700         MOVE "IN NOT FUNCTION OR VARIABLE NAME"                  90/01/01
028800           TO CALCPK-ERROR-RETURN                                 90/01/01
028900         PERFORM SIGNAL-SYNTAX-ERROR                              90/01/01
029000     .                                                            90/01/01
029100                                                                  90/01/01
029200 PICK-UP-THE-NAME-CHAR.                                           90/01/01
029300     IF EXPR-INDEX < EXPR-SIZE                                    90/01/01
029400         MOVE EXPR-CHAR (EXPR-INDEX) TO CUR-CHAR                  90/01/01
029500         IF VALID-NAME-CHAR                                       90/01/01
029600             MOVE CUR-CHAR TO THE-NAME-CHAR (CHAR-NBR)            90/01/01
029700             ADD 1 TO EXPR-INDEX                                  90/01/01
029800     .                                                            90/01/01
029900                                                                  90/01/01
030000 HAVE-VARIABLE.                                                   90/01/01
030100     MOVE OPERAND-EVENT TO THE-NEXT-EVENT                         90/01/01
030200     IF THE-NAME = "A"                                            90/01/01
030300         MOVE CALCPK-VARIABLE (1) TO THE-NUMBER                   90/01/01
030400     ELSE                                                         90/01/01
030500     IF THE-NAME = "B"                                            90/01/01
030600         MOVE CALCPK-VARIABLE (2) TO THE-NUMBER                   90/01/01
030700     ELSE                                                         90/01/01
030800     IF THE-NAME = "C"                                            90/01/01
030900         MOVE CALCPK-VARIABLE (3) TO THE-NUMBER                   90/01/01
031000     ELSE                                                         90/01/01
031100     IF THE-NAME = "D"                                            90/01/01
031200         MOVE CALCPK-VARIABLE (4) TO THE-NUMBER                   90/01/01
031300     ELSE                                                         90/01/01
031400     IF THE-NAME = "E"                                            90/01/01
031500         MOVE CALCPK-VARIABLE (5) TO THE-NUMBER                   90/01/01
031600     .                                                            90/01/01
031700                                                                  90/01/01
031800 HAVE-FUNCTION.                                                   90/01/01
031900     MOVE FUNCTION-TOKEN    TO THE-TOKEN                          90/01/01
032000     MOVE FUNCTION-PRIORITY TO THE-PRIORITY                       90/01/01
032100     MOVE FUNCTION-EVENT    TO THE-NEXT-EVENT                     90/01/01
032200     .                                                            90/01/01
032300                                                                  90/01/01
032400 HAVE-END-MARK.                                                   90/01/01
032500     IF EXPR-INDEX < EXPR-SIZE                                    90/01/01
032600         PERFORM SIGNAL-INVALID-CHAR                              90/01/01
032700     ELSE                                                         90/01/01
032800         MOVE END-MARK-PRIORITY TO THE-PRIORITY                   90/01/01
032900         MOVE END-MARK-EVENT    TO THE-NEXT-EVENT                 90/01/01
033000     .                                                            90/01/01
033100                                                                  90/01/01
033200 SIGNAL-INVALID-CHAR.                                             90/01/01
033300     MOVE "IC INVALID CHARACTER" TO CALCPK-ERROR-RETURN           90/01/01
033400     PERFORM SIGNAL-SYNTAX-ERROR                                  90/01/01
033500     .                                                            90/01/01
033600                                                                  90/01/01
033700 SIGNAL-SYNTAX-ERROR.                                             90/01/01
033800     MOVE "SYNTAX ERROR" TO CALCPK-FEEDBACK                       95/04/26
033900     MOVE TOKEN-POSITION TO CALCPK-ERROR-POSN                     90/01/01
034000     PERFORM RAISE-EXCEPTION                                      90/01/01
034100     .                                                            90/01/01
034200                                                                  90/01/01
034300******************    CHECK IF SIGNED NUMBER    ******************93/02/12
034400                                                                  93/02/12
034500 CHECK-IF-SIGNED-NUMBER.                                          93/02/12
034600     IF THE-TOKEN = "+" OR "-"                                    93/02/12
034700         MOVE THE-TOKEN              TO SIGN-OF-NUMBER            93/02/12
034800         MOVE EXPR-CHAR (EXPR-INDEX) TO THE-TOKEN                 93/02/12
034900         IF THE-TOKEN-IS-NUMERIC                                  93/02/12
035000             PERFORM HAVE-SIGNED-NUMBER                           93/02/12
035100             MOVE THE-NEXT-EVENT TO THE-EXCEPTION-EVENT           93/02/12
035200             MOVE     "YES"      TO EXCEPTION-RAISED              93/02/12
035300     .                                                            93/02/12
035400                                                                  93/02/12
035500 HAVE-SIGNED-NUMBER.                                              93/02/12
035600     PERFORM HAVE-NUMBER                                          93/02/12
035700     IF SIGN-OF-NUMBER = "-"                                      93/02/12
035800         COMPUTE THE-NUMBER = ZERO - THE-NUMBER                   93/02/12
035900     .                                                            93/02/12
036000                                                                  93/02/12
036100********************    STACK THE OPERAND    *********************90/01/01
036200                                                                  90/01/01
036300 STACK-THE-OPERAND.                                               90/01/01
036400     IF OPERAND-PTR < OPERAND-MAX-PTR                             90/01/01
036500         ADD 1 TO OPERAND-PTR                                     90/01/01
036600         MOVE THE-NUMBER TO STACK-OPERAND (OPERAND-PTR)           90/01/01
036700     ELSE                                                         90/01/01
036800         MOVE "DF OPERAND STACK FULL" TO CALCPK-ERROR-RETURN      90/01/01
036900         PERFORM SIGNAL-OVERFLOW-ERROR                            90/01/01
037000     .                                                            90/01/01
037100                                                                  90/01/01
037200*******************    STACK THE OPERATOR   **********************90/01/01
037300                                                                  90/01/01
037400 STACK-THE-OPERATOR.                                              90/01/01
037500     IF OPERATOR-PTR < OPERATOR-MAX-PTR                           90/01/01
037600         ADD 1 TO OPERATOR-PTR                                    90/01/01
037700         MOVE THE-TOKEN    TO STACK-OPERATOR (OPERATOR-PTR)       90/01/01
037800         MOVE THE-NAME     TO STACK-OP-NAME  (OPERATOR-PTR)       90/01/01
037900         MOVE THE-PRIORITY TO STACK-PRIORITY (OPERATOR-PTR)       90/01/01
038000     ELSE                                                         90/01/01
038100         MOVE "RF OPERATOR STACK FULL" TO CALCPK-ERROR-RETURN     90/01/01
038200         PERFORM SIGNAL-OVERFLOW-ERROR                            90/01/01
038300     .                                                            90/01/01
038400                                                                  90/01/01
038500*******************    UNSTACK GE OPERATORS    *******************90/01/01
038600                                                                  90/01/01
038700 UNSTACK-GE-OPERATORS.                                            90/01/01
038800     PERFORM UNSTACK-OPERATOR                                     90/01/01
038900       UNTIL STACK-PRIORITY (OPERATOR-PTR) < THE-PRIORITY         90/01/01
039000     .                                                            90/01/01
039100                                                                  90/01/01
039200 UNSTACK-OPERATOR.                                                90/01/01
039300     MOVE STACK-OPERATOR (OPERATOR-PTR) TO THE-OPERATOR           90/01/01
039400     MOVE STACK-OP-NAME  (OPERATOR-PTR) TO THE-NAME               90/01/01
039500     SUBTRACT 1 FROM OPERATOR-PTR                                 90/01/01
039600     PERFORM EXECUTE-THE-OPERATION                                90/01/01
039700     .                                                            90/01/01
039800                                                                  90/01/01
039900 EXECUTE-THE-OPERATION.                                           90/01/01
040000     MOVE STACK-OPERAND (OPERAND-PTR) TO OPERAND-1                90/01/01
040100     IF BINARY-OPERATOR                                           90/01/01
040200         PERFORM BINARY-OPERATION                                 90/01/01
040300     ELSE                                                         90/01/01
040400     IF THE-OPERATOR = FUNCTION-TOKEN                             90/01/01
040500         PERFORM FUNCTION-OPERATION                               90/01/01
040600     ELSE                                                         90/01/01
040700     IF THE-OPERATOR = END-MARK-TOKEN                             90/01/01
040800         PERFORM END-OPERATION                                    90/01/01
040900     .                                                            90/01/01
041000     MOVE OPERAND-1 TO STACK-OPERAND (OPERAND-PTR)                90/01/01
041100     .                                                            90/01/01
041200                                                                  90/01/01
041300 BINARY-OPERATION.                                                90/01/01
041400     SUBTRACT 1 FROM OPERAND-PTR                                  90/01/01
041500     MOVE OPERAND-1                   TO OPERAND-2                90/01/01
041600     MOVE STACK-OPERAND (OPERAND-PTR) TO OPERAND-1                90/01/01
041700     IF THE-OPERATOR = "+"                                        90/01/01
041800         COMPUTE OPERAND-1 = OPERAND-1 + OPERAND-2                90/01/01
041900     ELSE                                                         90/01/01
042000     IF THE-OPERATOR = "-"                                        90/01/01
042100         COMPUTE OPERAND-1 = OPERAND-1 - OPERAND-2                90/01/01
042200     ELSE                                                         90/01/01
042300     IF THE-OPERATOR = "*"                                        90/01/01
042400         COMPUTE OPERAND-1 = OPERAND-1 * OPERAND-2                90/01/01
042500     ELSE                                                         90/01/01
042600     IF THE-OPERATOR = "/"                                        90/01/01
042700         IF OPERAND-2 = ZERO                                      90/01/01
042800             COMPUTE OPERAND-1 = ZERO                             90/01/01
042900         ELSE                                                     90/01/01
043000             COMPUTE OPERAND-1 = OPERAND-1 / OPERAND-2            90/01/01
043100     .                                                            90/01/01
043200     MOVE OPERAND-1 TO STACK-OPERAND (OPERAND-PTR)                90/01/01
043300     .                                                            90/01/01
043400                                                                  90/01/01
043500 FUNCTION-OPERATION.                                              90/01/01
043600     IF THE-NAME = "ABS"                                          90/01/01
043700         PERFORM UNARY-ABS-OPERATION                              90/01/01
043800     ELSE                                                         90/01/01
043900     IF THE-NAME = "NEG"                                          90/01/01
044000         PERFORM UNARY-NEG-OPERATION                              90/01/01
044100     ELSE                                                         90/01/01
044200     IF THE-NAME = "RND"                                          90/01/01
044300         PERFORM UNARY-RND-OPERATION                              90/01/01
044400     .                                                            90/01/01
044500                                                                  90/01/01
044600 UNARY-ABS-OPERATION.                                             90/01/01
044700     IF OPERAND-1 < ZERO                                          90/01/01
044800         COMPUTE OPERAND-1 = ZERO - OPERAND-1                     90/01/01
044900     .                                                            90/01/01
045000                                                                  90/01/01
045100 UNARY-NEG-OPERATION.                                             90/01/01
045200     COMPUTE OPERAND-1 = ZERO - OPERAND-1                         90/01/01
045300     .                                                            90/01/01
045400                                                                  90/01/01
045500 UNARY-RND-OPERATION.                                             95/10/22
045600     IF OPERAND-1 > ZERO                                          95/10/22
045700         COMPUTE OPERAND-1 = OPERAND-1 + ROUND-UP-VALUE           95/10/22
045800     ELSE                                                         95/10/22
045900         COMPUTE OPERAND-1 = OPERAND-1 - ROUND-UP-VALUE           95/10/22
046000     .                                                            95/10/22
046100     MOVE OPERAND-1       TO OPERAND-INTEGER                      95/10/22
046200     MOVE OPERAND-INTEGER TO OPERAND-1                            90/01/01
046300     .                                                            90/01/01
046400                                                                  90/01/01
046500 END-OPERATION.                                                   90/01/01
046600     IF OPERAND-PTR = 1                                           90/01/01
046700         MOVE OPERAND-1 TO CALCPK-RESULT                          90/01/01
046800     .                                                            90/01/01
046900                                                                  90/01/01
047000******************    UNSTACK ALL OPERATORS    *******************90/01/01
047100                                                                  90/01/01
047200 UNSTACK-ALL-OPERATORS.                                           90/01/01
047300     PERFORM UNSTACK-OPERATOR                                     90/01/01
047400       UNTIL STACK-PRIORITY (OPERATOR-PTR) < LOWEST-OPR-PRIORITY  90/01/01
047500     .                                                            90/01/01
047600                                                                  90/01/01
047700*******************    UNSTACK IF LEFT PAR    ********************90/01/01
047800                                                                  90/01/01
047900 UNSTACK-IF-LEFT-PAR.                                             90/01/01
048000     IF STACK-OPERATOR (OPERATOR-PTR) = "("                       90/01/01
048100         SUBTRACT 1 FROM OPERATOR-PTR                             90/01/01
048200     ELSE                                                         90/01/01
048300         MOVE "LP MISSING LEFT PARENTHESIS" TO CALCPK-ERROR-RETURN90/01/01
048400         PERFORM SIGNAL-PARENTHESIS-MISSING                       90/01/01
048500     .                                                            90/01/01
048600                                                                  90/01/01
048700 SIGNAL-PARENTHESIS-MISSING.                                      90/01/01
048800     PERFORM SIGNAL-SYNTAX-ERROR                                  90/01/01
048900     PERFORM SET-ERROR-POSN-AT-END                                90/01/01
049000     .                                                            90/01/01
049100                                                                  90/01/01
049200 SET-ERROR-POSN-AT-END.                                           90/01/01
049300     MOVE SPACE TO EXPR-CHAR (EXPR-SIZE)                          90/01/01
049400     IF CALCPK-EXPRESSION = SPACES                                90/01/01
049500         MOVE 1 TO CALCPK-ERROR-POSN                              90/01/01
049600     ELSE                                                         90/01/01
049700         PERFORM FIND-LAST-INPUT-CHAR                             90/01/01
049800           VARYING EXPR-INDEX FROM EXPR-SIZE BY -1                90/01/01
049900             UNTIL EXPR-CHAR (EXPR-INDEX) > SPACE                 90/01/01
050000                                                                  90/01/01
050100         MOVE EXPR-INDEX TO CALCPK-ERROR-POSN                     90/01/01
050200     .                                                            90/01/01
050300                                                                  90/01/01
050400 FIND-LAST-INPUT-CHAR.                                            90/01/01
050500     EXIT                                                         90/01/01
050600     .                                                            90/01/01
050700                                                                  90/01/01
050800*******************    UNSTACK IF END MARK    ********************90/01/01
050900                                                                  90/01/01
051000 UNSTACK-IF-END-MARK.                                             90/01/01
051100     IF STACK-OPERATOR (OPERATOR-PTR) = END-MARK-TOKEN            90/01/01
051200         PERFORM UNSTACK-OPERATOR                                 90/01/01
051300     ELSE                                                         90/01/01
051400         PERFORM SIGNAL-END-MARK-EXPECTED                         90/01/01
051500     .                                                            90/01/01
051600                                                                  90/01/01
051700 SIGNAL-END-MARK-EXPECTED.                                        90/01/01
051800     MOVE "RP MISSING RIGHT PARENTHESIS" TO CALCPK-ERROR-RETURN   90/01/01
051900     PERFORM SIGNAL-PARENTHESIS-MISSING                           90/01/01
052000     .                                                            90/01/01
052100                                                                  90/01/01
052200*******************    SIGNAL INVALID TOKEN    *******************90/01/01
052300                                                                  90/01/01
052400 SIGNAL-INVALID-TOKEN.                                            90/01/01
052500     MOVE "IT INVALID TOKEN" TO CALCPK-ERROR-RETURN               90/01/01
052600     MOVE "SYNTAX ERROR"     TO CALCPK-FEEDBACK                   95/04/26
052700     MOVE TOKEN-POSITION     TO CALCPK-ERROR-POSN                 90/01/01
052800     .                                                            90/01/01
052900                                                                  90/01/01
053000*******************    SIGNAL TOKEN MISSING    *******************90/01/01
053100                                                                  90/01/01
053200 SIGNAL-TOKEN-MISSING.                                            90/01/01
053300     MOVE "MT UNEXPECTED END OF EXPRESSION" TO CALCPK-ERROR-RETURN90/01/01
053400     PERFORM SIGNAL-SYNTAX-ERROR                                  90/01/01
053500     PERFORM SET-ERROR-POSN-AT-END                                90/01/01
053600     .                                                            90/01/01
053700                                                                  90/01/01
053800*********************   GET EXTERNAL EVENT   *********************95/04/26
053900                                                                  95/04/26
054000 GET-EXTERNAL-EVENT.                                              95/04/26
054100     EXIT                                                         95/04/26
054200     .                                                            95/04/26
054300                                                                  95/04/26
054400*******************    TERMINATE THE PROGRAM    ******************90/01/01
054500                                                                  90/01/01
054600 TERMINATE-THE-PROGRAM.                                           90/01/01
054700     MOVE TERMINATE-EVENT TO THE-NEXT-EVENT                       90/01/01
054800     .                                                            90/01/01