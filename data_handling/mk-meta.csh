#! /bin/tcsh -f 

#setenv BELLE_MESSAGE_LEVEL DDEBUG

####################################
setenv module "FindAttributes"
setenv moduledir ./
####################################


#setenv BASF /belle/belle/b20070528_1559/i686-pc-linux-gnu/opt/../debug/bin/basf

#================== BASF Main ====================
basf<<EOF >&./outdata/log-test.log

module register $module 
path create main
path add_module main $module 

module put_parameter $module DataType\on_resonance
module put_parameter $module OutputFlag\1
module put_parameter $module SkimFileName\./test.list

nprocess $nPro
initialize



terminate


EOF

