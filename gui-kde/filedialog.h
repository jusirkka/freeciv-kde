#pragma once

#include <QDialog>
#include <KPreviewWidgetBase>
#include <KFileWidget>

class KFileWidget;
class QLabel;

namespace KV {

class FileDialog: public QDialog
{
  Q_OBJECT
public:
  FileDialog(const QStringList& dirs, KFileWidget::OperationMode, QWidget* parent = nullptr);
  QString selectedFile() const;

private:
  KFileWidget* m_main;
};

class SavedGamePreview: public KPreviewWidgetBase {
  Q_OBJECT
public:
  SavedGamePreview(QWidget* parent = nullptr);
  void showPreview(const QUrl &url) override;
  void clearPreview() override;
private:
  void showScenario(const QString& f);
  void showSaved(const QString& f);
private:
   QLabel* m_label;
};

} // namespace KV

