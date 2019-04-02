project(freeciv-kde)

cmake_minimum_required (VERSION 3.13 FATAL_ERROR)
set (KF5_MIN_VERSION "5.56.0")

find_package(ECM ${KF5_MIN_VERSION} REQUIRED CONFIG)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${ECM_MODULE_PATH} ${ECM_KDE_MODULE_DIR})

find_package(Qt5 ${QT_MIN_VERSION} REQUIRED NO_MODULE COMPONENTS Widgets)
find_package(KF5 ${KF5_MIN_VERSION} REQUIRED COMPONENTS
  Config
  ConfigWidgets
  I18n
  TextWidgets
  XmlGui
)

find_library(LPTHREAD pthread)
find_library(LZ z)
find_library(LICUUC icuuc)
find_library(LBZ2 bz2)
find_library(LLZMA lzma)
find_library(LSDL2Mixer SDL2_mixer)
find_library(LSDL2 SDL2)
find_library(LMagick MagickWand-7.Q16)
find_library(LCURL curl)

include(FeatureSummary)
include(ECMAddAppIcon)
include(ECMInstallIcons)
include(KDEInstallDirs)
include(KDECMakeSettings)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wno-tautological-constant-out-of-range-compare")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-tautological-constant-out-of-range-compare")

########### sources ###############

set(fc_kde_srcs
    themesmanager.cpp
    messagebox.cpp
    inputbox.cpp
    main.cpp
    application.cpp
    mainwindow.cpp
    spritefactory.cpp
    state.cpp
    startdialog.cpp
    networkdialog.cpp
    chatwindow.cpp
    chatlineedit.cpp
    nationdialog.cpp
    mapview.cpp
    unitinfo.cpp
    gameinfo.cpp
    endturnrect.cpp
    spritewidget.cpp
    govmenu.cpp
    minimapview.cpp
    tileinfo.cpp
    buildables.cpp
    unitselector.cpp
    outputpanemanager.cpp
    messagepane.cpp
    textbrowser.cpp
    chatpane.cpp
    reportpane.cpp
    treatydialog.cpp
    citydialog.cpp
    productionheader.cpp
    citymap.cpp
    unitlistwidget.cpp
    cityview.cpp
    cityinfowidget.cpp
    productiondialog.cpp
    governordialog.cpp
    workmodel.cpp
    playerdialog.cpp
    playerwidget.cpp
    sciencedialog.cpp
    researchtreewidget.cpp
    actionselector.cpp
    messageconfigdialog.cpp
    proxytablemodel.cpp
    ioutputpane.h
    unitactionchecker.cpp
)

ki18n_wrap_ui(fc_kde_srcs
    inputbox.ui
    startdialog.ui
    networkdialog.ui
    nationdialog.ui
    treatydialog.ui
    citydialog.ui
    productionheader.ui
    cityview.ui
    productiondialog.ui
    governordialog.ui
    playerwidget.ui
    sciencedialog.ui
    messageconfigdialog.ui
)

kconfig_add_kcfg_files(fc_kde_srcs
  conf_mainwindow.kcfgc
  conf_outputpanemanager.kcfgc
  conf_playerwidget.kcfgc
  conf_cityview.kcfgc
)

qt5_add_resources(fc_kde_srcs freeciv-kde.qrc)

add_executable(freeciv-kde ${fc_kde_srcs})

include_directories(freeciv-kde
  ../freeciv/common
  ../freeciv/common/networking
  ../freeciv/common/aicore
  ../freeciv/common/scriptcore
  ../freeciv/utility
  ../freeciv/gen_headers
  ../freeciv/client
  ../freeciv/client/agents
  ../freeciv/client/include
)

target_compile_definitions(freeciv-kde PUBLIC
  HAVE_CONFIG_H
)


add_subdirectory(client)



target_link_libraries(freeciv-kde
  KF5::TextWidgets
  KF5::XmlGui
  $ENV{HOME}/lib/libfreeciv.a
  pthread
  z
  icuuc
  bz2
  lzma
  SDL2
  SDL2_mixer
  MagickWand-7.Q16
  curl
)

feature_summary(WHAT ALL INCLUDE_QUIET_PACKAGES FATAL_ON_MISSING_REQUIRED_PACKAGES)

########### install files ###############
install(TARGETS freeciv-kde  ${KDE_INSTALL_TARGETS_DEFAULT_ARGS})

