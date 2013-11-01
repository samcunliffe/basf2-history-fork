#!/bin/bash

export B2SC_SERVER_HOST=`hostname`;

## setup for slow control system libraries ##
export B2SLC_PATH=$PWD
export PATH=$PATH:$B2SLC_PATH/bin
export LD_LIBRARY_PATH=$B2SLC_PATH/lib:$LD_LIBRARY_PATH

## setup for slow control system java libraries ##
export JAVA_HOME=/usr/java/latest/
export PATH=$JAVA_HOME/bin:$PATH
export CLASSPATH=.:$JAVA_HOME/jre/lib:$JAVA_HOME/lib\
:$JAVA_HOME/lib/tools.jar\
:$B2SLC_PATH/javalib/mysql-connector-java-5.1.26-bin.jar

## NSM configuration ##
export NSM2_HOST=${B2SC_SERVER_HOST}
export NSM2_PORT=8222
export NSM2_SHMKEY=8222
export NSM2_INCDIR=$B2SLC_PATH/bin
export NSMD2_DEBUG=1
export NSMD2_LOGDIR=$B2SLC_PATH/log/nsm2

## setup for slow control database access ##
export B2SC_DB_HOST=${B2SC_SERVER_HOST};
export B2SC_DB_NAME="b2slow_test";
export B2SC_DB_USER="slcdaq";
export B2SC_DB_PASS="slcdaq";
export B2SC_DB_PORT=3306;

## setup for slow control database access ##
export B2SC_XML_ENTRY="CDC"
export B2SC_XML_PATH=$B2SLC_PATH/config/cdc_test
export B2SC_CPRLIB_PATH=$B2SLC_PATH/lib/cdc_test

## setup for slow control database access ##
export B2SC_DQM_CONFIG_PATH=$B2SLC_PATH/dqmserver/config/test.conf
export B2SC_DQM_MAP_PATH=$B2SLC_PATH/log
export B2SC_DQM_LIB_PATH=$B2SLC_PATH/dqmserver/lib

## setup for directory 
export HSLB_FIRMEWATE_PATH=${B2SLC_PATH}/cprcontrold/mgt/
export TTRX_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FTSW_FIRMEWATE_PATH=/home/usr/nakao/daq/ftsw/
export FEE_FIRMWARE_PATH=/home/usr/tkonno/cdc