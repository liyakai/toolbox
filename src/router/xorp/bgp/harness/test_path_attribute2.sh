#!/usr/bin/env bash

#
# $XORP: xorp/bgp/harness/test_path_attribute2.sh,v 1.1 2007/12/10 23:26:33 mjh Exp $
#

#
# Test various 
#
# This script started with no arguments will start all required process and
# terminate them at the end of the tests.
#
# Preconditons
# 1) Run a finder process
# 2) Run a FEA process.
# 3) Run a RIB process.
# 4) Run xorp "../xorp_bgp"
# 5) Run "./test_peer -s peer1"
# 6) Run "./test_peer -s peer2"
# 7) Run "./test_peer -s peer3"
# 8) Run "./coord"
#
set -e
. ./setup_paths.sh

# srcdir is set by make for check target
if [ "X${srcdir}" = "X" ] ; then srcdir=`dirname $0` ; fi
. ${srcdir}/xrl_shell_funcs.sh ""
. $BGP_FUNCS ""

onexit()
{
    last=$?
    if [ $last = "0" ]
    then
	echo "$0: Tests Succeeded (BGP: $TESTS)"
    else
	echo "$0: Tests Failed (BGP: $TESTS)"
    fi

    trap '' 0 2
}

trap onexit 0 2

HOST=127.0.0.1
LOCALHOST=$HOST
ID=192.150.187.78
AS=65008
USE4BYTEAS=true
NEXT_HOP=192.150.187.78

# EBGP
PORT1=10001
PEER1_PORT=20001
PEER1_AS=65001

# IBGP
PORT2=10002
PEER2_PORT=20002
PEER2_AS=$AS

# EBGP
PORT3=10003
PEER3_PORT=20002
PEER3_AS=65003

HOLDTIME=5

configure_bgp()
{
    local_config 0.$AS $ID $USE4BYTEAS

    # Don't try and talk to the rib.
    register_rib ""

    PEER=$HOST
    IPTUPLE="$LOCALHOST $PORT1 $PEER $PEER1_PORT"
    add_peer lo $IPTUPLE $PEER1_AS $NEXT_HOP $HOLDTIME
    set_parameter $IPTUPLE MultiProtocol.IPv4.Unicast true
    enable_peer $IPTUPLE

    PEER=$HOST
    IPTUPLE="$LOCALHOST $PORT2 $PEER $PEER2_PORT"
    add_peer lo $IPTUPLE $PEER2_AS $NEXT_HOP $HOLDTIME
    set_parameter $IPTUPLE MultiProtocol.IPv4.Unicast true
    enable_peer $IPTUPLE

    PEER=$HOST
    IPTUPLE="$LOCALHOST $PORT3 $PEER $PEER3_PORT"
    add_peer lo $IPTUPLE $PEER3_AS $NEXT_HOP $HOLDTIME
    set_parameter $IPTUPLE MultiProtocol.IPv4.Unicast true
    enable_peer $IPTUPLE
}

reset()
{
    coord reset

    coord target $HOST $PORT1
    coord initialise attach peer1

    coord target $HOST $PORT2
    coord initialise attach peer2

    coord target $HOST $PORT3
    coord initialise attach peer3
}

test1()
{
    echo "TEST1 (exactly as in test_path_attribute1.sh):"
    echo "	1) Send an update packet with an optional nontransitive path"
    echo "	   attribute. This path attribute should not be propogated"
    echo "         by the BGP process"
    echo "         Note: 4byte ASnums are enabled on the BGP instance, though not on the test peers."
    echo "         This is a simple test that enabling 4-byte ASnums doesn't break anything."

    # Reset the peers
    reset

    # Establish a connection
    coord peer1 establish AS $PEER1_AS holdtime 0 id 192.150.187.100
    coord peer1 assert established

    coord peer2 establish AS $PEER2_AS holdtime 0 id 192.150.187.101
    coord peer2 assert established

    coord peer3 establish AS $PEER3_AS holdtime 0 id 192.150.187.102
    coord peer3 assert established

    ASPATH="$PEER1_AS,1,2,[3,4,5],6,[7,8],9"
    NEXTHOP="20.20.20.20"

    PACKET1="packet update
	origin 2
	aspath $ASPATH
	nexthop $NEXTHOP
	pathattr 0x80,19,1,1
	nlri 10.10.10.0/24
	nlri 20.20.20.20/24"

    PACKET2="packet update
	origin 2
	aspath $ASPATH
	nexthop $NEXTHOP
	localpref 100
	nlri 10.10.10.0/24
	nlri 20.20.20.20/24"

    PACKET3="packet update
	origin 2
	aspath $AS,$ASPATH
	nexthop $NEXT_HOP
	med 1
	nlri 10.10.10.0/24
	nlri 20.20.20.20/24"

    coord peer2 expect $PACKET2
    coord peer3 expect $PACKET3

    coord peer1 send $PACKET1

    sleep 2
    coord peer2 assert queue 0
    coord peer3 assert queue 0

    # Verify that the peers are still connected.
    coord peer1 assert established
    coord peer2 assert established
    coord peer3 assert established
}



TESTS_NOT_FIXED=''
TESTS='test1'

# Include command line
. ${srcdir}/args.sh

if [ $START_PROGRAMS = "yes" ]
then
    CXRL="$CALLXRL -r 10"
    runit $QUIET $VERBOSE -c "$0 -s -c $*" <<EOF
    $XORP_FINDER
    $XORP_FEA_DUMMY           = $CXRL finder://fea/common/0.1/get_target_name
    $XORP_RIB                 = $CXRL finder://rib/common/0.1/get_target_name
    $XORP_BGP                 = $CXRL finder://bgp/common/0.1/get_target_name
    ./test_peer -s peer1      = $CXRL finder://peer1/common/0.1/get_target_name
    ./test_peer -s peer2      = $CXRL finder://peer1/common/0.1/get_target_name
    ./test_peer -s peer3      = $CXRL finder://peer1/common/0.1/get_target_name
    ./coord                   = $CXRL finder://coord/common/0.1/get_target_name
EOF
    trap '' 0
    exit $?
fi

if [ $CONFIGURE = "yes" ]
then
    configure_bgp
fi

for i in $TESTS
do
# Temporary fix to let TCP sockets created by call_xrl pass through TIME_WAIT
    TIME_WAIT=`time_wait_seconds`
    echo "Waiting $TIME_WAIT seconds for TCP TIME_WAIT state timeout"
    sleep $TIME_WAIT
    $i
done

# Local Variables:
# mode: shell-script
# sh-indentation: 4
# End:
