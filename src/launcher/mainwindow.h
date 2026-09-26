#pragma once

#include <QMainWindow>

class QDataWidgetMapper;
class QProcess;
class ServerModel;
class Ticker;

namespace Ui
{
    class MainWindow;
}

class MainWindow : public QMainWindow
{
public:
    MainWindow();
    virtual ~MainWindow() override;

public slots:
    void startProcess();

private:
    void setLineEditsEnabled(bool enabled);
    void openOptions();
    void showError(const QString &summary, const QString &informativeText,
                   const QString &details = QString(), bool offerOptions = false);

    Ui::MainWindow *m_pUi;
    ServerModel *m_pServerModel;
    QDataWidgetMapper *m_pMapper;
};
