#include "sources/steam/ValveKeyValues.h"

#include <QFile>

#include <cctype>

namespace {
QString matchingKey(const auto& values, const QString& wanted) {
  for (auto iterator = values.cbegin(); iterator != values.cend(); ++iterator) {
    if (iterator.key().compare(wanted, Qt::CaseInsensitive) == 0) {
      return iterator.key();
    }
  }
  return {};
}

class Tokenizer final {
public:
  explicit Tokenizer(const QByteArray& input) : m_input(input) {}

  bool next(QString* result) {
    skipSpaceAndComments();
    if (m_position >= m_input.size()) {
      return false;
    }

    const char first = m_input.at(m_position++);
    if (first == '{' || first == '}') {
      *result = QString(first);
      return true;
    }
    if (first != '"') {
      QByteArray token(1, first);
      while (m_position < m_input.size()) {
        const char current = m_input.at(m_position);
        if (current == '{' || current == '}' || std::isspace(static_cast<unsigned char>(current))) {
          break;
        }
        token.append(current);
        ++m_position;
      }
      *result = QString::fromUtf8(token);
      return true;
    }

    QByteArray token;
    while (m_position < m_input.size()) {
      const char current = m_input.at(m_position++);
      if (current == '"') {
        *result = QString::fromUtf8(token);
        return true;
      }
      if (current == '\\' && m_position < m_input.size()) {
        const char escaped = m_input.at(m_position++);
        if (escaped == 'n') {
          token.append('\n');
        } else if (escaped == 't') {
          token.append('\t');
        } else {
          token.append(escaped);
        }
      } else {
        token.append(current);
      }
    }
    m_error = QStringLiteral("Unterminated quoted string");
    return false;
  }

  [[nodiscard]] QString error() const { return m_error; }

private:
  void skipSpaceAndComments() {
    while (m_position < m_input.size()) {
      const char current = m_input.at(m_position);
      if (std::isspace(static_cast<unsigned char>(current))) {
        ++m_position;
        continue;
      }
      if (current == '/' && m_position + 1 < m_input.size() && m_input.at(m_position + 1) == '/') {
        m_position += 2;
        while (m_position < m_input.size() && m_input.at(m_position) != '\n') {
          ++m_position;
        }
        continue;
      }
      break;
    }
  }

  QByteArray m_input;
  qsizetype m_position = 0;
  QString m_error;
};

bool parseObject(Tokenizer* tokenizer, ValveKeyValues* result, bool nested, QString* error) {
  while (true) {
    QString key;
    if (!tokenizer->next(&key)) {
      if (!tokenizer->error().isEmpty()) {
        *error = tokenizer->error();
        return false;
      }
      if (nested) {
        *error = QStringLiteral("Object is missing a closing brace");
        return false;
      }
      return true;
    }
    if (key == QStringLiteral("}")) {
      if (!nested) {
        *error = QStringLiteral("Unexpected closing brace");
        return false;
      }
      return true;
    }

    QString value;
    if (!tokenizer->next(&value) || value == QStringLiteral("}")) {
      *error = QStringLiteral("Missing value for key '%1'").arg(key);
      return false;
    }
    if (value == QStringLiteral("{")) {
      ValveKeyValues child;
      if (!parseObject(tokenizer, &child, true, error)) {
        return false;
      }
      result->objects.insert(key, child);
    } else {
      result->values.insert(key, value);
    }
  }
}
} // namespace

QString ValveKeyValues::value(const QString& key) const {
  return values.value(matchingKey(values, key));
}

const ValveKeyValues* ValveKeyValues::object(const QString& key) const {
  const auto iterator = objects.constFind(matchingKey(objects, key));
  return iterator == objects.cend() ? nullptr : &iterator.value();
}

bool ValveKeyValuesParser::parse(const QByteArray& contents, ValveKeyValues* result,
                                 QString* error) {
  if (result == nullptr) {
    return false;
  }
  *result = {};
  QString localError;
  Tokenizer tokenizer(contents);
  const bool success = parseObject(&tokenizer, result, false, &localError);
  if (error != nullptr) {
    *error = localError;
  }
  return success;
}

bool ValveKeyValuesParser::parseFile(const QString& path, ValveKeyValues* result, QString* error) {
  QFile file(path);
  if (!file.open(QIODevice::ReadOnly)) {
    if (error != nullptr) {
      *error = file.errorString();
    }
    return false;
  }
  return parse(file.readAll(), result, error);
}
