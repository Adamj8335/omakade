#pragma once

#include <QAbstractListModel>
#include <QSqlDatabase>
#include <QUrl>
#include <QVector>

class UnifiedGameModel final : public QAbstractListModel {
  Q_OBJECT

public:
  explicit UnifiedGameModel(const QString& databasePath = {}, QObject* parent = nullptr);
  ~UnifiedGameModel() override;

  void addSourceModel(QAbstractItemModel* model);
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE void toggleHidden(int row);
  Q_INVOKABLE bool setCustomCover(int row, const QUrl& sourceUrl);
  Q_INVOKABLE bool resetCustomCover(int row);
  Q_INVOKABLE QVariantList installations(int row) const;
  Q_INVOKABLE QVariantList linkCandidates(int row, const QString& search) const;
  Q_INVOKABLE bool linkGames(int row, const QString& source, const QString& runner,
                             const QString& appId);
  Q_INVOKABLE bool unlinkGames(int row);

private:
  struct SourceRow {
    QAbstractItemModel* model = nullptr;
    int row = -1;
  };
  [[nodiscard]] SourceRow mapRow(int row) const;
  [[nodiscard]] QString gameKey(const SourceRow& source) const;
  [[nodiscard]] SourceRow sourceForKey(const QString& key) const;
  [[nodiscard]] QVector<SourceRow> groupRows(const SourceRow& source) const;
  [[nodiscard]] QVariantMap gameMap(const SourceRow& source) const;
  void rebuildRows();
  bool openArtworkDatabase(const QString& path);
  void loadArtworkOverrides();
  void loadLinks();

  QVector<QAbstractItemModel*> m_models;
  QVector<SourceRow> m_rows;
  QSqlDatabase m_database;
  QString m_connectionName;
  QString m_databasePath;
  QString m_artworkRoot;
  QHash<QString, QString> m_coverOverrides;
  QHash<QString, QString> m_groupForGame;
  QHash<QString, QString> m_primaryForGroup;
};
