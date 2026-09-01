#pragma once

#include <QAbstractListModel>
#include <QVector>

class UnifiedGameModel final : public QAbstractListModel {
  Q_OBJECT

public:
  explicit UnifiedGameModel(QObject* parent = nullptr);

  void addSourceModel(QAbstractItemModel* model);
  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE void toggleHidden(int row);

private:
  struct SourceRow {
    QAbstractItemModel* model = nullptr;
    int row = -1;
  };
  [[nodiscard]] SourceRow mapRow(int row) const;
  [[nodiscard]] int offsetFor(const QAbstractItemModel* model) const;

  QVector<QAbstractItemModel*> m_models;
};
