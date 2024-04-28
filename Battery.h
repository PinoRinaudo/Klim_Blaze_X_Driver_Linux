#include <iostream>
#include <fstream>
#include <string>

using namespace std;

class Battery
{

private:
    static Battery *instance;
    string folderPath;

    Battery()
    {
    }

    string readLineFile(string path)
    {
        string str;
        ifstream file(path);
        if (file.is_open())
        {
            getline(file, str);
            file.close();
            return str;
        }
        return "";
    }

public:
    static Battery *getInstance()
    {
        if (!instance)
            instance = new Battery();
        return instance;
    }

    int8_t getBatteryLevel()
    {
        string level = readLineFile("/sys/class/power_supply/BAT0/capacity");
        if (level != "")
            return stoi(level);
        return -1;
    }

    int8_t getBatteryPlugged()
    {
        string status = readLineFile("/sys/class/power_supply/AC/online");
        if (status != "")
            return status == "1";
        return -1;
    }
};
Battery *Battery::instance = nullptr;
