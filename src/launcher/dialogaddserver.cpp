#include <QPushButton>

#include "dialogaddserver.h"
#include "ui_dialogaddserver.h"

const QString invalidStyle = QStringLiteral("background-color: #B22222; color: white;");

DialogAddServer::DialogAddServer(QWidget *parent) :
    QDialog(parent),
    m_pUi(new Ui::DialogAddServer) {

    m_pUi->setupUi(this);

	connect(m_pUi->editUrl, &QLineEdit::textChanged, [this]() {
        QLineEdit* addressLine = this->m_pUi->editUrl;
        const QUrl url = QUrl::fromUserInput(addressLine->text());
        // Only allow URLs with <hostname>:<port> format.
        if (!url.isValid() || url.hasFragment() || url.hasQuery() || url.isLocalFile() || url.port() == -1 || url.port() == 0) {
            this->m_pUi->buttonAddServer->setEnabled(false);
            addressLine->setStyleSheet(invalidStyle);
        } else {
            this->m_pUi->buttonAddServer->setEnabled(true);
            addressLine->setStyleSheet(nullptr);
        }
	});
    connect(m_pUi->buttonCancel, &QPushButton::clicked, [this]() {
        close();
    });
	connect(m_pUi->buttonAddServer, &QPushButton::clicked, [this]() {
		QLineEdit* serverNameLine = this->m_pUi->editServerName;
		QLineEdit* addressLine = this->m_pUi->editUrl;
        const QUrl url = QUrl::fromUserInput(addressLine->text());

        emit selected(serverNameLine->text(), url.host(), url.port());

		close();
	});
}

DialogAddServer::~DialogAddServer() {

    delete m_pUi;

}
