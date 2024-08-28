/*
 * Create by fuzhuo at 11/12/2013
 */
#ifndef CONFIGINI_H
#define CONFIGINI_H

#include<iostream>
#include<fstream>
#include<cstring>
#include<string>
#include<vector>
#include<cstdlib>

using namespace std;
typedef enum
{
    SECTION,
    COMMONT,
    NODE
}CNodeType;

struct ConfigIniEntry{
    ConfigIniEntry():type(COMMONT){}
    string index;
    string name;
    string value;
    string comment;
    CNodeType type;
};

class ConfigIni
{
public:
    ConfigIni(const char *fileName, bool autoCreate=false);
    void save(const char *fileName=NULL);
    ~ConfigIni();
    /***********getter*************/
    bool getBoolValue(const char* index, const char *name);
    bool getBoolValue(const char* index, const char *name,bool m_default);
    int getIntValue(const char* index, const char *name);
    int getIntValue(const char* index, const char *name,int m_default);
    const char* getStringValue(const char* index, const char *name);
    const char* getStringValue(const char* index, const char *name,char *m_default);
    float getFloatValue(const char* index, const char *name);
    
    /***********setter*************/
    void setBoolValue(const char* index, const char *name, bool value);
    void setIntValue(const char* index, const char *name, int value);
    void setFloatValue(const char* index, const char *name, float value);
    void setStringValue(const char *index, const char* name, const char* value);

    /******* for test only *******/
    void printAll();
    
    /******* get all Entry *******/
    vector<ConfigIniEntry> datas;
private:
    char str[4096];//for temp string data
    void setStringValueWithIndex(const char *index, const char* name, const char* value);
    char iniFileName[4096];
    char *data;
    const char (*lineData)[4096];
    void loadConfigFile();
    fstream *fStream;
    bool autoSave;
    bool autoCreate;
};

#endif // CONFIGINI_H

