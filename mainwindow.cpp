#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "duplicate_finder.h"
#include "foundfile.h"
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

#include <algorithm>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    current_directory(""), // TODO: is it right ?
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // columns width
    //ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    //ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    //ui->treeWidget->header()->setSectionResizeMode(2, QHeaderView::Interactive);

    QCommonStyle style;
    ui->actionOpen_Directory->setIcon(style.standardIcon(QCommonStyle::SP_DialogOpenButton));
    ui->actionStart_Scanning ->setIcon(style.standardIcon(QCommonStyle::SP_MediaPlay));
    ui->actionStop_Scanning->setIcon(style.standardIcon(QCommonStyle::SP_MediaStop));
    ui->actionRemove_Files ->setIcon(style.standardIcon(QCommonStyle::SP_TrashIcon));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionOpen_Directory, &QAction::triggered, this, &MainWindow::select_directory);
    connect(ui->actionStart_Scanning, &QAction::triggered, this, &MainWindow::start_scanning);
    connect(ui->actionStop_Scanning, &QAction::triggered, this, &MainWindow::stop_scanning);
    connect(ui->actionRemove_Files, &QAction::triggered, this, &MainWindow::remove_files);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &MainWindow::show_about_dialog);

    // ui->listStart_Directories->setEditTriggers(QAbstractItemView::AnyKeyPressed | QAbstractItemView::DoubleClicked);

    QVector<FoundFile> files = {
        { 1  , "Group1    " , 0 },
        { 2  , "Group1.1  " , 1 },
        { 3  , "Group1.2  " , 1 },
        { 4  , "Group2    " , 0 },
        { 5  , "Group2.1  " , 4 },
        { 6  , "Group1.3  " , 1 },
        { 7  , "Group3    " , 0 },
        { 8  , "Group3.1  " , 7 },
        { 9  , "Group3.1.1" , 8 },
        { 10 , "Group3.1.2" , 8 },
    };
    model = new ModelDir(this);
    model->found_files = files;

    ui->treeView->setModel(model);
    ui->treeView->repaint();
}

MainWindow::~MainWindow()
{}

void MainWindow::on_addDirectoryButton_clicked()
{
    QString adding_path = get_selected_directory();

    if (adding_path.size() != 0 && start_directories.find(adding_path) == start_directories.end()) {
        start_directories.insert(adding_path);
        QListWidgetItem *item = new QListWidgetItem(adding_path);
        ui->listStart_Directories->addItem(item);
    }
}

void MainWindow::on_deleteDirectoryButton_clicked()
{
    QModelIndexList selected = ui->listStart_Directories->selectionModel()->selectedIndexes();
    if (!selected.isEmpty())
    {
        for (auto i=0; i < selected.count(); ++i)
        {
            // start_directories.erase(ui->listStart_Directories->model()->);
            ui->listStart_Directories->model()->removeRow(i);
        }
    }
}

void MainWindow::on_checkRecursively_stateChanged(int state)
{
    show_message_box("state changed");
}

void MainWindow::add_root(QByteArray &name, QVector<QString> &vec)
{
    //    QTreeWidgetItem *itm = new QTree(ui->treeView);
    //    itm->setText(0,name);
    //    ui->treeView->addTopLevelItem(itm);
    //    for(int i = 0; i < vec.size(); ++i)
    //    {
    //        add_child(itm,vec[i]);
    //    }
}
void MainWindow::add_child(QTreeWidgetItem * parent, QString &name)
{
    //    QTreeWidgetItem *itm = new QTreeWidgetItem();
    //    itm->setText(0, name);
    //    parent->addChild(itm);
}

void MainWindow::select_directory()
{
    get_selected_directory();
}

void MainWindow::start_scanning()
{
    QTime t;
    t.start();

    if (current_directory.size() != 0) {
        dublicate_finder finder;
        bool complete = finder.process_drive(current_directory);

        // ====================================
        QProgressDialog progress(this);
        progress.setLabelText("Scanning");
        progress.setRange(0, 100);
        progress.setModal(true);

        for (int row = 0; row < 100; ++row) {
            progress.setValue(row);
            qApp->processEvents();
            if (progress.wasCanceled()) {
                ui->statusBar->clearMessage();
                ui->statusBar->showMessage("Сорян");
                break;
            }
            sleep(100);
        }
        // ====================================

        if (complete) {
            ui->statusBar->clearMessage();
            ui->statusBar->showMessage(QString("Scan complete. Elapsed time: %1 ms").arg(t.elapsed()));
        } else {

        }
    } else {
        no_directory_selected();
    }
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

void MainWindow::show_directory(QString const& dir)
{
    //    ui->treeView->reset(); // ???
    //    QDir d(dir);
    //    QFileInfoList list = d.entryInfoList();
    //    for (QFileInfo& file_info : list)
    //    {
    //        QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    //        item->setText(0, file_info.fileName());
    //        item->setText(1, QString::number(file_info.size()));
    //        item->setText(2, file_info.absolutePath());
    //        ui->treeWidget->addTopLevelItem(item);
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
    current_directory = dir;
    return dir;
}

void MainWindow::show_message_box(QString const& message)
{
    QMessageBox msgBox;
    msgBox.setText(message);
    msgBox.exec();
}

void MainWindow::no_directory_selected()
{
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "No directory selected", "Select first directory?",
                                  QMessageBox::Yes|QMessageBox::Cancel);
    if (reply == QMessageBox::Yes) {
        select_directory();
    }
}

void MainWindow::sleep(const int msecs)
{
    QTime dieTime= QTime::currentTime().addMSecs(msecs);
    while (QTime::currentTime() < dieTime) {
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
    }
}
