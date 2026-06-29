#include "adblockinterceptor.h"
#include <QDebug>

AdBlockInterceptor::AdBlockInterceptor(QObject *parent)
    : QWebEngineUrlRequestInterceptor(parent)
{
    // Populate some common ad/tracking domains
    m_blockedHosts = {
        "doubleclick.net",
        "googlesyndication.com",
        "adservice.google.com",
        "pubmatic.com",
        "adnxs.com",
        "openx.net",
        "criteo.com",
        "casalemedia.com",
        "rubiconproject.com",
        "outbrain.com",
        "taboola.com",
        "adroll.com",
        "adsystem.com",
        "quantserve.com",
        "scorecardresearch.com"
    };
}

void AdBlockInterceptor::interceptRequest(QWebEngineUrlRequestInfo &info)
{
    if (!m_enabled)
        return;

    QUrl url = info.requestUrl();
    QString host = url.host().toLower();

    // Check if host ends with or matches any of the blocked hosts
    for (const QString &blockedHost : m_blockedHosts)
    {
        if (host == blockedHost || host.endsWith("." + blockedHost))
        {
            qDebug() << "[AdBlocker] Blocked request to:" << url.toString();
            info.block(true);
            return;
        }
    }
}

void AdBlockInterceptor::setEnabled(bool enabled)
{
    m_enabled = enabled;
}
