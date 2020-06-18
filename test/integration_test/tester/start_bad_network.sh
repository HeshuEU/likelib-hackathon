#!/bin/bash
IP_LIST="user@192.168.100.141 user@192.168.100.142 user@192.168.100.143"
SOURCE_DIR="../.."
WORK_DIR="${SOURCE_DIR}/test/integration_test/tester"
KEY=${WORK_DIR}/bad_network
OPT="-i $KEY -o StrictHostKeyChecking=no"
for IP in $IP_LIST
do
  echo "Start on $IP"
  chmod 400 $KEY || exit 1
  ssh $OPT $IP 'systemctl stop likelib.service --user' || exit 1
  scp $OPT -r ${SOURCE_DIR}/build/bin $IP:/home/user/test/ || exit 1
  scp $OPT ${WORK_DIR}/config.json $IP:/home/user/test/bin/ || exit 1
  # ssh $OPT $IP 'sudo tc qdisc replace dev ens7 root netem delay 100ms 2000ms distribution normal loss 9% corrupt 9% duplicate 7% reorder 30%' || exit 1
  ssh $OPT $IP 'sudo tc qdisc replace dev ens7 root netem delay 100ms 200ms distribution normal reorder 0% corrupt 0%' || exit 1
  ssh $OPT $IP 'systemctl start likelib.service --user' || exit 1
  echo "Finish on $IP"
done
