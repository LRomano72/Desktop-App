#ifndef IKEV2CONNECTION_MAC_H
#define IKEV2CONNECTION_MAC_H

#include <QObject>
#include <QTimer>
#include "iconnection.h"
#include "networkextensionlog_mac.h"

class IKEv2Connection_mac : public IConnection
{
    Q_OBJECT
public:
    explicit IKEv2Connection_mac(QObject *parent, IHelper *helper);
    virtual ~IKEv2Connection_mac();

    // config path for openvpn, url for ikev2
    virtual void startConnect(const QString &configPathOrUrl, const QString &ip, const QString &dnsHostName,
                              const QString &username, const QString &password, const ProxySettings &proxySettings,
                              bool isEnableIkev2Compression, bool isAutomaticConnectionMode);
    virtual void startDisconnect();
    virtual bool isDisconnected();
    virtual QString getConnectedTapTunAdapterName();

    virtual void continueWithUsernameAndPassword(const QString &username, const QString &password);
    virtual void continueWithPassword(const QString &password);


    static void removeIkev2ConnectionFromOS();

private slots:
    void handleNotificationImpl(int status);
    void onStatisticsTimer();

private:
    enum {STATE_DISCONNECTED, STATE_START_CONNECT, STATE_START_DISCONNECTING, STATE_CONNECTED, STATE_DISCONNECTING_AUTH_ERROR, STATE_DISCONNECTING_ANY_ERROR};

    int state_;

    bool bConnected_;
    QMutex mutex_;
    void *notificationId_;
    bool isStateConnectingAfterClick_;
    bool isDisconnectClicked_;
    NetworkExtensionLog_mac networkExtensionLog_;

    const int STATISTICS_UPDATE_PERIOD = 1000;
    QTimer statisticsTimer_;
    QString ipsecAdapterName_;

    int prevConnectionStatus_;
    bool isPrevConnectionStatusInitialized_;

    void handleNotification(void *notification);
    bool isFailedAuthError(QMap<time_t, QString> &logs);
    bool isSocketError(QMap<time_t, QString> &logs);
};

#endif // IKEV2CONNECTION_MAC_H