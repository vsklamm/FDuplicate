#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "modeldir.h"

#include <QMainWindow>
#include <QStringListModel>
#include <QTreeWidget>
#include <memory>
#include <set>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void add_root(QByteArray &name, QVector<QString> &vec);
    void add_child(QTreeWidgetItem * parent, QString &name);
    QString get_selected_directory();

    void show_message_box(QString const& message);
    void sleep(const int msecs);

private slots:
    void on_addDirectoryButton_clicked();
    void on_deleteDirectoryButton_clicked();
    void on_checkRecursively_stateChanged(int state);

    void select_directory();
    void start_scanning();
    void stop_scanning();
    void remove_files();
    void show_directory(QString const& dir);

    // messages
    void no_directory_selected();

    // menu bar
    void show_about_dialog();

public:
    QString current_directory;
    bool recursively;
    std::set<QString> start_directories;
    ModelDir * model;

private:
    std::unique_ptr<Ui::MainWindow> ui;
};

#endif // MAINWINDOW_H
