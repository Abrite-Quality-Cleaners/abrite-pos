#include "Session.h"
#include "WindowController.h"

#include <QApplication>
#include <QThread>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // Construct the Session and MongoManager singletons
    // Doesn't matter which db is specified here, as long as it exists, the actual database will
    // be set later based on the selected store
    Session::instance().getMongoManager("mongodb://localhost:27017", "SparkleCleaners");

    WindowController winController;
    winController.start();

    return a.exec();
}
