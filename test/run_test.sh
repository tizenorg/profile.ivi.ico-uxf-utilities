#!/bin/sh

########################
#
# Setting value
#
########################

# API Tests Data (information, num of test items)
### (1) 1 server / 1 client
### (2) 1 server / 2 client
### (3) 2 server / 1 client (2 thread)
### (4) 1 server (2 thread) / 1 client (2 thread)
### (5) 1 server (2 thread) / 2 client (2 process)
test_list=(
 "1 server, 1 client"
 "1 server, 2 clients"
 "2 server, multi client"
 "multi server, multi client"
 "multi server, 2 clients"
)

# number of test items (Test(1) Test(2) ..)
num_server_test=(15 22 15 18 18)
num_client_test=(11 11 18 18 11)

# directory to put test's result in
rslt_dir="./result"
log_dir="${rslt_dir}/`date '+%Y%m%d_%H%M'`"

# number of loop tests
num_loop=1

# test log tag
tst_tag="TestCase"

# log file name
log_server="server.txt"
log_client="client.txt"
log_total="${log_dir}/`date '+%Y%m%d'`_total.txt"
log_summary="${log_dir}/`date '+%Y%m%d'`_summary.txt"

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
        num_loop=$1
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
	local l_id=0

    while :
    do
        proc_srv=`pgrep -lf "$1"`
        if [ -n "${proc_srv}" ]; then
            break
        fi
        # sleep while process of server does not exist
		sleep 0.001

		l_id=`expr ${l_id} + 1`
		if [ ${l_id} -gt 500 ]; then
			echo "Error: Server does not exist"
			exit 1
		fi
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

print_title()
{
    local l_type="$1"   ## start/end
    local l_num="$2"    ## number of Test
    local l_info="$3"   ## test information

    echo "=== API Test (${l_num}) <<${l_info}>> ${l_type} ===" | tee -a ${log_total}
    echo "" >> ${log_total}

    if [ "${l_type}" = "End" ]; then
        echo "" | tee -a ${log_total}
    fi
}

print_result()
{
    local l_type="$1"   ## server/client
    local l_log="$2"
    local l_num="$3"    ## number of Test
    local l_cnt_ok=0
    local l_cnt_ng=0
    local l_str=""

    # get number of test items
    local l_item=0
    local l_id=`expr ${l_num} - 1`
    case ${l_type} in
        *Server*)
            l_item="${num_server_test[${l_id}]}" ;;
        *)
            l_item="${num_client_test[${l_id}]}" ;;
    esac
    l_item=`expr ${l_item} \* ${num_loop}`

    # title
    echo "----- ${l_type} result -----" | tee -a ${log_total}

    # count OK/NG, and output console and file
    l_cnt_ok=`grep ${tst_tag} ${l_log} | grep "OK" | wc -l`
    l_cnt_ng=`grep ${tst_tag} ${l_log} | grep "NG" | wc -l`
    l_str="API Test ($3) <<Results>> OK: ${l_cnt_ok}, NG: ${l_cnt_ng} [${l_type}, Total Items: ${l_item}]"

    echo "${l_str}" | tee -a ${log_total}

    # grep test result, and output to file
    grep ${tst_tag} ${l_log} >> ${log_total}
}

exec_test()
{
    local l_tst_no="$3"
    local l_app_srv="./$1"
    local l_app_clt="./$2"

    local l_log_srv="${log_dir}/tst0${l_tst_no}_${log_server}"
    local l_log_clt="${log_dir}/tst0${l_tst_no}_${log_client}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_loop}`
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

    print_result "Server" ${l_log_srv} ${l_tst_no}
    print_result "Client" ${l_log_clt} ${l_tst_no}
    sleep 1
}

exec_test_multi_clt()
{
    local l_tst_no="$4"
    local l_app_srv="./$1"
    local l_app_clt="./$2"
    local l_app_clt_sec="./$3"

    local l_log_srv="${log_dir}/tst0${l_tst_no}_${log_server}"
    local l_log_clt="${log_dir}/tst0${l_tst_no}_no0_${log_client}"
    local l_log_clt_sec="${log_dir}/tst0${l_tst_no}_no1_${log_client}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_loop}`
    do
        # execute server
        ${l_app_srv} >> ${l_log_srv} &
        # sleep while process of server does not exist
        check_srv_no_exist ${l_app_srv}
        # execute client
        ${l_app_clt} >> ${l_log_clt} &
        ${l_app_clt_sec} >> ${l_log_clt_sec}

        # sleep while process of server exists
        check_srv_exist ${l_app_srv}
    done

    print_result "Server" ${l_log_srv} ${l_tst_no}
    print_result "Client 0" ${l_log_clt} ${l_tst_no}
    print_result "Client 1" ${l_log_clt_sec} ${l_tst_no}
    sleep 1
}

exec_test_multi_srv()
{
    local l_tst_no="$4"
    local l_app_srv="./$1"
    local l_app_srv_sec="./$2"
    local l_app_clt="./$3"

    local l_log_srv="${log_dir}/tst0${l_tst_no}_no0_${log_server}"
    local l_log_srv_sec="${log_dir}/tst0${l_tst_no}_no1_${log_server}"
    local l_log_clt="${log_dir}/tst0${l_tst_no}_${log_client}"

    # kill old process if exists
    kill_old_proc

    sleep 1

    for i in `seq 1 ${num_loop}`
    do
        # execute server
        ${l_app_srv} >> ${l_log_srv} &
		sleep 0.01
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

    print_result "Server 0" ${l_log_srv} ${l_tst_no}
    print_result "Server 1" ${l_log_srv_sec} ${l_tst_no}
    print_result "Client" ${l_log_clt} ${l_tst_no}
    sleep 1
}

########################
#
# API Test Main
#
########################
echo ""
echo "=== API Test Start ==="

total=${#test_list[*]}
id=0
while [ $id -lt ${total} ];
do
    info="${test_list[$id]}"
    id=`expr $id + 1`

    print_title "Start" "${id}" "${info}"

    # exec test
    if [ ${id} -eq "1" ]; then
        app_srv="tst_ico_uws_server -p 8080"
        app_clt="tst_ico_uws_client -p 8080"
        exec_test "${app_srv}" "${app_clt}" ${id}
    elif [ ${id} -eq "2" ]; then
        app_srv="tst_ico_uws_server -p 8080"
        app_clt="tst_ico_uws_client -p 8080"
        exec_test_multi_clt "${app_srv}" "${app_clt}" "${app_clt}" ${id}
    elif [ ${id} -eq "3" ]; then
        app_srv="tst_ico_uws_server -p 8080"
        app_srv_sec="tst_ico_uws_server -p 9090"
        app_clt="tst_ico_uws_multi_client"
        exec_test_multi_srv "${app_srv}" "${app_srv_sec}" "${app_clt}" ${id}
    elif [ ${id} -eq "4" ]; then
        app_srv="tst_ico_uws_multi_server"
        app_clt="tst_ico_uws_multi_client"
        exec_test "${app_srv}" "${app_clt}" ${id}
    elif [ ${id} -eq "5" ]; then
        app_srv="tst_ico_uws_multi_server"
        app_clt="tst_ico_uws_client -p 8080"
        app_clt_sec="tst_ico_uws_client -p 9090"
        exec_test_multi_clt "${app_srv}" "${app_clt}" "${app_clt_sec}" ${id}
    fi

    print_title "End" "${id}" "${info}"
done

echo ""
echo ""

echo "[Summary] Results of API Tests (${num_loop} Loops)" | tee -a ${log_summary}
grep "Total" ${log_total} | tee -a ${log_summary}
echo ""

echo "=== API Test End ==="
echo ""
