SOURCES += $${PWD}/game.c \
    $${PWD}/ai.c \
    $${PWD}/fc_cmdhelp.c \
    $${PWD}/player.c \
    $${PWD}/capstr.c \
    $${PWD}/fc_interface.c \
    $${PWD}/mapimg.c \
    $${PWD}/packets_gen.c \
    $${PWD}/style.c \
    $${PWD}/nation.c \
    $${PWD}/featured_text.c \
    $${PWD}/tile.c \
    $${PWD}/city.c \
    $${PWD}/extras.c \
    $${PWD}/actions.c \
    $${PWD}/unittype.c \
    $${PWD}/movement.c \
    $${PWD}/improvement.c \
    $${PWD}/map.c \
    $${PWD}/terrain.c \
    $${PWD}/unit.c \
    $${PWD}/research.c \
    $${PWD}/tech.c \
    $${PWD}/traderoutes.c \
    $${PWD}/multipliers.c \
    $${PWD}/unitlist.c \
    $${PWD}/road.c \
    $${PWD}/government.c \
    $${PWD}/effects.c \
    $${PWD}/diptreaty.c \
    $${PWD}/events.c \
    $${PWD}/team.c \
    $${PWD}/specialist.c \
    $${PWD}/reqtext.c \
    $${PWD}/requirements.c \
    $${PWD}/server_settings.c \
    $${PWD}/base.c \
    $${PWD}/idex.c \
    $${PWD}/citizens.c \
    $${PWD}/spaceship.c \
    $${PWD}/worklist.c \
    $${PWD}/rgbcolor.c \
    $${PWD}/achievements.c \
    $${PWD}/disaster.c \
    $${PWD}/combat.c \
    $${PWD}/calendar.c \
    $${PWD}/clientutils.c \
    $${PWD}/victory.c \
    $${PWD}/metaknowledge.c \
    $${PWD}/culture.c \
    $${PWD}/version.c

include(networking/networking.pro)
include(aicore/aicore.pro)
include(scriptcore/scriptcore.pro)

#    $${PWD}/.c \
