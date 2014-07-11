#include "svc-channel-proxy.h"

#include <TelepathyQt/Constants>
#include <TelepathyQt/MethodInvocationContext>

namespace Tp
{
namespace Service
{

ChannelProxyInterfaceOTRAdaptor::ChannelProxyInterfaceOTRAdaptor(const QDBusConnection& bus, QObject* adaptee, QObject* parent)
    : Tp::AbstractAdaptor(bus, adaptee, parent)
{
    connect(adaptee, SIGNAL(messageSent(const Tp::MessagePartList&, uint, const QString&)), SIGNAL(MessageSent(const Tp::MessagePartList&, uint, const QString&)));
    connect(adaptee, SIGNAL(messageReceived(const Tp::MessagePartList&)), SIGNAL(MessageReceived(const Tp::MessagePartList&)));
    connect(adaptee, SIGNAL(pendingMessagesRemoved(const Tp::UIntList&)), SIGNAL(PendingMessagesRemoved(const Tp::UIntList&)));
    connect(adaptee, SIGNAL(sessionRefreshed()), SIGNAL(SessionRefreshed()));
}

ChannelProxyInterfaceOTRAdaptor::~ChannelProxyInterfaceOTRAdaptor()
{
}

QDBusObjectPath ChannelProxyInterfaceOTRAdaptor::WrappedChannel() const
{
    return qvariant_cast< QDBusObjectPath >(adaptee()->property("wrappedChannel"));
}

bool ChannelProxyInterfaceOTRAdaptor::Connected() const
{
    return qvariant_cast< bool >(adaptee()->property("connected"));
}

Tp::MessagePartListList ChannelProxyInterfaceOTRAdaptor::PendingMessages() const
{
    return qvariant_cast< Tp::MessagePartListList >(adaptee()->property("pendingMessages"));
}

uint ChannelProxyInterfaceOTRAdaptor::TrustLevel() const
{
    return qvariant_cast< uint >(adaptee()->property("trustLevel"));
}

QString ChannelProxyInterfaceOTRAdaptor::LocalFingerprint() const
{
    return qvariant_cast< QString >(adaptee()->property("localFingerprint"));
}

QString ChannelProxyInterfaceOTRAdaptor::RemoteFingerprint() const
{
    return qvariant_cast< QString >(adaptee()->property("remoteFingerprint"));
}

void ChannelProxyInterfaceOTRAdaptor::ConnectProxy(const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("connectProxy(Tp::Service::ChannelProxyInterfaceOTRAdaptor::ConnectProxyContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    ConnectProxyContextPtr ctx = ConnectProxyContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "connectProxy",
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::ConnectProxyContextPtr, ctx));
}

void ChannelProxyInterfaceOTRAdaptor::DisconnectProxy(const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("disconnectProxy(Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    DisconnectProxyContextPtr ctx = DisconnectProxyContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "disconnectProxy",
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::DisconnectProxyContextPtr, ctx));
}

QString ChannelProxyInterfaceOTRAdaptor::SendMessage(const Tp::MessagePartList& message, uint flags, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("sendMessage(Tp::MessagePartList,uint,Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return QString();
    }

    SendMessageContextPtr ctx = SendMessageContextPtr(
            new Tp::MethodInvocationContext< QString >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "sendMessage",
        Q_ARG(Tp::MessagePartList, message), Q_ARG(uint, flags),
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::SendMessageContextPtr, ctx));
    return QString();
}

void ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessages(const Tp::UIntList& IDs, const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("acknowledgePendingMessages(Tp::UIntList,Tp::Service::ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessagesContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    AcknowledgePendingMessagesContextPtr ctx = AcknowledgePendingMessagesContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "acknowledgePendingMessages",
        Q_ARG(Tp::UIntList, IDs),
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::AcknowledgePendingMessagesContextPtr, ctx));
}

void ChannelProxyInterfaceOTRAdaptor::Initialize(const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("initialize(Tp::Service::ChannelProxyInterfaceOTRAdaptor::InitializeContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    InitializeContextPtr ctx = InitializeContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "initialize",
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::InitializeContextPtr, ctx));
}

void ChannelProxyInterfaceOTRAdaptor::Stop(const QDBusMessage& dbusMessage)
{
    if (!adaptee()->metaObject()->indexOfMethod("stop(Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr)") == -1) {
        dbusConnection().send(dbusMessage.createErrorReply(TP_QT_ERROR_NOT_IMPLEMENTED, QLatin1String("Not implemented")));
        return;
    }

    StopContextPtr ctx = StopContextPtr(
            new Tp::MethodInvocationContext<  >(dbusConnection(), dbusMessage));
    QMetaObject::invokeMethod(adaptee(), "stop",
        Q_ARG(Tp::Service::ChannelProxyInterfaceOTRAdaptor::StopContextPtr, ctx));
}

}
}