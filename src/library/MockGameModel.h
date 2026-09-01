#pragma once

#include "library/GameRoles.h"

#include <QAbstractListModel>
#include <QColor>
#include <QString>
#include <QVector>

class MockGameModel final : public QAbstractListModel {
  Q_OBJECT

public:
  explicit MockGameModel(QObject* parent = nullptr, int gameCount = 100,
                         bool firstUninstalled = false);

  [[nodiscard]] int rowCount(const QModelIndex& parent = QModelIndex()) const override;
  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override;
  [[nodiscard]] QHash<int, QByteArray> roleNames() const override;

  Q_INVOKABLE QVariantMap get(int row) const;
  Q_INVOKABLE void toggleFavorite(int row);

private:
  struct Game {
    QString title;
    QString subtitle;
    QString description;
    int hours = 0;
    int progress = 0;
    int achievementsUnlocked = 0;
    int achievementsTotal = 0;
    bool favorite = false;
    bool recent = false;
    qint64 lastPlayed = 0;
    QColor accentStart;
    QColor accentEnd;
    QString coverMark;
    int year = 0;
    QString appId;
    QString coverPath;
    QString heroPath;
    QString logoPath;
    QString installPath;
    QString source = QStringLiteral("Demo");
    bool installed = true;
  };

  [[nodiscard]] QVariant valueForRole(const Game& game, int role) const;

  QVector<Game> m_games;
};
