#include "spritefactory.h"
#include "sprite.h"
#include "colors.h"
#include "tilespec.h"
#include <QImageReader>
#include <QPixmapCache>
#include <QPainter>
#include <cmath>
#include "logging.h"

using namespace KV;

const char** SpriteFactory::FileExtensions() {
  return instance()->m_formats;
}

struct sprite* SpriteFactory::Load(const char* fileName) {
  sprite *s = new sprite;

  if (QPixmapCache::find(QString(fileName), &s->pm)) {
    return s;
  }
  s->pm.load(QString(fileName));
  QPixmapCache::insert(QString(fileName), s->pm);

  return s;
}

struct sprite* SpriteFactory::Crop(struct sprite *source,
                                   int x, int y, int width, int height,
                                   struct sprite *mask,
                                   int mask_offset_x, int mask_offset_y,
                                   float scale, bool smooth) {

  fc_assert_ret_val(source, NULL);

  if (!width || !height) {
    return NULL;
  }

  int hex = 0;
  if (scale != 1.0f && (tileset_hex_height(tileset) > 0
      || tileset_hex_width(tileset) > 0)) {
    hex = 1;
  }
  int widthzoom = ceil(width * scale) + hex;
  int heightzoom = ceil(height * scale) + hex;
  auto cropped = new sprite;
  cropped->pm = QPixmap(widthzoom, heightzoom);
  cropped->pm.fill(Qt::transparent);
  auto source_rect = QRectF(x, y, width, height);
  auto dest_rect = QRectF(0, 0, widthzoom, heightzoom);

  QPainter p;
  p.begin(&cropped->pm);
  if (smooth) {
    p.setRenderHint(QPainter::SmoothPixmapTransform);
  }
  p.setRenderHint(QPainter::Antialiasing);
  p.drawPixmap(dest_rect, source->pm, source_rect);
  p.end();

  if (mask) {
    int mw = mask->pm.width();
    int mh = mask->pm.height();

    source_rect = QRectF(0, 0, mw, mh);
    dest_rect = QRectF(mask_offset_x - x, mask_offset_y - y, mw, mh);
    p.begin(&cropped->pm);
    p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);
    p.drawPixmap(dest_rect, mask->pm, source_rect);
    p.end();
  }

  return cropped;

}

void SpriteFactory::Dimensions(struct sprite *sprite, int *width, int *height) {
  *width = sprite->pm.width();
  *height = sprite->pm.height();
}

void SpriteFactory::Free(struct sprite *s) {
  delete s;
}

struct sprite *SpriteFactory::Create(int width, int height, struct color *pcolor) {
  struct sprite *created = new sprite;

  created->pm = QPixmap(width, height);

  created->pm.fill(pcolor->qcolor);

  return created;
}


SpriteFactory* SpriteFactory::instance() {
    static SpriteFactory* f = new SpriteFactory();
    return f;
}

SpriteFactory::SpriteFactory() {

  auto formats = QImageReader::supportedImageFormats();

  m_formats = new const char* [formats.size()];
  m_numFormats = formats.size();

  for (int j = 0; j < formats.size(); j++) {
    auto fmt = formats[j];
    auto bytes = new char[fmt.size() + 1];
    strncpy(bytes, fmt.data(), fmt.size() + 1);
    m_formats[j] = bytes;
  }
}

SpriteFactory::~SpriteFactory() {
  for (int i = 0; i < m_numFormats; i++) {
    delete m_formats[i];
  }
  delete m_formats;
}

