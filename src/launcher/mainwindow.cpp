#include <QPushButton>
#include <QSettings>
#include <QListWidget>
#include <QMessageBox>
#include <QDataWidgetMapper>
#include <QAction>
#include <QFileInfo>

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

    connect(m_pUi->actionOptions, &QAction::triggered, []()
    {
        Options *pOptions = new Options;
        pOptions->setModal(true);
        pOptions->exec();
        delete pOptions;
    });
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
    const QString gothicDir = s.value("working_directory", QCoreApplication::applicationDirPath()).toString() + "/System/";
    s.endGroup();

    const QFileInfo gothicExePath(gothicDir + s.value("gothic_binary", "Gothic2.exe").toString());
    const QFileInfo gmpDllPath(s.value("gmp_dll", "gmp/gmp.dll").toString());

    const int row = index.front().row();
    const QString host = m_pServerModel->data(m_pServerModel->index(row, Server::P_Url), Qt::DisplayRole).toString()
            + '|'
            + QString::number(m_pServerModel->data(m_pServerModel->index(row, Server::P_Port), Qt::DisplayRole).toUInt());
    const QString nick = m_pServerModel->data(m_pServerModel->index(row, Server::P_Nick), Qt::DisplayRole).toString();

#ifdef _WIN32
    const QString program = QStringLiteral("gmpinjector.exe");
#else
    const QString program = QStringLiteral("./gmpinjector.sh");
#endif

    QString command = QStringLiteral("\"%1\" \"--gothic=%2\" \"--gmp=%3\" \"--host=%4\" \"--nickname=%5\"")
            .arg(program, gothicExePath.filePath(), gmpDllPath.filePath(), host, nick);

    int result;
    QString error;
#ifdef _WIN32
    PROCESS_INFORMATION pi{};
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    if (CreateProcessW(program.toStdWString().c_str(), command.toStdWString().data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_TIMEOUT) {
            error = QStringLiteral("WaitForSingleObject time out");
            TerminateProcess(pi.hProcess, EXIT_FAILURE);
            result = EXIT_FAILURE;
        } else {
            DWORD ec;
            GetExitCodeProcess(pi.hProcess, &ec);
            result = static_cast<int>(ec);
            error = QStringLiteral("Unkown error"); // TODO: Get stdout from process
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        result = EXIT_FAILURE;
        error = QStringLiteral("Couldn't create Process.\nGetLastError: %1").arg(GetLastError());
    }
#else
    command += " 2>&1"; // Redirect stderr to stdout
    FILE* pipe = popen(command.toStdString().c_str(), "r");
    if (pipe) {
        char buffer[128];
        while (std::fgets(buffer, sizeof(buffer), pipe) != nullptr) {
            error += buffer;
        }
        result = pclose(pipe);
    } else {
        error = QStringLiteral("Couldn't execute command: \"%1\".\nerrno: %2").arg(command, std::strerror(errno));
        result = EXIT_FAILURE;
    }
#endif

    if (result != EXIT_SUCCESS) {
        QMessageBox::critical(this, "Error", error);
    }
}

void MainWindow::setLineEditsEnabled(bool enabled)
{
    m_pUi->buttonJoin->setEnabled(enabled);
    m_pUi->buttonRemoveServer->setEnabled(enabled);
    m_pUi->labelPort->setEnabled(enabled);
    m_pUi->labelUrl->setEnabled(enabled);
    m_pUi->editAlias->setEnabled(enabled);
}
