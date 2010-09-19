#ifndef DPS_ERRORCODE_H_
#define DPS_ERRORCODE_H_

///////////////////////////////////////////////////////////////////////////////////
//
//每个模块分配100个错误码，分系统统一给出各个模块错误码偏移，各个模块分别定义自己的错误码
//
//例如AP模块可以定义错误码如下：
//		#include "DpsErrorCode.h
//
//		#ifndef OPP_AP_ERRORCODE_H_
//		#define OPP_AP_ERRORCODE_H_
//
//		#define ERROR_IEP_WRONG		OPP_AP_ERR_OFFSET + 1
//		#define ERROR_PARSE_XML		OPP_AP_ERR_OFFSET + 2
//		#define ERROR_WRITE_FILE	OPP_AP_ERR_OFFSET + 3
//  ..................
//
//		#define WARNING_SYN_WRONG		OPP_AP_WARNING_OFFSET - 1
//
//		#endif /*OPP_AP_ERRORCODE_H_*/
//
///////////////////////////////////////////////////////////////////////////////////


//////////////////////////////////////////////////////////////////////////////////
//HJ1AB 软件包各模块错误码偏移

//OPP_FlowCtrl:流程控制模块
#define FLOWCTRL_ERR_OFFSET    20100

//OPP_RDR：原始数据回放模块
#define OPP_RDR_ERR_OFFSET     20200

//OPP_QCDV：快视模块
#define OPP_QCDV_ERR_OFFSET    20300

//OPP_CA：编目模块
#define OPP_CA_ERR_OFFSET      20400

//OPP_BG：浏览图生成模块
#define OPP_BG_ERR_OFFSET      20500

//OPP_DX：数据提取模块
#define OPP_DX_ERR_OFFSET      20600

//OPP_AP：辅助数据处理模块
#define OPP_AP_ERR_OFFSET	 	    20700

//OPP_RC：辐射校正模块
#define OPP_RC_ERR_OFFSET      20800

//OPP_MTFC：MTFC模块
#define OPP_MTFC_ERR_OFFSET    20900

//OPP_OP：姿轨数据处理模块
#define OPP_OP_ERR_OFFSET      21000

//OPP_GC：几何校正模块
#define OPP_GC_ERR_OFFSET	 	    21100

//OPP_AGM：控制点自动匹配模块
#define DPSP_AGM_ERR_OFFSET  	  21200

//OPP_GM：控制点手动匹配模块
#define DPSP_GM_ERR_OFFSET	 	    21300

//OPP_PO：参数优化模块
#define OPP_PO_ERR_OFFSET       21400

//OPP_FGC：几何精校正模块
#define OPP_FGC_ERR_OFFSET       21500

//OPP_OC：正射校正模块
#define OPP_OC_ERR_OFFSET        21600

//OPP_PG：产品生产模块
#define OPP_PG_ERR_OFFSET	 	      21700

//OPP_LDA：长条带数据归档模块
#define OPP_LDA_ERR_OFFSET       21800

//OPP_SDA：0级景归档模块
#define OPP_SDA_ERR_OFFSET       21900

//OPP_PDA：产品归档模块
#define OPP_PDA_ERR_OFFSET       22000

//OPP_GCPM：控制点管理模块
#define DPSP_GCPM_ERR_OFFSET      22100

//OPP_RCMA：定标数据处理模块
#define OPP_RCMA_ERR_OFFSET       22200

//OPP_MTFA：MTF函数获取模块
#define OPP_MTFA_ERR_OFFSET       22300

//OPP_QC：质量检测模块
#define OPP_QC_ERR_OFFSET         22400

//OPP_MU：编目元信息文件更新模块
#define OPP_MU_ERR_OFFSET         22500


/////////////////////////////////////////////////////////////////////////////////////////////////
//HJ1C 软件包各模块错误码偏移

//SPP_FlowCtrl:流程控制模块
#define SPP_FLOWCTRL_ERR_OFFSET    23100

//SPP_DFPR：数据格式化处理记录模块
#define SPP_DFPR_ERR_OFFSET        23200

//SPP_SDQV：数据快视模块
#define SPP_SDQV_ERR_OFFSET        23300

//SPP_CA：编目模块
#define SPP_CA_ERR_OFFSET          23400

//SPP_BG：浏览图生成模块
#define SPP_BG_ERR_OFFSET          23500

//SPP_DX：数据提取模块
#define SPP_DX_ERR_OFFSET          23600

//SPP_AP：辅助数据处理模块
#define SPP_AP_ERR_OFFSET	 	        23700

//SPP_OP：姿轨数据处理模块
#define SPP_OP_ERR_OFFSET          23800

//SPP_DE：多普勒参数估计模块
#define SPP_DE_ERR_OFFSET          23900

//SPP_StripSAR：条带成像模块
#define SPP_StripSAR_ERR_OFFSET    24000

//SPP_ScanSAR：扫描成像模块
#define SPP_ScanSAR_ERR_OFFSET	 	  24100

//SPP_DP：降斑处理模块
#define SPP_DP_ERR_OFFSET  	      24200

//SPP_GC：几何校正模块
#define SPP_GC_ERR_OFFSET	 	       24300

//SPP_FGC：几何精校正模块
#define SPP_FGC_ERR_OFFSET        24400

//SPP_OC：正射校正模块
#define SPP_OC_ERR_OFFSET         24500

//SPP_PG：产品生产模块
#define SPP_PG_ERR_OFFSET	 	       24600

//SPP_LDA：长条带数据归档模块
#define SPP_LDA_ERR_OFFSET        24700

//SPP_SDA：0级景归档模块
#define SPP_SDA_ERR_OFFSET        24800

//SPP_PDA：产品归档模块
#define SPP_PDA_ERR_OFFSET        24900

//SPP_RDAP：原始数据分析与处理模块
#define SPP_RDAP_ERR_OFFSET       25000

//SPP_RCAP：辐射定标数据分析与处理模块
#define SPP_RCAP_ERR_OFFSET       25100

//SPP_QAP：质量评价处理器模块
#define SPP_QAP_ERR_OFFSET        25200

//SPP_QA：产品图像质量评价模块
#define SPP_QA_ERR_OFFSET         25300

//SPP_MU：编目元信息文件更新模块
#define SPP_MU_ERR_OFFSET         25400
////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////
//Warning code define
//OPP_FlowCtrl:流程控制模块
#define FLOWCTRL_WARNING_OFFSET    -20100

//OPP_RDR：原始数据回放模块
#define OPP_RDR_WARNING_OFFSET     -20200

//OPP_QCDV：快视模块
#define OPP_QCDV_WARNING_OFFSET    -20300

//OPP_CA：编目模块
#define OPP_CA_WARNING_OFFSET      -20400

//OPP_BG：浏览图生成模块
#define OPP_BG_WARNING_OFFSET      -20500

//OPP_DX：数据提取模块
#define OPP_DX_WARNING_OFFSET      -20600

//OPP_AP：辅助数据处理模块
#define OPP_AP_WARNING_OFFSET	 	    -20700

//OPP_RC：辐射校正模块
#define OPP_RC_WARNING_OFFSET      -20800

//OPP_MTFC：MTFC模块
#define OPP_MTFC_WARNING_OFFSET    -20900

//OPP_OP：姿轨数据处理模块
#define OPP_OP_WARNING_OFFSET      -21000

//OPP_GC：几何校正模块
#define OPP_GC_WARNING_OFFSET	 	    -21100

//OPP_AGM：控制点自动匹配模块
#define DPSP_AGM_WARNING_OFFSET  	  -21200

//OPP_GM：控制点手动匹配模块
#define DPSP_GM_WARNING_OFFSET	 	    -21300

//OPP_PO：参数优化模块
#define OPP_PO_WARNING_OFFSET       -21400

//OPP_FGC：几何精校正模块
#define OPP_FGC_WARNING_OFFSET       -21500

//OPP_OC：正射校正模块
#define OPP_OC_WARNING_OFFSET        -21600

//OPP_PG：产品生产模块
#define OPP_PG_WARNING_OFFSET	 	      -21700

//OPP_LDA：长条带数据归档模块
#define OPP_LDA_WARNING_OFFSET       -21800

//OPP_SDA：0级景归档模块
#define OPP_SDA_WARNING_OFFSET       -21900

//OPP_PDA：产品归档模块
#define OPP_PDA_WARNING_OFFSET       -22000

//OPP_GCPM：控制点管理模块
#define DPSP_GCPM_WARNING_OFFSET      -22100

//OPP_RCMA：定标数据处理模块
#define OPP_RCMA_WARNING_OFFSET       -22200

//OPP_MTFA：MTF函数获取模块
#define OPP_MTFA_WARNING_OFFSET       -22300

//OPP_QC：质量检测模块
#define OPP_QC_WARNING_OFFSET         -22400

//OPP_MU：编目元信息文件更新模块
#define OPP_MU_WARNING_OFFSET         -22500


/////////////////////////////////////////////////////////////////////////////////////////////////
//HJ1C 软件包各模块警告码偏移

//SPP_FlowCtrl:流程控制模块
#define SPP_FLOWCTRL_WARNING_OFFSET    -23100

//SPP_DFPR：数据格式化处理记录模块
#define SPP_DFPR_WARNING_OFFSET        -23200

//SPP_SDQV：数据快视模块
#define SPP_SDQV_WARNING_OFFSET        -23300

//SPP_CA：编目模块
#define SPP_CA_WARNING_OFFSET          -23400

//SPP_BG：浏览图生成模块
#define SPP_BG_WARNING_OFFSET          -23500

//SPP_DX：数据提取模块
#define SPP_DX_WARNING_OFFSET          -23600

//SPP_AP：辅助数据处理模块
#define SPP_AP_WARNING_OFFSET	 	        -23700

//SPP_OP：姿轨数据处理模块
#define SPP_OP_WARNING_OFFSET          -23800

//SPP_DE：多普勒参数估计模块
#define SPP_DE_WARNING_OFFSET          -23900

//SPP_StripSAR：条带成像模块
#define SPP_StripSAR_WARNING_OFFSET    -24000

//SPP_ScanSAR：扫描成像模块
#define SPP_ScanSAR_WARNING_OFFSET	 	  -24100

//SPP_DP：降斑处理模块
#define SPP_DP_WARNING_OFFSET  	       -24200

//SPP_GC：几何校正模块
#define SPP_GC_WARNING_OFFSET	 	       -24300

//SPP_FGC：几何精校正模块
#define SPP_FGC_WARNING_OFFSET        -24400

//SPP_OC：正射校正模块
#define SPP_OC_WARNING_OFFSET         -24500

//SPP_PG：产品生产模块
#define SPP_PG_WARNING_OFFSET	 	       -24600

//SPP_LDA：长条带数据归档模块
#define SPP_LDA_WARNING_OFFSET        -24700

//SPP_SDA：0级景归档模块
#define SPP_SDA_WARNING_OFFSET        -24800

//SPP_PDA：产品归档模块
#define SPP_PDA_WARNING_OFFSET        -24900

//SPP_RDAP：原始数据分析与处理模块
#define SPP_RDAP_WARNING_OFFSET       -25000

//SPP_RCAP：辐射定标数据分析与处理模块
#define SPP_RCAP_WARNING_OFFSET       -25100

//SPP_QAP：质量评价处理器模块
#define SPP_QAP_WARNING_OFFSET        -25200

//SPP_QA：产品图像质量评价模块
#define SPP_QA_WARNING_OFFSET         -25300

//SPP_MU：编目元信息文件更新模块
#define SPP_MU_WARNING_OFFSET         -25400
///////////////////////////////////////////////////////////////////////////////////////////

#endif /*DPSERRORCODE_H_*/

