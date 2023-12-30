#pragma once

#include <QDialog>

namespace Ui {
    class Options;
}

class Options : public QDialog
{
    Q_OBJECT

public:
    explicit Options(QWidget *pParent = nullptr);
    virtual ~Options() override;

public slots:
    void save();

private:
    Ui::Options *m_pUi;
};
