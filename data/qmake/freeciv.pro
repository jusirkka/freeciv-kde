TEMPLATE = lib
CONFIG += staticlib

DEFINES += HAVE_CONFIG_H \
    BINDIR=\\\"$${DESTDIR}/bin/\\\" \
    LOCALEDIR=\\\"$${DESTDIR}/share/locale\\\"

INCLUDEPATH += utility \
    client \
    common \
    common/networking \
    client/include \
    client/agents \
    client/luascript \
    gen_headers \
    common/aicore \
    common/scriptcore \
    dependencies/tolua-5.2/include \
    /usr/include/ImageMagick-7 \
    dependencies/cvercmp \
    dependencies/lua-5.3/src

include(client/client.pro)
include(common/common.pro)
include(utility/utility.pro)
include(dependencies/dependencies.pro)


