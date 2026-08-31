#pragma once

#include <QObject>

class AppSettings final : public QObject {
  Q_OBJECT
  Q_PROPERTY(
      bool reducedMotion READ reducedMotion WRITE setReducedMotion NOTIFY reducedMotionChanged)
  Q_PROPERTY(int artworkCacheLimitMb READ artworkCacheLimitMb WRITE setArtworkCacheLimitMb NOTIFY
                 artworkCacheLimitMbChanged)

public:
  explicit AppSettings(const QString& path = {}, QObject* parent = nullptr);

  [[nodiscard]] bool reducedMotion() const;
  void setReducedMotion(bool value);
  [[nodiscard]] int artworkCacheLimitMb() const;
  void setArtworkCacheLimitMb(int value);

signals:
  void reducedMotionChanged();
  void artworkCacheLimitMbChanged();

private:
  [[nodiscard]] static QString defaultPath();
  void load();
  void save() const;

  QString m_path;
  bool m_reducedMotion = false;
  int m_artworkCacheLimitMb = 1024;
};
