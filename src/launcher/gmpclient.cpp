#include <QStringList>
#include <QDebug>
#include <QtConcurrent/QtConcurrent>

#include <slikenet/MessageIdentifiers.h>
#include <slikenet/BitStream.h>

#include "gmpclient.h"

GMPClient::GMPClient(QObject *pParent) :
    QObject(pParent),
    m_pClient(SLNet::RakPeerInterface::GetInstance())
{
    m_Timer.setInterval(100);
    connect(&m_Timer, &QTimer::timeout, this, &GMPClient::update);
    connect(this, &GMPClient::startTimer, this, [this] {
        m_Timer.start();
    });

    SLNet::SocketDescriptor sd;
    m_pClient->Startup(1, &sd, 1);
}

GMPClient::~GMPClient()
{
    m_Timer.stop();
    SLNet::RakPeerInterface::DestroyInstance(m_pClient);
}

void GMPClient::start(const QString &address, quint16 port)
{
    if (m_Timer.isActive())
        return;

    if (m_Timer.isActive())
        m_Timer.stop();

    if (port == 0) {
        ServerInfo info;
        info.serverName = "Port 0 not allowed";
        emit serverChecked(info);
        return;
    }

    // Avoid blocking GUI thread. Important when SLNet does domain resolving.
    (void)QtConcurrent::run(QThreadPool::globalInstance(), [this, address, port]{
        const char password[] = "b5r6kQ6gp0GcpK4x";

        SLNet::ConnectionAttemptResult result = m_pClient->Connect(address.toStdString().c_str(), port, password, sizeof(password) - 1);

        if (result != SLNet::ConnectionAttemptResult::CONNECTION_ATTEMPT_STARTED) {
            ServerInfo info;
            switch(result){
                case SLNet::INVALID_PARAMETER:
                    info.serverName = "Invalid Parameter";
                    break;
                case SLNet::CANNOT_RESOLVE_DOMAIN_NAME:
                    info.serverName = "Can't resolve domain";
                    break;
                case SLNet::ALREADY_CONNECTED_TO_ENDPOINT:
                    info.serverName = "Already connected";
                    break;
                case SLNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
                    info.serverName = "Already connecting";
                    break;
                case SLNet::SECURITY_INITIALIZATION_FAILED:
                    info.serverName = "Security init failed";
                    break;
            }
            emit serverChecked(info);
            return;
        }

        emit startTimer();
    });
}

void GMPClient::update()
{
	for (auto* pPacket = m_pClient->Receive(); pPacket; m_pClient->DeallocatePacket(pPacket), pPacket = m_pClient->Receive())
	{
		switch (static_cast<uint8_t>(pPacket->data[0]))
		{
			case static_cast<uint8_t>(DefaultMessageIDTypes::ID_CONNECTION_REQUEST_ACCEPTED):
			{
				SLNet::BitStream stream;
				stream.Write(MessageIdentifiers::GET_SERVER_INFO);

				m_pClient->Send(&stream, HIGH_PRIORITY, RELIABLE, 0, pPacket->guid, false);
				break;
			}
			case static_cast<uint8_t>(MessageIdentifiers::GET_SERVER_INFO):
			{
				ServerInfo info; // write data from server into info-object
				size_t seek = 2;
				if (!info.deserialize(pPacket->data, pPacket->length, seek))
				{
					qWarning() << "invalid packet";
					m_pClient->DeallocatePacket(pPacket);
					return;
				}

				info.averagePing = m_pClient->GetLastPing(pPacket->systemAddress);

				emit serverChecked(info); // give info-object to checking server-object
				break;
			}
			case static_cast<uint8_t>(DefaultMessageIDTypes::ID_CONNECTION_ATTEMPT_FAILED):
			{
				ServerInfo info;
				info.serverName = "N/A";
				emit serverChecked(info); // give info-object to checking server-object
			}
			[[fallthrough]];
			case static_cast<uint8_t>(DefaultMessageIDTypes::ID_DISCONNECTION_NOTIFICATION):
			case static_cast<uint8_t>(DefaultMessageIDTypes::ID_REMOTE_DISCONNECTION_NOTIFICATION):
			{
				m_Timer.stop();
				break;
			}
		}
	}
}

bool readString(const uint8_t *pData, size_t maxlen, size_t &seek, QString &string)
{
    if (sizeof(uint8_t) + seek > maxlen)
        return false;

    uint8_t len = pData[seek];
    ++seek;
    if (len > maxlen - seek)
        return false;

    string = QString::fromUtf8(reinterpret_cast<const char *>(pData + seek), len);
    seek += len + 1;
    return true;
}

bool ServerInfo::deserialize(const uint8_t *pData, size_t maxlen, size_t &seek)
{
    if (!readString(pData, maxlen, seek, serverName))
        return false;

    if (!readString(pData, maxlen, seek, gamemode))
        return false;

    if (!readString(pData, maxlen, seek, version))
        return false;

    if (!readString(pData, maxlen, seek, player))
        return false;

    if (!readString(pData, maxlen, seek, bots))
        return false;

    if (!readString(pData, maxlen, seek, description))
        return false;

    return true;
}
