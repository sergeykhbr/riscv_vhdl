export TOP_ROOT=../../..
export RTL_HOME=${TOP_ROOT}/rtl
export PRJ_ROOT=${TOP_ROOT}/prj/impl/kc705_sim
export IMPL_ROOT=${TOP_ROOT}/prj/impl
export COMMON_HOME=${TOP_ROOT}/prj/common
export LISTS_HOME=${COMMON_HOME}/lists
export VIPS_HOME=${COMMON_HOME}/vips
export MEM_MACRO_HOME=/data/ip_v2/mem



export CDS_INST_DIR = /data/special/eda/cadence/2017-18/RHELx86/INCISIVE_15.20.038
export CDS_ARCH = lnx86
export PATH := ${CDS_INST_DIR}/tools.${CDS_ARCH}/bin:${PATH}
export SPECMAN_PATH = ${CDN_VIP_ROOT}/packages:${CDN_VIP_LIB_PATH}/64bit
export LD_LIBRARY_PATH := ${CDN_VIP_LIB_PATH}/64bit:${DENALI}/verilog:${CDS_INST_DIR}/tools.${CDS_ARCH}/specman/lib/64bit:${CDS_INST_DIR}/tools.${CDS_ARCH}/lib/64bit:${LD_LIBRARY_PATH}
export CDN_VIP_ROOT=/data/special/eda/cadence/2017-18/RHELx86/VIPCAT_11.30.061
export CDN_VIP_LIB_PATH=./vip_lib
export ABVIP_INST_DIR=${CDN_VIP_ROOT}/tools/abvip
export DENALI = ${CDN_VIP_ROOT}/tools.${CDS_ARCH}/denali_64bit

# Location of UVM source:
export CDN_VIP_UVMHOME=${CDS_INST_DIR}/tools/methodology/UVM/CDNS-1.1d/sv
export CDN_SV_UVMHOME_ADD=${CDS_INST_DIR}/tools/methodology/UVM/CDNS-1.1d/additions/sv
# Location of Additional PLI for IES UVM:
export CDN_SV_UVMHOME=${CDS_INST_DIR}/tools/methodology/UVM/CDNS-1.1d/sv
