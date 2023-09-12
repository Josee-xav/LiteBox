#pragma once
// credits to https://github.com/button-chen/inifile 
// simple ini parser
#include <string>
#include <map>
#include <strstream>
#include <vector>
#include <sstream>
#include <algorithm>
#include <windows.h>

#include "Logging/FLogger.h"
namespace ini
{
    void Trim(std::string& str);

    class KeyVal
    {
    public:
        KeyVal() { m_strVal.clear(); }
        KeyVal(std::string strVal) :m_strVal(strVal) { }
        void operator= (const std::string& v) { m_strVal = v; }
        template<typename T>
        T StringTo(std::string val)
        {
            std::istrstream ins(val.c_str());
            T tmp;
            ins >> tmp;
            return tmp;
        }

        template <> std::wstring StringTo<std::wstring>(std::string val)
        {

            if(val.empty())
                return std::wstring();

            size_t charsNeeded = ::MultiByteToWideChar(CP_UTF8, 0,
                val.data(), (int)val.size(), NULL, 0);
            if(charsNeeded == 0)
                FLogger::error("Failed converting UTF-8 string to UTF-16");

            std::vector<wchar_t> buffer(charsNeeded);
            int charsConverted = ::MultiByteToWideChar(CP_UTF8, 0,
                val.data(), (int)val.size(), &buffer[0], buffer.size());
            if(charsConverted == 0)
                FLogger::error("Failed converting UTF-8 string to UTF-16");

            return std::wstring(&buffer[0], charsConverted);
        }

        template<typename T>
        T GetValue(T defval = T())
        {
            if(m_strVal.empty()) {
                return defval;
            }
            return StringTo<T>(m_strVal);
        }
        template<typename T>
        std::vector<T> GetArray(std::string defval = std::string(), char sep = char())
        {
            if(m_strVal.empty()) {
                m_strVal = defval;
            }
            std::vector<T> vRet;
            std::istringstream iss(m_strVal);
            std::string toke;
            while(getline(iss, toke, sep)) {
                Trim(toke);
                vRet.push_back(StringTo<T>(toke));
            }
            return vRet;
        }
        void Set(std::string other) { m_strVal = other; }
        std::string Get() { return m_strVal; }

    private:
        std::string m_strVal;
    };

    class Section
    {
    public:
        Section() { m_sectionName.clear(); }
        Section(std::string secName) : m_sectionName(secName) { }
        KeyVal& operator [](std::string key) { return m_sectionConf[key]; }
        Section& operator= (const Section& sec)
        {
            if(&sec == this) {
                return *this;
            }
            m_sectionName = sec.m_sectionName;
            m_sectionConf = sec.m_sectionConf;
            m_pre_comment = sec.m_pre_comment;
            m_next_comment = sec.m_next_comment;
            m_key_order = sec.m_key_order;
            return *this;
        }
        bool IsValid() { return !m_sectionName.empty(); }
        std::string GetName() { return m_sectionName; }
        std::map<std::string, KeyVal> GetData() { return m_sectionConf; }

        std::vector<std::string> m_key_order;
        std::map<std::string, std::vector<std::string> > m_pre_comment;
        std::map<std::string, std::vector<std::string> > m_next_comment;
    private:
        std::string   m_sectionName;
        std::map<std::string, KeyVal> m_sectionConf;
    };

    class IniFile
    {
    public:
        IniFile();
        explicit IniFile(const std::string& ConfigFile);
        ~IniFile();

    public:
        bool Load();
        Section& GetSection(std::string StrSecName);
        bool PushSection(Section& sec);
        bool Flush(std::string savepath = "");

    private:
        std::string GetSectionName(std::string inputLine);
        std::string GetKey(std::string inputLine);
        std::string GetValue(std::string inputLine);

    private:
        std::string m_fileName;
        std::map<std::string, Section> m_mapConfig;
        std::vector<std::string> m_order_sec;
    };

} // end namespace ini
