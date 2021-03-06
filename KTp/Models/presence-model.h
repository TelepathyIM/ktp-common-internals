/*
 * Presence Model - A model of settable presences.
 *
 * Copyright (C) 2016 James D. Smith <smithjd15@gmail.com>
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
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

#ifndef PRESENCEMODEL_H
#define PRESENCEMODEL_H

#include <QAbstractListModel>
#include <QVariant>

#include <KConfigGroup>

#include <KTp/presence.h>
#include "ktpmodels_export.h"

namespace KTp
{

class KTPMODELS_EXPORT PresenceModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int count READ rowCount)

public:
    explicit PresenceModel(QObject *parent = nullptr);
    ~PresenceModel() override;

    enum Roles {
        //Also supplies Qt::DisplayRole and Qt::DecorationRole
        PresenceRole = Qt::UserRole,
        IconNameRole
    };

    /** Adds a custom presence to the model, writes it to the config file, and
      * propagates it to other models.
      * @return the newly added item
      */
    QModelIndex addPresence(const KTp::Presence &presence);

    void removePresence(const KTp::Presence &presence);

    /** Load all presences from disk */
    void loadPresences();

    /** Write all presences to disk */
    void syncCustomPresencesToDisk();

    Q_SCRIPTABLE QVariant get(int row, const QByteArray& role) const;

    //protected:
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1, Qt::MatchFlags flags =  Qt::MatchFlags(Qt::MatchStartsWith | Qt::MatchWrap)) const override;
    QHash<int, QByteArray> roleNames() const override;

public Q_SLOTS:
    /** Incoming changes from other models */
    void propagationChange(const QVariantList modelChange);

private:
    void modifyModel(const KTp::Presence &presence);
    void propagateChange(const KTp::Presence &presence);

    /** Loads standard presences (online, away etc) into the model */
    void loadDefaultPresences();

    /** Loads any user custom presences into the model */
    void loadCustomPresences();

    QList<KTp::Presence> m_presences;

    //this is wrong, KConfigGroup is a sharedptr..
    KConfigGroup m_presenceGroup;
};

}

#endif // PRESENCEMODEL_H
