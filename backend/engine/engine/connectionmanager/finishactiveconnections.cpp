#include "finishactiveconnections.h"
#include "../openvpnversioncontroller.h"
#include "ikev2connection_win.h"
#include <windows.h>
#include "Utils/logger.h"

void FinishActiveConnections::finishAllActiveConnections_win(IHelper *helper)
{
    finishOpenVpnActiveConnections_win(helper);
    finishIkev2ActiveConnections_win(helper);
}

void FinishActiveConnections::finishOpenVpnActiveConnections_win(IHelper *helper)
{
    QStringList strOpenVpnExeList = OpenVpnVersionController::instance().getAvailableOpenVpnExecutables();
    Q_FOREACH (const QString &strExe, strOpenVpnExeList)
    {
        helper->executeTaskKill(strExe);
    }
}

void FinishActiveConnections::finishIkev2ActiveConnections_win(IHelper *helper)
{
    QVector<HRASCONN> v = IKEv2Connection_win::getActiveWindscribeConnections();

    if (!v.isEmpty())
    {
        Q_FOREACH(HRASCONN hRas, v)
        {
            IKEv2ConnectionDisconnectLogic_win::blockingDisconnect(hRas);
        }

        helper->disableDnsLeaksProtection();
        helper->removeHosts();
    }
}