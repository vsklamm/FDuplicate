#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "duplicate_finder.h"
#include "modeldir.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QProgressDialog>
#include <QPushButton>
#include <QStringListModel>
#include <QTime>
#include <QListWidgetItem>
#include <QThread>
#include <QDebug>
#include <QToolButton>

#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    workingThread = new QThread;

    finder = new duplicate_finder();
    finder->moveToThread(workingThread);

    taskTimer = new QTime();

    connect(this, &MainWindow::transmit_data, finder, &duplicate_finder::process_drive);
    connect(this, &MainWindow::stop_scanning, finder, &duplicate_finder::cancel_scanning, Qt::DirectConnection);
    connect(finder, &duplicate_finder::tree_changed, this, &MainWindow::on_updateTree);
    connect(finder, &duplicate_finder::scanning_finished, this, &MainWindow::on_scanningFinished);

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::Stretch);

    QCommonStyle style;
    ui->actionOpen_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionStart_Scanning->setIcon(style.standardIcon(QCommonStyle::SP_MediaPlay));
    ui->actionStop_Scanning->setIcon(style.standardIcon(QCommonStyle::SP_MediaStop));
    ui->actionRemove_Files->setIcon(style.standardIcon(QCommonStyle::SP_TrashIcon));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionOpen_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionStart_Scanning, &QAction::triggered, this, &MainWindow::start_scanning);
    connect(ui->actionStop_Scanning, &QAction::triggered, this, &MainWindow::on_cancelButton_clicked);
    connect(ui->actionRemove_Files, &QAction::triggered, this, &MainWindow::remove_files);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);
    connect(ui->toolExpandAll, &QToolButton::clicked, this, &MainWindow::on_expandAll_clicked);
    connect(ui->toolClearTable, &QToolButton::clicked, this, &MainWindow::on_clearTable_clicked);

    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    progressBar->setVisible(false);

    labelDupes = new QLabel(ui->statusBar);
    labelDupes->setAlignment(Qt::AlignRight);
    labelDupes->setMinimumSize(labelDupes->sizeHint());

    ui->statusBar->addPermanentWidget(labelDupes);
    ui->statusBar->addPermanentWidget(progressBar);

    // model = new ModelDir(this);
}

MainWindow::~MainWindow()
{
    workingThread->wait();

    delete workingThread;
    delete finder;
    delete taskTimer;
    // TODO: anything else?
}

void MainWindow::on_addDirectoryButton_clicked()
{
    select_directory();
}

void MainWindow::on_deleteDirectoryButton_clicked()
{
    QModelIndexList selected = ui->listStart_Directories->selectionModel()->selectedIndexes();
    if (!selected.isEmpty()) // TODO: if only one item too
    {
        for (auto i = 0; i < selected.count(); ++i)
        {
            int row = selected[i].row();
            start_directories.erase(ui->listStart_Directories->item(row)->text());
            ui->listStart_Directories->model()->removeRow(row);
        }
    }
}

void MainWindow::on_expandAll_clicked()
{
    ui->treeWidget->expandAll();
}

void MainWindow::on_clearTable_clicked()
{
    progressBar->setVisible(false);
    ui->statusBar->showMessage("");
    labelDupes->setText("");
    ui->treeWidget->clear();
}

void MainWindow::on_checkRecursively_stateChanged([[maybe_unused]] int state)
{
    // TODO: message ?
}

void MainWindow::start_scanning()
{
    if (!start_directories.empty())
    {
        qDebug() << QString(__func__) << " from Main thread: " << QThread::currentThreadId();

        taskTimer->restart();

        workingThread->start();

        emit transmit_data(start_directories, ui->checkRecursively->isChecked());

        ui->statusBar->showMessage("Scanning...");
        progressBar->setVisible(true);
        progressBar->setMaximum(100);
        progressBar->setValue(100);
    }
    else
    {
        no_directory_selected();
    }
}

void MainWindow::on_preprocessingFinished(int files_count)
{
    progressBar->setMaximum(files_count);
}

void MainWindow::on_updateTree(int completed_files, QVector<QVector<extended_file_info>> new_duplicates)
{
    progressBar->setValue(completed_files * 100 / progressBar->maximum()); // TODO: update

    for (auto i = 0; i < new_duplicates.size(); ++i)
    {
        QTreeWidgetItem * item = new QTreeWidgetItem(ui->treeWidget);
        item->setText(0, new_duplicates[i][0].file_name + QString(". (%1 files)").arg(new_duplicates[i].size()));

        for (auto j = 0; j < new_duplicates[i].size(); ++j)
        {
            QTreeWidgetItem * child_item = new QTreeWidgetItem();
            child_item->setText(0, new_duplicates[i][j].file_name);
            child_item->setText(1, new_duplicates[i][j].path);
            child_item->setText(2, QString::number(new_duplicates[i][j].size)); // TODO: formatting size
            // modified time
            item->addChild(child_item);
        }
    }
}

void MainWindow::on_scanningFinished(int dupes)
{
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(QString("Scan complete. Elapsed time: %1 ms").arg(taskTimer->elapsed()));
    labelDupes->setText(QString("Duplicates found: %1").arg(dupes));
}

void MainWindow::remove_files()
{
    //    bool success = true;
    //    QString s = "";
    //    for (size_t i = 0; i < equals.size(); i++) {
    //        for (size_t j = 1; j < equals[i].size(); j++) {
    //            QFile f(equals[i][j]);
    //            if (!f.remove()) {
    //                success = false;
    //                s += equals[i][j] + ' ';
    //            }
    //        }
    //    }
    //    if (success) {
    //        s = "successful";
    //    } else {
    //        s = "failed to remove: " + s;
    //    }
}

void MainWindow::select_directory()
{
    QString adding_path = get_selected_directory();

    if (adding_path.size() != 0 && start_directories.find(adding_path) == start_directories.end())
    {
        start_directories.insert(adding_path);
        QListWidgetItem *item = new QListWidgetItem(adding_path);
        ui->listStart_Directories->addItem(item);
    }
}

void MainWindow::on_cancelButton_clicked()
{
    emit stop_scanning();
}

void MainWindow::show_about_dialog()
{
    QMessageBox::about(this, "FDuplicate", "");
}

QString MainWindow::get_selected_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    return dir;
}

void MainWindow::show_message_box(QString const &message)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void MainWindow::no_directory_selected()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "No directory selected", "Select first directory to scan?",
                                  QMessageBox::Cancel | QMessageBox::Yes);
    if (reply == QMessageBox::Yes)
    {
        select_directory();
    }
}
