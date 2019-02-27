#!/bin/bash

find ../freeciv/ -name "*_g.h"| \
egrep -v 'sprite_g|editgui_g|themes_g|canvas_g|citydlg_g|connectdlg_g|gui_main'| \
while read f; do
  t=${f:0: -4}.cpp
  ./graphics_stubs.py -f $f > $(basename $t);
done


