#pragma once

#include <QAbstractListModel>
#include <QNetworkAccessManager>
#include <QSqlDatabase>
#include <QString>
#include <QVector>

class AppSettings;

class AchievementModel final : public QAbstractListModel {
  Q_OBJECT
  Q_PROPERTY(QString appId READ appId NOTIFY summaryChanged)
  Q_PROPERTY(int unlocked READ unlocked NOTIFY summaryChanged)
  Q_PROPERTY(int total READ total NOTIFY summaryChanged)
  Q_PROPERTY(int knownCount READ knownCount NOTIFY summaryChanged)
  Q_PROPERTY(QString statusText READ statusText NOTIFY summaryChanged)
  Q_PROPERTY(qint64 cacheBytes READ cacheBytes NOTIFY cacheChanged)

public:
  enum Role {
    ApiNameRole = Qt::UserRole + 1,
    TitleRole,
    DescriptionRole,
    IconPathRole,
    UnlockedRole,
    UnlockTimeRole,
    RarityRole,
    HiddenRole,
    CurrentProgressRole,
    MaximumProgressRole,
  };
  Q_ENUM(Role)

  explicit AchievementModel(const QString& databasePath, AppSettings* settings,
                            QObject* parent = nullptr);
  ~AchievementModel() override;

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  [[nodiscard]] QString appId() const;
  [[nodiscard]] int unlocked() const;
  [[nodiscard]] int total() const;
  [[nodiscard]] int knownCount() const;
  [[nodiscard]] QString statusText() const;
  [[nodiscard]] qint64 cacheBytes() const;

  Q_INVOKABLE void load(const QString& appId);
  Q_INVOKABLE void clearCache();

signals:
  void summaryChanged();
  void cacheChanged();

private:
  struct Achievement {
    QString apiName;
    QString title;
    QString description;
    QString iconUrl;
    QString iconPath;
    bool unlocked = false;
    qint64 unlockTime = 0;
    double rarity = 0.0;
    bool hidden = false;
    double currentProgress = 0.0;
    double maximumProgress = 0.0;
  };

  [[nodiscard]] QString cacheRoot() const;
  [[nodiscard]] QString pathForIcon(const QString& appId, const QString& url) const;
  void requestMissingIcons();
  void requestIcon(int row);
  void pruneCache();
  void updateCacheBytes();

  QVector<Achievement> m_achievements;
  QString m_appId;
  QString m_statusText;
  int m_unlocked = 0;
  int m_total = 0;
  qint64 m_cacheBytes = 0;
  AppSettings* m_settings = nullptr;
  QSqlDatabase m_database;
  QString m_connectionName;
  QNetworkAccessManager m_network;
};
