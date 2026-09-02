#include "app/SingleInstance.h"

#include <QCoreApplication>
#include <QDebug>
#include <QLocalSocket>

#include <unistd.h>

SingleInstance::SingleInstance(const QString& serverName, QObject* parent)
    : QObject(parent),
      m_serverName(serverName.isEmpty() ? QStringLiteral("omakade-%1").arg(getuid()) : serverName) {
  connect(&m_server, &QLocalServer::newConnection, this, [this] {
    while (QLocalSocket* socket = m_server.nextPendingConnection()) {
      connect(socket, &QLocalSocket::readyRead, this, [this, socket] {
        if (socket->readAll().contains("activate")) {
          emit activationRequested();
        }
        socket->disconnectFromServer();
      });
      connect(socket, &QLocalSocket::disconnected, socket, &QObject::deleteLater);
    }
  });
}

bool SingleInstance::claimOrNotify() {
  if (m_server.listen(m_serverName)) {
    return true;
  }

  QLocalSocket socket;
  socket.connectToServer(m_serverName, QIODevice::WriteOnly);
  if (socket.waitForConnected(120)) {
    socket.write("activate");
    socket.flush();
    socket.waitForBytesWritten(120);
    return false;
  }

  if (m_server.serverError() != QAbstractSocket::AddressInUseError) {
    // An unwritable runtime directory must not leave the user with no window at all.
    qWarning() << "Running without single-instance activation:" << m_server.errorString();
    return true;
  }
  QLocalServer::removeServer(m_serverName);
  if (!m_server.listen(m_serverName)) {
    qWarning() << "Running without single-instance activation:" << m_server.errorString();
  }
  return true;
}
