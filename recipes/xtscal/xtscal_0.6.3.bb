LICENSE = "GPL"
DESCRIPTION = "Touchscreen calibration utility"
SECTION = "x11/base"

DEPENDS = "virtual/libx11 libxft libxcalibrate"

PR = "r8"

SRC_URI = "${GPE_MIRROR}/xtscal-${PV}.tar.bz2 \
           file://change-cross.patch;patch=1 \
	   file://cleanup.patch;patch=1 \
	   file://xtscal-0.6.3-verbose-XInitExt-error.patch;patch=1 \
	"

inherit autotools
