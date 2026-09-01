#include "library/UnifiedGameModel.h"

UnifiedGameModel::UnifiedGameModel(QObject* parent) : QAbstractListModel(parent) {}

void UnifiedGameModel::addSourceModel(QAbstractItemModel* model) {
  if (model == nullptr || m_models.contains(model)) {
    return;
  }
  beginResetModel();
  m_models.append(model);
  endResetModel();

  connect(model, &QAbstractItemModel::modelReset, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::rowsInserted, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::rowsRemoved, this, [this] {
    beginResetModel();
    endResetModel();
  });
  connect(model, &QAbstractItemModel::dataChanged, this,
          [this, model](const QModelIndex& topLeft, const QModelIndex& bottomRight,
                        const QList<int>& roles) {
            const int offset = offsetFor(model);
            if (offset >= 0) {
              emit dataChanged(index(offset + topLeft.row()), index(offset + bottomRight.row()),
                               roles);
            }
          });
}

int UnifiedGameModel::rowCount(const QModelIndex& parent) const {
  if (parent.isValid()) {
    return 0;
  }
  int count = 0;
  for (const QAbstractItemModel* model : m_models) {
    count += model->rowCount();
  }
  return count;
}

QVariant UnifiedGameModel::data(const QModelIndex& index, int role) const {
  const SourceRow source = mapRow(index.row());
  return source.model == nullptr ? QVariant{} : source.model->index(source.row, 0).data(role);
}

QHash<int, QByteArray> UnifiedGameModel::roleNames() const {
  return m_models.isEmpty() ? QHash<int, QByteArray>{} : m_models.constFirst()->roleNames();
}

void UnifiedGameModel::toggleFavorite(int row) {
  const SourceRow source = mapRow(row);
  if (source.model != nullptr) {
    QMetaObject::invokeMethod(source.model, "toggleFavorite", Q_ARG(int, source.row));
  }
}

void UnifiedGameModel::toggleHidden(int row) {
  const SourceRow source = mapRow(row);
  if (source.model != nullptr) {
    QMetaObject::invokeMethod(source.model, "toggleHidden", Q_ARG(int, source.row));
  }
}

UnifiedGameModel::SourceRow UnifiedGameModel::mapRow(int row) const {
  if (row < 0) {
    return {};
  }
  for (QAbstractItemModel* model : m_models) {
    if (row < model->rowCount()) {
      return {.model = model, .row = row};
    }
    row -= model->rowCount();
  }
  return {};
}

int UnifiedGameModel::offsetFor(const QAbstractItemModel* model) const {
  int offset = 0;
  for (const QAbstractItemModel* candidate : m_models) {
    if (candidate == model) {
      return offset;
    }
    offset += candidate->rowCount();
  }
  return -1;
}
