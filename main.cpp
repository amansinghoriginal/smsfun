#include <QApplication>
#include <QDebug>
#include <QtDBus/QDBusConnection>
#include <QtDBus/QDBusMessage>
#include <QtDBus/QDBusReply>
#include <QtDBus/QDBusInterface>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  if(!QDBusConnection::systemBus().isConnected()) {
    
    qWarning() << "Could Not Connect to System Bus";
  
  } else {

    qDebug() << "Connected to System Bus";
  
    QDBusMessage enumerateDevices = QDBusMessage::createMethodCall(
        "org.freedesktop.ModemManager",
        "/org/freedesktop/ModemManager",
        "org.freedesktop.ModemManager",
        "EnumerateDevices");
    QDBusReply< QList<QDBusObjectPath> > reply = QDBusConnection::systemBus().call(enumerateDevices);

    if(!reply.isValid()) {
      qWarning() << "Could not enumerate devices";
      qWarning() << reply.error().message();

    } else {

      if(reply.value().size()==0) {
        qWarning() << "No Modem Found";

      } else {
        QDBusObjectPath modemPath = reply.value()[0];
        qDebug() << "Device Found at : " << modemPath.path();

        QDBusInterface modemInterface(
            "org.freedesktop.ModemManager",
            modemPath.path(),
            "org.freedesktop.ModemManager.Modem",
            QDBusConnection::systemBus());

        bool enabled = modemInterface.property("Enabled").toBool();
        qDebug() << "Modem Enabled: " <<enabled;

        if(!enabled) {
          qDebug() << "Trying to Enable....";
          QDBusMessage enableModem = QDBusMessage::createMethodCall(
              "org.freedesktop.ModemManager",
              modemPath.path(),
              "org.freedesktop.ModemManager.Modem",
              "Enable");
          enableModem << true;

          QDBusConnection::systemBus().call(enableModem);
          enabled = modemInterface.property("Enabled").toBool();
          qDebug() << "Modem Enabled: " << enabled;
        }
      }
    }
  }

  return app.exec();
}
