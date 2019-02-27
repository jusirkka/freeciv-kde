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
packhand.cpp \
gotodlg.cpp \
mapctrl.cpp \
cityrep.cpp \
voteinfo_bar.cpp \
chatline.cpp \
luaconsole.cpp \
spaceshipdlg.cpp \
optiondlg.cpp \
sprite.cpp \
messagewin.cpp \
wldlg.cpp \
repodlgs.cpp \
graphics.cpp \
ratesdlg.cpp \
colors.cpp \
mapview.cpp \
plrdlg.cpp \
helpdlg.cpp \
inteldlg.cpp \
menu.cpp \
finddlg.cpp \
dialogs.cpp \
messagedlg.cpp \
diplodlg.cpp \
pages.cpp

HEADERS += \
    themesmanager.h \
    messagebox.h \
    inputbox.h \
    application.h \
    logging.h \
    mainwindow.h \
    sprite.h \
    colors.h \
    spritefactory.h

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
    mainwindow.ui
