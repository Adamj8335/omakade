#include "library/LibraryFilterModel.h"

#include "library/GameRoles.h"
#include "library/UnifiedGameModel.h"

#include <algorithm>

LibraryFilterModel::LibraryFilterModel(QObject* parent) : QSortFilterProxyModel(parent) {
  setDynamicSortFilter(true);
  sort(0);
}

void LibraryFilterModel::setSourceModel(QAbstractItemModel* source) {
  if (sourceModel() != nullptr) {
    disconnect(sourceModel(), nullptr, this, nullptr);
  }
  QSortFilterProxyModel::setSourceModel(source);
  if (auto* games = qobject_cast<UnifiedGameModel*>(source)) {
    connect(games, &UnifiedGameModel::collectionsChanged, this, [this] {
      beginFilterChange();
      endFilterChange(Direction::Rows);
      emit organizationNamesChanged();
    });
  }
  emit organizationNamesChanged();
}

LibraryFilterModel::SortMode LibraryFilterModel::sortMode() const { return m_sortMode; }

void LibraryFilterModel::setSortMode(SortMode value) {
  if (m_sortMode == value) {
    return;
  }
  m_sortMode = value;
  invalidate();
  sort(0);
  emit sortModeChanged();
}

LibraryFilterModel::Availability LibraryFilterModel::availability() const { return m_availability; }

void LibraryFilterModel::setAvailability(Availability value) {
  if (m_availability == value) {
    return;
  }
  m_availability = value;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit availabilityChanged();
}

bool LibraryFilterModel::showHidden() const { return m_showHidden; }

void LibraryFilterModel::setShowHidden(bool value) {
  if (m_showHidden == value) {
    return;
  }
  m_showHidden = value;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit showHiddenChanged();
}

QString LibraryFilterModel::sourceFilter() const { return m_sourceFilter; }

void LibraryFilterModel::setSourceFilter(const QString& value) {
  const QString normalized = value.trimmed();
  if (m_sourceFilter.compare(normalized, Qt::CaseInsensitive) == 0) {
    return;
  }
  m_sourceFilter = normalized;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit sourceFilterChanged();
}

QString LibraryFilterModel::completionFilter() const { return m_completionFilter; }

void LibraryFilterModel::setCompletionFilter(const QString& value) {
  const QString normalized = value.trimmed().toLower();
  if (m_completionFilter == normalized) {
    return;
  }
  m_completionFilter = normalized;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit organizationFilterChanged();
}

QString LibraryFilterModel::collectionFilter() const { return m_collectionFilter; }

void LibraryFilterModel::setCollectionFilter(const QString& value) {
  const QString normalized = value.trimmed();
  if (m_collectionFilter.compare(normalized, Qt::CaseInsensitive) == 0) {
    return;
  }
  m_collectionFilter = normalized;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit organizationFilterChanged();
}

QString LibraryFilterModel::tagFilter() const { return m_tagFilter; }

void LibraryFilterModel::setTagFilter(const QString& value) {
  const QString normalized = value.trimmed();
  if (m_tagFilter.compare(normalized, Qt::CaseInsensitive) == 0) {
    return;
  }
  m_tagFilter = normalized;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit organizationFilterChanged();
}

QStringList LibraryFilterModel::collectionNames() const {
  const auto* games = qobject_cast<const UnifiedGameModel*>(sourceModel());
  return games == nullptr ? QStringList{} : games->collectionNames();
}

QStringList LibraryFilterModel::tagNames() const {
  const auto* games = qobject_cast<const UnifiedGameModel*>(sourceModel());
  return games == nullptr ? QStringList{} : games->tagNames();
}

QString LibraryFilterModel::searchText() const { return m_searchText; }

void LibraryFilterModel::setSearchText(const QString& value) {
  const QString normalized = value.trimmed();
  if (m_searchText == normalized) {
    return;
  }

  m_searchText = normalized;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit searchTextChanged();
}

LibraryFilterModel::Mode LibraryFilterModel::mode() const { return m_mode; }

void LibraryFilterModel::setMode(Mode value) {
  if (m_mode == value) {
    return;
  }

  m_mode = value;
  beginFilterChange();
  endFilterChange(Direction::Rows);
  emit modeChanged();
}

QVariantMap LibraryFilterModel::get(int row) const {
  if (row < 0 || row >= rowCount()) {
    return {};
  }

  const QModelIndex sourceIndex = mapToSource(index(row, 0));
  QVariantMap result;
  const auto roles = sourceModel()->roleNames();
  for (auto iterator = roles.cbegin(); iterator != roles.cend(); ++iterator) {
    result.insert(QString::fromUtf8(iterator.value()), sourceIndex.data(iterator.key()));
  }
  return result;
}

void LibraryFilterModel::toggleFavorite(int row) {
  if (row < 0 || row >= rowCount()) {
    return;
  }

  QMetaObject::invokeMethod(sourceModel(), "toggleFavorite",
                            Q_ARG(int, mapToSource(index(row, 0)).row()));
}

void LibraryFilterModel::toggleHidden(int row) {
  if (row < 0 || row >= rowCount()) {
    return;
  }
  QMetaObject::invokeMethod(sourceModel(), "toggleHidden",
                            Q_ARG(int, mapToSource(index(row, 0)).row()));
}

bool LibraryFilterModel::setCustomCover(int row, const QUrl& sourceUrl) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return false;
  }
  return games->setCustomCover(mapToSource(index(row, 0)).row(), sourceUrl);
}

bool LibraryFilterModel::resetCustomCover(int row) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return false;
  }
  return games->resetCustomCover(mapToSource(index(row, 0)).row());
}

QVariantList LibraryFilterModel::installations(int row) const {
  const auto* games = qobject_cast<const UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return {};
  }
  return games->installations(mapToSource(index(row, 0)).row());
}

QVariantList LibraryFilterModel::linkCandidates(int row, const QString& search) const {
  const auto* games = qobject_cast<const UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return {};
  }
  return games->linkCandidates(mapToSource(index(row, 0)).row(), search);
}

bool LibraryFilterModel::linkGames(int row, const QString& source, const QString& runner,
                                   const QString& appId) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return false;
  }
  return games->linkGames(mapToSource(index(row, 0)).row(), source, runner, appId);
}

bool LibraryFilterModel::recordLaunch(int row, const QString& source, const QString& runner,
                                      const QString& appId) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return false;
  }
  return games->recordLaunch(mapToSource(index(row, 0)).row(), source, runner, appId);
}

bool LibraryFilterModel::unlinkGames(int row) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || row < 0 || row >= rowCount()) {
    return false;
  }
  return games->unlinkGames(mapToSource(index(row, 0)).row());
}

bool LibraryFilterModel::setCompletionStatus(int row, const QString& status) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  return games != nullptr && row >= 0 && row < rowCount() &&
         games->setCompletionStatus(mapToSource(index(row, 0)).row(), status);
}

bool LibraryFilterModel::setTags(int row, const QString& tags) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  return games != nullptr && row >= 0 && row < rowCount() &&
         games->setTags(mapToSource(index(row, 0)).row(), tags);
}

bool LibraryFilterModel::createCollection(const QString& name) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  return games != nullptr && games->createCollection(name);
}

bool LibraryFilterModel::deleteCollection(const QString& name) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  if (games == nullptr || !games->deleteCollection(name)) {
    return false;
  }
  if (m_collectionFilter.compare(name.trimmed(), Qt::CaseInsensitive) == 0) {
    setCollectionFilter({});
  }
  return true;
}

bool LibraryFilterModel::setCollectionMembership(int row, const QString& name, bool included) {
  auto* games = qobject_cast<UnifiedGameModel*>(sourceModel());
  return games != nullptr && row >= 0 && row < rowCount() &&
         games->setCollectionMembership(mapToSource(index(row, 0)).row(), name, included);
}

bool LibraryFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const {
  const QModelIndex sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);

  const QVariant installedValue = sourceIndex.data(GameRoles::Installed);
  const bool installed = !installedValue.isValid() || installedValue.toBool();
  if ((m_availability == Availability::Installed && !installed) ||
      (m_availability == Availability::ReadyToInstall && installed)) {
    return false;
  }

  if (!m_sourceFilter.isEmpty()) {
    const QString primarySource = sourceIndex.data(GameRoles::Source).toString();
    const QStringList linkedSources =
        sourceIndex.data(GameRoles::LinkedSources).toString().split(QStringLiteral(" + "));
    if (primarySource.compare(m_sourceFilter, Qt::CaseInsensitive) != 0 &&
        std::none_of(linkedSources.cbegin(), linkedSources.cend(), [this](const QString& source) {
          return source.compare(m_sourceFilter, Qt::CaseInsensitive) == 0;
        })) {
      return false;
    }
  }

  const bool hidden = sourceIndex.data(GameRoles::Hidden).toBool();
  if (m_mode == Mode::Hidden && !hidden) {
    return false;
  }
  if (m_mode != Mode::Hidden && !m_showHidden && hidden) {
    return false;
  }

  if (m_mode == Mode::Favorites && !sourceIndex.data(GameRoles::Favorite).toBool()) {
    return false;
  }
  if (m_mode == Mode::Recent && !sourceIndex.data(GameRoles::Recent).toBool()) {
    return false;
  }

  if (!m_completionFilter.isEmpty() &&
      sourceIndex.data(GameRoles::CompletionStatus).toString() != m_completionFilter) {
    return false;
  }
  const auto containsCaseInsensitive = [](const QStringList& values, const QString& expected) {
    return std::any_of(values.cbegin(), values.cend(), [&expected](const QString& value) {
      return value.compare(expected, Qt::CaseInsensitive) == 0;
    });
  };
  if (!m_collectionFilter.isEmpty() &&
      !containsCaseInsensitive(sourceIndex.data(GameRoles::Collections).toStringList(),
                               m_collectionFilter)) {
    return false;
  }
  if (!m_tagFilter.isEmpty() &&
      !containsCaseInsensitive(sourceIndex.data(GameRoles::Tags).toStringList(), m_tagFilter)) {
    return false;
  }

  if (m_searchText.isEmpty()) {
    return true;
  }

  const QString title = sourceIndex.data(GameRoles::Title).toString();
  const QString subtitle = sourceIndex.data(GameRoles::Subtitle).toString();
  const QString tags = sourceIndex.data(GameRoles::Tags).toStringList().join(QLatin1Char(' '));
  return title.contains(m_searchText, Qt::CaseInsensitive) ||
         subtitle.contains(m_searchText, Qt::CaseInsensitive) ||
         tags.contains(m_searchText, Qt::CaseInsensitive);
}

bool LibraryFilterModel::lessThan(const QModelIndex& left, const QModelIndex& right) const {
  if (m_sortMode == SortMode::RecentlyPlayed) {
    const qint64 leftPlayed = left.data(GameRoles::LastPlayed).toLongLong();
    const qint64 rightPlayed = right.data(GameRoles::LastPlayed).toLongLong();
    if (leftPlayed != rightPlayed) {
      return leftPlayed > rightPlayed;
    }
  }
  if (m_sortMode == SortMode::Playtime) {
    return left.data(GameRoles::Hours).toInt() > right.data(GameRoles::Hours).toInt();
  }
  return left.data(GameRoles::Title)
             .toString()
             .localeAwareCompare(right.data(GameRoles::Title).toString()) < 0;
}
