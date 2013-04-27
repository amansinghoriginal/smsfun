#include <QApplication>
#include <QtDBus/QDBusConnection>

int main(int argc, char **argv)
{
  QApplication app(argc, argv);

  if(!QDBusConnection::systemBus().isConnected()) {
    
    qWarning() << "Could Not Connect to System Bus";
  
  } else {

    qDebug() << "Connected to System Bus";
  
  }

  return app.exec();
}
