#include "logging.h"
#include "colors.h"
#include "rgbcolor.h"

int color_brightness_score(color *pcolor) {
  // QColor has color space conversions, but nothing giving a
  // perceptually even color space
  rgbcolor *prgb = rgbcolor_new(pcolor->qcolor.red(),
                                pcolor->qcolor.green(),
                                pcolor->qcolor.blue());
  int score = rgbcolor_brightness_score(prgb);
  rgbcolor_destroy(prgb);
  return score;
}

