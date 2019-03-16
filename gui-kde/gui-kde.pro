TEMPLATE = app
TARGET = freeciv-kde
QT += core gui widgets

QMAKE_CXXFLAGS_DEBUG = -g -Wall -pipe -Wno-unused-parameter

SOURCES += \
    themesmanager.cpp \
    messagebox.cpp \
    inputbox.cpp \
    main.cpp \
    application.cpp \
    mainwindow.cpp \
    spritefactory.cpp \
    state.cpp \
    startdialog.cpp \
    networkdialog.cpp \
    chatwindow.cpp \
    chatlineedit.cpp \
    chatline_g.cpp \
    cityrep_g.cpp \
    colors_g.cpp \
    dialogs_g.cpp \
    diplodlg_g.cpp \
    finddlg_g.cpp \
    gotodlg_g.cpp \
    graphics_g.cpp \
    helpdlg_g.cpp \
    inteldlg_g.cpp \
    luaconsole_g.cpp \
    mapctrl_g.cpp \
    mapview_g.cpp \
    menu_g.cpp \
    messagedlg_g.cpp \
    messagewin_g.cpp \
    optiondlg_g.cpp \
    packhand_g.cpp \
    pages_g.cpp \
    plrdlg_g.cpp \
    ratesdlg_g.cpp \
    repodlgs_g.cpp \
    spaceshipdlg_g.cpp \
    sprite_g.cpp \
    voteinfo_bar_g.cpp \
    wldlg_g.cpp \
    nationdialog.cpp \
    mapitem.cpp \
    mapview.cpp \
    unitinfo.cpp \
    gameinfo.cpp \
    endturnrect.cpp \
    spritewidget.cpp \
    govmenu.cpp \
    minimapview.cpp \
    tileinfo.cpp \
    buildables.cpp \
    unitselector.cpp \
    outputpanemanager.cpp \
    messagepane.cpp \
    textbrowser.cpp \
    chatpane.cpp \
    reportpane.cpp \
    playerdialog.cpp \
    treatydialog.cpp


HEADERS += \
    themesmanager.h \
    messagebox.h \
    inputbox.h \
    application.h \
    logging.h \
    mainwindow.h \
    sprite.h \
    colors.h \
    spritefactory.h \
    state.h \
    startdialog.h \
    networkdialog.h \
    chatwindow.h \
    chatlineedit.h \
    nationdialog.h \
    mapitem.h \
    mapview.h \
    unitinfo.h \
    gameinfo.h \
    endturnrect.h \
    spritewidget.h \
    govmenu.h \
    minimapview.h \
    tileinfo.h \
    buildables.h \
    unitselector.h \
    ioutputpane.h \
    outputpanemanager.h \
    messagepane.h \
    textbrowser.h \
    chatpane.h \
    reportpane.h \
    playerdialog.h \
    treatydialog.h

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

FORMS += \
    inputbox.ui \
    mainwindow.ui \
    startdialog.ui \
    networkdialog.ui \
    nationdialog.ui \
    playerdialog.ui \
    treatydialog.ui
