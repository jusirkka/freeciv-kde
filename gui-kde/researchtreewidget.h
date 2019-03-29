#pragma once

#include <QWidget>
#include <QVector>

struct reqtree;

namespace KV {

class HelpData;

class ResearchTreeWidget : public QWidget
{
  Q_OBJECT
public:
  explicit ResearchTreeWidget(QWidget *parent = nullptr);
  ~ResearchTreeWidget() override;

signals:

public slots:

  void updateTree();

private:

  void mousePressEvent(QMouseEvent *event) override;
  void paintEvent(QPaintEvent *event) override;

  void updateHelp();

private:

  reqtree* m_reqTree = nullptr;
  QPixmap m_pix;
  QVector<HelpData*> m_helpData;

};

} // namespace KV

