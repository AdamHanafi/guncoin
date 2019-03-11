// Copyright (c) 2009-2010 Satoshi Nakamoto
// Copyright (c) 2009-2018 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <alert.h>
#include <arith_uint256.h>
#include <sync.h>
#include <clientversion.h>
#include <checkpointsync.h>
#include <util.h>
#include <warnings.h>
#include <utilstrencodings.h>

CCriticalSection cs_warnings;
std::string strMiscWarning GUARDED_BY(cs_warnings);
bool fLargeWorkForkFound GUARDED_BY(cs_warnings) = false;
bool fLargeWorkInvalidChainFound GUARDED_BY(cs_warnings) = false;

void SetMiscWarning(const std::string& strWarning)
{
    LOCK(cs_warnings);
    strMiscWarning = strWarning;
}

void SetfLargeWorkForkFound(bool flag)
{
    LOCK(cs_warnings);
    fLargeWorkForkFound = flag;
}

bool GetfLargeWorkForkFound()
{
    LOCK(cs_warnings);
    return fLargeWorkForkFound;
}

void SetfLargeWorkInvalidChainFound(bool flag)
{
    LOCK(cs_warnings);
    fLargeWorkInvalidChainFound = flag;
}

std::string GetWarnings(const std::string& strFor)
{
    int nPriority = 0;
    std::string strStatusBar;
    std::string strGUI;
    const std::string uiAlertSeperator = "<hr />";

    LOCK(cs_warnings);

    if (!CLIENT_VERSION_IS_RELEASE) {
        strStatusBar = "This is a pre-release test build - use at your own risk - do not use for mining or merchant applications";
        strGUI = _("This is a pre-release test build - use at your own risk - do not use for mining or merchant applications");
    }

    // Checkpoint warning
    if (strCheckpointWarning != "")
    {
        strStatusBar = strCheckpointWarning;
        strGUI += (strGUI.empty() ? "" : uiAlertSeperator) + strCheckpointWarning;
    }

    // Misc warnings like out of disk space and clock is wrong
    if (strMiscWarning != "")
    {
        strStatusBar = strMiscWarning;
        strGUI += (strGUI.empty() ? "" : uiAlertSeperator) + strMiscWarning;
    }

    if (fLargeWorkForkFound)
    {
        nPriority = 2000;
        strStatusBar = "Warning: The network does not appear to fully agree! Some miners appear to be experiencing issues.";
        strGUI += (strGUI.empty() ? "" : uiAlertSeperator) + _("Warning: The network does not appear to fully agree! Some miners appear to be experiencing issues.");
    }
    else if (fLargeWorkInvalidChainFound)
    {
        nPriority = 2000;
        strStatusBar = "Warning: We do not appear to fully agree with our peers! You may need to upgrade, or other nodes may need to upgrade.";
        strGUI += (strGUI.empty() ? "" : uiAlertSeperator) + _("Warning: We do not appear to fully agree with our peers! You may need to upgrade, or other nodes may need to upgrade.");
    }

    // If detected invalid checkpoint enter safe mode
    if (hashInvalidCheckpoint != ArithToUint256(arith_uint256(0)))
    {
        strStatusBar = "WARNING: Inconsistent checkpoint found! Stop enforcing checkpoints and notify developers to resolve the issue.";
        strGUI += (strGUI.empty() ? "" : uiAlertSeperator) + _("WARNING: Inconsistent checkpoint found! Stop enforcing checkpoints and notify developers to resolve the issue.");
    }

    // Alerts
    {
        LOCK(cs_mapAlerts);
        for (auto& item : mapAlerts)
        {
            const CAlert& alert = item.second;
            if (alert.AppliesToMe() && alert.nPriority > nPriority)
            {
                nPriority = alert.nPriority;
                strStatusBar = strGUI = alert.strStatusBar;
            }
        }
    }

    if (strFor == "gui")
        return strGUI;
    else if (strFor == "statusbar")
        return strStatusBar;
    assert(!"GetWarnings(): invalid parameter");
    return "error";
}
