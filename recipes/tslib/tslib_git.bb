SRC_URI = "git://github.com/kergoth/tslib.git;protocol=git \
           file://0001-relax-EV_VERSION-check-fail-only-if-version-of-runni.patch"
S = "${WORKDIR}/git"
SRCREV = "860d69cadedef0dec8ba6259ab5850691d3402e7"
PV = "1.0+gitr${SRCPV}"
PR = "${INC_PR}.5"

DEFAULT_PREFERENCE = "-1"

# force old revision, because include-cy8mrln-header.patch does not apply to current
SRCREV_palmpre = "5243db505b109df3001f"

# move this to tslib.inc when upstream releases a new version which includes support for
# the palmpre machine
SRC_URI_append_palmpre = " \
  file://include-cy8mrln-header.patch \
"
EXTRA_OECONF_palmpre = " \
   --enable-shared \
   --enable-cy8mrln-palmpre \
   --enable-input \
   --disable-h3600 \
   --disable-corgi \
   --disable-collie \
   --disable-mk712 \
   --disable-artic2 \
   --disable-ucb1x00 \
   --disable-tatung \
"

include tslib.inc
