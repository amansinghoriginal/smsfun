#include <QApplication>
#include <QDebug>
#include <QMetaType>
#include <QtDBus>
#include <QVariant>

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

        QDBusInterface modemUssdInterface(
            "org.freedesktop.ModemManager",
            modemPath.path(),
            "org.freedesktop.ModemManager.Modem.Gsm.Ussd",
            QDBusConnection::systemBus());

        QString ussdState = modemUssdInterface.property("State").toString();
        if(ussdState != "idle")
          modemUssdInterface.call("Cancel");

        qDebug() << "Dialing *444# to check balance";
        QDBusReply<QString> balanceMessage = modemUssdInterface.call("Initiate","*444#");
        qDebug() << balanceMessage;

        QDBusInterface modemSMSInterface(
            "org.freedesktop.ModemManager",
            modemPath.path(),
            "org.freedesktop.ModemManager.Modem.Gsm.SMS",
            QDBusConnection::systemBus());

        qDebug() << "Displaying first SMS on the SIM card";
        QDBusReply < QVariantMap >  firstSMS = modemSMSInterface.call("Get",uint(0));
        qDebug() << firstSMS.value()["text"].toString();

        qDebug() << "Sending my first SMS Hello World to +919013380180";
        QVariantMap helloWorld;
        helloWorld["number"]="+919013380180";
        helloWorld["text"]="Hello World!";
        QDBusReply< QList<QVariant> > helloWorldSent = modemSMSInterface.call("Send",helloWorld);

        if(helloWorldSent.isValid()) {
          qDebug() << helloWorldSent.value();
        } else {
          qWarning() << "Unable to send message |= " << helloWorldSent.error().message();
        }
      }
    }
  }

  return app.exec();
}
