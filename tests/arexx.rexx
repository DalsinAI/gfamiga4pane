/* arexx.rexx: 4Pane's ARexx port (4PANE.1). Start 4Pane first, with
 * RexxMast running. Prints PASS or FAIL for each check, then QUITs it and
 * checks that the port has gone. rx tests/arexx.rexx
 * Copyright (c) 2026 AmiCradle Team */
OPTIONS RESULTS
PARSE ARG dummy
bad = 0
IF ~SHOW('P', '4PANE.1') THEN DO
    SAY 'FAIL 4Pane is not running (no 4PANE.1)'
    EXIT 20
END
ADDRESS '4PANE.1'

'VERSION'
CALL check 'VERSION', rc = 0 & POS('4Pane ', result) = 1
'HELP'
CALL check 'HELP', rc = 0 & POS('QUIT', result) > 0 & POS('GOTO', result) > 0 & POS('GETPATH', result) > 0 & POS('GETSELECTED', result) > 0 & POS('REFRESH', result) > 0
'FAULT 205'
CALL check 'FAULT', rc = 0 & result ~= ''
'NOSUCH'
CALL check 'unknown command', rc = 20
'DEACTIVATE'
CALL check 'DEACTIVATE', rc = 0
'ACTIVATE'
CALL check 'ACTIVATE', rc = 0
'GOTO "RAM:"'
CALL check 'GOTO RAM:', rc = 0
'GETPATH'
CALL check 'GETPATH', rc = 0 & POS('RAM', UPPER(result)) > 0
'GETSELECTED'
CALL check 'GETSELECTED', rc = 0
'REFRESH'
CALL check 'REFRESH', rc = 0
'GOTO "NoSuchVolume:none"'
CALL check 'GOTO missing', rc = 10
'QUIT'
CALL check 'QUIT', rc = 0
DO i = 1 TO 30
    IF ~SHOW('P', '4PANE.1') THEN LEAVE
    ADDRESS COMMAND 'Wait 1'
END
CALL check 'port gone after QUIT', ~SHOW('P', '4PANE.1')
IF bad = 0 THEN SAY '4pane arexx: ALL PASS'
ELSE SAY '4pane arexx:' bad 'FAILED'
EXIT bad

check: PARSE ARG what, ok
IF ok THEN SAY 'PASS' what
ELSE DO
    SAY 'FAIL' what '(rc' rc')'
    bad = bad + 1
END
RETURN
