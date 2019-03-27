#include "researchtreewidget.h"
#include <QPaintEvent>
#include "canvas.h"
#include "sprite.h"
#include <QPainter>

extern "C" {
#include "helpdlg_g.h"
}

#include "research.h"
#include "reqtree.h"
#include "client_main.h"
#include "government.h"
#include "tilespec.h"


namespace KV {

class HelpData {
public:
  HelpData(QRect r): rect(r) {}
  QRect rect;
  virtual void popup() const = 0;
};

class TechHelp: public HelpData {
public:
  TechHelp(Tech_type_id tid, QRect r)
    : HelpData(r)
    , m_id (tid) {}
  void popup() const override;
private:

  Tech_type_id m_id;
};

void TechHelp::popup() const {
  popup_help_dialog_typed(
        research_advance_name_translation(
          research_get(client_player()), m_id), HELP_TECH);
}

class UnitHelp: public HelpData {
public:
  UnitHelp(unit_type* u, QRect r)
    : HelpData(r)
    , m_unit(u) {}
  void popup() const override;
private:
  unit_type* m_unit;
};

void UnitHelp::popup() const {
  popup_help_dialog_typed(utype_name_translation(m_unit),
                          HELP_UNIT);
}


class ImproHelp: public HelpData {
public:
  ImproHelp(impr_type* t, QRect r)
    : HelpData(r)
    , m_impr(t) {}
  void popup() const override;
private:
  impr_type* m_impr;
};

void ImproHelp::popup() const {
  popup_help_dialog_typed(
        improvement_name_translation(m_impr),
        is_great_wonder(m_impr) ? HELP_WONDER
                                : HELP_IMPROVEMENT);
}

class GovHelp: public HelpData {
public:
  GovHelp(government* g, QRect r)
    : HelpData(r)
    , m_gov(g) {}
  void popup() const override;
private:
  government* m_gov;
};

void GovHelp::popup() const {
  popup_help_dialog_typed(government_name_translation(m_gov),
                          HELP_GOVERNMENT);
}
}

using namespace KV;

ResearchTreeWidget::ResearchTreeWidget(QWidget *parent)
  : QWidget(parent)
{
  updateTree();
  setMouseTracking(true);
}

void ResearchTreeWidget::paintEvent(QPaintEvent */*event*/) {

  QPainter p;

  p.begin(this);
  p.drawPixmap(0, 0, m_pix.width(), m_pix.height(), m_pix);
  p.end();
}


ResearchTreeWidget::~ResearchTreeWidget() {
  destroy_reqtree(m_reqTree);
  qDeleteAll(m_helpData);
}


void ResearchTreeWidget::updateTree()
{
  if (m_reqTree != nullptr) {
    destroy_reqtree(m_reqTree);
  }
  if (!client_has_player()) return;
  m_reqTree = create_reqtree(client_player(), true);
  int w;
  int h;
  get_reqtree_dimensions(m_reqTree, &w, &h);
  auto c = canvas_create(w, h);
  c->map_pixmap.fill(Qt::transparent);
  draw_reqtree(m_reqTree, c, 0, 0, 0, 0, w, h);
  m_pix = c->map_pixmap;
  canvas_free(c);
  resize(w, h);
  updateHelp();
  update();
}

void ResearchTreeWidget::mousePressEvent(QMouseEvent *event)
{
  Tech_type_id tech = get_tech_on_reqtree(m_reqTree, event->x(), event->y());

  if (event->button() == Qt::LeftButton && can_client_issue_orders()) {
    auto state = research_invention_state(research_get(client_player()), tech);
    if (state == TECH_PREREQS_KNOWN) {
      dsend_packet_player_research(&client.conn, tech);
      return;
    }
    if (state == TECH_UNKNOWN) {
      dsend_packet_player_tech_goal(&client.conn, tech);
      return;
    }
    return;
  }

  if (event->button() == Qt::RightButton) {
    for (auto help: m_helpData) {
      if (help->rect.contains(event->pos())) {
        help->popup();
        return;
      }
    }
    return;
  }
}

//  From reqtree.c - needed for helpdata

struct tree_node {
  bool is_dummy;
  Tech_type_id tech;
  int nrequire;
  struct tree_node **require;
  int nprovide;
  struct tree_node **provide;
  int order, layer;
  int node_x, node_y, node_width, node_height;
  int number;
};

struct reqtree {
  int num_nodes;
  struct tree_node **nodes;
  int num_layers;
  int *layer_size;
  struct tree_node ***layers;
  int diagram_width, diagram_height;
};

void ResearchTreeWidget::updateHelp() {

  qDeleteAll(m_helpData);
  m_helpData.clear();

  for (int i = 0; i < m_reqTree->num_layers; i++) {
    for (int j = 0; j < m_reqTree->layer_size[i]; j++) {
      tree_node *node = m_reqTree->layers[i][j];
      if (node->is_dummy) continue;

      int startx = node->node_x;
      int starty = node->node_y;
      int nwidth = node->node_width;
      int nheight = node->node_height;

      const char *text = research_advance_name_translation(
            research_get(client_player()), node->tech);

      int text_w, text_h;
      get_text_size(&text_w, &text_h, FONT_REQTREE_TEXT, text);

      auto r = QRect(startx + (nwidth - text_w) / 2, starty + 4,
                     text_w, text_h);
      m_helpData.append(new TechHelp(node->tech, r));

      int icon_startx = startx + 5;
      int swidth, sheight;

      unit_type_iterate(u) {
        if (advance_number(u->require_advance) != node->tech) continue;
        auto s = get_unittype_sprite(get_tileset(), u, direction8_invalid());
        get_sprite_dimensions(s, &swidth, &sheight);
        r = QRect(icon_startx, starty + text_h + 4
                  + (nheight - text_h - 4 - sheight) / 2,
                  swidth, sheight);
        m_helpData.append(new UnitHelp(u, r));
        icon_startx += swidth + 2;
      } unit_type_iterate_end;

      improvement_iterate(p) {
        requirement_vector_iterate(&(p->reqs), req) {
          if (req->source.kind == VUT_ADVANCE
              && advance_number(req->source.value.advance) == node->tech) {
            auto s = get_building_sprite(get_tileset(), p);
            get_sprite_dimensions(s, &swidth, &sheight);
            r = QRect(icon_startx, starty + text_h + 4
                      + (nheight - text_h - 4 - sheight) / 2,
                      swidth, sheight);
            m_helpData.append(new ImproHelp(p, r));
            icon_startx += swidth + 2;
          }
        } requirement_vector_iterate_end;
      } improvement_iterate_end;

      governments_iterate(gov) {
        requirement_vector_iterate(&(gov->reqs), req) {
          if (req->source.kind == VUT_ADVANCE
              && advance_number(req->source.value.advance) == node->tech) {
            auto s = get_government_sprite(get_tileset(), gov);
            get_sprite_dimensions(s, &swidth, &sheight);
            r = QRect(icon_startx, starty + text_h + 4
                      + (nheight - text_h - 4 - sheight) / 2,
                      swidth, sheight);
            m_helpData.append(new GovHelp(gov, r));
            icon_startx += swidth + 2;
          }
        } requirement_vector_iterate_end;
      } governments_iterate_end;
    }
  }
}

