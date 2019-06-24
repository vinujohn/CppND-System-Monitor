#include <algorithm>
#include <iostream>
#include <math.h>
#include <thread>
#include <chrono>
#include <iterator>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <vector>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <cerrno>
#include <cstring>
#include <dirent.h>
#include <time.h>
#include <unistd.h>
#include "constants.h"


using namespace std;

class ProcessParser{
    private:
        std::ifstream stream;
        static string getProcessStatusElem(string statusFieldName, int elemIndex, string pid);
    public:
        static string getCmd(string pid);
        static vector<string> getPidList();
        static std::string getVmSize(string pid);
        static std::string getCpuPercent(string pid);
        static long int getSysUpTime();
        static std::string getProcUpTime(string pid);
        static string getProcUser(string pid);
        static vector<string> getSysCpuPercent(string coreNumber = "");
        static float getSysRamPercent();
        static string getSysKernelVersion();
        static int getNumberOfCores();
        static int getTotalThreads();
        static int getTotalNumberOfProcesses();
        static int getNumberOfRunningProcesses();
        static string getOSName();
        static std::string PrintCpuStats(std::vector<std::string> values1, std::vector<std::string>values2);
        static bool isPidExisting(string pid);
};

// Private members
string ProcessParser::getProcessStatusElem(string statusFieldName, int elemIndex, string pid){    
    auto lines = Util::getFileLines(Path::basePath() + pid + Path::statusPath());
        
    for(auto line : lines){
        if(line.compare(0, statusFieldName.size(), statusFieldName) == 0){
            return Util::getStringElements(line)[elemIndex];
        }
    }

    throw runtime_error("Could not find process status element");
}



// TODO: Define all of the above functions below:
string ProcessParser::getCmd(string pid){
    return Util::getFileLines(Path::basePath() + pid + Path::cmdPath())[0];
}

vector<string> ProcessParser::getPidList(){
    vector<string> ret;

    DIR* dir;
    if(!(dir = opendir("/proc"))){
        throw runtime_error(strerror(errno));
    }

    while(dirent* dirp = readdir(dir)) {
        if(dirp->d_type != DT_DIR){
            continue;
        }

        if(all_of(dirp->d_name, dirp->d_name + std::strlen(dirp->d_name), [](char c) {return isdigit(c);})){
            ret.push_back(dirp->d_name);
        }
    }

    if(closedir(dir)){
        throw runtime_error(strerror(errno));
    }

    return ret;
}

string ProcessParser::getVmSize(string pid){
    string elemName = "VmData";
    int elemIndex = 1;

    auto val = getProcessStatusElem(elemName, elemIndex, pid);

    return to_string(stof(val)/float(1024*1024));
}

string ProcessParser::getCpuPercent(string pid){
        
    auto lines = Util::getFileLines(Path::basePath() + pid + "/" + Path::statPath());
    auto values = Util::getStringElements(lines[0]);

    float utime = stof(ProcessParser::getProcUpTime(pid));
    float stime = stof(values[14]); 
    float cutime = stof(values[15]);
    float cstime = stof(values[16]);
    float starttime = stof(values[21]);
    float uptime = ProcessParser::getSysUpTime();
    float freq = sysconf(_SC_CLK_TCK);
    float total_time = utime + stime + cutime + cstime;
    float seconds = uptime - (starttime/freq);
    
    return to_string(100.0*((total_time/freq)/seconds));
}

long int ProcessParser::getSysUpTime() {
    
    auto lines = Util::getFileLines(Path::basePath() + Path::upTimePath());
    auto values = Util::getStringElements(lines[0]);
       
    return stoi(values[0]);
}

string ProcessParser::getProcUpTime(string pid){
    
    auto lines = Util::getFileLines(Path::basePath() + pid + "/" + Path::statPath());
    auto values = Util::getStringElements(lines[0]);
       
    return to_string(float(stof(values[13])/sysconf(_SC_CLK_TCK)));
}

string ProcessParser::getProcUser(string pid){
    string elemName = "Uid";
    int elemIndex = 1;

    auto val = getProcessStatusElem(elemName, elemIndex, pid);

    ifstream stream;
    Util::getStream("/etc/passwd", stream);
    
    auto token = "x:" + val;
    string line;
    while(getline(stream, line)){
        if(line.find(token) != std::string::npos) {
            return line.substr(0, line.find(":"));
        }
    }

     throw runtime_error("Could not user name for process");
}

vector<string> ProcessParser::getSysCpuPercent(string coreNumber){
    return vector<string>();
}

float ProcessParser::getSysRamPercent(){
    return 0.0;
}

string ProcessParser::getSysKernelVersion(){
    return "";
}

int ProcessParser::getNumberOfCores(){
    string name = "cpu cores";
    auto lines = Util::getFileLines(Path::basePath() + "cpuinfo");
    for(auto line : lines){
        if(line.compare(0, name.size(), name) == 0){
            return stoi(Util::getStringElements(line)[3]);
        }
    }
    return 0;
}

int ProcessParser::getTotalThreads(){
    return 0;
}

int ProcessParser::getTotalNumberOfProcesses(){
    return 0;
}

int ProcessParser::getNumberOfRunningProcesses(){
    return 0;
}

string ProcessParser::getOSName(){
    return "";
}

string ProcessParser::PrintCpuStats(vector<string> values1, vector<string> values2){
    return "";
}

bool ProcessParser::isPidExisting(string pid){
    return false;
}
