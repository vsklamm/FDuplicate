#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "duplicate_finder.h"
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
    void on_expandAll_clicked();
    void on_clearTable_clicked();
    void on_cancelButton_clicked();
    void on_removeFilesButton_clicked();
    void on_checkRecursively_stateChanged(int state);

    void select_directory();
    void start_scanning();
    void remove_files();

    void on_preprocessingFinished(int files_count);
    void on_updateTree(int completed_files, QVector<QVector<extended_file_info>> new_duplicates);
    void on_scanningFinished(int dupes);

    void show_cancel_yes_dialog(const QString &title, const QString &text, std::function<void()> func);

    void show_about_dialog();

    void on_treeWidget_itemSelectionChanged();

signals:
    void transmit_data(std::set<QString> sDir, bool recursively = true);
    void stop_scanning();

public:
    // ModelDir *model;
    QLabel * labelDupes;
    QProgressBar * progressBar;

    std::set<QString> start_directories;

private:
    QTime * taskTimer; // TODO: unique_ptr
    duplicate_finder * finder;

    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QThread> workingThread;
    // Ui::MainWindow * ui;
};

#endif // MAINWINDOW_H
