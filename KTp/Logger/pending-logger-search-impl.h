/*
 * Copyright (C) 2013  Daniel Vrátil <dvratil@redhat.com>
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
 *
 */

#ifndef PENDINGLOGGERSEARCHIMPL_H
#define PENDINGLOGGERSEARCHIMPL_H

#include <KTp/Logger/pending-logger-search.h>

namespace Tpl {
class PendingOperation;
}

class PendingLoggerSearchImpl : public KTp::PendingLoggerSearch
{
    Q_OBJECT

  public:
    explicit PendingLoggerSearchImpl(const QString& term, QObject* parent = nullptr);
    ~PendingLoggerSearchImpl() override;


  public Q_SLOTS:
    void operationFinished(KTp::PendingLoggerOperation *operation);

  private:
    QList<KTp::PendingLoggerOperation*> mRunningOps;
};

#endif // PENDINGLOGGERSEARCHIMPL_H
