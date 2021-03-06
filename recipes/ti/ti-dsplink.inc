DESCRIPTION = "DSPLINK Inter-Processor Communications (IPC) for TI ARM/DSP processors"
HOMEPAGE = "http://software-dl.ti.com/dsps/dsps_public_sw/DSPLink/index.html"
SECTION = "devel"
LICENSE = "GPLv2"

# TODO :: KERNEL_CC, should use for kernel cc for module build?
# TODO :: Need to understand why OBJDUMP is required for kernel module
# TODO :: Unset required since LDFLAGS gets picked up and used incorrectly - investigate
# TODO :: Do we need to pass so many variables to each make step?

require ti-paths.inc
require ti-staging.inc

PROVIDES = "ti-dsplink-module"
PROVIDES += "ti-dsplink-examples"

# This package builds a kernel module, use kernel PR as base and append a local version
PR = "${MACHINE_KERNEL_PR}"
PR_append = "i"

S = "${WORKDIR}/dsplink_linux_${PV}"

SRC_URI = "http://software-dl.ti.com/dsps/dsps_public_sw/${PV_DL_PATH}/dsplink_linux_${PV}.tar.gz;name=dsplinktarball \
           file://ti-dsplink-examples-run.sh \
           file://ti-dsplink-examples-loadmodules.sh \
           file://ti-dsplink-examples-unloadmodules.sh "

DEPENDS = "ti-dspbios ti-xdctools ti-cgt6x" 
DEPENDS += "virtual/kernel perl-native" 

# SOC_FAMILY configuration

DSPLINKPLATFORM_dm6446    = "DAVINCI"
DSPLINKPLATFORM_dm6467    = "DAVINCIHD"
DSPLINKPLATFORM_omapl137  = "OMAPL1XX"
DSPLINKPLATFORM_omapl138  = "OMAPL138"
DSPLINKPLATFORM_omap3     = "OMAP3530"
DSPLINKPLATFORM          ?= "<UNDEFINED_DSPLINKPLATFORM>"

DSPLINKDSPCFG_dm6446      = "DM6446GEMSHMEM"
DSPLINKDSPCFG_dm6467      = "DM6467GEMSHMEM"
DSPLINKDSPCFG_omapl137    = "OMAPL1XXGEMSHMEM"
DSPLINKDSPCFG_omapl138    = "OMAPL138GEMSHMEM"
DSPLINKDSPCFG_omap3       = "OMAP3530SHMEM"
DSPLINKDSPCFG            ?= "<UNDEFINED_DSPLINKDSPCFG>"

DSPLINKDSP_dm6446         = "DM6446GEM_0"
DSPLINKDSP_dm6467         = "DM6467GEM_0"
DSPLINKDSP_omapl137       = "OMAPL1XXGEM_0"
DSPLINKDSP_omapl138       = "OMAPL138GEM_0"
DSPLINKDSP_omap3          = "OMAP3530_0"
DSPLINKDSP               ?= "<UNDEFINED_DSPLINKDSP>"

DSPLINKGPPOS_dm6446       = "DM6446LSP"
DSPLINKGPPOS_dm6467       = "DM6467LSP"
DSPLINKGPPOS_omapl137     = "ARM"
DSPLINKGPPOS_omapl138     = "ARM"
DSPLINKGPPOS_omap3        = "OMAPLSP"
DSPLINKGPPOS             ?= "<DEFINED_DSPLINKGPPOS>" 

# Exported Variable needed by build
DSPLINK = "${S}/dsplink"
export DSPLINK

do_configure() {

    # Run perl script to create appropriate makefiles (v1.60 and up)
    (
    cd ${DSPLINK}
    perl config/bin/dsplinkcfg.pl --platform=${DSPLINKPLATFORM} --nodsp=1 \
	--dspcfg_0=${DSPLINKDSPCFG} --dspos_0=DSPBIOS5XX \
	--gppos=${DSPLINKGPPOS} --comps=ponslrmc
    )
}

do_prepsources () {
	
    # Prepare the tree for rebuiling - clean and generate interfaces
    XDCPATH=${DSPLINK} ${XDC_INSTALL_DIR}/xdc .make -PR .
    XDCPATH=${DSPLINK} ${XDC_INSTALL_DIR}/xdc clean -PR .
    XDCPATH=${DSPLINK} ${XDC_INSTALL_DIR}/xdc .interfaces -PR .
}

addtask prepsources after do_configure before do_compile

do_compile() {
    # TODO :: KERNEL_CC, should use for kernel module build?
    # TODO :: Need to understand why OBJDUMP is required for kernel module
    # Unset these since LDFLAGS gets picked up and used incorrectly.... need 
    # investigation

    unset CFLAGS CPPFLAGS CXXFLAGS LDFLAGS

    # Build the gpp user space library
    cd ${DSPLINK}/gpp/src/api && make \
      CROSS_COMPILE="${TARGET_PREFIX}" \
      CC="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      AR="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      LD="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ld" \
      COMPILER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      ARCHIVER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      KERNEL_DIR="${STAGING_KERNEL_DIR}" \
      all

    # Build the gpp kernel space (debug and release)
    cd ${DSPLINK}/gpp/src && make \
      OBJDUMP="${TARGET_PREFIX}objdump" \
      CROSS_COMPILE="${TARGET_PREFIX}" \
      CC="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      AR="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      LD="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ld" \
      COMPILER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      ARCHIVER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      KERNEL_DIR="${STAGING_KERNEL_DIR}" \
      BASE_BUILDOS="${STAGING_KERNEL_DIR}" \
      all

    # Build the gpp samples
    cd ${DSPLINK}/gpp/src/samples && make \
      BASE_TOOLCHAIN="${TOOLCHAIN_PATH}" \
      BASE_CGTOOLS="${BASE_TOOLCHAIN}/bin" \
      OSINC_PLATFORM="${TOOLCHAIN_PATH}/lib/gcc/${TARGET_SYS}/$(${TARGET_PREFIX}gcc -dumpversion)/include" \
      OSINC_TARGET="${BASE_TOOLCHAIN}/target/usr/include" \
      CROSS_COMPILE="${TARGET_PREFIX}" \
      CC="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      LD="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      AR="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      COMPILER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      LINKER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}gcc" \
      ARCHIVER="${TOOLCHAIN_PATH}/bin/${TARGET_PREFIX}ar" \
      KERNEL_DIR="${STAGING_KERNEL_DIR}" \
      all

    # Build the dsp library (debug and release)
     cd ${DSPLINK}/dsp/src && make \
      BASE_CGTOOLS="${CODEGEN_INSTALL_DIR}" \
      BASE_SABIOS="${BIOS_INSTALL_DIR}" \
      all

    # Build the dsp samples (debug and release)
    cd ${DSPLINK}/dsp/src/samples && make \
      BASE_CGTOOLS="${CODEGEN_INSTALL_DIR}" \
      BASE_SABIOS="${BIOS_INSTALL_DIR}" \
      all
}

KERNEL_VERSION = "${@base_read_file('${STAGING_KERNEL_DIR}/kernel-abiversion')}"

do_install () {
	
    # Install the kernel module
    install -d ${D}/lib/modules/${KERNEL_VERSION}/kernel/drivers/dsp
    install -m 0755 ${DSPLINK}/gpp/export/BIN/Linux/${DSPLINKPLATFORM}/RELEASE/dsplinkk.ko ${D}/lib/modules/${KERNEL_VERSION}/kernel/drivers/dsp/ 

    # Install the example apps (gpp and dsp)
    install -d ${D}/${installdir}/ti-dsplink-examples/
    cp ${DSPLINK}/gpp/export/BIN/Linux/${DSPLINKPLATFORM}/RELEASE/*gpp ${D}/${installdir}/ti-dsplink-examples
    cp ${DSPLINK}/dsp/export/BIN/DspBios/${DSPLINKPLATFORM}/${DSPLINKDSP}/RELEASE/*.out ${D}/${installdir}/ti-dsplink-examples    

    # Install the example apps module un/load scripts
    install ${WORKDIR}/ti-dsplink-examples-loadmodules.sh ${D}/${installdir}/ti-dsplink-examples
    install ${WORKDIR}/ti-dsplink-examples-unloadmodules.sh ${D}/${installdir}/ti-dsplink-examples
    install ${WORKDIR}/ti-dsplink-examples-run.sh ${D}/${installdir}/ti-dsplink-examples   
 
    # Install/Stage the Source Tree
    install -d ${D}${LINK_INSTALL_DIR_RECIPE}
    cp -pPrf ${S}/* ${D}${LINK_INSTALL_DIR_RECIPE}
    chmod -R +w ${D}${LINK_INSTALL_DIR_RECIPE}

    # Changes path of include txt file to use LINK_INSTALL_DIR variable for GPP
    for i in $(find ${D}${LINK_INSTALL_DIR_RECIPE}/dsplink/gpp/export/BIN/Linux/${DSPLINKPLATFORM}/*/ -name "*.txt"); do
       sed -i ${i} -e s=${S}=\$\{LINK_INSTALL_DIR\}=g
    done

    # Changes path of include txt file to use LINK_INSTALL_DIR variable for DSP 
    for i in $(find ${D}${LINK_INSTALL_DIR_RECIPE}/dsplink/dsp/export/BIN/DspBios/${DSPLINKPLATFORM}/${DSPLINKDSP}/*/ -name "*.txt"); do
    sed -i ${i} -e s=${S}=\$\{LINK_INSTALL_DIR\}=g
    done

}

PACKAGES += "ti-dsplink-module"
FILES_ti-dsplink-module = "/lib/modules/${KERNEL_VERSION}/kernel/drivers/dsp/dsplinkk.ko" 
RDEPENDS_ti-dsplink-module += "update-modules"

pkg_postinst_ti-dsplink-module () {
#!/bin/sh
if [ -n "$D" ]; then
	exit 1
fi

depmod -a
update-modules || true
}

pkg_postrm_ti-dsplink-module () {
#!/bin/sh
update-modules || true
}

PACKAGES += "ti-dsplink-examples"
RDEPENDS_ti-dsplink-examples_append = " ti-dsplink-module"
RDEPENDS_ti-dsplink-examples_append_omap3 += " ti-lpm-module ti-lpm-utils"
FILES_ti-dsplink-examples = "${installdir}/ti-dsplink-examples/*"
INSANE_SKIP_ti-dsplink-examples = True
