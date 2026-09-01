#pragma once

#include <QObject>
#include <QString>

class AppSettings final : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY reducedMotionChanged)
  Q_PROPERTY(int artworkCacheLimitMb READ artworkCacheLimitMb WRITE setArtworkCacheLimitMb NOTIFY
                 artworkCacheLimitMbChanged)
  Q_PROPERTY(QString steamId READ steamId WRITE setSteamId NOTIFY steamIdChanged)

public:
  explicit AppSettings(const QString& path = {}, QObject* parent = nullptr);

  [[nodiscard]] bool reducedMotion() const;
  void setReducedMotion(bool value);
  [[nodiscard]] int artworkCacheLimitMb() const;
  void setArtworkCacheLimitMb(int value);
  [[nodiscard]] QString steamId() const;
  void setSteamId(const QString& value);

signals:
  void reducedMotionChanged();
  void artworkCacheLimitMbChanged();
  void steamIdChanged();

private:
  [[nodiscard]] static QString defaultPath();
  void load();
  void save() const;

  QString m_path;
  bool m_reducedMotion = false;
  int m_artworkCacheLimitMb = 1024;
  QString m_steamId;
};
