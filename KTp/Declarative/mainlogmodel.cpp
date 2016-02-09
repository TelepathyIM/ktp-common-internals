/*
    Copyright (C) 2016  Martin Klapetek <mklapetek@kde.org>

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

#include "mainlogmodel.h"

#include <QSqlQueryModel>
#include <QSqlRecord>
#include <QSqlDatabase>
#include <QStandardPaths>
// #include <QDBusArgument>

#include <TelepathyQt/AccountManager>
#include <TelepathyQt/PendingOperation>
#include <TelepathyQt/PendingChannel>
#include <TelepathyQt/TextChannel>
#include <TelepathyQt/Channel>

#include "conversation.h"

#include <QDebug>

MainLogModel::MainLogModel(QObject *parent)
    : QIdentityProxyModel(parent),
      m_dbModel(new QSqlQueryModel(this))
{
    const QString dbLocation = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + QStringLiteral("/ktp-mobile-logger/");

    QSqlDatabase db = QSqlDatabase::addDatabase(QStringLiteral("QSQLITE"));
    db.setDatabaseName(dbLocation + QStringLiteral("history.db3"));
    qDebug() << dbLocation << db.open();

    m_dbModel->setQuery(QStringLiteral("SELECT data.messageDateTime, data.message, "
                                       "accountData.accountObjectPath, contactData.targetContact "
                                       "FROM data LEFT JOIN contactData ON data.targetContactId = contactData.id "
                                       "LEFT JOIN accountData ON data.accountId = accountData.id "
                                       "GROUP BY data.targetContactId ORDER BY data.messageDateTime DESC"));

    setSourceModel(m_dbModel);
}

MainLogModel::~MainLogModel()
{

}

QVariant MainLogModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) {
        return QVariant();
    }

//     qDebug() << m_dbModel->record(index.row()) << m_dbModel->record(index.row()).value("targetContact");
//     qDebug() << index << index.row() << index.column();

    const int row = index.row();

    switch (role) {
        case MainLogModel::ContactDisplayNameRole:
        case MainLogModel::ContactIdRole:
            return m_dbModel->record(row).value(QStringLiteral("targetContact"));
        case MainLogModel::AccountIdRole:
            return m_dbModel->record(row).value(QStringLiteral("accountObjectPath")).toString().mid(35);
        case MainLogModel::LastMessageDateRole:
            return m_dbModel->record(row).value(QStringLiteral("messageDateTime"));
        case MainLogModel::LastMessageTextRole:
            return m_dbModel->record(row).value(QStringLiteral("message"));
        case MainLogModel::ConversationRole:
            const QString hashKey = m_dbModel->record(row).value(QStringLiteral("targetContact")).toString()
                                  + m_dbModel->record(row).value(QStringLiteral("accountObjectPath")).toString().mid(35);

            // this can return null/invalid Conversation
            return QVariant::fromValue(m_conversations.value(hashKey));
    }

    return QVariant();
}

QHash<int, QByteArray> MainLogModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();

    roles.insert(ContactDisplayNameRole, "contactDisplayName");
    roles.insert(ContactIdRole, "contactId");
    roles.insert(AccountIdRole, "accountId");
    roles.insert(LastMessageDateRole, "lastMessageDate");
    roles.insert(LastMessageTextRole, "lastMessageText");
    roles.insert(ConversationRole, "conversation");

    return roles;
}

void MainLogModel::startChat(const QString &accountId, const QString &contactId)
{
    const QString objectPath = TP_QT_ACCOUNT_OBJECT_PATH_BASE + QLatin1Char('/') + accountId;
    const Tp::AccountPtr account = m_accountManager->accountForObjectPath(objectPath);

    if (account.isNull()) {
        qWarning() << "Cannot get account for" << accountId;
    }

    Tp::PendingChannel *pendingChannel = account->ensureAndHandleTextChat(contactId);
    connect(pendingChannel, &Tp::PendingChannel::finished, [=](Tp::PendingOperation *op) {
        if (op->isError()) {
            qWarning() << "Requesting text channel failed:" << op->errorName() << op->errorMessage();
            return;
        }

        Tp::PendingChannel *pc = qobject_cast<Tp::PendingChannel*>(op);
        if (pc) {
            Tp::TextChannel *channel = qobject_cast<Tp::TextChannel*>(pc->channel().data());
            if (channel) {
                const QHash<QString, Conversation*>::const_iterator i = m_conversations.find(accountId + contactId);
                if (i == m_conversations.end()) {
                    Conversation *conversation = new Conversation(Tp::TextChannelPtr(channel), account, this);
                    m_conversations.insert(accountId + contactId, conversation);
                } else {
                    (*i)->setTextChannel(Tp::TextChannelPtr(channel));
                }
            }
        }
    });
}

void MainLogModel::setAccountManager(const Tp::AccountManagerPtr &accountManager)
{
    m_accountManager = accountManager;
}