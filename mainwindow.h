#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "duplicate_finder.h"
#include "extended_file_info.h"

#include <QMainWindow>
#include <QTreeWidget>
#include <QProgressBar>
#include <QLabel>
#include <QElapsedTimer>

#include <set>
#include <memory>

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

    void selectDirectory();
    void startScanning();
    void removeFiles();

    void on_preprocessingFinished(int files_count);
    void on_updateTree(int completed_files, QVector<QVector<extended_file_info>> new_duplicates);
    void on_scanningFinished(int dupes);

    void showCancelYesDialog(const QString &title, const QString &text, std::function<void()> func);

    void showAboutDialog();

    void on_treeWidget_itemSelectionChanged();

signals:
    void transmitData(std::set<QString> sDir, bool recursively = true);
    void stopScanning();

public:
    std::unique_ptr<QLabel> labelDupes;
    std::unique_ptr<QProgressBar> progressBar;
    std::set<QString> start_directories;

private:
    std::unique_ptr<Ui::MainWindow> ui;
    std::unique_ptr<QThread> workingThread;
    std::unique_ptr<duplicate_finder> finder;
    std::unique_ptr<QElapsedTimer> taskTimer;
    bool scan_is_running;
};

#endif // MAINWINDOW_H
