#!/bin/bash

rm -f /var/log/ico/test_result_ico_log_*.log*
rm -fr test_result_ico_log

export LD_LIBRARY_PATH=../src/.libs:$LD_LIBRARY_PATH
# N P T D I W C E
# o x x x x x x x
# x o x x x x x x
# x x o x x x x x
# x x x o x x x x
# x x x x o x x x
# x x x x x o x x
# x x x x x x o x
# x x x x x x x o
./tst_ico_log none
./tst_ico_log performance
./tst_ico_log trace
./tst_ico_log debug
./tst_ico_log info
./tst_ico_log warning
./tst_ico_log critical
./tst_ico_log error

# N P T D I W C E
# o o o o o o o o
# o x o o o o o o
# o o x o o o o o
# o o o x o o o o
# o o o o x o o o
# o o o o o x o o
# o o o o o o x o
# o o o o o o o x
./tst_ico_log performance,trace,debug,info,warning,critical,error
./tst_ico_log trace,debug,info,warning,critical,error
./tst_ico_log performance,debug,info,warning,critical,error
./tst_ico_log performance,trace,info,warning,critical,error
./tst_ico_log performance,trace,debug,warning,critical,error
./tst_ico_log performance,trace,debug,info,critical,error
./tst_ico_log performance,trace,debug,info,warning,error
./tst_ico_log performance,trace,debug,info,warning,critical

LOG_N=/var/log/ico/test_result_ico_log_00000000.log
LOG_P=/var/log/ico/test_result_ico_log_00000200.log
LOG_T=/var/log/ico/test_result_ico_log_00000100.log
LOG_D=/var/log/ico/test_result_ico_log_00000080.log
LOG_I=/var/log/ico/test_result_ico_log_00000040.log
LOG_W=/var/log/ico/test_result_ico_log_00000010.log
LOG_C=/var/log/ico/test_result_ico_log_00000008.log
LOG_E=/var/log/ico/test_result_ico_log_00000004.log
LOG_PTDIWCE=/var/log/ico/test_result_ico_log_000003DC.log
LOG_TDIWCE=/var/log/ico/test_result_ico_log_000001DC.log
LOG_PDIWCE=/var/log/ico/test_result_ico_log_000002DC.log
LOG_PTIWCE=/var/log/ico/test_result_ico_log_0000035C.log
LOG_PTDWCE=/var/log/ico/test_result_ico_log_0000039C.log
LOG_PTDICE=/var/log/ico/test_result_ico_log_000003CC.log
LOG_PTDIWE=/var/log/ico/test_result_ico_log_000003D4.log
LOG_PTDIWC=/var/log/ico/test_result_ico_log_000003D8.log

######
LOG=$LOG_N

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=none             : [\e[32m OK \e[m]"
else
	echo -e "test_case level=none             : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_P

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=performance only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=performance only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_T

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=trace       only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=trace       only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_D

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=debug       only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=debug       only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_I

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=info        only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=info        only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_W

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a $RET_W -eq 0 -a ! $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=warning     only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=warning     only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_C

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=critical    only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=critical    only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_E

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a ! $RET_T -eq 0 -a ! $RET_D -eq 0 -a ! $RET_I -eq 0 -a ! $RET_W -eq 0 -a ! $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=error       only : [\e[32m OK \e[m]"
else
	echo -e "test_case level=error       only : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTDIWCE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=all              : [\e[32m OK \e[m]"
else
	echo -e "test_case level=                 : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_TDIWCE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ ! $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=performance off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=performance off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PDIWCE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a ! $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=trace       off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=trace       off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTIWCE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a ! $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=debug       off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=debug       off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTDWCE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a ! $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=info        off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=info        off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTDICE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a ! $RET_W -eq 0 -a $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=warning     off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=warning     off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTDIWE

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a ! $RET_C -eq 0 -a $RET_E -eq 0 ]
then
	echo -e "test_case level=critical    off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=critical    off  : [\e[31m NG \e[m]"
fi

######
LOG=$LOG_PTDIWC

grep PRF $LOG > /dev/null 2>&1
RET_P=$?
grep TRA $LOG > /dev/null 2>&1
RET_T=$?
grep DBG $LOG > /dev/null 2>&1
RET_D=$?
grep INF $LOG > /dev/null 2>&1
RET_I=$?
grep WRN $LOG > /dev/null 2>&1
RET_W=$?
grep CRI $LOG > /dev/null 2>&1
RET_C=$?
grep ERR $LOG > /dev/null 2>&1
RET_E=$?
if [ $RET_P -eq 0 -a $RET_T -eq 0 -a $RET_D -eq 0 -a $RET_I -eq 0 -a $RET_W -eq 0 -a $RET_C -eq 0 -a ! $RET_E -eq 0 ]
then
	echo -e "test_case level=error       off  : [\e[32m OK \e[m]"
else
	echo -e "test_case level=error       off  : [\e[31m NG \e[m]"
fi

mkdir test_result_ico_log
mv /var/log/ico/test_result_ico_log_*.log* test_result_ico_log
