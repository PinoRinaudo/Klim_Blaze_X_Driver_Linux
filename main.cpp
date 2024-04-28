#include <iostream>
#include <fstream>
#include <unistd.h>
#include "KlimBlazeX.h"
#include "Battery.h"
#include <cmath>
#include <syslog.h>
#include <signal.h>
#include <stdlib.h>
#include <sys/stat.h>

#define DELAY_SEC 2
#define LOG_NAME "KlimBlazeX"
using namespace std;

uint8_t *HSVToRGB(uint8_t h, uint8_t s, uint8_t v)
{
    double c = v * s;
    double x = c * (1 - abs(fmod((h / 60.0), 2) - 1));
    double m = v - c;

    double r, g, b;
    if (h >= 0 && h < 60)
    {
        r = c;
        g = x;
        b = 0;
    }
    else if (h >= 60 && h < 120)
    {
        r = x;
        g = c;
        b = 0;
    }
    else if (h >= 120 && h < 180)
    {
        r = 0;
        g = c;
        b = x;
    }
    else if (h >= 180 && h < 240)
    {
        r = 0;
        g = x;
        b = c;
    }
    else if (h >= 240 && h < 300)
    {
        r = x;
        g = 0;
        b = c;
    }
    else
    {
        r = c;
        g = 0;
        b = x;
    }

    static uint8_t rgb[3];
    rgb[0] = static_cast<uint8_t>((r + m) * 255);
    rgb[1] = static_cast<uint8_t>((g + m) * 255);
    rgb[2] = static_cast<uint8_t>((b + m) * 255);

    return rgb;
}

void log(uint8_t level, const string msg)
{
    openlog(LOG_NAME, 0, LOG_DAEMON);

    string fullMsg = msg;
    syslog(level, "%s", fullMsg.c_str());
    closelog();
}

void logError(const string msg)
{
    cerr << msg << endl;
    log(LOG_ERR, msg);
}

void logInfo(const string msg)
{

    cout << msg << endl;
    log(LOG_INFO, msg);
}

void createDaemon()
{
    pid_t pid = fork();

    if (pid < 0)
        exit(EXIT_FAILURE);

    if (pid > 0)
        exit(EXIT_SUCCESS);

    if (setsid() < 0)
        exit(EXIT_FAILURE);

    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    umask(0);

    chdir("/");

    for (int x = sysconf(_SC_OPEN_MAX); x >= 0; x--)
    {
        close(x);
    }

}

int main()
{
    if (geteuid() != 0)
    {
        logError("You need to be root");
        return -1;
    }

    createDaemon();
    logInfo("Daemon started, pid " + to_string(getpid()));

    KlimBlazeX *mouse = KlimBlazeX::getInstance();
    Battery *battery = Battery::getInstance();
    int8_t prevBatteryLevel = 200, actualBatteryLevel = -1;
    uint8_t *rgb = new uint8_t[3];
    while (true)
    {
        if (!mouse->isBind())
        {
            logError(mouse->getMessageError() + " (" + to_string(mouse->getLastErrorCode()) + ")");
            logInfo("Try rebinding");
            while (!mouse->isBind())
                mouse->bind();
            prevBatteryLevel = 200;
            logInfo("Rebind");
        }
        if (battery->getBatteryLevel() < 0 || battery->getBatteryPlugged() < 0)
        {
            logError("Unable to read battery level or AC file");
            mouse->setColor(255, 255, 255);
            while (battery->getBatteryLevel() < 0 || battery->getBatteryPlugged() < 0)
                sleep(DELAY_SEC);
        }

        if (battery->getBatteryPlugged())
        {
            mouse->setStream();
            while (battery->getBatteryPlugged())
                sleep(DELAY_SEC);
            prevBatteryLevel = 200;
        }

        actualBatteryLevel = battery->getBatteryLevel();

        if (abs(prevBatteryLevel - actualBatteryLevel) > 3)
        {
            if (actualBatteryLevel < 21)
            {
                rgb[0] = 255;
                rgb[1] = rgb[2] = 0;
            }
            else
                rgb = HSVToRGB((actualBatteryLevel * 120) / 100, 1, 1);

            mouse->setColor(rgb[0], rgb[1], rgb[2]);
            prevBatteryLevel = actualBatteryLevel;
        }

        sleep(DELAY_SEC);
    }

    delete[] rgb;
    return 0;
}
