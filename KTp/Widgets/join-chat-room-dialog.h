/*
 * This file is part of telepathy-contactslist-prototype
 *
 * Copyright (C) 2011 Francesco Nwokeka <francesco.nwokeka@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef JOINCHATROOMDIALOG_H
#define JOINCHATROOMDIALOG_H

#include <TelepathyQt/AccountManager>
#include <QDialog>
#include <KTp/Models/rooms-model.h>

#include <KTp/ktpcommoninternals_export.h>

namespace Ui {
class JoinChatRoomDialog;
}

class RoomsModel;
class FavoriteRoomsModel;
class QSortFilterProxyModel;
class QAbstractButton;

namespace KTp {

class KTPCOMMONINTERNALS_EXPORT JoinChatRoomDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JoinChatRoomDialog(Tp::AccountManagerPtr accountManager, QWidget *parent = nullptr);
    ~JoinChatRoomDialog() override;

    Tp::AccountPtr selectedAccount() const;     /** returns selected account */
    QString selectedChatRoom() const;           /** returns selected chat room */
    void accept() override;

protected:
    void closeEvent(QCloseEvent *e) override;

private Q_SLOTS:
    void onTextChanged(QString newText);
    void onAccountSelectionChanged(int newIndex);
    void addRecentRoom();
    void clearRecentRooms();
    void getRoomList();
    void stopListing();
    void onRoomListChannelReadyForHandling(Tp::PendingOperation *operation);
    void onRoomListChannelReady(Tp::PendingOperation *operation);
    void onRoomListChannelClosed(Tp::PendingOperation *operation);
    void onListing(bool isListing);
    void onGotRooms(Tp::RoomInfoList roomInfoList);
    void onFavoriteRoomSelectionChanged(const QModelIndex &current, const QModelIndex &previous);
    void onFavoriteRoomDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight);
    void onRoomClicked(const QModelIndex &index);
    void onAccountManagerReady(Tp::PendingOperation*);
    void onStartChatFinished(Tp::PendingOperation *op);

private:
    void sendNotificationToUser(const QString& errorMsg);
    void loadFavoriteRooms();
    void setJoinInProgress(bool);

    struct Private;
    Private * const d;
};

} //namespace KTp


#endif  // JOINCHATROOMDIALOG_H
