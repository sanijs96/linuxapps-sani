SUMMARY = "${PN} file recipe"
LICENSE = "CLOSED"

SRC_URI = "file://source \
           "

# list of runtime dependencies
RDEPENDS:${PN}:append = " bash "

# list of buildtime dependencies
DEPENDS:append = " "

S = "${WORKDIR}/source"

TARGET_CC_ARCH += "${LDFLAGS}"

do_compile () {
    make clean
    make all
}
addtask compile

do_install () {
    install -d ${D}${bindir}
    install -m 0755 ${B}/linuxapps-sani ${D}${bindir}/
}
addtask installl after do_compile
