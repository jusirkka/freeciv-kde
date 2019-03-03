TEMPLATE = app
TARGET = freeciv-kde
QT += core gui widgets

SOURCES += \
    themesmanager.cpp \
    messagebox.cpp \
    inputbox.cpp \
    main.cpp \
    application.cpp \
    mainwindow.cpp \
    spritefactory.cpp \
    state.cpp \
    mapwidget.cpp \
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
    wldlg_g.cpp


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
    mapwidget.h \
    startdialog.h \
    networkdialog.h \
    chatwindow.h \
    chatlineedit.h

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
    networkdialog.ui
