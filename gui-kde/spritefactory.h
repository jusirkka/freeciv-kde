#ifndef SPRITEFACTORY_H
#define SPRITEFACTORY_H


#include "sprite.h"

struct color;
struct sprite;

namespace KV {


class SpriteFactory
{


public:

  static const char** FileExtensions();

  /**
   * Load the given graphics file into a sprite.  This function loads an
   * entire image file, which may later be broken up into individual sprites
   * with crop_sprite.
   */
  static struct sprite* Load(const char* fileName);

  /**
   * Create a new sprite by cropping and taking only the given portion of
   * the image.
   *
   * source gives the sprite that is to be cropped.
   *
   * x,y, width, height gives the rectangle to be cropped.  The pixel at
   * position of the source sprite will be at (0,0) in the new sprite, and
   * the new sprite will have dimensions (width, height).
   *
   * mask gives an additional mask to be used for clipping the new
   * sprite. Only the transparency value of the mask is used in
   * crop_sprite. The formula is: dest_trans = src_trans *
   * mask_trans. Note that because the transparency is expressed as an
   * integer it is common to divide it by 256 afterwards.
   *
   * mask_offset_x, mask_offset_y is the offset of the mask relative to the
   * origin of the source image.  The pixel at (mask_offset_x,mask_offset_y)
   * in the mask image will be used to clip pixel (0,0) in the source image
   * which is pixel (-x,-y) in the new image.
   */
  static struct sprite* Crop(struct sprite *source,
                             int x, int y, int width, int height,
                             struct sprite *mask,
                             int mask_offset_x, int mask_offset_y,
                             float scale, bool smooth);

  static void Dimensions(struct sprite *sprite, int *width, int *height);

  static void Free(struct sprite *s);

  static struct sprite *Create(int width, int height, struct color *pcolor);

  ~SpriteFactory();

private:

  static SpriteFactory* instance();
  SpriteFactory();
  SpriteFactory(const SpriteFactory&);
  SpriteFactory& operator=(const SpriteFactory&);



private:

  const char** m_formats;
  int m_numFormats;

};


}


#endif // SPRITEFACTORY_H
