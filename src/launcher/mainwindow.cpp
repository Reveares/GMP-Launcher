#include <QPushButton>
#include <QSettings>
#include <QListWidget>
#include <QMessageBox>
#include <QDataWidgetMapper>
#include <QAction>
#include <QDir>
#include <QFileInfo>
#include <QProcess>

#ifdef _WIN32
#include <windows.h>
#endif

#include "dialogaddserver.h"
#include "ui_mainwindow.h"
#include "mainwindow.h"
#include "servermodel.h"
#include "dialoginfo.h"
#include "server.h"
#include "options.h"

MainWindow::MainWindow() :
    QMainWindow(nullptr),
    m_pUi(new Ui::MainWindow),
    m_pServerModel(new ServerModel),
    m_pMapper(new QDataWidgetMapper(this))
{
    m_pUi->setupUi(this);

    m_pServerModel->Initialize();

    m_pMapper->setModel(m_pServerModel);
    m_pMapper->setSubmitPolicy(QDataWidgetMapper::AutoSubmit);

    m_pUi->listServer->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_pUi->listServer->setModel(m_pServerModel);
    m_pUi->listServer->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_pUi->listServer->hideColumn(Server::P_Description);
    m_pUi->listServer->hideColumn(Server::P_Port);
    m_pUi->listServer->hideColumn(Server::P_Url);
    m_pUi->listServer->hideColumn(Server::P_Nick);
    m_pUi->listServer->horizontalHeader()->setVisible(true);

    connect(m_pUi->buttonJoin, &QPushButton::clicked, this, &MainWindow::startProcess);
    connect(m_pUi->listServer, &QTableView::doubleClicked, this, &MainWindow::startProcess);

    connect(m_pUi->buttonUpdateServerList, &QPushButton::clicked, m_pServerModel, &ServerModel::updateRecords);

    connect(m_pUi->buttonRemoveServer, &QPushButton::clicked, [this]()
    {
        QModelIndex index = m_pUi->listServer->selectionModel()->currentIndex();
        m_pServerModel->removeRow(index.row());
    });

    connect(m_pUi->listServer->selectionModel(), &QItemSelectionModel::selectionChanged, [this]()
    {
        m_pMapper->submit();
        if(m_pUi->listServer->selectionModel()->selectedRows().empty())
        {
            setLineEditsEnabled(false);
            m_pMapper->clearMapping();
            m_pMapper->setCurrentModelIndex(QModelIndex());
            m_pUi->labelPort->clear();
            m_pUi->labelUrl->clear();
            m_pUi->serverDescription->clear();
            m_pUi->nickname->clear();
            m_pUi->editAlias->clear();
        }
        else
        {
            setLineEditsEnabled(true);
            m_pMapper->addMapping(m_pUi->labelPort, Server::P_Port);
            m_pMapper->addMapping(m_pUi->labelUrl, Server::P_Url);
            m_pMapper->addMapping(m_pUi->serverDescription, Server::P_Description);
            m_pMapper->addMapping(m_pUi->nickname, Server::P_Nick);
            m_pMapper->addMapping(m_pUi->editAlias, Server::P_Name);
        	const auto idx = m_pUi->listServer->selectionModel()->selectedRows().at(0).row();
            m_pMapper->setCurrentIndex(idx);
        	// update selected row
			m_pServerModel->updateRecord(idx);
        }
    });

    connect(m_pUi->buttonAddServer, &QPushButton::clicked, [this]()
    {
        DialogAddServer *pDialog = new DialogAddServer(this);
        connect(pDialog, &DialogAddServer::selected, m_pServerModel, &ServerModel::appendRecord);
        pDialog->setModal(true);
        pDialog->exec();
        delete pDialog;
    });

    connect(m_pUi->actionOptions, &QAction::triggered, this, &MainWindow::openOptions);
    connect(m_pUi->actionAbout, &QAction::triggered, []()
    {
        DialogInfo *pInfo = new DialogInfo;
        pInfo->exec();
        delete pInfo;
    });

    setLineEditsEnabled(false);
}

MainWindow::~MainWindow()
{
    delete m_pUi;
}

void MainWindow::startProcess()
{
    QModelIndexList index = m_pUi->listServer->selectionModel()->selectedRows();
    if (index.size() != 1)
        return;

    QSettings s;
    s.beginGroup("gothic");
    const QString applicationDir = QCoreApplication::applicationDirPath();
    const QString gothicDir = s.value("working_directory", applicationDir).toString() + "/System/";
    s.endGroup();

    const QFileInfo gothicExePath(gothicDir + s.value("gothic_binary", "Gothic2.exe").toString());
    QFileInfo gmpDllPath(s.value("gmp_dll", "gmp/gmp.dll").toString());
    if (gmpDllPath.isRelative())
        gmpDllPath.setFile(QDir(applicationDir), gmpDllPath.filePath());

#ifdef _WIN32
    if (!gothicExePath.isFile()) {
        showError(
            QStringLiteral("Gothic was not found."),
            QStringLiteral("The Gothic executable \"%1\" does not exist.\n\n"
                           "Check the Gothic path in the options. It must point to your Gothic installation folder, "
                           "which contains the System folder.")
                .arg(QDir::toNativeSeparators(gothicExePath.filePath())),
            QString(), true);
        return;
    }

    if (!gmpDllPath.isFile()) {
        showError(
            QStringLiteral("The GMP client is missing."),
            QStringLiteral("The GMP client \"%1\" does not exist. The launcher installation seems to be incomplete.")
                .arg(QDir::toNativeSeparators(gmpDllPath.filePath())));
        return;
    }
#endif

    const int row = index.front().row();
    QString host = m_pServerModel->data(m_pServerModel->index(row, Server::P_Url), Qt::DisplayRole).toString();
    if (host.contains(':')) // IPv6 address
        host.prepend('[').append(']');
    host += ':' + QString::number(m_pServerModel->data(m_pServerModel->index(row, Server::P_Port), Qt::DisplayRole).toUInt());
    const QString nick = m_pServerModel->data(m_pServerModel->index(row, Server::P_Nick), Qt::DisplayRole).toString();

#ifdef _WIN32
    const QString program = QDir::toNativeSeparators(QDir(applicationDir).filePath(QStringLiteral("gmpinjector.exe")));
#else
    const QString program = QDir(applicationDir).filePath(QStringLiteral("gmpinjector.sh"));
#endif

    QProcess injector;
    injector.setWorkingDirectory(applicationDir);
    injector.setProcessChannelMode(QProcess::MergedChannels); // Capture stdout and stderr
#ifdef _WIN32
    injector.setCreateProcessArgumentsModifier([](QProcess::CreateProcessArguments *args)
    {
        args->flags |= CREATE_NO_WINDOW;
    });
#endif

    const QStringList arguments{
        QStringLiteral("--gothic=%1").arg(gothicExePath.filePath()),
        QStringLiteral("--dll=%1").arg(gmpDllPath.filePath()),
        QStringLiteral("--host=%1").arg(host),
        QStringLiteral("--nickname=%1").arg(nick),
    };
    injector.start(program, arguments);

    if (!injector.waitForStarted()) {
        showError(
            QStringLiteral("The GMP injector could not be started."),
            QStringLiteral("The injector \"%1\" could not be executed. The launcher installation seems to be "
                           "incomplete, or the file is blocked by security software.")
                .arg(QDir::toNativeSeparators(program)),
            injector.errorString());
        return;
    }

    if (!injector.waitForFinished(-1)) {
        injector.kill();
        injector.waitForFinished();
        showError(
            QStringLiteral("The GMP injector stopped responding."),
            QStringLiteral("The injector \"%1\" did not finish and was terminated.")
                .arg(QDir::toNativeSeparators(program)),
            injector.errorString());
        return;
    }

    const QString injectorOutput = QString::fromLocal8Bit(injector.readAllStandardOutput()).trimmed();
    if (injector.exitStatus() != QProcess::NormalExit) {
        showError(
            QStringLiteral("Gothic could not be started."),
            QStringLiteral("The GMP injector stopped unexpectedly."),
            injectorOutput);
        return;
    }

    if (injector.exitCode() != EXIT_SUCCESS) {
        QString details = QStringLiteral("Injector: %1\nExit code: %2")
                .arg(QDir::toNativeSeparators(program))
                .arg(injector.exitCode());
        if (!injectorOutput.isEmpty())
            details += QStringLiteral("\n\n") + injectorOutput;
        showError(
            QStringLiteral("Gothic could not be started."),
            QStringLiteral("The GMP injector reported an error."),
            details);
    }
}

void MainWindow::openOptions()
{
    Options *pOptions = new Options(this);
    pOptions->setModal(true);
    pOptions->exec();
    delete pOptions;
}

void MainWindow::showError(const QString &summary, const QString &informativeText,
                           const QString &details, bool offerOptions)
{
    QMessageBox box(QMessageBox::Critical, QStringLiteral("Could not start Gothic"), summary, QMessageBox::Ok, this);
    box.setInformativeText(informativeText);
    if (!details.isEmpty())
        box.setDetailedText(details);

    QPushButton *pOptionsButton = nullptr;
    if (offerOptions)
        pOptionsButton = box.addButton(QStringLiteral("Open Options"), QMessageBox::ActionRole);

    box.exec();
    if (pOptionsButton != nullptr && box.clickedButton() == pOptionsButton)
        openOptions();
}

void MainWindow::setLineEditsEnabled(bool enabled)
{
    m_pUi->buttonJoin->setEnabled(enabled);
    m_pUi->buttonRemoveServer->setEnabled(enabled);
    m_pUi->labelPort->setEnabled(enabled);
    m_pUi->labelUrl->setEnabled(enabled);
    m_pUi->editAlias->setEnabled(enabled);
}
