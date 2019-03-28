#include "settingsmanager.h"

using namespace KV;

void SettingsManager::Register(QObject* client) {
  instance()->addClient(client);
}

void SettingsManager::Read() {
  instance()->signalRead();
}

void SettingsManager::Write() {
  instance()->signalWrite();
}

SettingsManager* SettingsManager::instance() {
  static SettingsManager* self = new SettingsManager;
  return self;
}

SettingsManager::SettingsManager(QObject *parent)
  : QObject(parent)
{}

void SettingsManager::addClient(QObject* client) {
  connect(this, SIGNAL(signalRead()), client, SLOT(readSettings()));
  connect(this, SIGNAL(signalWrite()), client, SLOT(writeSettings()));
}

