/*
 * Create by fuzhuo at 11/12/2013
 */
#include "ConfigIni.h"

#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>

#define CONFIGINI_DEBUG 0
#define log printf
#define mlog(msg...) do{\
if(CONFIGINI_DEBUG) printf(msg);\
}while(0)

ConfigIni::ConfigIni(const char *fileNameWithPath, bool _autoCreate):
    data(NULL),
    fStream(NULL),
    autoSave(false),
    autoCreate(_autoCreate)
{
    strcpy(iniFileName, fileNameWithPath);
    loadConfigFile();
}
std::string trim(const std::string& line)
{
    const char* WhiteSpace = " \t\r\n";
    std::size_t start = line.find_first_not_of(WhiteSpace);
    std::size_t end = line.find_last_not_of(WhiteSpace);
    return start == end ? line : line.substr(start, end - start + 1);
}
void ConfigIni::loadConfigFile()
{
    std::ifstream input(iniFileName);
    std::size_t pos;
    string str,section(""),key,value;
    for( std::string line; getline( input, line ); )
    {
       line=trim(line);

       if (line.length()<3) {
            continue;
       }

        ConfigIniEntry entry;
        //comment
        if(line[0]=='#'||line[0]==';')
        {
             //#key=value
            pos = line.find_first_of('=');
            if ( pos == std::string::npos) //not found = ,it's a bad line
            {
                entry.index = "";
                entry.name = "";
                entry.value="";
                entry.comment = line;
                entry.type = COMMONT;
            }
            else
            {
                key = line.substr(1,pos-1);
                key = trim(key);

                value = line.substr(pos+1);
                value=trim(value);
                
                entry.index = section;
                entry.name = key;
                entry.value = value;
                entry.type = NODE;
            }                        
            datas.push_back(entry);
            continue;
        }

        //section
        if(line[0]=='[')
        {
            pos = line.find_first_of(']');
            if (pos == std::string::npos) //not found ], it's a bad line
                continue;
            section = line.substr(1,pos-1);
            entry.type = SECTION;
            entry.comment = "";
            entry.name = section;
            entry.index=section;
            datas.push_back(entry);
            continue;
        }
        
        //key = value
        pos = line.find_first_of('=');
        if ( pos == std::string::npos) //not found = ,it's a bad line
            continue;

        key = line.substr(0,pos);
        key = trim(key);

        value = line.substr(pos+1);
        value=trim(value);
        
        entry.index = section;
        entry.name = key;
        entry.value = value;
        entry.type=NODE;
        datas.push_back(entry);

        //cout<<"["<<entry.index<<"] ["<<entry.name <<"] = " <<entry.value<<endl;

    }
    input.close();
}

ConfigIni::~ConfigIni()
{
    if(autoSave){
        //cout<<"AUTO save Config file["<<iniFileName<<"]"<<'\n';
        save();
    }
}

void ConfigIni::save(const char* fileName)
{
    autoSave=false;

    if(fileName==NULL) fileName = iniFileName;
    fstream fStream;
    fStream.open(fileName, ios_base::out | ios_base::trunc);
    string index = string("");

    ConfigIniEntry entry;
    for(vector<ConfigIniEntry>::iterator it=datas.begin(); it!= datas.end(); it++){
        ConfigIniEntry entry = *it;
        if(SECTION==entry.type)
        {
            fStream<<"\n["<<entry.index.c_str()<<"]"<<endl;         
            continue;
        }
        if(entry.type==COMMONT)
        {
            fStream<<entry.comment<<endl;
            continue;
        }
        if(entry.type==NODE)
        {
            fStream<<entry.name.c_str()<<"="<<entry.value.c_str()<<endl;
        }
    }
    fStream<<endl;
    fStream.close();
}

void ConfigIni::setStringValueWithIndex(const char *index, const char* name, const char* value)
{
    autoSave = true;
    ConfigIniEntry entry;
    entry.index = index;
    entry.name = name;
    entry.value = value;
    entry.type = NODE;
    if(datas.size() == 0) 
    {
        entry.type = SECTION;
        entry.name="";
        datas.push_back(entry);

        entry.type = NODE;
        entry.name=name;
        datas.push_back(entry);
        return;
    }

    vector<ConfigIniEntry>::iterator it=datas.begin();
    bool findIndex=false;
    for(; it!=datas.end(); it++)
    {
        if(it->type==COMMONT)
            continue;

        if(strcasecmp(it->index.c_str(), index) == 0)
        {
            findIndex=true;
            break;
        }
    }
    
    if(!findIndex)
    {//没有此段
        entry.type = SECTION;
        entry.name="";
        datas.push_back(entry);

        entry.type = NODE;
        entry.name=name;
        datas.push_back(entry);
        return;
    }

    ++it;
    for(; it!=datas.end(); it++)
    {
        if(it->type==COMMONT)
            continue;
        
        if( it->type==SECTION )
        {//本段已结束，未找到
            break;
        }
 
        if(strcasecmp(it->name.c_str(), name)==0)
        {
            it->value = string(value);
            return;
        }
    }
    datas.insert(it, 1, entry);
}

/***********getter*************/
const char* ConfigIni::getStringValue(const char* index, const char *name)
{
    char m_default[32];
    strcpy(m_default,"空值");
    return getStringValue(index,name,m_default);
}

const char* ConfigIni::getStringValue(const char* index, const char *name,char *m_default)
{
    vector<ConfigIniEntry>::iterator it=datas.begin();
    bool findIndex=false;
    for(; it!=datas.end(); it++)
    {
        if(it->type==COMMONT)
            continue;

        if(strcasecmp(it->index.c_str(), index) == 0)
        {
            findIndex=true;
            break;
        }
    }
    
    if(!findIndex)
    {//没有此段
        return m_default;
    }

    ++it;
    for(; it!=datas.end(); it++)
    {
        if(it->type==COMMONT)
            continue;
        
        if( it->type==SECTION )
        {//本段已结束，未找到
            return m_default;
        }
 
        if(strcasecmp(it->name.c_str(), name)==0)
        {
            //log("[%s/%s]=%s\n",index,name,it->value.c_str());
            return ( it->value.c_str() );
        }
    }
    return m_default;
}

bool ConfigIni::getBoolValue(const char* index, const char *name)
{
    return getBoolValue(index,name,false);
}

bool ConfigIni::getBoolValue(const char *index, const char* name,bool b_default)
{
    char m_default[32];
    sprintf(m_default,"%s",b_default?"true":"false");
    const char *str = getStringValue(index, name,m_default);
    if(str == NULL) 
    {
        log("not found for [%s]-[%s]\n", index, name); return m_default;
    }
    return strcasecmp(str,"true") == 0;
}

int ConfigIni::getIntValue(const char *index, const char* name)
{
    return getIntValue(index,name,-1);
}

int ConfigIni::getIntValue(const char *index, const char* name,int m_default)
{
    const char *str = getStringValue(index, name);
    if(!str){
        return m_default;
    }else{
        return atoi(str);
    }
}

float ConfigIni::getFloatValue(const char* index, const char *name)
{
    const char *str = getStringValue(index, name);
    if(str == NULL) {cout<<"notfound"<<'\n'; return -1.0;}
    return atof(str);
}

/***********setter*************/
void ConfigIni::setBoolValue(const char* index, const char *name, bool value)
{
    if(value) sprintf(str, "True");
    else sprintf(str, "False");
    setStringValueWithIndex(index,name,str);
}

void ConfigIni::setIntValue(const char* index, const char *name, int value)
{
    sprintf(str, "%d", value);
    setStringValueWithIndex(index,name,str);
}

void ConfigIni::setFloatValue(const char* index, const char *name, float value)
{
    sprintf(str, "%f", value);
    setStringValueWithIndex(index,name,str);
}

void ConfigIni::setStringValue(const char *index, const char* name, const char* value)
{
    setStringValueWithIndex(index,name,value);
}

/*------------------------------------ for DEBUG ---------------------------------------*/
void ConfigIni::printAll()
{
    //log("\n--All Entry of file[%s]--\n", iniFileName);
    for(vector<ConfigIniEntry>::iterator it=datas.begin(); it!= datas.end(); it++){
        ConfigIniEntry entry = *it;
        if(SECTION==entry.type)
        {
            //log("\n%d = [%s]\n", entry.type,entry.index.c_str());
            continue;
        }
        if(entry.type==COMMONT)
        {
            cout<<entry.type<<" = "<< entry.comment<<endl;
            continue;
        }
        if(entry.type==NODE)
        {
            cout<<entry.type << " = " << entry.name.c_str()<<"="<<entry.value.c_str()<<endl<<endl;
        }
    }
}

