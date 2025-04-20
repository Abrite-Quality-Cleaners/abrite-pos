#ifndef STYLESHEETS_H
#define STYLESHEETS_H

static const QString WINDOW_STYLESHEET     = "QMainWindow         {background: black; color: white;}";
static const QString LABEL_STYLESHEET      = "QLabel              {color: white;}";
static const QString BUTTON_STYLESHEET     = "QPushButton         {width: 100; height: 40; border-radius: 3px; border: 1px solid white; background: black; color: white}"
                                             "QPushButton:hover   {background: white; color: black;}"
                                             "QPushButton:pressed {background: darkgray; color: black;}";
static const QString QDIALOG_STYLESHEET    = "QDialog             {background: black; color: white;}";
static const QString LINEEDIT_STYLESHEET   = "QLineEdit           {background: black; color: white; border: 1px solid white; border-radius: 3px;}";
static const QString LISTWIDGET_STYLESHEET = "QListWidget         {background: black; color: white; border: 1px solid white; border-radius: 3px;}";

static const QString PRICES_INI = "/home/abrite/abrite-ui/prices.ini";
static const QString STORE_INI = "/home/abrite/abrite-ui/store.ini";

#endif // STYLESHEETS_H
