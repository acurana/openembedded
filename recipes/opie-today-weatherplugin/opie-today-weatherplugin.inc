DESCRIPTION = "Weather plugin for Today"
SECTION = "opie/today"
PRIORITY = "optional"
LICENSE = "GPL"
DEPENDS = "libopiecore2 libopiepim2"
RDEPENDS_${PN} = "opie-today"
APPNAME = "todayweatherplugin"


S = "${WORKDIR}/weather"

inherit opie

# FILES plugins/today/libtodayweatherplugin.so* pics/todayweatherplugin
do_install() {
        install -d ${D}${palmtopdir}/pics/${APPNAME}/
        install -m 0644 ${WORKDIR}/pics/${APPNAME}/*.png ${D}${palmtopdir}/pics/${APPNAME}/
}

