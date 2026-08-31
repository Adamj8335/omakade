#pragma once

#include "sources/steam/SteamScanner.h"

#include <QAbstractListModel>
#include <QColor>
#include <QFileSystemWatcher>
#include <QFutureWatcher>
#include <QSqlDatabase>
#include <QTimer>

class SteamGameModel final : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(bool scanning READ scanning NOTIFY scanningChanged)
  Q_PROPERTY(bool steamDetected READ steamDetected NOTIFY statusChanged)
  Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
  Q_PROPERTY(QString errorText READ errorText NOTIFY statusChanged)
  Q_PROPERTY(int artworkCount READ artworkCount NOTIFY statusChanged)
  Q_PROPERTY(QString databasePath READ databasePath CONSTANT)

public:
  explicit SteamGameModel(const QString& databasePath = {}, QObject* parent = nullptr);
  ~SteamGameModel() override;

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] bool scanning() const;
  [[nodiscard]] bool steamDetected() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] QString errorText() const;
  [[nodiscard]] int artworkCount() const;
  [[nodiscard]] QString databasePath() const;

  Q_INVOKABLE QVariantMap get(int row) const;
  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE void toggleHidden(int row);
  Q_INVOKABLE void refresh();
  void refreshFromRoots(const QStringList& roots);

signals:
  void scanningChanged();
  void statusChanged();

private:
  struct Game {
    SteamGameRecord steam;
    bool favorite = false;
    bool hidden = false;
    QColor accentStart;
    QColor accentEnd;
  };

  [[nodiscard]] static QString defaultDatabasePath();
  [[nodiscard]] QVariant valueForRole(const Game& game, int role) const;
  bool openDatabase(const QString& path);
  bool ensureSchema();
  void loadDatabase();
  void applyScan(const SteamScanResult& result);
  void rebuildWatchPaths(const SteamScanResult& result);
  void setStatus(const QString& status, const QString& error = {});

  QVector<Game> m_games;
  QSqlDatabase m_database;
  QString m_connectionName;
  QString m_databasePath;
  QFutureWatcher<SteamScanResult> m_scanWatcher;
  QFileSystemWatcher m_fileWatcher;
  QTimer m_rescanTimer;
  bool m_scanning = false;
  bool m_steamDetected = false;
  QString m_statusText;
  QString m_errorText;
};
