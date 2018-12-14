#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "extended_file_info.h"
#include "modeldir.h"

#include <QLabel>
#include <QMainWindow>
#include <QProgressBar>
#include <QStringListModel>
#include <QTime>
#include <QTreeWidget>
#include <memory>
#include <set>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QString get_selected_directory();

    void show_message_box(QString const &message);

private slots:
    void on_addDirectoryButton_clicked();
    void on_deleteDirectoryButton_clicked();
    void on_checkRecursively_stateChanged(int state);

    void add_root(QByteArray &name, QVector<extended_file_info> &vec);
    void add_child(QTreeWidgetItem * parent, extended_file_info &file_info);

    void select_directory();
    void start_scanning();
    void stop_scanning();

    void remove_files();

    void update_tree(int dupes, QVector<extended_file_info> &new_duplicates);
    void on_scanningFinished();

    // messages
    void no_directory_selected();

    // menu bar
    void show_about_dialog();

signals:
    void transmit_data(std::set<QString> sDir, bool recursively = true);

public:
    std::set<QString> start_directories;
    ModelDir *model;
    QProgressBar *progressBar;
    QLabel *labelDupes;

private:
    QTime *t;
    std::unique_ptr<Ui::MainWindow> ui;
};

#endif // MAINWINDOW_H
