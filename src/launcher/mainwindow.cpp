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
#if WIN32
#include "Windows.h"
#endif

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

    const QString gothicBin = s.value("gothic_binary", "Gothic2.exe").toString();
    QFileInfo gothicExePath(gothicDir + gothicBin);
    if (!gothicExePath.isExecutable())
    {
        QMessageBox::critical(this, "Error",
                              "Couldn't find Gothic EXE in path:\n" + gothicExePath.filePath() + "\n\nMake sure the path is correct.");
        return;
    }

    QFileInfo gmpDllPath("gmp/gmp.dll");
    if (!gmpDllPath.isFile()) {
        QMessageBox::critical(this, "Error", "Couldn't find GMP DLL in path:\n" + gmpDllPath.filePath() +
                                             "\n\nMake sure the file exists and is not blocked by an anti virus.");
        return;
    }

    const int row = index.front().row();
    QString host = m_pServerModel->data(m_pServerModel->index(row, Server::P_Url), Qt::DisplayRole).toString()
            + ':'
            + QString::number(m_pServerModel->data(m_pServerModel->index(row, Server::P_Port), Qt::DisplayRole).toUInt());
    const QString nick = m_pServerModel->data(m_pServerModel->index(row, Server::P_Nick), Qt::DisplayRole).toString();

    int result;
#ifdef WIN32
    const std::wstring program(L"gmpinjector.exe");
    std::wstring command =
            program + L" \"--gothic=" + gothicExePath.filePath().toStdWString() + L"\" \"--gmp=" + gmpDllPath.absoluteFilePath().toStdWString() +
                L"\" \"--host=" + host.toStdWString() + L"\" \"--nickname=" + nick.toStdWString() + L"\"";

    PROCESS_INFORMATION pi{};
    STARTUPINFOW si{};
    si.cb = sizeof(si);
    if (CreateProcessW(program.c_str(), command.data(), nullptr, nullptr, FALSE, CREATE_NO_WINDOW, nullptr, nullptr, &si, &pi)) {
        if (WaitForSingleObject(pi.hProcess, INFINITE) == WAIT_TIMEOUT) {
            TerminateProcess(pi.hProcess, EXIT_FAILURE);
            result = EXIT_FAILURE;
        } else {
            DWORD ec;
            GetExitCodeProcess(pi.hProcess, &ec);
            result = static_cast<int>(ec);
        }
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    } else {
        result = EXIT_FAILURE;
    }
#else
    const std::string command =
            "./gmpinjector.sh \"--gothic=" + gothicExePath.filePath().toStdString() + "\" \"--gmp=" + gmpDllPath.filePath().toStdString() +
            "\" \"--host=" + host.toStdString() + "\" \"--nickname=" + nick.toStdString() + "\"";
    result = system(command.c_str());
#endif

    if (result != EXIT_SUCCESS) {
        QMessageBox::critical(this, "Error", "Couldn't start GMP.");
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
