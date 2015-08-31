#!/bin/bash

export EXPOSE_MASTER_HOST=127.0.0.1
export EXPOSE_MASTER_PORT=8001

cd $HOME/libmsr2.0/demoapps/powsched

# start the cache server to support the run
cache -p $EXPOSE_MASTER_PORT &
cachepid=$!

striplf cluster.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf newstate.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf metrictoggles.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf availmetrics.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf kickactuator.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf powertargets.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf poweractuals.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
striplf powerin.tbl | cacheclient -h $EXPOSE_MASTER_HOST -p $EXPOSE_MASTER_PORT
daemonreg "$(striplf stateupdate.gapl)"
daemonreg "$(striplf powerstate.gapl)"

haltwait
sleep 10
kill $cachepid
echo "Exit condition occured. Killed cacheserver"
