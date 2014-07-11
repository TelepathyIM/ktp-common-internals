/***************************************************************************
 *   Copyright (C) 2014 by Marcin Ziemiński <zieminn@gmail.com>            *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2.1 of the License, or   *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA            *
 ***************************************************************************/

#ifndef KTP_PROXY_OTR_PROXY_CHANNEL_ADAPTEE_HEADER
#define KTP_PROXY_OTR_PROXY_CHANNEL_ADAPTEE_HEADER

#include "svc-channel-proxy.h"
#include "otr-proxy-channel.h"

#include <TelepathyQt/ReceivedMessage>

#include <QDBusObjectPath>
#include <QDBusConnection>

class OtrProxyChannel::Adaptee : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QDBusObjectPath wrappedChannel READ wrappedChannel)
    Q_PROPERTY(bool connected READ connected)
    Q_PROPERTY(Tp::MessagePartListList pendingMessages READ pendingMessages)
    Q_PROPERTY(uint trustLevel READ trustLevel)
    Q_PROPERTY(QString localFingerprint READ localFingerprint)
    Q_PROPERTY(QString remoteFingerprint READ remoteFingerprint)

    public:
        Adaptee(OtrProxyChannel *pc, const QDBusConnection &dbusConnection, const Tp::TextChannelPtr &channel);

        QDBusObjectPath wrappedChannel() const;
        bool connected() const;
        Tp::MessagePartListList pendingMessages() const;
        uint trustLevel() const;
        QString localFingerprint() const;
        QString remoteFingerprint() const;

        Tp::TextChannelPtr channel() const;

    private Q_SLOTS:
        void connectProxy(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::ConnectProxyContextPtr &context);
        void disconnectProxy(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr &context);
        void sendMessage(const Tp::MessagePartList &message, uint flags,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr &context);
        void acknowledgePendingMessages(const Tp::UIntList &ids,
                const Tp::Service::ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessagesContextPtr &context);

        void initialize(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::InitializeContextPtr &context);
        void stop(const Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr &context);

        void onMessageReceived(const Tp::ReceivedMessage &receivedMessage);
        void onPendingMessageRemoved(const Tp::ReceivedMessage &receivedMessage);

        void onPendingSendFinished(Tp::PendingOperation *pendingSend);

    Q_SIGNALS:
        void messageSent(const Tp::MessagePartList &content, uint flags, const QString &messageToken);
        void messageReceived(const Tp::MessagePartList &message);
        void pendingMessagesRemoved(const Tp::UIntList &ids);
        void sessionRefreshed();
        void closed();

    private:
        Tp::Service::ChannelProxyInterfaceOTRAdaptor *adaptor;
        OtrProxyChannel *pc;
        Tp::TextChannelPtr chan;
        bool isConnected;

};

#endif