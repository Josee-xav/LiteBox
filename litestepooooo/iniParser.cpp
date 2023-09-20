#include "iniParser.h"
#include <fstream>
#include <locale>
#include <Windows.h>

namespace ini
{
    IniFile::IniFile(const std::string& ConfigFile)
        :m_fileName(ConfigFile)
    {
    }

    IniFile::IniFile() { }

    IniFile::~IniFile(void) { }

    //Value value supports multi-line mode
    bool IniFile::Load()
    {
        std::ifstream in(m_fileName.c_str());
        if (!in) {
            MessageBoxA(NULL, "ifstream failed when trying to parse style file. File path incorrect?", "error", MB_OK);
            return false;
        }
        std::string line, secName, lastSecName;
        while (!in.eof()) {
            getline(in, line);
            secName = GetSectionName(line);
            if (secName.empty()) {
                continue;
            }
            m_order_sec.push_back(secName);
            break;
        }
        lastSecName = secName;
        Section secConf;
        std::string strline;

        std::vector<std::string> vPreComments;
        std::vector<std::string> vNextComments;
        do {
        __reboot:
            Trim(line);
            if (line.empty() || line.find(';') == 0 || line.find('#') == 0) {
                vPreComments.push_back(line);
                continue;
            }
            secName = GetSectionName(line);
            if (!secName.empty()) {
                if (lastSecName != secName) {
                    m_mapConfig[lastSecName] = secConf;
                    lastSecName = secName;
                    m_order_sec.push_back(secName);
                }
                secConf = Section(secName);
                continue;
            }
            strline.clear();
            strline += line;
            bool  flag = false;
            while (getline(in, line)) {
                Trim(line);
                if (line.empty() || line.find(';') == 0 || line.find('#') == 0) {
                    vNextComments.push_back(line);
                    break;
                }
                std::string tmp = GetSectionName(line);
                if (!tmp.empty()) {
                    flag = true;
                    break;
                }
                if (line.find('=') == std::string::npos) {
                    strline += line;
                }
                else {
                    flag = true;
                    break;
                }
            }
            std::string key = GetKey(strline);
            std::string svalue = GetValue(strline);
            secConf[key] = svalue;
            secConf.m_key_order.push_back(key);
            secConf.m_pre_comment[key] = vPreComments;
            secConf.m_next_comment[key] = vNextComments;
            vPreComments.clear();
            vNextComments.clear();
            if (flag) {
                goto __reboot;
            }

        } while (getline(in, line));
        m_mapConfig[lastSecName] = secConf;
        return true;
    }

    Section& IniFile::GetSection(const std::string strSecName)
    {
        std::map<std::string, Section>::iterator itr = m_mapConfig.find(strSecName);
        if (itr != m_mapConfig.end()) {
            return itr->second;
        }
        return m_mapConfig[strSecName];
    }

    std::string IniFile::GetSectionName(std::string inputLine)
    {
        size_t sec_begin_pos = inputLine.find('[');
        if (sec_begin_pos == std::string::npos || sec_begin_pos != 0) {
            return std::string();
        }
        size_t sec_end_pos = inputLine.find(']', sec_begin_pos);
        if (sec_end_pos == std::string::npos) {
            return std::string();
        }

        inputLine = (inputLine.substr(sec_begin_pos + 1, sec_end_pos - sec_begin_pos - 1));
        Trim(inputLine);
        return inputLine;
    }

    std::string IniFile::GetKey(std::string inputLine)
    {
        size_t keyPos = inputLine.find('=');
        if (keyPos == std::string::npos) {
            return std::string();
        }
        std::string key = inputLine.substr(0, keyPos);
        Trim(key);
        return key;
    }

    std::string IniFile::GetValue(std::string inputLine)
    {
        size_t keyPos = inputLine.find('=');
        if (keyPos == std::string::npos) {
            return std::string();
        }
        size_t notePos1 = inputLine.find(';');
        //size_t notePos2 = inputLine.find('#');
        size_t pos = notePos1;
        pos = (pos == std::string::npos) ? inputLine.size() : pos;
        std::string svalue = inputLine.substr(keyPos + 1, pos - keyPos - 1);
        Trim(svalue);
        return svalue;
    }

    bool IniFile::Flush(std::string savepath)
    {
        std::ofstream ou;
        if (savepath.empty()) {
            ou.open(m_fileName.c_str());
        }
        else {
            ou.open(savepath.c_str());
        }
        ou.clear();

        for (int i = 0; i < m_order_sec.size(); i++) {
            std::string secname = m_order_sec[i];
            Section& sec = m_mapConfig[secname];
            ou << "[" << secname << "]" << std::endl;
            std::map<std::string, KeyVal> data = sec.GetData();

            for (int j = 0; j < sec.m_key_order.size(); j++) {
                std::string key = sec.m_key_order[j];

                std::vector<std::string> vprcomm = sec.m_pre_comment[key];
                std::vector<std::string> vnextcomm = sec.m_next_comment[key];
                if (vprcomm.size() > 0) {
                    for (int k = 0; k < vprcomm.size(); k++) {
                        ou << vprcomm[k] << std::endl;
                    }
                }
                ou << key << " = " << data[key].Get() << std::endl;
                if (vnextcomm.size() > 0) {
                    for (int k = 0; k < vnextcomm.size(); k++) {
                        ou << vnextcomm[k] << std::endl;
                    }
                }
            }
            ou << std::endl;
        }
        ou.close();
        return true;
    }

    bool IniFile::PushSection(Section& sec)
    {
        m_mapConfig[sec.GetName()] = sec;
        return true;
    }

    void Trim(std::string& str)
    {
        int poss = -1, pose = -1;
        for (int i = 0; i < str.size(); i++) {
            if (isspace(str[i], std::locale(".OCP")))
                continue;
            poss = i;
            break;
        }
        for (int i = str.size() - 1; i >= 0; i--) {
            if (isspace(str[i], std::locale(".OCP")))
                continue;
            pose = i;
            break;
        }
        if (poss != -1 && pose != -1) {
            str = str.substr(poss, pose + 1);
        }
        else {
            str.clear();
        }
    }

}; // end namespace ini

