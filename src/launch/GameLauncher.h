#pragma once

#include <QObject>
#include <QStringList>

struct LaunchCommand {
  QString program;
  QStringList arguments;
  [[nodiscard]] bool isValid() const { return !program.isEmpty(); }
};

class GameLauncher final : public QObject {
  Q_OBJECT
  Q_PROPERTY(QString lastError READ lastError NOTIFY lastErrorChanged)

public:
  explicit GameLauncher(QObject* parent = nullptr);

  [[nodiscard]] QString lastError() const;
  [[nodiscard]] static LaunchCommand lutrisCommand(const QString& id, bool flatpak);
  Q_INVOKABLE bool launch(const QString& source, const QString& id, bool flatpak = false);
  Q_INVOKABLE bool manage(const QString& source, const QString& id, bool flatpak = false);

signals:
  void lastErrorChanged();

private:
  bool launchLutris(const QString& id, bool flatpak, bool manageOnly);
  void setError(const QString& error);
  QString m_lastError;
};
