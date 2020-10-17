#include <QString>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QActionGroup>
#include <QDesktopServices>
#include "settingsdialog.h"
#include "plugindialog.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.h"
#include "cheatdialog.h"
#include "vidext.h"
#include "netplay/createroom.h"
#include "netplay/joinroom.h"

#include "osal/osal_preproc.h"
#include "interface/core_commands.h"
#include "interface/version.h"
#include "m64p_common.h"

#define RECENT_SIZE 10

m64p_video_extension_functions vidExtFunctions = {14,
                                                 qtVidExtFuncInit,
                                                 qtVidExtFuncQuit,
                                                 qtVidExtFuncListModes,
                                                 qtVidExtFuncListRates,
                                                 qtVidExtFuncSetMode,
                                                 qtVidExtFuncSetModeWithRate,
                                                 qtVidExtFuncGLGetProc,
                                                 qtVidExtFuncGLSetAttr,
                                                 qtVidExtFuncGLGetAttr,
                                                 qtVidExtFuncGLSwapBuf,
                                                 qtVidExtFuncSetCaption,
                                                 qtVidExtFuncToggleFS,
                                                 qtVidExtFuncResizeWindow,
                                                 qtVidExtFuncGLGetDefaultFramebuffer};

void MainWindow::updatePlugins()
{
    QDir PluginDir(settings->value("pluginDirPath").toString());
    PluginDir.setFilter(QDir::Files);
    QStringList Filter;
    Filter.append("");
    QStringList current;
    QString default_value;

    if (!settings->contains("videoPlugin")) {
        Filter.replace(0,"mupen64plus-video*");
        current = PluginDir.entryList(Filter);
        default_value = "mupen64plus-video-GLideN64";
        default_value += OSAL_DLL_EXTENSION;
        if (current.isEmpty())
            settings->setValue("videoPlugin", "dummy");
        else if (current.indexOf(default_value) != -1)
            settings->setValue("videoPlugin", default_value);
        else
            settings->setValue("videoPlugin", current.at(0));
    }
    if (!settings->contains("audioPlugin")) {
        Filter.replace(0,"mupen64plus-audio*");
        current = PluginDir.entryList(Filter);
        default_value = "mupen64plus-audio-sdl2";
        default_value += OSAL_DLL_EXTENSION;
        if (current.isEmpty())
            settings->setValue("audioPlugin", "dummy");
        else if (current.indexOf(default_value) != -1)
            settings->setValue("audioPlugin", default_value);
        else
            settings->setValue("audioPlugin", current.at(0));
    }
    if (!settings->contains("rspPlugin")) {
        Filter.replace(0,"mupen64plus-rsp*");
        current = PluginDir.entryList(Filter);
        default_value = "mupen64plus-rsp-hle";
        default_value += OSAL_DLL_EXTENSION;
        if (current.isEmpty())
            settings->setValue("rspPlugin", "dummy");
        else if (current.indexOf(default_value) != -1)
            settings->setValue("rspPlugin", default_value);
        else
            settings->setValue("rspPlugin", current.at(0));
    }
    if (!settings->contains("inputPlugin")) {
        Filter.replace(0,"mupen64plus-input*");
        current = PluginDir.entryList(Filter);
        default_value = "mupen64plus-input-qt";
        default_value += OSAL_DLL_EXTENSION;
        if (current.isEmpty())
            settings->setValue("inputPlugin", "dummy");
        else if (current.indexOf(default_value) != -1)
            settings->setValue("inputPlugin", default_value);
        else
            settings->setValue("inputPlugin", current.at(0));
    }
}

void MainWindow::updatePIF(Ui::MainWindow *ui)
{
    QMenu *PIF = new QMenu(this);
    PIF->setTitle("PIF ROM");
    ui->menuFile->insertMenu(ui->actionTake_Screenshot, PIF);
    ui->menuFile->insertSeparator(ui->actionTake_Screenshot);

    QAction *fileSelect = new QAction(this);
    QString current = settings->value("PIF_ROM").toString();
    fileSelect->setText("PIF ROM: " + current);
    PIF->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered, [=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("PIF ROM"), NULL, tr("PIF ROM File (*.bin)"));
        if (!filename.isNull()) {
            settings->setValue("PIF_ROM", filename);
            QString current = filename;
            fileSelect->setText("PIF ROM: " + current);
        }
    });

    QAction *clearSelect = new QAction(this);
    clearSelect->setText("Clear PIF ROM Selection");
    PIF->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("PIF_ROM");
        fileSelect->setText("PIF ROM: ");
    });
}

void MainWindow::updateDD(Ui::MainWindow *ui)
{
    QMenu *DD = new QMenu(this);
    DD->setTitle("64DD");
    ui->menuFile->insertMenu(ui->actionTake_Screenshot, DD);

    QAction *fileSelect = new QAction(this);
    QString current = settings->value("DD_ROM").toString();
    fileSelect->setText("64DD IPL ROM: " + current);
    DD->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered, [=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("64DD IPL ROM"), NULL, tr("64DD ROM File (*.n64)"));
        if (!filename.isNull()) {
            settings->setValue("DD_ROM", filename);
            QString current = filename;
            fileSelect->setText("64DD IPL ROM: " + current);
        }
    });

    QAction *fileSelect2 = new QAction(this);
    current = settings->value("DD_DISK").toString();
    fileSelect2->setText("64DD Disk: " + current);
    DD->addAction(fileSelect2);
    connect(fileSelect2, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("64DD Disk File"), NULL, tr("64DD Disk File (*.ndd)"));
        if (!filename.isNull()) {
            settings->setValue("DD_DISK", filename);
            QString current = filename;
            fileSelect2->setText("64DD Disk: " + current);
        }
    });

    QAction *clearSelect = new QAction(this);
    clearSelect->setText("Clear 64DD Selections");
    DD->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("DD_ROM");
        settings->remove("DD_DISK");
        fileSelect->setText("64DD IPL ROM: ");
        fileSelect2->setText("64DD Disk: ");
    });

    //DD->addSeparator();

    //QAction *startGame = new QAction(this);
    //startGame->setText("Start 64DD");
    //DD->addAction(startGame);
    //connect(startGame, &QAction::triggered,[=](){
    //    openROM("");
    //});
}

void MainWindow::updateGB(Ui::MainWindow *ui)
{
    QMenu *GB = new QMenu(this);
    GB->setTitle("Game Boy Cartridges");
    ui->menuFile->insertMenu(ui->actionTake_Screenshot, GB);

    QAction *fileSelect = new QAction(this);
    QString current = settings->value("Player1GBROM").toString();
    fileSelect->setText("Player 1 ROM: " + current);
    GB->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB ROM File"), NULL, tr("GB ROM Files (*.gb)"));
        if (!filename.isNull()) {
            settings->setValue("Player1GBROM", filename);
            QString current = filename;
            fileSelect->setText("Player 1 ROM: " + current);
        }
    });

    QAction *fileSelect2 = new QAction(this);
    current = settings->value("Player1GBRAM").toString();
    fileSelect2->setText("Player 1 RAM: " + current);
    GB->addAction(fileSelect2);
    connect(fileSelect2, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB RAM File"), NULL, tr("GB RAM Files (*.sav)"));
        if (!filename.isNull()) {
            settings->setValue("Player1GBRAM", filename);
            QString current = filename;
            fileSelect2->setText("Player 1 RAM: " + current);
        }
    });

    QAction *clearSelect = new QAction(this);
    clearSelect->setText("Clear Player 1 Selections");
    GB->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("Player1GBROM");
        settings->remove("Player1GBRAM");
        fileSelect->setText("Player 1 ROM: ");
        fileSelect2->setText("Player 1 RAM: ");
    });
    GB->addSeparator();

    fileSelect = new QAction(this);
    current = settings->value("Player2GBROM").toString();
    fileSelect->setText("Player 2 ROM: " + current);
    GB->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB ROM File"), NULL, tr("GB ROM Files (*.gb)"));
        if (!filename.isNull()) {
            settings->setValue("Player2GBROM", filename);
            QString current = filename;
            fileSelect->setText("Player 2 ROM: " + current);
        }
    });

    fileSelect2 = new QAction(this);
    current = settings->value("Player2GBRAM").toString();
    fileSelect2->setText("Player 2 RAM: " + current);
    GB->addAction(fileSelect2);
    connect(fileSelect2, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB RAM File"), NULL, tr("GB RAM Files (*.sav)"));
        if (!filename.isNull()) {
            settings->setValue("Player2GBRAM", filename);
            QString current = filename;
            fileSelect2->setText("Player 2 RAM: " + current);
        }
    });

    clearSelect = new QAction(this);
    clearSelect->setText("Clear Player 2 Selections");
    GB->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("Player2GBROM");
        settings->remove("Player2GBRAM");
        fileSelect->setText("Player 2 ROM: ");
        fileSelect2->setText("Player 2 RAM: ");
    });
    GB->addSeparator();

    fileSelect = new QAction(this);
    current = settings->value("Player3GBROM").toString();
    fileSelect->setText("Player 3 ROM: " + current);
    GB->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB ROM File"), NULL, tr("GB ROM Files (*.gb)"));
        if (!filename.isNull()) {
            settings->setValue("Player3GBROM", filename);
            QString current = filename;
            fileSelect->setText("Player 3 ROM: " + current);
        }
    });

    fileSelect2 = new QAction(this);
    current = settings->value("Player3GBRAM").toString();
    fileSelect2->setText("Player 3 RAM: " + current);
    GB->addAction(fileSelect2);
    connect(fileSelect2, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB RAM File"), NULL, tr("GB RAM Files (*.sav)"));
        if (!filename.isNull()) {
            settings->setValue("Player3GBRAM", filename);
            QString current = filename;
            fileSelect2->setText("Player 3 RAM: " + current);
        }
    });

    clearSelect = new QAction(this);
    clearSelect->setText("Clear Player 3 Selections");
    GB->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("Player3GBROM");
        settings->remove("Player3GBRAM");
        fileSelect->setText("Player 3 ROM: ");
        fileSelect2->setText("Player 3 RAM: ");
    });
    GB->addSeparator();

    fileSelect = new QAction(this);
    current = settings->value("Player4GBROM").toString();
    fileSelect->setText("Player 4 ROM: " + current);
    GB->addAction(fileSelect);
    connect(fileSelect, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB ROM File"), NULL, tr("GB ROM Files (*.gb)"));
        if (!filename.isNull()) {
            settings->setValue("Player4GBROM", filename);
            QString current = filename;
            fileSelect->setText("Player 4 ROM: " + current);
        }
    });

    fileSelect2 = new QAction(this);
    current = settings->value("Player4GBRAM").toString();
    fileSelect2->setText("Player 4 RAM: " + current);
    GB->addAction(fileSelect2);
    connect(fileSelect2, &QAction::triggered,[=](){
        QString filename = QFileDialog::getOpenFileName(this,
            tr("GB RAM File"), NULL, tr("GB RAM Files (*.sav)"));
        if (!filename.isNull()) {
            settings->setValue("Player4GBRAM", filename);
            QString current = filename;
            fileSelect2->setText("Player 4 RAM: " + current);
        }
    });

    clearSelect = new QAction(this);
    clearSelect->setText("Clear Player 4 Selections");
    GB->addAction(clearSelect);
    connect(clearSelect, &QAction::triggered,[=](){
        settings->remove("Player4GBROM");
        settings->remove("Player4GBRAM");
        fileSelect->setText("Player 4 ROM: ");
        fileSelect2->setText("Player 4 RAM: ");
    });
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    verbose = 0;
    nogui = 0;
    gles = 0;
    ui->setupUi(this);

    m_title = "mupen64plus-gui    Build Date: ";
    m_title += __DATE__;
    this->setWindowTitle(m_title);

    QString ini_path = QDir(QCoreApplication::applicationDirPath()).filePath("mupen64plus-gui.ini");
    settings = new QSettings(ini_path, QSettings::IniFormat, this);

    if (!settings->isWritable()) {
        settings->deleteLater();
        settings = new QSettings("mupen64plus", "gui", this);
    }

    restoreGeometry(settings->value("geometry").toByteArray());
    restoreState(settings->value("windowState").toByteArray());

    QActionGroup *my_slots_group = new QActionGroup(this);
    QAction *my_slots[10];
    OpenRecent = new QMenu(this);
    QMenu * SaveSlot = new QMenu(this);
    OpenRecent->setTitle("Open Recent");
    SaveSlot->setTitle("Change Save Slot");
    ui->menuFile->insertMenu(ui->actionSave_State, OpenRecent);
    ui->menuFile->insertSeparator(ui->actionSave_State);
    ui->menuFile->insertMenu(ui->actionSave_State_To, SaveSlot);
    ui->menuFile->insertSeparator(ui->actionSave_State_To);
    for (int i = 0; i < 10; ++i) {
        my_slots[i] = new QAction(this);
        my_slots[i]->setCheckable(true);
        my_slots[i]->setText("Slot " + QString::number(i));
        my_slots[i]->setActionGroup(my_slots_group);
        SaveSlot->addAction(my_slots[i]);
        QAction *temp_slot = my_slots[i];
        connect(temp_slot, &QAction::triggered,[=](bool checked){
            if (checked) {
                int slot = temp_slot->text().remove("Slot ").toInt();
                (*CoreDoCommand)(M64CMD_STATE_SET_SLOT, slot, NULL);
            }
        });
    }
    my_slots[0]->setChecked(true);

    updateOpenRecent();
    updateGB(ui);
    updateDD(ui);
    updatePIF(ui);

    if (!settings->contains("coreLibPath"))
        settings->setValue("coreLibPath", QDir(QCoreApplication::applicationDirPath()).filePath(OSAL_DEFAULT_DYNLIB_FILENAME));
    if (!settings->contains("pluginDirPath"))
        settings->setValue("pluginDirPath", QCoreApplication::applicationDirPath());

    updatePlugins();

    if (!settings->contains("volume"))
        settings->setValue("volume", 100);
    VolumeAction * volumeAction = new VolumeAction(tr("Volume"));
    connect(volumeAction->slider(), SIGNAL(valueChanged(int)), this, SLOT(volumeValueChanged(int)));
    volumeAction->slider()->setValue(settings->value("volume").toInt());
    ui->menuEmulation->insertAction(ui->actionMute, volumeAction);

    coreStarted = 0;
    coreLib = nullptr;
    gfxPlugin = nullptr;
    rspPlugin = nullptr;
    audioPlugin = nullptr;
    inputPlugin = nullptr;
    loadCoreLib();
    loadPlugins();
}

MainWindow::~MainWindow()
{
    closePlugins();
    closeCoreLib();

    delete ui;
}

void MainWindow::volumeValueChanged(int value)
{
    if (value != settings->value("volume").toInt())
    {
        settings->setValue("volume", value);
        if (coreStarted)
            (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_AUDIO_VOLUME, &value);
    }
}

void MainWindow::setVerbose()
{
    verbose = 1;
}

void MainWindow::setNoGUI()
{
    nogui = 1;
    if (!menuBar()->isNativeMenuBar())
        menuBar()->hide();
    statusBar()->hide();
}

void MainWindow::setGLES()
{
    gles = 1;
}

int MainWindow::getNoGUI()
{
    return nogui;
}

int MainWindow::getVerbose()
{
    return verbose;
}

int MainWindow::getGLES()
{
    return gles;
}

void MainWindow::resizeMainWindow(int Width, int Height)
{
    QSize size = this->size();
    Height += (menuBar()->isNativeMenuBar() ? 0 : ui->menuBar->height()) + ui->statusBar->height();
    if (size.width() != Width || size.height() != Height)
        resize(Width, Height);
}

void MainWindow::toggleFS(int force)
{
    int response = M64VIDEO_NONE;
    if (force == M64VIDEO_NONE)
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_VIDEO_MODE, &response);
    if (response == M64VIDEO_WINDOWED || force == M64VIDEO_FULLSCREEN) {
        if (!menuBar()->isNativeMenuBar())
            menuBar()->hide();
        statusBar()->hide();
        showFullScreen();
    } else if (response == M64VIDEO_FULLSCREEN || force == M64VIDEO_WINDOWED) {
        if (!nogui) {
            if (!menuBar()->isNativeMenuBar())
                menuBar()->show();
            statusBar()->show();
        }
        showNormal();
    }
}

void MainWindow::findRecursion(const QString &path, const QString &pattern, QStringList *result)
{
    QDir currentDir(path);
    const QString prefix = path + QLatin1Char('/');
    foreach (const QString &match, currentDir.entryList(QStringList(pattern), QDir::Files))
        result->append(prefix + match);
    foreach (const QString &dir, currentDir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDotAndDotDot))
        findRecursion(prefix + dir, pattern, result);
}

void MainWindow::closeEvent (QCloseEvent *event)
{
    stopGame();

    settings->setValue("geometry", saveGeometry());
    settings->setValue("windowState", saveState());

    event->accept();
}

void MainWindow::updateOpenRecent()
{
    OpenRecent->clear();
    QAction *recent[RECENT_SIZE];
    QStringList list = settings->value("RecentROMs").toString().split(";");
    if (list.first() == "") {
        OpenRecent->setEnabled(false);
        return;
    }

    OpenRecent->setEnabled(true);
    for (int i = 0; i < list.size() && i < RECENT_SIZE; ++i) {
        recent[i] = new QAction(this);
        recent[i]->setText(list.at(i));
        OpenRecent->addAction(recent[i]);
        QAction *temp_recent = recent[i];
        connect(temp_recent, &QAction::triggered,[=](){
                    openROM(temp_recent->text(), "", 0, 0);
                });
    }
    OpenRecent->addSeparator();

    QAction *clearRecent = new QAction(this);
    clearRecent->setText("Clear List");
    OpenRecent->addAction(clearRecent);
    connect(clearRecent, &QAction::triggered,[=](){
        settings->remove("RecentROMs");
        updateOpenRecent();
    });
}

void MainWindow::setTitle(std::string title)
{
    QString _title = QString::fromStdString(title);
    _title.prepend("mupen64plus: ");
    this->setWindowTitle(_title);
}

void MainWindow::pluginWarning(QString name)
{
    QMessageBox msgBox;
    msgBox.setText("No " + name + " plugin attached. Please go to \"Settings->Plugin Selection\" and choose one.");
    msgBox.exec();
}

void MainWindow::showMessage(QString message)
{
    QMessageBox *msgBox = new QMessageBox(this);
    msgBox->setText(message);
    msgBox->show();
}

void MainWindow::createOGLWindow(QSurfaceFormat* format)
{
    if (my_window) my_window->deleteLater();

    my_window = new OGLWindow();
    QWidget *container = QWidget::createWindowContainer(my_window, this);
    container->setFocusPolicy(Qt::StrongFocus);

    my_window->setCursor(Qt::BlankCursor);
    my_window->setFormat(*format);

    setCentralWidget(container);

    my_window->installEventFilter(&keyPressFilter);
    this->installEventFilter(&keyPressFilter);
}

void MainWindow::deleteOGLWindow()
{
    QWidget *container = new QWidget(this);
    setCentralWidget(container);
    my_window->doneCurrent();
    my_window->deleteLater();
    my_window = nullptr;
}

void MainWindow::stopGame()
{
    if (coreStarted)
    {
        int response;
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE, &response);
        if (response == M64EMU_RUNNING)
        {
            (*CoreDoCommand)(M64CMD_STOP, 0, NULL);
            while (workerThread->isRunning())
                QCoreApplication::processEvents();
        }
    }
}

void MainWindow::openROM(QString filename, QString netplay_ip, int netplay_port, int netplay_player)
{
    stopGame();

    logViewer.clearLog();

    workerThread = new WorkerThread(netplay_ip, netplay_port, netplay_player, this);
    workerThread->setFileName(filename);
    connect(workerThread, SIGNAL(finished()), workerThread, SLOT(deleteLater()));
    workerThread->start();

    QStringList list;
    if (settings->contains("RecentROMs"))
        list = settings->value("RecentROMs").toString().split(";");
    list.removeAll(filename);
    list.prepend(filename);
    if (list.size() > RECENT_SIZE)
        list.removeLast();
    settings->setValue("RecentROMs",list.join(";"));
    updateOpenRecent();
}

void MainWindow::on_actionOpen_ROM_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open ROM"), settings->value("ROMdir").toString(), tr("ROM Files (*.n64 *.N64 *.z64 *.Z64 *.v64 *.V64 *.zip *.ZIP *.7z)"));
    if (!filename.isNull()) {
        QFileInfo info(filename);
        settings->setValue("ROMdir", info.absoluteDir().absolutePath());
        openROM(filename, "", 0, 0);
    }
}

void MainWindow::on_actionPlugin_Paths_triggered()
{
    SettingsDialog *settings = new SettingsDialog(this);
    settings->show();
}

void MainWindow::on_actionStop_Game_triggered()
{
    stopGame();
}

void MainWindow::on_actionExit_triggered()
{
    this->close();
}

void MainWindow::on_actionPlugin_Settings_triggered()
{
    PluginDialog *settings = new PluginDialog(this);
    settings->show();
}

void MainWindow::on_actionPause_Game_triggered()
{
    if (coreStarted) {
        int response;
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_EMU_STATE, &response);
        if (response == M64EMU_RUNNING)
            (*CoreDoCommand)(M64CMD_PAUSE, 0, NULL);
        else if (response == M64EMU_PAUSED)
            (*CoreDoCommand)(M64CMD_RESUME, 0, NULL);
    }
}

void MainWindow::on_actionMute_triggered()
{
    if (coreStarted) {
        int response;
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_AUDIO_MUTE, &response);
        if (response == 0) {
            response = 1;
            (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_AUDIO_MUTE, &response);
        } else if (response == 1) {
            response = 0;
            (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_AUDIO_MUTE, &response);
        }
    }
}

void MainWindow::on_actionHard_Reset_triggered()
{
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_RESET, 1, NULL);
}

void MainWindow::on_actionSoft_Reset_triggered()
{
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_RESET, 0, NULL);
}

void MainWindow::on_actionTake_Screenshot_triggered()
{
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_TAKE_NEXT_SCREENSHOT, 0, NULL);
}

void MainWindow::on_actionSave_State_triggered()
{
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_STATE_SAVE, 1, NULL);
}

void MainWindow::on_actionLoad_State_triggered()
{
    if (coreStarted)
        (*CoreDoCommand)(M64CMD_STATE_LOAD, 1, NULL);
}

void MainWindow::on_actionToggle_Fullscreen_triggered()
{
    if (coreStarted) {
        int response;
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_VIDEO_MODE, &response);
        if (response == M64VIDEO_WINDOWED) {
            response = M64VIDEO_FULLSCREEN;
            (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_VIDEO_MODE, &response);
        } else if (response == M64VIDEO_FULLSCREEN) {
            response = M64VIDEO_WINDOWED;
            (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_VIDEO_MODE, &response);
        }
    }
}

void MainWindow::on_actionSave_State_To_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this,
        tr("Save State File"), NULL, NULL);
    if (!filename.isNull()) {
        if (!filename.contains(".st"))
            filename.append(".state");
        if (coreStarted) {
            (*CoreDoCommand)(M64CMD_STATE_SAVE, 1, filename.toLatin1().data());
        }
    }
}

void MainWindow::on_actionLoad_State_From_triggered()
{
    QString filename = QFileDialog::getOpenFileName(this,
        tr("Open Save State"), NULL, tr("State Files (*.st* *.pj*)"));
    if (!filename.isNull()) {
        if (coreStarted) {
            (*CoreDoCommand)(M64CMD_STATE_LOAD, 1, filename.toLatin1().data());
        }
    }
}

void MainWindow::on_actionCheats_triggered()
{
    CheatDialog *cheats = new CheatDialog(this);
    cheats->show();
}

void MainWindow::on_actionController_Configuration_triggered()
{
    typedef void (*Config_Func)();
    Config_Func PluginConfig = (Config_Func) osal_dynlib_getproc(inputPlugin, "PluginConfig");
    if (PluginConfig)
        PluginConfig();
}

void MainWindow::on_actionToggle_Speed_Limiter_triggered()
{
    if (coreStarted) {
        int value;
        (*CoreDoCommand)(M64CMD_CORE_STATE_QUERY, M64CORE_SPEED_LIMITER, &value);
        value = !value;
        (*CoreDoCommand)(M64CMD_CORE_STATE_SET, M64CORE_SPEED_LIMITER, &value);
    }
}

void MainWindow::on_actionView_Log_triggered()
{
    logViewer.show();
}

void MainWindow::on_actionVideo_Settings_triggered()
{
    typedef void (*Config_Func)();
    Config_Func PluginConfig = (Config_Func) osal_dynlib_getproc(gfxPlugin, "PluginConfig");
    if (PluginConfig)
        PluginConfig();
}

void MainWindow::on_actionCreate_Room_triggered()
{
    CreateRoom *createRoom = new CreateRoom(this);
    createRoom->show();
}

void MainWindow::on_actionJoin_Room_triggered()
{
    JoinRoom *joinRoom = new JoinRoom(this);
    joinRoom->show();
}

void MainWindow::on_actionSupport_on_Patreon_triggered()
{
    QDesktopServices::openUrl(QUrl("https://www.patreon.com/m64p"));
}

void MainWindow::on_actionOpen_Discord_Channel_triggered()
{
    QDesktopServices::openUrl(QUrl("https://discord.gg/B5svWhR"));
}

WorkerThread* MainWindow::getWorkerThread()
{
    return workerThread;
}

OGLWindow* MainWindow::getOGLWindow()
{
    return my_window;
}

QSettings* MainWindow::getSettings()
{
    return settings;
}

LogViewer* MainWindow::getLogViewer()
{
    return &logViewer;
}

m64p_dynlib_handle MainWindow::getCoreLib()
{
    return coreLib;
}

m64p_dynlib_handle MainWindow::getAudioPlugin()
{
    return audioPlugin;
}

m64p_dynlib_handle MainWindow::getRspPlugin()
{
    return rspPlugin;
}

m64p_dynlib_handle MainWindow::getInputPlugin()
{
    return inputPlugin;
}

m64p_dynlib_handle MainWindow::getGfxPlugin()
{
    return gfxPlugin;
}

void MainWindow::setCoreStarted(int value)
{
    coreStarted = value;

    if (value == 0)
    {
        this->setWindowTitle(m_title);
        loadCoreLib();
        loadPlugins();
    }
}

int MainWindow::getCoreStarted()
{
    return coreStarted;
}

void MainWindow::closeCoreLib()
{
    if (coreLib != nullptr)
    {
        (*CoreShutdown)();
        osal_dynlib_close(coreLib);
        coreLib = nullptr;
    }
}

void MainWindow::loadCoreLib()
{
    closeCoreLib();

    osal_dynlib_open(&coreLib, settings->value("coreLibPath").toString().toLatin1().data());

    CoreStartup =                 (ptr_CoreStartup) osal_dynlib_getproc(coreLib, "CoreStartup");
    CoreShutdown =                (ptr_CoreShutdown) osal_dynlib_getproc(coreLib, "CoreShutdown");
    CoreDoCommand =               (ptr_CoreDoCommand) osal_dynlib_getproc(coreLib, "CoreDoCommand");
    CoreAttachPlugin =            (ptr_CoreAttachPlugin) osal_dynlib_getproc(coreLib, "CoreAttachPlugin");
    CoreDetachPlugin =            (ptr_CoreDetachPlugin) osal_dynlib_getproc(coreLib, "CoreDetachPlugin");
    CoreOverrideVidExt =          (ptr_CoreOverrideVidExt) osal_dynlib_getproc(coreLib, "CoreOverrideVidExt");

    ConfigGetUserConfigPath =     (ptr_ConfigGetUserConfigPath) osal_dynlib_getproc(coreLib, "ConfigGetUserConfigPath");
    ConfigSaveFile =              (ptr_ConfigSaveFile) osal_dynlib_getproc(coreLib, "ConfigSaveFile");
    ConfigGetParameterHelp =      (ptr_ConfigGetParameterHelp) osal_dynlib_getproc(coreLib, "ConfigSaveFile");
    ConfigGetParamInt =           (ptr_ConfigGetParamInt) osal_dynlib_getproc(coreLib, "ConfigGetParamInt");
    ConfigGetParamFloat =         (ptr_ConfigGetParamFloat) osal_dynlib_getproc(coreLib, "ConfigGetParamFloat");
    ConfigGetParamBool =          (ptr_ConfigGetParamBool) osal_dynlib_getproc(coreLib, "ConfigGetParamBool");
    ConfigGetParamString =        (ptr_ConfigGetParamString) osal_dynlib_getproc(coreLib, "ConfigGetParamString");
    ConfigSetParameter =          (ptr_ConfigSetParameter) osal_dynlib_getproc(coreLib, "ConfigSetParameter");
    ConfigDeleteSection =         (ptr_ConfigDeleteSection) osal_dynlib_getproc(coreLib, "ConfigDeleteSection");
    ConfigOpenSection =           (ptr_ConfigOpenSection) osal_dynlib_getproc(coreLib, "ConfigOpenSection");
    ConfigListParameters =        (ptr_ConfigListParameters) osal_dynlib_getproc(coreLib, "ConfigListParameters");
    ConfigGetSharedDataFilepath = (ptr_ConfigGetSharedDataFilepath) osal_dynlib_getproc(coreLib, "ConfigGetSharedDataFilepath");

    CoreAddCheat                = (ptr_CoreAddCheat) osal_dynlib_getproc(coreLib, "CoreAddCheat");
    CoreCheatEnabled            = (ptr_CoreCheatEnabled) osal_dynlib_getproc(coreLib, "CoreCheatEnabled");

    QString qtConfigDir = settings->value("configDirPath").toString();
    if (!qtConfigDir.isEmpty())
        (*CoreStartup)(CORE_API_VERSION, qtConfigDir.toLatin1().data() /*Config dir*/, QCoreApplication::applicationDirPath().toLatin1().data(), (char*)"Core", DebugCallback, NULL, NULL);
    else
        (*CoreStartup)(CORE_API_VERSION, NULL /*Config dir*/, QCoreApplication::applicationDirPath().toLatin1().data(), (char*)"Core", DebugCallback, NULL, NULL);

    CoreOverrideVidExt(&vidExtFunctions);
}

void MainWindow::closePlugins()
{
    ptr_PluginShutdown PluginShutdown;

    if (gfxPlugin != nullptr)
    {
        PluginShutdown = (ptr_PluginShutdown) osal_dynlib_getproc(gfxPlugin, "PluginShutdown");
        (*PluginShutdown)();
        osal_dynlib_close(gfxPlugin);
        gfxPlugin = nullptr;
    }
    if (audioPlugin != nullptr)
    {
        PluginShutdown = (ptr_PluginShutdown) osal_dynlib_getproc(audioPlugin, "PluginShutdown");
        (*PluginShutdown)();
        osal_dynlib_close(audioPlugin);
        audioPlugin = nullptr;
    }
    if (inputPlugin != nullptr)
    {
        PluginShutdown = (ptr_PluginShutdown) osal_dynlib_getproc(inputPlugin, "PluginShutdown");
        (*PluginShutdown)();
        osal_dynlib_close(inputPlugin);
        inputPlugin = nullptr;
    }
    if (rspPlugin != nullptr)
    {
        PluginShutdown = (ptr_PluginShutdown) osal_dynlib_getproc(rspPlugin, "PluginShutdown");
        (*PluginShutdown)();
        osal_dynlib_close(rspPlugin);
        rspPlugin = nullptr;
    }
}

void MainWindow::loadPlugins()
{
    closePlugins();

    ptr_PluginStartup PluginStartup;

    osal_dynlib_open(&gfxPlugin, QDir(settings->value("pluginDirPath").toString()).filePath(settings->value("videoPlugin").toString()).toLatin1().data());
    PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(gfxPlugin, "PluginStartup");
    (*PluginStartup)(coreLib, (char*)"Video", DebugCallback);

    osal_dynlib_open(&audioPlugin, QDir(settings->value("pluginDirPath").toString()).filePath(settings->value("audioPlugin").toString()).toLatin1().data());
    PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(audioPlugin, "PluginStartup");
    (*PluginStartup)(coreLib, (char*)"Audio", DebugCallback);

    osal_dynlib_open(&inputPlugin, QDir(settings->value("pluginDirPath").toString()).filePath(settings->value("inputPlugin").toString()).toLatin1().data());
    PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(inputPlugin, "PluginStartup");
    (*PluginStartup)(coreLib, (char*)"Input", DebugCallback);

    osal_dynlib_open(&rspPlugin, QDir(settings->value("pluginDirPath").toString()).filePath(settings->value("rspPlugin").toString()).toLatin1().data());
    PluginStartup = (ptr_PluginStartup) osal_dynlib_getproc(rspPlugin, "PluginStartup");
    (*PluginStartup)(coreLib, (char*)"RSP", DebugCallback);
}
