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

#include <algorithm>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //  columns width
    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::Interactive);
    ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::Interactive);
    ui->treeWidget->header()->setSectionResizeMode(3, QHeaderView::Interactive);

    QCommonStyle style;
    ui->actionOpen_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionStart_Scanning->setIcon(style.standardIcon(QCommonStyle::SP_MediaPlay));
    ui->actionStop_Scanning->setIcon(style.standardIcon(QCommonStyle::SP_MediaStop));
    ui->actionRemove_Files->setIcon(style.standardIcon(QCommonStyle::SP_TrashIcon));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionOpen_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionStart_Scanning, &QAction::triggered, this, &MainWindow::start_scanning);
    connect(ui->actionStop_Scanning, &QAction::triggered, this, &MainWindow::stop_scanning);
    connect(ui->actionRemove_Files, &QAction::triggered, this, &MainWindow::remove_files);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);

    progressBar = new QProgressBar(ui->statusBar);
    progressBar->setAlignment(Qt::AlignRight);
    progressBar->setMaximumSize(180, 19);
    progressBar->setVisible(false);

    labelDupes = new QLabel(ui->statusBar);
    labelDupes->setAlignment(Qt::AlignRight);
    labelDupes->setMinimumSize(labelDupes->sizeHint());

    ui->statusBar->addPermanentWidget(labelDupes);
    ui->statusBar->addPermanentWidget(progressBar);

    model = new ModelDir(this);
}

MainWindow::~MainWindow()
{
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

void MainWindow::on_checkRecursively_stateChanged([[maybe_unused]] int state)
{
    // TODO: message ?
}

void MainWindow::add_root(QByteArray &name, QVector<extended_file_info> &vec)
{
    QTreeWidgetItem * treeItem = new QTreeWidgetItem(ui->treeWidget);
    treeItem->setText(0, name);
    treeItem->setText(0, name);
    ui->treeWidget->addTopLevelItem(treeItem);
    for(int i = 0; i < vec.size(); ++i)
    {
        add_child(treeItem, vec[i]);
    }
}
void MainWindow::add_child(QTreeWidgetItem * parent, extended_file_info &file_info)
{
    QTreeWidgetItem * treeItem = new QTreeWidgetItem();
    treeItem->setText(0, file_info.file_name);
    treeItem->setText(1, file_info.path);
    treeItem->setText(2, QString("%1").arg(file_info.size));
    treeItem->setText(3, file_info.file_name);
    parent->addChild(treeItem);
}

void MainWindow::update_tree(int dupes, QVector<extended_file_info> &new_duplicates)
{
    model->found_files = new_duplicates;

    labelDupes->setText(QString("Duplicates found: %1").arg(dupes));
    progressBar->setValue(50); // TODO: update

    // ui->treeView->setModel(model);

    // for (int column = 0; column < model->columnCount(); ++column)
    //        ui->treeView->resizeColumnToContents(column);
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

void MainWindow::start_scanning()
{
    t = new QTime();
    t->start();

    if (!start_directories.empty())
    {
        // TODO: ui in another function
        progressBar->setVisible(true);
        ui->statusBar->showMessage("Scanning...");

        qDebug() << "From main thread: " << QThread::currentThreadId();

        QThread * thread= new QThread;
        duplicate_finder * finder = new duplicate_finder();

        // connect(thread, &QThread::started, finder, &duplicate_finder::cancel_scanning);
        connect(finder, &duplicate_finder::tree_changed, this, &MainWindow::update_tree);
        connect(finder, &duplicate_finder::scanning_finished, this, &MainWindow::on_scanningFinished);
        connect(this, &MainWindow::transmit_data, finder, &duplicate_finder::process_drive, Qt::DirectConnection);

        finder->moveToThread(thread);

        thread->start();

        bool checked = ui->checkRecursively->isChecked();
        emit transmit_data(start_directories, checked);
    }
    else
    {
        no_directory_selected();
    }
}

void MainWindow::on_scanningFinished()
{
    ui->statusBar->clearMessage();
    ui->statusBar->showMessage(QString("Scan complete. Elapsed time: %1 ms").arg(t->elapsed()));
}

void MainWindow::stop_scanning()
{
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
