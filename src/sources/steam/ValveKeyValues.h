#pragma once

#include <QHash>
#include <QString>

struct ValveKeyValues {
  QHash<QString, QString> values;
  QHash<QString, ValveKeyValues> objects;

  [[nodiscard]] QString value(const QString& key) const;
  [[nodiscard]] const ValveKeyValues* object(const QString& key) const;
};

class ValveKeyValuesParser final {
public:
  [[nodiscard]] static bool parse(const QByteArray& contents, ValveKeyValues* result,
                                  QString* error = nullptr);
  [[nodiscard]] static bool parseFile(const QString& path, ValveKeyValues* result,
                                      QString* error = nullptr);
};
