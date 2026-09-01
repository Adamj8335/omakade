#pragma once

#include "sources/lutris/LutrisScanner.h"

#include <QAbstractListModel>
#include <QColor>
#include <QSqlDatabase>

class LutrisGameModel final : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(bool lutrisDetected READ lutrisDetected NOTIFY statusChanged)
  Q_PROPERTY(QString statusText READ statusText NOTIFY statusChanged)
  Q_PROPERTY(QString errorText READ errorText NOTIFY statusChanged)

public:
  explicit LutrisGameModel(const QString& omakadeDatabasePath, QObject* parent = nullptr);
  ~LutrisGameModel() override;

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;
  [[nodiscard]] bool lutrisDetected() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] QString errorText() const;

  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE void toggleHidden(int row);
  Q_INVOKABLE void refresh();
  void refreshFromDatabases(const QStringList& paths);

signals:
  void statusChanged();

private:
  struct Game {
    LutrisGameRecord lutris;
    bool favorite = false;
    bool hidden = false;
    QColor accentStart;
    QColor accentEnd;
  };

  bool openDatabase(const QString& path);
  bool ensureSchema();
  void loadDatabase();
  void applyScan(const LutrisScanResult& result);
  [[nodiscard]] QVariant valueForRole(const Game& game, int role) const;
  void setStatus(const QString& status, const QString& error = {});

  QVector<Game> m_games;
  QSqlDatabase m_database;
  QString m_connectionName;
  bool m_lutrisDetected = false;
  QString m_statusText;
  QString m_errorText;
};
