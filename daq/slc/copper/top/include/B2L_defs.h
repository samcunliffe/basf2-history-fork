#ifndef __B2LDEFS__
#define __B2LDEFS__

#include <string>
//###################;
// SCROD registers;
//###################;

//# Scrod(%d):AxiVersion(0):*     (register, scrod);
const int SCROD_AxiVersion_FpgaVersion    = 0x200;
const int SCROD_AxiVersion_ScratchPad     = 0x201;
const int SCROD_AxiVersion_DeviceDnaHigh  = 0x202;
const int SCROD_AxiVersion_DeviceDnaLow   = 0x203;
const int SCROD_AxiVersion_FdSerialHigh   = 0x204;
const int SCROD_AxiVersion_FdSerialLow    = 0x205;
const int SCROD_AxiVersion_MasterReset    = 0x206;
const int SCROD_AxiVersion_FpgaReload     = 0x207;
const int SCROD_AxiVersion_UserID = 0x208;
const int SCROD_AxiVersion_AxiReset       = 0x210;
const int SCROD_AxiVersion_UserConstants  = 0x240;
const int SCROD_AxiVersion_BuildString    = 0x280;

//# Scrod(%d):AxiCommon(0):*      (register, scrod);
const int SCROD_AxiCommon_rxReadyCnt  = 0x413;
const int SCROD_AxiCommon_txReadyCnt  = 0x414;
const int SCROD_AxiCommon_statusBits  = 0x470;
const int SCROD_AxiCommon_eventCnt  = 0x481;
const int SCROD_AxiCommon_carrierDataSelect = 0x482;
const int SCROD_AxiCommon_regTrigger  = 0x4AA;
const int SCROD_AxiCommon_trigMask  = 0x4AB;
const int SCROD_AxiCommon_fanoutResetL  = 0x4B0;
const int SCROD_AxiCommon_rollOverEn  = 0x4F0;
const int SCROD_AxiCommon_cntRst  = 0x4FF;

//# Scrod(%d):ScrodPs(0):*      (register, scrod);
const int SCROD_XADC_I_AUXIO_MAX    = 0x1800; // AD03P-AD03N 1V8_AUXIO_VIMON+    LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_PINT_MAX     = 0x1801; // AD04P-AD04N 1V0_PINT_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_GTX1p0_MAX   = 0x1802; // AD05P-AD05N 1V0_GTX_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_2p5_MAX      = 0x1803; // AD06P-AD06N 2V5_VIMON+          LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_GTX1p2_MAX   = 0x1804; // AD07P-AD07N 1V2_GTX_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_MIO2p5_MAX   = 0x1805; // AD08P-AD08N 2V5_MIO_VIMON+      LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_VCCO1p8_MAX  = 0x1806; // AD09P-AD09N 1V8_VCCO_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_VCCO2p5_MAX  = 0x1807; // AD10P-AD10N 2V5_VCCO_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_PLL_PAUX_MAX = 0x1808; // AD11P-AD11N 1V8_PLL_PAUX_VIMON+ LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_BRAM_MAX     = 0x1809; // AD12P-AD12N 1V0_BRAM_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_AUX_MAX      = 0x180a; // AD13P-AD13N 1V8_AUX_VIMON+      LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_INT_MAX      = 0x180b; // AD14P-AD14N 1V0_INT_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_DDR_MAX      = 0x180c; // AD15P-AD15N 1V2_DDR_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_3p3_MAX      = 0x180d; // VP0-VN0     3V3_VIMON+          LT3086 SCROD revB/revB2;
const int SCROD_PS_elfVersion = 0x1810;
const int SCROD_PS_liveCounter  = 0x1811;
const int SCROD_PS_pendingWaves = 0x1812;
const int SCROD_PS_procEventCnt = 0x1813;
const int SCROD_PS_totalWaves = 0x1814;
const int SCROD_PS_temperature_of_data_transceiver = 0x1815;
const int SCROD_PS_temperature_of_trig_transceiver = 0x1816;
const int SCROD_PS_relative_humidity = 0x1817;
const int SCROD_PS_mza_debug0 = 0x1818;
const int SCROD_PS_mza_debug1 = 0x1819;
const int SCROD_PS_mza_debug2 = 0x181a;
const int SCROD_PS_mza_debug3 = 0x181b;
const int SCROD_PS_WallTemp                           = 0x181c;
const int SCROD_PS_temperature_of_humidity_sensor     = 0x181d;
const int SCROD_PS_WallTemp_MAX                       = 0x181e;
const int SCROD_PS_temperature_of_humidity_sensor_MAX = 0x181f;
const int SCROD_PS_featureExtMode = 0x1820;
const int SCROD_PS_cfdThreshold = 0x1821;
const int SCROD_PS_cfdPercent = 0x1822;
const int SCROD_PS_carriersDetected = 0x1823;
const int SCROD_AxiPsReg_ScratchPad0 = 0x1824;
const int SCROD_AxiPsReg_ScratchPad1 = 0x1825;
const int SCROD_AxiPsReg_FIDELITY_ERRORS = 0x1826;
const int SCROD_PS_pedCalcMode  = 0x1827;
const int SCROD_PS_pedCalcStartStatus = 0x1828;
const int SCROD_PS_pedCalcTimeout = 0x1829;
const int SCROD_PS_pedCalcNumAvg  = 0x182A;
const int SCROD_PS_pedCalcErrorMask = 0x182B;
const int SCROD_PS_pedCalcErrorDet = 0x182C;
const int SCROD_PS_readbackPedsStatus       = 0x182D;
const int SCROD_PS_pedRetry                 = 0x182e;

const int SCROD_PS_execute_command_bitfield = 0x182f;
const int COMMAND_BITMASK_SOFT_RESET_PS = 0x00000001; // not implemented;
const int COMMAND_BITMASK_TEST_DDR      = 0x00000002;

const int SCROD_PS_configAsicStatus = 0x1830;
const int SCROD_PS_configAsicErrorDet = 0x1831;
const int SCROD_PS_temperature_of_data_transceiver_MAX = 0x1834;
const int SCROD_PS_temperature_of_trig_transceiver_MAX = 0x1835;
const int SCROD_ADC0_CH0_2V5_ADDR                      = 0x1836;
const int SCROD_ADC0_CH1_3V3_ADDR                      = 0x1837;
const int SCROD_PS_dataExceptionCode                   = 0x1838;
const int SCROD_PS_dataExceptionAddr                   = 0x1839;
const int SCROD_ADC0_CH2_2V5_MIO_ADDR                  = 0x183a;
const int SCROD_ADC0_CH3_1V8_AUXIO_ADDR                = 0x183b;
const int SCROD_ADC1_CH0_1V8_VCCO_ADDR                 = 0x183c;
const int SCROD_ADC1_CH1_2V5_VCCO_ADDR                 = 0x183d;
const int SCROD_ADC1_CH2_1V0_GTX_ADDR                  = 0x183e;
const int SCROD_ADC1_CH3_1V2_GTX_ADDR                  = 0x183f;

const int SCROD_PS_EXTENDED_STATUS_ADDR                  = 0x1840;
const int THIS_IS_FINE_BITMASK                     = 0x00000001;
const int DATA_ABORT_STATUS_BITMASK                = 0x00000002;
const int PREFETCH_ABORT_STATUS_BITMASK            = 0x00000004;
const int DDR_MEMORY_FIDELITY_ERROR_STATUS_BITMASK = 0x00000008;
const int OCM_MEMORY_FIDELITY_ERROR_STATUS_BITMASK = 0x00000010;
const int PSAXIREG_FIDELITY_ERROR_STATUS_BITMASK   = 0x00000020;
const int SCROD_PS_OFFENDING_ADDRESS0_ADDR               = 0x1841;
const int SCROD_PS_OFFENDING_ADDRESS1_ADDR               = 0x1842;
const int SCROD_PS_NUMBER_OF_RESETS_SINCE_AXIPSREG_CLEAR = 0x1843;
const int SCROD_PS_UART_VERBOSITY_ADDR                   = 0x184f;

const int SCROD_PS_EXTENDED_SCRATCHPAD_START_ADDR      = 0x1850;
const int SCROD_PS_EXTENDED_SCRATCHPAD_END_ADDR        = 0x185f;

//# feature extraction modes;
const int featExtractMode_normal  = 0;
const int featExtractMode_passthru  = 1;
const int featExtractMode_full_norm = 2;
const int featExtractMode_full_ped  = 3;
const int featExtractMode_debug = 4;    // not defined yet;
//const std::string featureExtModeList[5] = {"Normal", "Passthru", "FE+raw", "FE+ped-sub", "debug"};
extern const std::string featureExtModeList[5];

//# pedestal calc modes;
const int pedCalcMode_normal  = 0;
const int pedCalcMode_zeros = 1;
const int pedCalcMode_pattern = 2;
extern const std::string pedCalcModeList[3];

const int SCROD_Trigger_    = 0x1000;

//# Scrod(%d):AxiXadc(0):* (register, scrod);
const int SCROD_XADC_Temperature     = 0x1280;
const int SCROD_XADC_V_INT           = 0x1281;
const int SCROD_XADC_V_AUX           = 0x1282;
const int SCROD_XADC_V_BRAM          = 0x1286;
const int SCROD_XADC_V_PINT          = 0x128D;
const int SCROD_XADC_V_PAUX          = 0x128E;
const int SCROD_XADC_V_DDR           = 0x128F;
const int SCROD_XADC_V_RAW1          = 0x1290; // AD00P-AD00N RAW1+                      SCROD revB/revB2;
const int SCROD_XADC_V_RAW2          = 0x1291; // AD01P-AD01N RAW2+                      SCROD revB/revB2;
const int SCROD_XADC_V_RAW3          = 0x1292; // AD02P-AD02N RAW3+                      SCROD revB/revB2;
const int SCROD_XADC_I_AUXIO         = 0x1293; // AD03P-AD03N 1V8_AUXIO_VIMON+    LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_PINT          = 0x1294; // AD04P-AD04N 1V0_PINT_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_GTX1p0        = 0x1295; // AD05P-AD05N 1V0_GTX_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_2p5           = 0x1296; // AD06P-AD06N 2V5_VIMON+          LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_GTX1p2        = 0x1297; // AD07P-AD07N 1V2_GTX_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_MIO2p5        = 0x1298; // AD08P-AD08N 2V5_MIO_VIMON+      LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_VCCO1p8       = 0x1299; // AD09P-AD09N 1V8_VCCO_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_VCCO2p5       = 0x129a; // AD10P-AD10N 2V5_VCCO_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_PLL_PAUX      = 0x129b; // AD11P-AD11N 1V8_PLL_PAUX_VIMON+ LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_BRAM          = 0x129c; // AD12P-AD12N 1V0_BRAM_VIMON+     LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_AUX           = 0x129d; // AD13P-AD13N 1V8_AUX_VIMON+      LT3055 SCROD revB/revB2;
const int SCROD_XADC_I_INT           = 0x129e; // AD14P-AD14N 1V0_INT_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_DDR           = 0x129f; // AD15P-AD15N 1V2_DDR_VIMON+      LT3086 SCROD revB/revB2;
const int SCROD_XADC_I_3p3           = 0x1283; // VP0-VN0     3V3_VIMON+          LT3086 SCROD revB/revB2;
const int SCROD_XADC_Temperature_MAX = 0x12a0;

//# Scrod(%d):Pgp2bAxi(%d):*      (ignore this one; let me know if you find it somewhere!);
const int PGP_CountReset  = 0x600; // 0x600 0x800 0xa00 0xc00;
const int PGP_ResetRx     = 0x601;
const int PGP_Flush       = 0x602;
const int PGP_Loopback    = 0x603;
const int PGP_LocData     = 0x604;
const int PGP_AutoStatus  = 0x605;
const int PGP_Status    = 0x608;
const int PGP_RxRemLinkData     = 0x609;
const int PGP_RxCellErrorCount  = 0x60A;
const int PGP_RxLinkDownCount   = 0x60B;
const int PGP_RxLinkErrorCount  = 0x60C;
const int PGP_RxRemOvfl0Count = 0x60D;
const int PGP_RxRemOvfl1Count = 0x60E;
const int PGP_RxRemOvfl2Count = 0x60F;
const int PGP_RxRemOvfl3Count = 0x610;
const int PGP_RxFrameErrorCount = 0x611;
const int PGP_RxFrameCount      = 0x612;
const int PGP_TxLocOvfl0Count = 0x613;
const int PGP_TxLocOvfl1Count = 0x614;
const int PGP_TxLocOvfl2Count = 0x615;
const int PGP_TxLocOvfl3Count = 0x616;
const int PGP_TxFrameErrorCount = 0x617;
const int PGP_TxFrameCount      = 0x618;
const int PGP_RxClkFreq         = 0x619;
const int PGP_TxClkFreq         = 0x61A;

//###################;
// CARRIER registers;
//###################;

//# Scrod(%d):Carrier(%d):AxiVersion(0):* (register, scrod, carrier);
const int CARRIER_AxiVersion_FpgaVersion  = 0x2200;
const int CARRIER_AxiVersion_ScratchPad   = 0x2201;
const int CARRIER_AxiVersion_DeviceDnaHigh  = 0x2202;
const int CARRIER_AxiVersion_DeviceDnaLow = 0x2203;
const int CARRIER_AxiVersion_FdSerialHigh = 0x2204;
const int CARRIER_AxiVersion_FdSerialLow  = 0x2205;
const int CARRIER_AxiVersion_MasterReset  = 0x2206;
const int CARRIER_AxiVersion_FpgaReload   = 0x2207;
const int CARRIER_AxiVersion_UserConstants      = 0x2240;
const int CARRIER_AxiVersion_BuildStamp         = 0x2280;

//# Scrod(%d):Carrier(%d):CarrierCommon(0):*  (register, scrod, carrier);
const int CARRIER_AxiCommon_rxReadyCnt    = 0x2413;
const int CARRIER_AxiCommon_txReadyCnt    = 0x2414;
const int CARRIER_AxiCommon_statusBits    = 0x2470;
const int CARRIER_AxiCommon_enableStreaming = 0x2480;
const int CARRIER_AxiCommon_eventCnt    = 0x2481;
const int CARRIER_AxiCommon_irsxDataSelect  = 0x2482;
const int CARRIER_AxiCommon_regTrigger    = 0x24AA;
const int CARRIER_AxiCommon_trigMask    = 0x24AB;
const int CARRIER_AxiCommon_iDelayRdy   = 0x24AF;
const int CARRIER_AxiCommon_rollOverEn    = 0x24F0;
const int CARRIER_AxiCommon_cntRst    = 0x24FF;

//# Scrod(%d):Carrier(%d):CarrierPs(0):*    (register, scrod, carrier);
const int CARRIER_PS_asicRegEn  = 0x3800;
const int CARRIER_PS_ampRegEn = 0x3801;
const int CARRIER_PS_ampEn  = 0x3802;
const int CARRIER_PS_calEn  = 0x3803;
const int CARRIER_PS_calCh  = 0x3804;
const int CARRIER_PS_vPed0  = 0x3805;
const int CARRIER_PS_vPed1  = 0x3806;
const int CARRIER_PS_vPed2  = 0x3807;
const int CARRIER_PS_vPed3  = 0x3808;
const int CARRIER_PS_FIDELITY_ERRORS = 0x380a;
const int CARRIER_PS_elfversion = 0x3810;
const int CARRIER_PS_ID   = 0x3811;
const int CARRIER_PS_WallTemp     = 0x3812;
const int CARRIER_PS_ASIC01Temp   = 0x3813;
const int CARRIER_PS_ASIC23Temp   = 0x3814;
const int CARRIER_PS_status       = 0x3815;
const int CARRIER_PS_livecounter  = 0x3816;
const int CARRIER_PS_MZA_DEBUG0     = 0x3818;
const int CARRIER_PS_MZA_DEBUG1     = 0x3819;
const int CARRIER_PS_MZA_DEBUG2     = 0x381a;
const int CARRIER_PS_MZA_DEBUG3     = 0x381b;
const int CARRIER_PS_WallTemp_MAX   = 0x381c;
const int CARRIER_PS_ASIC01Temp_MAX = 0x381d;
const int CARRIER_PS_ASIC23Temp_MAX = 0x381e;

const int CARRIER_PS_ADC0_CH0_2V5_ASIC0     = 0x3820;
const int CARRIER_PS_ADC0_CH1_2V5_ASIC1     = 0x3821;
const int CARRIER_PS_ADC0_CH2_2V5_ASIC2     = 0x3822;
const int CARRIER_PS_ADC0_CH3_2V5_ASIC3     = 0x3823;
const int CARRIER_PS_ADC0_CH4_2I5_ASIC0     = 0x3824;
const int CARRIER_PS_ADC0_CH5_2I5_ASIC1     = 0x3825;
const int CARRIER_PS_ADC0_CH6_2I5_ASIC2     = 0x3826;
const int CARRIER_PS_ADC0_CH7_2I5_ASIC3     = 0x3827;
const int CARRIER_PS_ADC0_TEMP              = 0x3828;
const int CARRIER_PS_ADC0_CH4_2I5_ASIC0_MAX = 0x3829;
const int CARRIER_PS_ADC0_CH5_2I5_ASIC1_MAX = 0x382a;
const int CARRIER_PS_ADC0_CH6_2I5_ASIC2_MAX = 0x382b;
const int CARRIER_PS_ADC0_CH7_2I5_ASIC3_MAX = 0x382c;
const int CARRIER_PS_ADC0_TEMP_MAX          = 0x382d;
const int CARRIER_AxiPsReg_ScratchPad0 = 0x382e;
const int CARRIER_AxiPsReg_ScratchPad1 = 0x382f;

//CARRIER_PS_ADC1_CH0_FLOATING    = 0x3830 // input not connected;
const int CARRIER_PS_ADC1_CH1_V_RAW1      = 0x3831;
const int CARRIER_PS_ADC1_CH2_V_RAW2      = 0x3832;
const int CARRIER_PS_ADC1_CH3_V_RAW3      = 0x3833;
const int CARRIER_PS_ADC1_CH4_3I7_VAMPS01 = 0x3834;
const int CARRIER_PS_ADC1_CH5_3I7_VAMPS23 = 0x3835;
const int CARRIER_PS_ADC1_CH6_3V7_VAMPS01 = 0x3836;
const int CARRIER_PS_ADC1_CH7_3V7_VAMPS23 = 0x3837;
const int CARRIER_PS_ADC1_TEMP            = 0x3838;
const int CARRIER_PS_ADC1_TEMP_MAX        = 0x3839;

const int CARRIER_PS_EXTENDED_STATUS                       = 0x3840;
const int CARRIER_PS_OFFENDING_ADDRESS0                    = 0x3841;
const int CARRIER_PS_OFFENDING_ADDRESS1                    = 0x3842;
const int CARRIER_PS_NUMBER_OF_RESETS_SINCE_AXIPSREG_CLEAR = 0x3843;

const int CARRIER_PS_EXTENDED_SCRATCHPAD_START_ADDR      = 0x3850;
const int CARRIER_PS_EXTENDED_SCRATCHPAD_END_ADDR        = 0x385f;

//# Scrod(%d):Carrier(%d):AxiIrsXDirect(%d):* (register, scrod, carrier, asic);
const int CARRIER_IRSX_irsxDirect = 0x2800;
const int CARRIER_IRSX_irsxRegWordOut = 0x2801;

//# Scrod(%d):Carrier(%d):AxiIrsX(%d):*   (register, scrod, carrier, asic);
const int CARRIER_IRSX_spgIn    = 0x2601;
const int CARRIER_IRSX_regLoadPeriod  = 0x2610;
const int ideal_IRSX_regLoadPeriod      = 0;
const int CARRIER_IRSX_regLatchPeriod = 0x2611;
const int ideal_IRSX_regLatchPeriod     = 0;
const int CARRIER_IRSX_regClr   = 0x2612;
const int CARRIER_IRSX_wrAddrMode = 0x2620;
const int CARRIER_IRSX_wrAddrBitSlip  = 0x2621;
const int CARRIER_IRSX_wrAddrFixed  = 0x2622;
const int CARRIER_IRSX_wrAddrStart  = 0x2623;
const int CARRIER_IRSX_wrAddrStop = 0x2624;
const int CARRIER_IRSX_wrAddrJunk = 0x2625;
const int CARRIER_IRSX_writesAfterTrig  = 0x2626;
const int CARRIER_IRSX_readoutMode  = 0x2630;
const int CARRIER_IRSX_readoutBitSlip = 0x2631;
const int CARRIER_IRSX_readoutLookback  = 0x2632;
const int CARRIER_IRSX_readoutWindows = 0x2633;
const int CARRIER_IRSX_readoutChannels  = 0x2634;
const int CARRIER_IRSX_convertResetWait = 0x2635;
const int CARRIER_IRSX_hsDataDelay  = 0x2636;
const int CARRIER_IRSX_enableTestPattern = 0x2640;
const int CARRIER_IRSX_scaler01all  = 0x2650;
const int CARRIER_IRSX_scaler23all  = 0x2651;
const int CARRIER_IRSX_scaler45all  = 0x2652;
const int CARRIER_IRSX_scaler67all  = 0x2653;
const int CARRIER_IRSX_scaler0    = 0x2654;
const int CARRIER_IRSX_scaler1    = 0x2655;
const int CARRIER_IRSX_scaler2    = 0x2656;
const int CARRIER_IRSX_scaler3    = 0x2657;
const int CARRIER_IRSX_scaler4    = 0x2658;
const int CARRIER_IRSX_scaler5    = 0x2659;
const int CARRIER_IRSX_scaler6    = 0x265A;
const int CARRIER_IRSX_scaler7    = 0x265B;
const int CARRIER_IRSX_scaler01dual = 0x265C;
const int CARRIER_IRSX_scaler23dual = 0x265D;
const int CARRIER_IRSX_scaler45dual = 0x265E;
const int CARRIER_IRSX_scaler67dual = 0x265F;
const int CARRIER_IRSX_trigWidth0 = 0x2660;
const int CARRIER_IRSX_trigWidth1 = 0x2661;
const int CARRIER_IRSX_trigWidth2 = 0x2662;
const int CARRIER_IRSX_trigWidth3 = 0x2663;
const int CARRIER_IRSX_trigWidth4 = 0x2664;
const int CARRIER_IRSX_trigWidth5 = 0x2665;
const int CARRIER_IRSX_trigWidth6 = 0x2666;
const int CARRIER_IRSX_trigWidth7 = 0x2667;
const int CARRIER_IRSX_dualTrigWidth0 = 0x2668;
const int CARRIER_IRSX_dualTrigWidth1 = 0x2669;
const int CARRIER_IRSX_dualTrigWidth2 = 0x266A;
const int CARRIER_IRSX_dualTrigWidth3 = 0x266B;
const int CARRIER_IRSX_wrAddrSpy  = 0x2680;
const int CARRIER_IRSX_tpgData    = 0x2681;
const int CARRIER_IRSX_scalerMontiming  = 0x2682;
const int CARRIER_IRSX_phaseRead  = 0x2683;
const int CARRIER_IRSX_dummyTrigEn  = 0x2690;
const int CARRIER_IRSX_dummyTrigChs = 0x2691;

//# wrAddr modes;
const int wrAddrMode_Calibration = 0;
const int wrAddrMode_Cyclic      = 1;
const int wrAddrMode_Full        = 2;
const int wrAddrMode_Unused      = 3;


//# readout modes;
const int readoutMode_Calibration = 0;
const int readoutMode_Forced      = 1;
const int readoutMode_Lookback    = 2;
const int readoutMode_ROI         = 3;


//# Scrod(%d):Carrier(%d):AxiXadc(0):*    (register, scrod, carrier);
const int CARRIER_XADC_Temperature     = 0x3C80;
const int CARRIER_XADC_V_INT           = 0x3C81;
const int CARRIER_XADC_V_AUX           = 0x3C82;
const int CARRIER_XADC_V_BRAM          = 0x3C86;
const int CARRIER_XADC_V_PINT          = 0x3C8D;
const int CARRIER_XADC_V_PAUX          = 0x3C8E;
const int CARRIER_XADC_V_DDR           = 0x3C8F; // meaningless because DDR is not used on carrier;
const int CARRIER_XADC_Temperature_MAX = 0x3Ca0;

#endif
