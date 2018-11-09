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


#include "message-processor.h"
#include "message-processor-private.h"
#include "message-filters-private.h"
#include "message-filter-config-manager.h"

#include <QMutex>
#include <QStringBuilder>

#include "ktp-debug.h"
#include <KService>
#include <KServiceTypeTrader>
#include <KPluginFactory>

Q_LOGGING_CATEGORY(KTP_MESSAGEPROCESSOR, "ktp-message-processor")

using namespace KTp;

FilterPlugin::FilterPlugin(const KPluginInfo &pluginInfo, KTp::AbstractMessageFilter *instance_):
    name(pluginInfo.pluginName()),
    instance(instance_)
{
    bool ok;
    weight = pluginInfo.service()->property(QLatin1String("X-KDE-PluginInfo-Weight"), QVariant::Int).toInt(&ok);
    if (!ok) {
        weight = 100;
    }
}

FilterPlugin::FilterPlugin(const QString &name_, int weight_, KTp::AbstractMessageFilter *instance_):
    name(name_),
    weight(weight_),
    instance(instance_)
{
}

bool FilterPlugin::operator<(const FilterPlugin &other) const
{
    return weight < other.weight;
}

bool FilterPlugin::operator==(const FilterPlugin &other) const
{
    return instance == other.instance &&
           name == other.name &&
           weight == other.weight;
}

void MessageProcessor::Private::loadFilter(const KPluginInfo &pluginInfo)
{
    KService::Ptr service = pluginInfo.service();

    KPluginFactory *factory = KPluginLoader(service->library()).factory();
    if (factory) {
        qCDebug(KTP_MESSAGEPROCESSOR) << "loaded factory :" << factory;
        AbstractMessageFilter *filter = factory->create<AbstractMessageFilter>(q);

        if (filter) {
            qCDebug(KTP_MESSAGEPROCESSOR) << "loaded message filter : " << filter;
            filters << FilterPlugin(pluginInfo, filter);
        }
    } else {
        qCWarning(KTP_MESSAGEPROCESSOR) << "error loading plugin :" << service->library();
    }

    // Re-sort filters by weight
    qSort(filters);
}

void MessageProcessor::Private::unloadFilter(const KPluginInfo &pluginInfo)
{
    QList<FilterPlugin>::Iterator iter = filters.begin();
    for ( ; iter != filters.end(); ++iter) {
        const FilterPlugin &plugin = *iter;

        if (plugin.name == pluginInfo.pluginName()) {
            qCDebug(KTP_MESSAGEPROCESSOR) << "unloading message filter : " << plugin.instance;
            plugin.instance->deleteLater();
            filters.erase(iter);
            return;
        }
    }
}

void MessageProcessor::Private::loadFilters()
{
    qCDebug(KTP_MESSAGEPROCESSOR) << "Starting loading filters...";

    KPluginInfo::List plugins = MessageFilterConfigManager::self()->enabledPlugins();

    Q_FOREACH (const KPluginInfo &plugin, plugins) {
        loadFilter(plugin);
    }
}

KTp::MessageProcessor* MessageProcessor::instance()
{
    static KTp::MessageProcessor *mp_instance;
    static QMutex mutex;
    mutex.lock();
    if (!mp_instance) {
        mp_instance= new MessageProcessor;
    }
    mutex.unlock();

    return mp_instance;
}


MessageProcessor::MessageProcessor():
    d(new MessageProcessor::Private(this))
{
    // Default weight is 100. Make sure these two plugins are always above those
    // which don't have weight specified and in this exact order.
    //
    // The escape filter also has the URL filter in it, see message-escape-filter.cpp for details
    d->filters << FilterPlugin(QLatin1String("__messageEscapeFilter"), 98, new MessageEscapeFilter(this));

    d->loadFilters();
}


MessageProcessor::~MessageProcessor()
{
    delete d;
}

QString MessageProcessor::header()
{
    QStringList scripts;
    QStringList stylesheets;
    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        Q_FOREACH (const QString &script, plugin.instance->requiredScripts()) {
            // Avoid duplicates
            if (!scripts.contains(script)) {
                scripts << script;
            }
        }
        Q_FOREACH (const QString &stylesheet, plugin.instance->requiredStylesheets()) {
            // Avoid duplicates
            if (!stylesheets.contains(stylesheet)) {
                stylesheets << stylesheet;
            }
        }
    }

    QString out(QLatin1String("\n    <!-- The following scripts and stylesheets are injected here by the plugins -->\n"));
    Q_FOREACH(const QString &script, scripts) {
        out = out % QLatin1String("    <script type=\"text/javascript\" src=\"")
                  % QStandardPaths::locate(QStandardPaths::GenericDataLocation, script)
                  % QLatin1String("\"></script>\n");
    }
    Q_FOREACH(const QString &stylesheet, stylesheets) {
        out = out % QLatin1String("    <link rel=\"stylesheet\" type=\"text/css\" href=\"")
                  % QStandardPaths::locate(QStandardPaths::GenericDataLocation, stylesheet)
                  % QLatin1String("\" />\n");
    }

    qCDebug(KTP_MESSAGEPROCESSOR) << out;

    return out;
}

KTp::Message MessageProcessor::processIncomingMessage(const Tp::Message &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    return processIncomingMessage(KTp::Message(message, context), context);
}

KTp::Message KTp::MessageProcessor::processIncomingMessage(const Tp::ReceivedMessage &message, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    return processIncomingMessage(KTp::Message(message, context), context);
}

KTp::Message MessageProcessor::processIncomingMessage(KTp::Message message, const KTp::MessageContext &context)
{
    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        qCDebug(KTP_MESSAGEPROCESSOR) << "running filter:" << plugin.instance->metaObject()->className();
        plugin.instance->filterMessage(message, context);
    }

    qWarning().noquote() << "message" << message.mainMessagePart() << "is forwarded:" << message.isForwarded();
    if (message.isForwarded()) {
        QString mainPart = message.mainMessagePart();
        mainPart.prepend(QStringLiteral("<b>Forwarded from %1</b><blockquote>").arg(message.forwardedSenderAlias()));
        mainPart.append(QStringLiteral("</blockquote>"));
        message.setMainMessagePart(mainPart);
    }
    for (const KTp::Message::Thumbnail &t : message.thumbnails()) {
        message.appendMessagePart(QStringLiteral("<img src=\"data:%1;base64, %2\" alt=\"Hey!\"/>")
                                  .arg(QString::fromLatin1(t.mimeType))
                                  .arg(QString::fromLatin1(t.content.toBase64()))
                                       );
    }

//    if (message.isReply() && !message.mainMessagePart().isEmpty()) {
//        QString mainPart = message.mainMessagePart();
//        qWarning() << "Process incoming:" << mainPart;
//        QStringList lines = mainPart.split(QRegularExpression(QStringLiteral("<br.{0,2}>")));
//        QString replyLine = lines.first();
//        qWarning().noquote() << "First:" << replyLine;

//        replyLine = QStringLiteral("<a href=\"javascript:;\" onclick=\"scrollToMessage('%1');\">%2</a>").arg(message.replyToMessageToken()).arg(replyLine);
//        qWarning().noquote() << "First processed:" << replyLine;
//        lines[0] = replyLine;

//        bool inQuote = false;

//        for (int i = 1; i < lines.count(); ++i) {
//            qWarning() << "Second processed:" << i << lines.at(i);

//            if (lines.at(i).startsWith(QLatin1String("&gt;"))) {
//                lines[i].remove(0, 4);
//                if (!inQuote) {
//                    inQuote = true;
//                    lines[i].prepend(QStringLiteral("<blockquote>"));
//                }
//                lines[i].append(QStringLiteral("<br/>"));
//            } else {
//                if (inQuote) {
//                    lines[i - 1].append(QStringLiteral("</blockquote>"));
//                    if (lines.at(i).simplified().isEmpty()) {
//                        lines.removeAt(i);
//                    }
//                }
//                break;
//            }
//        }

//        message.setMainMessagePart(lines.join(QStringLiteral("<br/>")));
//    }
    return message;
}

KTp::OutgoingMessage MessageProcessor::processOutgoingMessage(const QString &messageText, const Tp::AccountPtr &account, const Tp::TextChannelPtr &channel)
{
    KTp::MessageContext context(account, channel);
    KTp::OutgoingMessage message(messageText);

    Q_FOREACH (const FilterPlugin &plugin, d->filters) {
        qCDebug(KTP_MESSAGEPROCESSOR) << "running outgoing filter: " << plugin.instance->metaObject()->className();
        plugin.instance->filterOutgoingMessage(message, context);
    }

    return message;
}
