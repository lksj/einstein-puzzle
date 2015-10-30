#ifdef WIN32                    // Win32 only
#include "regstorage.h"
#include "unicode.h"


RegistryStorage::RegistryStorage()
{
    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, 
                "SOFTWARE\\Flowix Games\\Einstein\\2.0",
                0, KEY_READ, &globalKey))
        globalKey = NULL;
    if (RegOpenKeyEx(HKEY_CURRENT_USER, 
                "SOFTWARE\\Flowix Games\\Einstein\\2.0",
                0, KEY_READ | KEY_WRITE, &userKey))
    {
        if (RegCreateKeyEx(HKEY_CURRENT_USER, 
                    "SOFTWARE\\Flowix Games\\Einstein\\2.0", 0, NULL, 
                    REG_OPTION_NON_VOLATILE, KEY_READ | KEY_WRITE,
                    NULL, &userKey, NULL))
            userKey = NULL;
    }
}

RegistryStorage::~RegistryStorage()
{
    if (globalKey)
        RegCloseKey(globalKey);
    if (userKey)
        RegCloseKey(userKey);
}

int RegistryStorage::get(const std::wstring &name, int dflt)
{
    std::string uname(toUtf8(name));
    
    if (globalKey) {
        DWORD data;
        DWORD size = sizeof(data);
        DWORD type;
        if (! RegQueryValueEx(globalKey, uname.c_str(), NULL, &type,
                    (BYTE*)&data, &size))
            if (type == REG_DWORD)
                return data;
    }
    
    if (userKey) {
        DWORD data;
        DWORD size = sizeof(data);
        DWORD type;
        if (! RegQueryValueEx(userKey, uname.c_str(), NULL, &type,
                    (BYTE*)&data, &size))
            if (type == REG_DWORD)
                return data;
    }
    
    return dflt;
}

std::wstring RegistryStorage::get(const std::wstring &name, const std::wstring &dflt)
{
    std::string uname(toUtf8(name));
    
    if (globalKey) {
        DWORD size = 0;
        DWORD type;
        if (! RegQueryValueEx(globalKey, uname.c_str(), NULL, &type,
                    NULL, &size)) 
        {
            if ((type == REG_SZ) && (size > 0)) {
                char *data = new char[size + 1];
                if (! RegQueryValueEx(globalKey, uname.c_str(), NULL, &type,
                            (BYTE*)data, &size)) 
                {
                    std::wstring s(fromUtf8(data));
                    delete[] data;
                    return s;
                }
                delete[] data;
            } else
                return L"";
        }
    }
    
    if (userKey) {
        DWORD size = 0;
        DWORD type;
        if (! RegQueryValueEx(userKey, uname.c_str(), NULL, &type,
                    NULL, &size)) 
        {
            if ((type == REG_SZ) && (size > 0)) {
                char *data = new char[size];
                if (! RegQueryValueEx(userKey, uname.c_str(), NULL, &type,
                            (BYTE*)data, &size)) 
                {
                    std::wstring s(fromUtf8(data));
                    delete[] data;
                    return s;
                }
                delete[] data;
            } else
                return L"";
        }
    }
    
    return dflt;
}

void RegistryStorage::set(const std::wstring &name, int value)
{
    std::string uname(toUtf8(name));
    
    if (userKey) {
        DWORD data = value;
        RegSetValueEx(userKey, uname.c_str(), 0, REG_DWORD, 
                (BYTE*)&data, sizeof(data));
    }
}

void RegistryStorage::set(const std::wstring &name, const std::wstring &value)
{
    std::string uname(toUtf8(name));
    std::string uval(toUtf8(value));

    if (userKey)
        RegSetValueEx(userKey, uname.c_str(), 0, REG_SZ, 
                (BYTE*)uval.c_str(), (uval.length() + 1) * sizeof(char));
}


#endif

