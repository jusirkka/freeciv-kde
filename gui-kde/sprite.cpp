extern "C" {
#include "sprite_g.h"
}

#include "spritefactory.h"

const char **gfx_fileextensions() {
  return KV::SpriteFactory::FileExtensions();
}

