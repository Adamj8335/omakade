#pragma once

#include <QLocalServer>
#include <QObject>

// Owns the per-user local socket that keeps one Omakade window open. A second launch
// forwards a short command instead of opening another window: "activate" raises the
// window, "play <key>" launches a library game, and "quit" closes Omakade.
class SingleInstance final : public QObject {
  Q_OBJECT

public:
  explicit SingleInstance(const QString& serverName = {}, QObject* parent = nullptr);
  [[nodiscard]] static QString defaultServerName();
  [[nodiscard]] bool claimOrNotify(const QByteArray& command = "activate");
  // Delivers a command to a running instance. Returns false when none is listening.
  [[nodiscard]] static bool sendCommand(const QString& serverName, const QByteArray& command);

signals:
  void activationRequested();
  void playRequested(const QString& launchKey);
  void quitRequested();

private:
  QString m_serverName;
  QLocalServer m_server;
};
