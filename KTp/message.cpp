/*
    Copyright (C) 2012  Lasath Fernando <kde@lasath.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "message.h"
#include "message-private.h"

#include "ktp-debug.h"
#include <QSharedData>

#include <TelepathyQt/ContactManager>
#include <TelepathyQt/Connection>

using namespace KTp;

Message& Message::operator=(const Message &other) {
    d = other.d;
    return *this;
}

Message::Message(Message::Private *dd):
    d(dd)
{
}

Message::Message(const Tp::Message &original, const KTp::MessageContext &context) :
    d(new Private)
{
    d->sentTime = original.sent();
    d->token = original.messageToken();
    d->messageType = original.messageType();
    d->isHistory = false;
    d->direction = KTp::Message::LocalToRemote;

    setMainMessagePart(original.text());

    if (context.account()->connection()) {
        d->sender = KTp::ContactPtr::qObjectCast(context.account()->connection()->selfContact());
    } else {
        d->senderAlias = context.account()->nickname();
        d->senderId = context.account()->uniqueIdentifier();
    }
}

Message::Message(const Tp::ReceivedMessage &original, const KTp::MessageContext &context) :
    d(new Private)
{
    Q_UNUSED(context)

    d->sentTime = original.sent();
    if (d->sentTime.isNull()) {
        d->sentTime = original.received();
    }

    d->token = original.messageToken();
    d->messageType = original.messageType();
    d->isHistory = original.isScrollback();
    d->replyToMessageToken = original.header().value(QLatin1String("reply-to-message-token")).variant().toString();

    QSet<QString> altGroupsUsed;
    QString text;


//    for (int i = 1; i < original.size(); i++) {
//        const Tp::MessagePart part = original.part(i);
//        const QString interface = part.value("interface").variant().toString();
//        const QString alternative = part.value("alternative").variant().toString();
//        if (!alternative.isEmpty() && altGroupsUsed.contains(alternative)) {
//            continue;
//        }
//        if (!interface.isEmpty()) {
//            continue;
//        }
//        const QString contentType = part.value("content-type").variant().toString();
//        if (contentType == QLatin1String("text/plain")) {
//            const QString content = part.value("content").variant().toString();
//            text += content;
//            if (!alternative.isEmpty()) {
//                altGroupsUsed << alternative;
//                continue;
//            }
//        }
//    }

    for (int i = 1; i < original.size(); i++) {
        const Tp::MessagePart part = original.part(i);
        const QString interface = part.value(QLatin1String("interface")).variant().toString();
        const QString alternative = part.value(QLatin1String("alternative")).variant().toString();

        if (!alternative.isEmpty() && altGroupsUsed.contains(alternative)) {
            continue;
        }

        if (interface == TP_QT_IFACE_CHANNEL + QLatin1String(".Interface.WebPage")) {
            d->webPages.append(WebPage());
            WebPage &page = d->webPages.last();
            page.url = part.value(QLatin1String("url")).variant().toString();
            page.displayUrl = part.value(QLatin1String("displayUrl")).variant().toString();
            page.siteName = part.value(QLatin1String("siteName")).variant().toString();
            page.title = part.value(QLatin1String("title")).variant().toString();
            page.description = part.value(QLatin1String("description")).variant().toString();

            if (!alternative.isEmpty()) {
                altGroupsUsed << alternative;
                continue;
            }
        }

        if (!interface.isEmpty()) {
            continue;
        }

        const QString contentType = part.value(QLatin1String("content-type")).variant().toString();
        if (contentType == QLatin1String("text/plain")) {
            const QString content = part.value(QLatin1String("content")).variant().toString();
            text += content;

            if (!alternative.isEmpty()) {
                altGroupsUsed << alternative;
                continue;
            }
        }
    }

    setMainMessagePart(text);

    if (!original.sender().isNull()) {
        d->sender = KTp::ContactPtr::qObjectCast(original.sender());
    } else {
        d->senderAlias = original.senderNickname();
    }

    bool isLocalToRemote = false;

    if (!d->sender.isNull()) {
        if (context.channel()->interfaces().contains(TP_QT_IFACE_CHANNEL_INTERFACE_GROUP)) {
            isLocalToRemote = d->sender->id() == context.channel()->groupSelfContact()->id();
        } else {
            isLocalToRemote = d->sender->id() == context.channel()->connection()->selfContact()->id();
        }
    }

    if (isLocalToRemote) {
        d->direction = KTp::Message::LocalToRemote;
    } else {
        d->direction = KTp::Message::RemoteToLocal;
    }

#if 0
    const Tp::MessagePart forwardedHeader = original.forwardedHeader();
#else
    const Tp::MessagePart forwardedHeader = [&original]() -> Tp::MessagePart {
        for (int i = 1; i < original.size(); ++i) {
            const Tp::MessagePart part = original.part(i);
            if (part.value(QLatin1String("interface")).variant().toString()
                    == TP_QT_IFACE_CHANNEL + QLatin1String(".Interface.Forwarding")) {
                return part;
            }
        }
        return {};
    }();
#endif
    for (int i = 1; i < original.size(); ++i) {
        const Tp::MessagePart part = original.part(i);
        if (part.value(QLatin1String("thumbnail")).variant().toBool()) {
            Message::Thumbnail t;
            t.content = part.value(QLatin1String("content")).variant().toByteArray();
            t.mimeType = part.value(QLatin1String("content-type")).variant().toByteArray();
            d->thumbnails.append(t);
        }
    }

    if (!forwardedHeader.isEmpty()) {
        d->forwardedMessageToken = forwardedHeader.value(QLatin1String("message-token")).variant().toString();
        d->forwardedSenderId = forwardedHeader.value(QLatin1String("message-sender-id")).variant().toString();
        d->forwardedSenderAlias = forwardedHeader.value(QLatin1String("message-sender-alias")).variant().toString();
    }
}

Message::Message(const Message& other):
    d(other.d)
{
}

Message::~Message()
{
}

QString Message::mainMessagePart() const
{
    return d->mainPart;
}

void Message::setMainMessagePart(const QString& message)
{
    d->mainPart = message;
}

void Message::appendMessagePart(const QString& part)
{
    d->parts << part;
}

void Message::appendScript(const QString& script)
{
    // Append the script only if it is not already appended to avoid multiple
    // execution of the scripts.
    if (!d->scripts.contains(script)) {
        d->scripts << script;
    }
}

QString Message::finalizedMessage() const
{
    QString msg = d->mainPart + QLatin1Char('\n') +
        d->parts.join(QLatin1String("\n"));

//     qCDebug(KTP_COMMONINTERNALS) << msg;
    return msg;
}

QString Message::finalizedScript() const
{
    if (d->scripts.empty()) {
        return QString();
    }

    QString finalScript = d->scripts.join(QString());

    if (!finalScript.isEmpty()) {
        finalScript.append(QLatin1String("false;"));
    }

//    qCDebug(KTP_COMMONINTERNALS) << finalScript;
    return finalScript;
}

QVariant Message::property(const char *name) const
{
    return d->properties[QLatin1String(name)];
}

void Message::setProperty(const char *name, const QVariant& value)
{
    d->properties[QLatin1String(name)] = value;
}

QDateTime Message::time() const
{
    return d->sentTime;
}

QString Message::token() const
{
    return d->token;
}

Tp::ChannelTextMessageType Message::type() const
{
    return d->messageType;
}

bool Message::isReply() const
{
    return !d->replyToMessageToken.isEmpty();
}

QString Message::replyToMessageToken() const
{
    return d->replyToMessageToken;
}

QString Message::senderAlias() const
{
    if (d->sender) {
        return d->sender->alias();
    }
    return d->senderAlias;
}

QString Message::senderId() const
{
    if (d->sender) {
        return d->sender->id();
    }
    return d->senderId;
}

KTp::ContactPtr Message::sender() const
{
    return d->sender;
}

int Message::partsSize() const
{
    return d->parts.size();
}

bool Message::isHistory() const
{
    return d->isHistory;
}

KTp::Message::MessageDirection Message::direction() const
{
    return d->direction;
}

bool Message::isForwarded() const
{
    return !d->forwardedSenderId.isEmpty();
}

QString Message::forwardedMessageToken() const
{
    return d->forwardedMessageToken;
}

QString Message::forwardedSenderAlias() const
{
    return d->forwardedSenderAlias;
}

QString Message::forwardedSenderId() const
{
    return d->forwardedSenderId;
}

ContactPtr Message::forwardedSender() const
{
    return ContactPtr();
}

QVector<Message::Thumbnail> Message::thumbnails() const
{
    return d->thumbnails;
}

QVector<Message::WebPage> Message::webPages() const
{
    return d->webPages;
}
