#pragma once

#include <QSortFilterProxyModel>
#include <QUrl>

class LibraryFilterModel final : public QSortFilterProxyModel {
  Q_OBJECT
  Q_PROPERTY(QString searchText READ searchText WRITE setSearchText NOTIFY searchTextChanged)
  Q_PROPERTY(Mode mode READ mode WRITE setMode NOTIFY modeChanged)
  Q_PROPERTY(SortMode sortMode READ sortMode WRITE setSortMode NOTIFY sortModeChanged)
  Q_PROPERTY(bool showHidden READ showHidden WRITE setShowHidden NOTIFY showHiddenChanged)
  Q_PROPERTY(
      QString sourceFilter READ sourceFilter WRITE setSourceFilter NOTIFY sourceFilterChanged)

public:
  enum class Mode { All = 0, Favorites, Recent, Hidden };
  Q_ENUM(Mode)
  enum class SortMode { Title = 0, RecentlyPlayed, Playtime };
  Q_ENUM(SortMode)

  explicit LibraryFilterModel(QObject* parent = nullptr);

  [[nodiscard]] QString searchText() const;
  void setSearchText(const QString& value);

  [[nodiscard]] Mode mode() const;
  void setMode(Mode value);
  [[nodiscard]] SortMode sortMode() const;
  void setSortMode(SortMode value);
  [[nodiscard]] bool showHidden() const;
  void setShowHidden(bool value);
  [[nodiscard]] QString sourceFilter() const;
  void setSourceFilter(const QString& value);

  Q_INVOKABLE QVariantMap get(int row) const;
  Q_INVOKABLE void toggleFavorite(int row);
  Q_INVOKABLE void toggleHidden(int row);
  Q_INVOKABLE bool setCustomCover(int row, const QUrl& sourceUrl);
  Q_INVOKABLE bool resetCustomCover(int row);
  Q_INVOKABLE QVariantList installations(int row) const;
  Q_INVOKABLE QVariantList linkCandidates(int row, const QString& search) const;
  Q_INVOKABLE bool linkGames(int row, const QString& source, const QString& runner,
                             const QString& appId);
  Q_INVOKABLE bool unlinkGames(int row);

signals:
  void searchTextChanged();
  void modeChanged();
  void sortModeChanged();
  void showHiddenChanged();
  void sourceFilterChanged();

protected:
  [[nodiscard]] bool filterAcceptsRow(int sourceRow,
                                      const QModelIndex& sourceParent) const override;
  [[nodiscard]] bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
  QString m_searchText;
  Mode m_mode = Mode::All;
  SortMode m_sortMode = SortMode::Title;
  bool m_showHidden = false;
  QString m_sourceFilter;
};
