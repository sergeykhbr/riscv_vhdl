#include "DbgMainWindow.h"
#include "moc_DbgMainWindow.h"
#include "CpuWidgets/RegsViewWidget.h"
#include <QtWidgets/QtWidgets>


namespace debugger {

DbgMainWindow::DbgMainWindow(IGui *igui, event_def *init_done) {
    igui_ = igui;
    initDone_ = init_done;
    igui_->registerMainWindow(this);

    setWindowTitle(tr("RISC-V platform debugger"));
    resize(QDesktopWidget().availableGeometry(this).size() * 0.7);

    listConsoleListeners_.make_list(0);

    createActions();
    createMenus();
    createStatusBar();
    addWidgets();
    
    setUnifiedTitleAndToolBarOnMac(true);

    riseSyncEvent_ = true;
    timer = new QTimer(this);
    timer->setSingleShot(false);
    connect(timer, SIGNAL(timeout()), this, SLOT(slotTimerUpdate()));
    timer->start(10);
// debug:
}

DbgMainWindow::~DbgMainWindow() {
}

void DbgMainWindow::handleResponse(AttributeType *req, AttributeType *resp) {
}

void DbgMainWindow::setConfiguration(AttributeType cfg) {
    config_ = cfg;
    emit signalConfigure(&config_);
}

void DbgMainWindow::getConfiguration(AttributeType &cfg) {
    cfg = config_;
}


void DbgMainWindow::closeEvent(QCloseEvent *e) {
    timer->stop();
    emit signalClosingMainForm();
    AttributeType exit_cmd;
    exit_cmd.from_config("['exit']");
    igui_->registerCommand(static_cast<IGuiCmdHandler *>(this), &exit_cmd);
    e->accept();
}

void DbgMainWindow::slotTimerUpdate() {
    if (riseSyncEvent_) {
        RISCV_event_set(initDone_);
        riseSyncEvent_ = false;
    }
    emit signalRedrawByTimer();
}


void DbgMainWindow::slotUartKeyPressed(QString str) {
}

void DbgMainWindow::slotActionAbout() {
    char date[128];
    memcpy(date, __DATE__, sizeof(date));
    QString build;
    build.sprintf("Version: 1.0\nBuild:     %s\n", date);
    build += tr("Author: Sergey Khabarov\n");
    build += tr("git:    http://github.com/sergeykhbr/riscv_vhdl\n");
    build += tr("e-mail: sergeykhbr@gmail.com\n\n")
       + tr("\tRISC-V debugger GUI plugin is the open source application\n")
       + tr("that upgrades the base RISC-V debugger funcitonality and can\n")
       + tr("provide more friendly interaction interface with SoC running\n")
       + tr("on FPGA or on simulator:\n\n");
    build += tr("www.gnss-sensor.com\n");

    QMessageBox::about(this, tr("About GUI plugin"),
        build
        /*tr("\tThe <b>GTerm</b> application allows to interact with the "
               "GNSS receiver via a serial port or ethernet.\n"
               "\tSet of plugins allow to view and analize binary data stream in "
               "convenient user-friendly ways.")*/
               );
}

void DbgMainWindow::createActions() {
    actionRegs_ = new QAction(QIcon(":/images/toggle.png"), tr("&Regs"), this);
    actionRegs_->setToolTip(tr("CPU Registers view"));
    actionRegs_->setShortcut(tr("Ctrl-R", "Registers"));
    actionRegs_->setCheckable(true);
    actionRegs_->setChecked(false);
    //connect(actionRegs_, SIGNAL(triggered()), this, SLOT(slotActionRegs()));


    actionQuit_ = new QAction(tr("&Quit"), this);
    actionQuit_->setShortcuts(QKeySequence::Quit);
    actionQuit_->setStatusTip(tr("Quit the application"));
    connect(actionQuit_, SIGNAL(triggered()), this, SLOT(close()));

    actionAbout_ = new QAction(tr("&About"), this);
    actionAbout_->setStatusTip(tr("Show the application's About box"));
    connect(actionAbout_, SIGNAL(triggered()), this, SLOT(slotActionAbout()));

    QToolBar *toolbar1 = addToolBar(tr("Tool1"));
    toolbar1->addAction(actionRegs_);
}

void DbgMainWindow::createMenus() {
    QMenu *menu = menuBar()->addMenu(tr("&File"));
    menu->addAction(actionQuit_);
    menu->addSeparator();
    
    menu = menuBar()->addMenu(tr("&Views"));
    menu->addAction(actionRegs_);
    
    menu = menuBar()->addMenu(tr("&Help"));
    menu->addAction(actionAbout_);
}


void DbgMainWindow::createStatusBar() {
    statusBar()->showMessage(tr("Ready"));
}

void DbgMainWindow::addWidgets() {
    AttributeType cfgMdi(Attr_Dict);
    cfgMdi["Tabbed"].make_boolean(false);
    MdiAreaWidget *mdiArea = new MdiAreaWidget(cfgMdi, this);
    setCentralWidget(mdiArea);


    QWidget *pnew;
    MyQMdiSubWindow *subw;

    /** Docked Widgets: */
    QDockWidget *dock = new QDockWidget(tr("Debugger console"), this);
    dock->setAllowedAreas(Qt::BottomDockWidgetArea);
    addDockWidget(Qt::BottomDockWidgetArea, dock);
    

    ConsoleWidget *consoleWidget = new ConsoleWidget(igui_, this);
    dock->setWidget(consoleWidget);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            consoleWidget, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
            consoleWidget, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            consoleWidget, SLOT(slotClosingMainForm()));


    /** MDI Widgets: */
    subw = new MyQMdiSubWindow(this);
    subw->setWidget(pnew = new UartWidget(igui_, this));
    subw->setMinimumWidth(size().width() / 2);
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            pnew, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
            pnew, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            pnew, SLOT(slotClosingMainForm()));

    subw = new MyQMdiSubWindow(this);
    subw->setWidget(pnew = new GpioWidget(igui_, this));
    mdiArea->addSubWindow(subw);
    connect(this, SIGNAL(signalConfigure(AttributeType *)), 
            pnew, SLOT(slotConfigure(AttributeType *)));
    connect(this, SIGNAL(signalRedrawByTimer()), 
              pnew, SLOT(slotRepaintByTimer()));
    connect(this, SIGNAL(signalClosingMainForm()), 
            pnew, SLOT(slotClosingMainForm()));

    subw = new MyQMdiSubWindow(this);
    subw->setWidget(pnew = new RegsViewWidget(igui_, this));
    mdiArea->addSubWindow(subw);
    connect(actionRegs_, SIGNAL(triggered(bool)), 
            subw, SLOT(slotVisible(bool)));
    connect(subw, SIGNAL(signalVisible(bool)), 
            actionRegs_, SLOT(setChecked(bool)));
   
    subw->setVisible(false);
}

}  // namespace debugger

