#!/bin/sh

########################
#
# Setting value
#
########################
# directory to put test's result in
rslt_dir="./result"
log_dir="${rslt_dir}/full_log"
# number of tests
num_tst_loop=1

# test log tag
tst_tag="TestCase"

# log file name (common)
date_str=`date '+%Y%m%d'`
time_str=`date '+%H%M'`
file_str="${date_str}_${time_str}.txt"
srv_file_str="server_${file_str}"
clt_file_str="client_${file_str}"

# set library path
export LD_LIBRARY_PATH=../src/.libs:$LD_LIBRARY_PATH

########################
#
# Make a directory to put test's result in
#
########################
if [ ! -e ${rslt_dir} ]; then
    mkdir ${rslt_dir}
fi
if [ ! -e ${log_dir} ]; then
    mkdir ${log_dir}
fi

########################
#
# Set the number of the test's loop
# (if argument exists)
#
########################
if [ $# -ne 0 ]; then
    if expr "$1" : '[0-9]*' > /dev/null ; then
        num_tst_loop=$1
    fi
fi

########################
#
# Function
#
########################
kill_old_proc()
{
    pids=(`ps -ef | grep tst_ico_uws | grep -v grep | awk '{ print $2 }'`)
    for pid in ${pids[*]}
    do
        kill -9 ${pid}
    done
}

check_srv_no_exist()
{
    while :
    do
        proc_srv=`pgrep -lf "$1"`
        if [ -n "${proc_srv}" ]; then
            break
        fi
        # sleep while process of server does not exist
        usleep 100000
    done
}

check_srv_exist()
{
    while :
    do
        proc_srv=`pgrep -lf "$1"`
        if [ -z "${proc_srv}" ]; then
            break
        fi
        # sleep while process of server exists
        sleep 1
    done
}

print_result()
{
    local l_type="$1"
    local l_log="$2"
    local l_log_total="$3"
    local l_cnt_ok=0
    local l_cnt_ng=0
    local l_str=""

    # title
    echo "" | tee -a ${l_log_total}
    echo "----- ${l_type} result -----" | tee -a ${l_log_total}
    # count OK/NG, and output console and file
    l_cnt_ok=`grep ${tst_tag} ${l_log} | grep "OK" | wc -l`
    l_cnt_ng=`grep ${tst_tag} ${l_log} | grep "NG" | wc -l`
    l_str="<<Results Total>> OK: ${l_cnt_ok}, NG: ${l_cnt_ng}"
    l_str="${l_str} (num of tests: ${num_tst_loop})"
    echo "${l_str}" | tee -a ${l_log_total}
    # grep test result, and output to file
    grep ${tst_tag} ${l_log} | tee -a ${l_log_total}
}

exec_test()
{
    local l_tst_no="0$3"
    local l_app_srv="./$1"
    local l_app_clt="./$2"

    local l_log_srv="${log_dir}/tst${l_tst_no}_${srv_file_str}"
    local l_log_clt="${log_dir}/tst${l_tst_no}_${clt_file_str}"
    local l_log_total="${rslt_dir}/tst${l_tst_no}_${file_str}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_tst_loop}`
    do
        # execute server
        ${l_app_srv} >> ${l_log_srv} &
        # sleep while process of server does not exist
        check_srv_no_exist ${l_app_srv}
        # execute client
        ${l_app_clt} >> ${l_log_clt}

        # sleep while process of server exists
        check_srv_exist ${l_app_srv}
    done

    print_result "Server" ${l_log_srv} ${l_log_total}
    print_result "Client" ${l_log_clt} ${l_log_total}
    sleep 1
}

exec_test_multi_clt()
{
    local l_tst_no="0$4"
    local l_app_srv="./$1"
    local l_app_clt="./$2"
    local l_app_clt_sec="./$3"

    local l_log_srv="${log_dir}/tst${l_tst_no}_${srv_file_str}"
    local l_log_clt="${log_dir}/tst${l_tst_no}_client0_${file_str}"
    local l_log_clt_sec="${log_dir}/tst${l_tst_no}_client1_${file_str}"
    local l_log_total="${rslt_dir}/tst${l_tst_no}_${file_str}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_tst_loop}`
    do
        # execute server
        ${l_app_srv} >> ${l_log_srv} &
        # sleep while process of server does not exist
        check_srv_no_exist ${l_app_srv}
        # execute client
        ${l_app_clt} >> ${l_log_clt} &
        usleep 100
        ${l_app_clt_sec} >> ${l_log_clt_sec}

        # sleep while process of server exists
        check_srv_exist ${l_app_srv}
    done

    print_result "Server" ${l_log_srv} ${l_log_total}
    print_result "Client 0" ${l_log_clt} ${l_log_total}
    print_result "Client 1" ${l_log_clt_sec} ${l_log_total}
    sleep 1
}

exec_test_multi_srv()
{
    local l_tst_no="0$4"
    local l_app_srv="./$1"
    local l_app_srv_sec="./$2"
    local l_app_clt="./$3"

    local l_log_srv="${log_dir}/tst${l_tst_no}_server0_${file_str}"
    local l_log_srv_sec="${log_dir}/tst${l_tst_no}_server0_${file_str}"
    local l_log_clt="${log_dir}/tst${l_tst_no}_${clt_file_str}"
    local l_log_total="${rslt_dir}/tst${l_tst_no}_${file_str}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_tst_loop}`
    do
        # execute server
        ${l_app_srv} >> ${l_log_srv} &
        usleep 500
        ${l_app_srv_sec} >> ${l_log_srv_sec} &
        # sleep while process of server does not exist
        check_srv_no_exist ${l_app_srv}
        check_srv_no_exist ${l_app_srv_sec}
        # execute client
        ${l_app_clt} >> ${l_log_clt}

        # sleep while process of server exists
        check_srv_exist ${l_app_srv}
        check_srv_exist ${l_app_srv_sec}
    done

    print_result "Server 0" ${l_log_srv} ${l_log_total}
    print_result "Server 1" ${l_log_srv_sec} ${l_log_total}
    print_result "Client" ${l_log_clt} ${l_log_total}
    sleep 1
}

########################
#
# Test Start
#
########################
echo ""
echo "=== API Test Start ==="

########################
#
# API Test (1)
# 1 server / 1 client
#
########################
# application
app_srv="tst_ico_uws_server -p 8080"
app_clt="tst_ico_uws_client -p 8080"

# test & output result
echo ""
tst_no=1
echo "=== API Test ($tst_no) <<1 server, 1 client>> Start ==="
exec_test "${app_srv}" "${app_clt}" ${tst_no}
echo "=== API Test ($tst_no) <<1 server, 1 client>> End ==="


########################
#
# API Test (2)
# 1 server / 2 client
#
########################
# application
app_srv="tst_ico_uws_server -p 8080"
app_clt="tst_ico_uws_client -p 8080"

# test & output result
echo ""
tst_no=2
echo "=== API Test ($tst_no) <<1 server, 2 client>> Start ==="
exec_test_multi_clt "${app_srv}" "${app_clt}" "${app_clt}" ${tst_no}
echo "=== API Test ($tst_no) <<1 server, 2 client>> End ==="


########################
#
# API Test (3)
# 2 server / 1 client (2 thread)
#
########################
# application
app_srv="tst_ico_uws_server -p 8080"
app_srv_sec="tst_ico_uws_server -p 9090"
app_clt="tst_ico_uws_multi_client"

# test & output result
echo ""
tst_no=3
echo "=== API Test ($tst_no) <<2 server, 1 client>> Start ==="
exec_test_multi_srv "${app_srv}" "${app_srv_sec}" "${app_clt}" ${tst_no}
echo "=== API Test ($tst_no) <<2 server, 1 client>> End ==="


########################
#
# API Test (4)
# 1 server (2 thread) / 1 client (2 thread)
#
########################
# application
app_srv="tst_ico_uws_multi_server"
app_clt="tst_ico_uws_multi_client"

# test & output result
echo ""
tst_no=4
echo "=== API Test ($tst_no) <<multi server, multi client>> Start ==="
exec_test "${app_srv}" "${app_clt}" ${tst_no}
echo "=== API Test ($tst_no) <<multi server, multi client>> End ==="


########################
#
# API Test (5)
# 1 server (2 thread) / 2 client (2 process)
#
########################
# application
app_srv="tst_ico_uws_multi_server"
app_clt="tst_ico_uws_client -p 8080"
app_clt_sec="tst_ico_uws_client -p 9090"

# test & output result
echo ""
tst_no=5
echo "=== API Test ($tst_no) <<multi server, 2 client>> Start ==="
exec_test_multi_clt "${app_srv}" "${app_clt}" "${app_clt_sec}" ${tst_no}
echo "=== API Test ($tst_no) <<multi server, 2 client>> End ==="


########################
#
# Test End
#
########################
echo "=== API Test End ==="
echo ""

