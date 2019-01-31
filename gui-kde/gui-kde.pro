TEMPLATE = app
TARGET = freeciv-kde
QT += core gui widgets

SOURCES += gui_main.cpp \
    dialogs.cpp \
    fc_client.cpp \
    mapview.cpp \
    fonts.cpp \
    citydlg.cpp \
    hudwidget.cpp \
    canvas.cpp \
    helpdlg.cpp \
    plrdlg.cpp \
    spaceshipdlg.cpp \
    repodlgs.cpp \
    cityrep.cpp \
    chatline.cpp \
    menu.cpp \
    pages.cpp \
    shortcuts.cpp \
    voteinfo_bar.cpp \
    optiondlg.cpp \
    sidebar.cpp \
    messagewin.cpp \
    messagedlg.cpp \
    gotodlg.cpp \
    mapctrl.cpp \
    ratesdlg.cpp \
    sprite.cpp \
    inteldlg.cpp \
    connectdlg.cpp \
    colors.cpp \
    qtg_cxxside.cpp \
    luaconsole.cpp \
    themes.cpp \
    graphics.cpp \
    diplodlg.cpp

HEADERS += gui_main.h \
    dialogs.h \
    fc_client.h \
    mapview.h \
    fonts.h \
    citydlg.h \
    hudwidget.h \
    canvas.h \
    helpdlg.h \
    plrdlg.h \
    spaceshipdlg.h \
    repodlgs.h \
    cityrep.h \
    chatline.h \
    menu.h \
    pages.h \
    shortcuts.h \
    voteinfo_bar.h \
    optiondlg.h \
    sidebar.h \
    messagewin.h \
    messagedlg.h \
    gotodlg.h \
    mapctrl.h \
    ratesdlg.h \
    sprite.h \
    inteldlg.h \
    connectdlg.h \
    colors.h \
    qtg_cxxside.h \
    luaconsole.h \
    themes.h \
    graphics.h \
    diplodlg.h \
    themes.h

INCLUDEPATH += ../freeciv/common \
    ../freeciv/common/networking \
    ../freeciv/common/aicore \
    ../freeciv/common/scriptcore \
    ../freeciv/utility \
    ../freeciv/gen_headers \
    ../freeciv/client \
    ../freeciv/client/include \

DEFINES += HAVE_CONFIG_H \
    BINDIR=\\\"$${DESTDIR}/bin/\\\" \
    LOCALEDIR=\\\"$${DESTDIR}/share/locale\\\"

LIBS += -lz -licuuc -lbz2 -llzma -lSDL2_mixer -lSDL2 \
    -lMagickWand-7.Q16 -lcurl \
    -L. -lfreeciv
