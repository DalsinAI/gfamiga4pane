/* Here.rexx: 4Pane shows a drawer, then says what is selected.
   rx Here.rexx Work:   (4Pane running) */
OPTIONS RESULTS
PARSE ARG path
IF path = '' THEN path = 'SYS:'
IF ~SHOW('P', '4PANE.1') THEN DO
    SAY '4Pane is not running.'
    EXIT 5
END
ADDRESS '4PANE.1'
'GOTO' '"'path'"'
'GETPATH'
SAY 'Showing:' RESULT
'GETSELECTED'
SAY 'Selected:' RESULT
