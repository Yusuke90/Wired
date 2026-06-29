#pragma once

#include <QWebEngineUrlRequestInterceptor>
#include <QWebEngineUrlRequestInfo>
#include <QUrl>
#include <QSet>
#include <QString>

class AdBlockInterceptor : public QWebEngineUrlRequestInterceptor
{
    Q_OBJECT

public:
    explicit AdBlockInterceptor(QObject *parent = nullptr);
    ~AdBlockInterceptor() override = default;

    void interceptRequest(QWebEngineUrlRequestInfo &info) override;

    void setEnabled(bool enabled);
    bool isEnabled() const { return m_enabled; }

private:
    bool m_enabled = true;
    QSet<QString> m_blockedHosts;
};
