#pragma once

#include <QObject>

namespace KV {

class SettingsManager: public QObject
{
  Q_OBJECT
public:

  static void Register(QObject* client);
  static void Read();
  static void Write();

signals:

  void signalRead();
  void signalWrite();

private:

  static SettingsManager* instance();
  SettingsManager(QObject *parent = nullptr);
  SettingsManager(const SettingsManager&);
  SettingsManager& operator=(const SettingsManager&);

  void addClient(QObject* client);

};

} // namespace KV

