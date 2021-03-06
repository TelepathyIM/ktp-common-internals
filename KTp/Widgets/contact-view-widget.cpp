/*
 * Copyright (C) 2011 David Edmundson <kde@davidedmundson.co.uk>
 * Copyright (C) 2012 Daniele E. Domenichelli <daniele.domenichelli@gmail.com>
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


#include "contact-view-widget.h"

#include <QLineEdit>
#include <QTextOption>
#include <QPainter>
#include <QtWidgets/QApplication>
#include <QtWidgets/QVBoxLayout>

#include "types.h"
#include <KTp/Models/contacts-list-model.h>
#include <KTp/Models/contacts-filter-model.h>

namespace KTp {

class ContactViewDelegate : public QAbstractItemDelegate
{
    Q_OBJECT
    Q_DISABLE_COPY(ContactViewDelegate)

public:
    ContactViewDelegate(QObject *parent);
    ~ContactViewDelegate() override;

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const override;

}; // class ContactViewDelegate

} // namespace KTp


KTp::ContactViewDelegate::ContactViewDelegate(QObject *parent)
    : QAbstractItemDelegate(parent)
{
}

KTp::ContactViewDelegate::~ContactViewDelegate()
{
}

void KTp::ContactViewDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyle *style = QApplication::style();
    int textHeight = option.fontMetrics.height() * 2;

    style->drawPrimitive(QStyle::PE_PanelItemViewItem, &option, painter);

    QRect avatarRect = option.rect.adjusted(0, 0, 0, -textHeight);
    QRect textRect = option.rect.adjusted(0, option.rect.height() - textHeight, 0, -3);

    QPixmap avatar;
    avatar.load(index.data(KTp::ContactAvatarPathRole).toString());
    if (avatar.isNull()) {
        avatar = QIcon::fromTheme(QStringLiteral("im-user-online")).pixmap(option.decorationSize);
    } else if (avatar.width() > option.decorationSize.width() || avatar.height() > option.decorationSize.height()) {
        //resize larger avatars if required
        avatar = avatar.scaled(option.decorationSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        //draw leaving paddings on smaller (or non square) avatars
    }
    style->drawItemPixmap(painter, avatarRect, Qt::AlignCenter, avatar);


    QTextOption textOption;
    textOption.setAlignment(Qt::AlignCenter);
    textOption.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    painter->drawText(textRect, index.data().toString(), textOption);

}

QSize KTp::ContactViewDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    Q_UNUSED(index);
    int textHeight = option.fontMetrics.height() * 2;
    return QSize(option.decorationSize.width() + 4, option.decorationSize.height() + textHeight + 3);
}

// -----------------------------------------------------------------------------

class KTp::ContactViewWidget::Private
{
public:
    Private(KTp::ContactViewWidget *parent)
        : q(parent),
          layout(new QVBoxLayout(parent)),
          contactView(new QListView(parent)),
          contactFilterLineEdit(new QLineEdit(parent)),
          contactsModel(nullptr),
          filterModel(nullptr)
    {
    }

    ~Private()
    {
    }

    void _k_onSelectionChanged(const QItemSelection &newSelection, const QItemSelection &oldSelection);
    void _k_onDoubleClicked(const QModelIndex &index);

    KTp::ContactViewWidget *q;
    QVBoxLayout *layout;
    QListView *contactView;
    QLineEdit *contactFilterLineEdit;
    KTp::ContactsListModel *contactsModel;
    KTp::ContactsFilterModel *filterModel;
};

void KTp::ContactViewWidget::Private::_k_onSelectionChanged(const QItemSelection &newSelection,
                                                            const QItemSelection &oldSelection)
{
    Q_UNUSED(oldSelection)

    if (newSelection.isEmpty()) {
        Q_EMIT q->selectionChanged(Tp::AccountPtr(), KTp::ContactPtr());
        return;
    }

    Q_EMIT q->selectionChanged(q->selectedAccount(), q->selectedContact());
}

void KTp::ContactViewWidget::Private::_k_onDoubleClicked(const QModelIndex& index)
{
    if (!index.isValid()) {
        return;
    }

    Q_EMIT q->contactDoubleClicked(index.data(KTp::AccountRole).value<Tp::AccountPtr>(),
                                   index.data(KTp::ContactRole).value<KTp::ContactPtr>());
}


// -----------------------------------------------------------------------------

KTp::ContactViewWidget::ContactViewWidget(KTp::ContactsListModel* model, QWidget *parent)
    : QWidget(parent),
      d(new Private(this))
{
    d->filterModel = new KTp::ContactsFilterModel(this);

    d->contactsModel = model;
    d->filterModel->setSourceModel(d->contactsModel);

    d->contactView->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    d->contactView->setResizeMode(QListView::Adjust);
    d->contactView->setSpacing(5);
    d->contactView->setViewMode(QListView::ListMode);
    d->contactView->setIconSize(QSize(80, 80));

    d->contactFilterLineEdit->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    d->contactFilterLineEdit->setClearButtonEnabled(true);

    d->layout->setMargin(0);
    d->layout->addWidget(d->contactView);
    d->layout->addWidget(d->contactFilterLineEdit);
    setLayout(d->layout);

    d->contactView->setModel(d->filterModel);
    d->contactView->setItemDelegate(new ContactViewDelegate(d->contactView));

    connect(d->contactView->selectionModel(),
            SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
            SLOT(_k_onSelectionChanged(QItemSelection,QItemSelection)));
    connect(d->contactView,
            SIGNAL(doubleClicked(QModelIndex)),
            SLOT(_k_onDoubleClicked(QModelIndex)));

    connect(d->contactFilterLineEdit,
            SIGNAL(textChanged(QString)),
            d->filterModel,
            SLOT(setDisplayNameFilterString(QString)));
}

KTp::ContactViewWidget::~ContactViewWidget()
{
    delete d;
}

QString KTp::ContactViewWidget::displayNameFilter() const
{
    return d->contactFilterLineEdit->text();
}

void KTp::ContactViewWidget::clearDisplayNameFilter()
{
    setDisplayNameFilter(QString());
}

void KTp::ContactViewWidget::setDisplayNameFilter(const QString& displayNameFilter)
{
    if (displayNameFilter != d->contactFilterLineEdit->text()) {
        d->contactFilterLineEdit->setText(displayNameFilter);
        Q_EMIT displayNameFilterChanged(displayNameFilter);
    }
}

QSize KTp::ContactViewWidget::iconSize() const
{
    return d->contactView->iconSize();
}

void KTp::ContactViewWidget::setIconSize(const QSize& iconSize)
{
    if (iconSize != d->contactView->iconSize()) {
        d->contactView->setIconSize(iconSize);
        Q_EMIT iconSizeChanged(iconSize);
    }
}

bool KTp::ContactViewWidget::hasSelection() const
{
    return d->contactView->selectionModel()->hasSelection();
}

Tp::AccountPtr KTp::ContactViewWidget::selectedAccount() const
{
    return d->contactView->currentIndex().data(KTp::AccountRole).value<Tp::AccountPtr>();
}

KTp::ContactPtr KTp::ContactViewWidget::selectedContact() const
{
    return d->contactView->currentIndex().data(KTp::ContactRole).value<KTp::ContactPtr>();
}

QList<KTp::ContactPtr> KTp::ContactViewWidget::selectedContacts() const
{
    QList<KTp::ContactPtr> selected;

    Q_FOREACH (const QModelIndex &index, d->contactView->selectionModel()->selectedIndexes()) {
        selected << index.data(KTp::ContactRole).value<KTp::ContactPtr>();
    }
    return selected;
}

void KTp::ContactViewWidget::setViewMode(QListView::ViewMode mode)
{
    d->contactView->setViewMode(mode);
}

QListView::ViewMode KTp::ContactViewWidget::viewMode() const
{
    return d->contactView->viewMode();
}

QAbstractItemView::SelectionMode KTp::ContactViewWidget::selectionMode()
{
    return d->contactView->selectionMode();
}

void KTp::ContactViewWidget::setSelectionMode(QAbstractItemView::SelectionMode mode)
{
    d->contactView->setSelectionMode(mode);
}

KTp::ContactsFilterModel* KTp::ContactViewWidget::filter() const
{
    return d->filterModel;
}

QLineEdit* KTp::ContactViewWidget::contactFilterLineEdit() const
{
    return d->contactFilterLineEdit;
}

#include "contact-view-widget.moc"
#include "moc_contact-view-widget.cpp"
