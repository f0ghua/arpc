#!/bin/bash
if [[ ! -n $PATHBK ]]
then
	export PATHBK=$PATH
fi

INSTALL_DIR=/home/fog/work/thrift/install
ARPC_LIB_PATH=/home/fog/work/arpc/lib/src/thrift
export PATH=$PATHBK:${INSTALL_DIR}/bin
export LD_LIBRARY_PATH=${ARPC_LIB_PATH}

echo "set PATH=$PATH"
echo "set LD_LIBRARY_PATH=$LD_LIBRARY_PATH"

