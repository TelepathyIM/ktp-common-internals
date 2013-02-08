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


#ifndef MESSAGE_PROCESSOR_H
#define MESSAGE_PROCESSOR_H

#include <QObject>
#include <QList>
#include <KPluginInfo>

#include <KTp/message.h>
#include <KTp/ktp-export.h>
#include <KTp/abstract-message-filter.h>
#include <TelepathyLoggerQt4/Types>

namespace Tp
{
class ReceivedMessage;
class Message;
}


namespace KTp
{

class AbstractMessageFilter;

//each thing that displays message will have an instance of this
class KTP_EXPORT MessageProcessor : public QObject
{
    Q_OBJECT

  public:
    static MessageProcessor* instance();
    ~MessageProcessor();

    //text-ui will call this somewhere when creating the template
    QString header();

    //text-ui will call this somewhere in handleIncommingMessage just before displaying it
    KTp::Message processMessage(const Tp::Message &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);
    KTp::Message processMessage(const Tp::ReceivedMessage &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);
    KTp::Message processMessage(const Tpl::TextEventPtr &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel);

  protected:
    KTp::Message processMessage(KTp::Message message, const KTp::MessageContext &context);
    explicit MessageProcessor();

  private:
    class Private;
    Private * const d;

};

}

#endif // MESSAGE_PROCESSOR_H