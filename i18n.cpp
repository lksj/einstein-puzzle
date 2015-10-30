#include "i18n.h"
#include <locale.h>
#include "unicode.h"
#include "convert.h"

#ifdef WIN32
#include <windows.h>
//#include <winnls.h>
#endif


Locale locale;


#ifdef WIN32

static struct _CountryMap {
  wchar_t iso2[3];
  char iso3[4];
} countries[] = {
        { L"AF", "AFG" },
        { L"AL", "ALB" },
        { L"DZ", "DZA" },
        { L"AS", "ASM" },
        { L"AD", "AND" },
        { L"AO", "AGO" },
        { L"AI", "AIA" },
        { L"AQ", "ATA" },
        { L"AG", "ATG" },
        { L"AR", "ARG" },
        { L"AM", "ARM" },  
        { L"AW", "ABW" },
        { L"AU", "AUS" },
        { L"AT", "AUT" },
        { L"AZ", "AZE" },  
        { L"BS", "BHS" },
        { L"BH", "BHR" },
        { L"BD", "BGD" },
        { L"BB", "BRB" },
        { L"BY", "BLR" },  
        { L"BE", "BEL" },
        { L"BZ", "BLZ" },
        { L"BJ", "BEN" },
        { L"BM", "BMU" },
        { L"BT", "BTN" },
        { L"BO", "BOL" },
        { L"BA", "BIH" },
        { L"BW", "BWA" },
        { L"BV", "BVT" },
        { L"BR", "BRA" },
        { L"IO", "IOT" },
        { L"BN", "BRN" },
        { L"BG", "BGR" },
        { L"BF", "BFA" },
        { L"BI", "BDI" },
        { L"KH", "KHM" },
        { L"CM", "CMR" },
        { L"CA", "CAN" },
        { L"CV", "CPV" },
        { L"KY", "CYM" },
        { L"CF", "CAF" },
        { L"TD", "TCD" },
        { L"CL", "CHL" },
        { L"CN", "CHN" },
        { L"CX", "CXR" },
        { L"CC", "CCK" },
        { L"CO", "COL" },
        { L"KM", "COM" },
        { L"CD", "COD" },
        { L"CG", "COG" },
        { L"CK", "COK" },
        { L"CR", "CRI" },
        { L"CI", "CIV" },
        { L"HR", "HRV" },      
        { L"CU", "CUB" },
        { L"CY", "CYP" },
        { L"CZ", "CZE" },  
        { L"DK", "DNK" },
        { L"DJ", "DJI" },
        { L"DM", "DMA" },
        { L"DO", "DOM" },
        { L"TL", "TLS" },
        { L"EC", "ECU" },
        { L"EG", "EGY" },
        { L"SV", "SLV" },
        { L"GQ", "GNQ" },
        { L"ER", "ERI" },
        { L"EE", "EST" },  
        { L"ET", "ETH" },
        { L"FK", "FLK" },
        { L"FO", "FRO" },
        { L"FJ", "FJI" },
        { L"FI", "FIN" },
        { L"FR", "FRA" },
        { L"FX", "FXX" },  
        { L"GF", "GUF" },
        { L"PF", "PYF" },
        { L"TF", "ATF" },
        { L"GA", "GAB" },
        { L"GM", "GMB" },
        { L"GE", "GEO" },  
        { L"DE", "DEU" },
        { L"GH", "GHA" },
        { L"GI", "GIB" },
        { L"GR", "GRC" },
        { L"GL", "GRL" },
        { L"GD", "GRD" },
        { L"GP", "GLP" },
        { L"GU", "GUM" },
        { L"GT", "GTM" },
        { L"GN", "GIN" },
        { L"GW", "GNB" },
        { L"GY", "GUY" },
        { L"HT", "HTI" },
        { L"HM", "HMD" },
        { L"HN", "HND" },
        { L"HK", "HKG" },
        { L"HU", "HUN" },
        { L"IS", "ISL" },
        { L"IN", "IND" },
        { L"ID", "IDN" },
        { L"IR", "IRN" },
        { L"IQ", "IRQ" },
        { L"IE", "IRL" },
        { L"IL", "ISR" },
        { L"IT", "ITA" },
        { L"JM", "JAM" },
        { L"JP", "JPN" },
        { L"JO", "JOR" },
        { L"KZ", "KAZ" },  
        { L"KE", "KEN" },
        { L"KI", "KIR" },
        { L"KP", "PRK" },
        { L"KR", "KOR" },
        { L"KW", "KWT" },
        { L"KG", "KGZ" },  
        { L"LA", "LAO" },
        { L"LV", "LVA" },  
        { L"LB", "LBN" },
        { L"LS", "LSO" },
        { L"LR", "LBR" },
        { L"LY", "LBY" },
        { L"LI", "LIE" },
        { L"LT", "LTU" },  
        { L"LU", "LUX" },
        { L"MO", "MAC" },
        { L"MK", "MKD" }, 
        { L"MG", "MDG" },
        { L"MW", "MWI" },
        { L"MY", "MYS" },
        { L"MV", "MDV" },
        { L"ML", "MLI" },
        { L"MT", "MLT" },
        { L"MH", "MHL" },
        { L"MQ", "MTQ" },
        { L"MR", "MRT" },
        { L"MU", "MUS" },
        { L"YT", "MYT" },  
        { L"MX", "MEX" },
        { L"FM", "FSM" },
        { L"MD", "MDA" },  
        { L"MC", "MCO" },
        { L"MN", "MNG" },
        { L"MS", "MSR" },
        { L"MA", "MAR" },
        { L"MZ", "MOZ" },
        { L"MM", "MMR" },
        { L"NA", "NAM" },
        { L"NR", "NRU" },
        { L"NP", "NPL" },
        { L"NL", "NLD" },
        { L"AN", "ANT" },
        { L"NC", "NCL" },
        { L"NZ", "NZL" },
        { L"NI", "NIC" },
        { L"NE", "NER" },
        { L"NG", "NGA" },
        { L"NU", "NIU" },
        { L"NF", "NFK" },
        { L"MP", "MNP" },
        { L"NO", "NOR" },
        { L"OM", "OMN" },
        { L"PK", "PAK" },
        { L"PW", "PLW" },
        { L"PS", "PSE" },
        { L"PA", "PAN" },
        { L"PG", "PNG" },
        { L"PY", "PRY" },
        { L"PE", "PER" },
        { L"PH", "PHL" },
        { L"PN", "PCN" },
        { L"PL", "POL" },
        { L"PT", "PRT" },
        { L"PR", "PRI" },
        { L"QA", "QAT" },
        { L"RE", "REU" },
        { L"RO", "ROU" },
        { L"RU", "RUS" },
        { L"RW", "RWA" },
        { L"KN", "KNA" },
        { L"LC", "LCA" },
        { L"VC", "VCT" },
        { L"WS", "WSM" },
        { L"SM", "SMR" },
        { L"ST", "STP" },
        { L"SA", "SAU" },
        { L"SN", "SEN" },
        { L"SC", "SYC" },
        { L"SL", "SLE" },
        { L"SG", "SGP" },
        { L"SK", "SVK" },  
        { L"SI", "SVN" },  
        { L"SB", "SLB" },
        { L"SO", "SOM" },
        { L"ZA", "ZAF" },
        { L"GS", "SGS" },
        { L"ES", "ESP" },
        { L"LK", "LKA" },
        { L"SH", "SHN" },
        { L"PM", "SPM" },
        { L"SD", "SDN" },
        { L"SR", "SUR" },
        { L"SJ", "SJM" },
        { L"SZ", "SWZ" },
        { L"SE", "SWE" },
        { L"CH", "CHE" },
        { L"SY", "SYR" },
        { L"TW", "TWN" },
        { L"TJ", "TJK" },  
        { L"TZ", "TZA" },
        { L"TH", "THA" },
        { L"TG", "TGO" },
        { L"TK", "TKL" },
        { L"TO", "TON" },
        { L"TT", "TTO" },
        { L"TN", "TUN" },
        { L"TR", "TUR" },
        { L"TM", "TKM" },  
        { L"TC", "TCA" },
        { L"TV", "TUV" },
        { L"UG", "UGA" },
        { L"UA", "UKR" },
        { L"AE", "ARE" },
        { L"GB", "GBR" },
        { L"US", "USA" },
        { L"UM", "UMI" },
        { L"UY", "URY" },
        { L"UZ", "UZB" },  
        { L"VU", "VUT" },
        { L"VA", "VAT" },
        { L"VE", "VEN" },
        { L"VN", "VNM" },
        { L"VG", "VGB" },
        { L"VI", "VIR" },
        { L"WF", "WLF" },
        { L"EH", "ESH" },
        { L"YE", "YEM" },
        { L"YU", "YUG" },
        { L"ZM", "ZMB" },
        { L"ZW", "ZWE" },
        { L"",   "" }
    };

static wchar_t* mapIso3ContryToIso2(char *iso3) 
{
    if (! iso3) 
        return L"";
    
    struct _CountryMap *m = countries;
    while (m && m->iso3[0]) {
        if (! strcmp(iso3, m->iso3))
            return m->iso2;
        m++;
    }
    
    return L"";
}
    
#endif


Locale::Locale()
{
#ifndef WIN32
    parseLocale(fromMbcs(setlocale(LC_ALL, "")));
#else
    setlocale(LC_ALL, "");

    char buf[100];
    int len;
    len = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVCTRYNAME, buf, 99);
    if (len > 0)
        country = mapIso3ContryToIso2(buf);
    
    len = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_SABBREVLANGNAME, buf, 99);
    if (len > 0) {
        /* according to MSDN:
           LOCALE_SABBREVLANGNAME   Abbreviated name of the language, 
           created by taking the 2-letter language abbreviation from the 
           ISO Standard 639 and adding a third letter, as appropriate, 
           to indicate the sublanguage.
         */
         if (len == 4)       // exclude subvariant letter
            buf[2] = 0;
        language = toLowerCase(fromMbcs(buf));
    }
    
    len = GetLocaleInfo(LOCALE_USER_DEFAULT, LOCALE_IDEFAULTANSICODEPAGE, buf, 99);
    if (len > 0)
        encoding = L"CP" + std::wstring(fromMbcs(buf));  // FIXME!
#endif

    setlocale(LC_NUMERIC, "C");  // hack because of numbers in Lua
}


Locale::Locale(const Locale &locale): language(locale.language),
    country(locale.country), encoding(locale.encoding)
{
}

void Locale::parseLocale(const std::wstring &name)
{
    int pos = name.find(L'.');
    std::wstring langAndCountry;
    if (pos >= 0) {
        encoding = name.substr(pos + 1);
        langAndCountry = name.substr(0, pos);
    } else {
        encoding = L"";
        langAndCountry = name;
    }
    pos = langAndCountry.find(L'_');
    if (pos < 0) {
        language = langAndCountry;
        country = L"";
    } else {
        language = langAndCountry.substr(0, pos);
        country = langAndCountry.substr(pos + 1);
    }
    language = toLowerCase(language);
    country = toUpperCase(country);
    encoding = toUpperCase(encoding);
}


static bool isLowerCase(const std::wstring &s)
{
    int len = s.length();
    for (int i = 0; i < len; i++) {
        char ch = s[i];
        if ((ch < L'a') || (ch > L'z'))
            return false;
    }
    return true;
}


static bool isUpperCase(const std::wstring &s)
{
    int len = s.length();
    for (int i = 0; i < len; i++) {
        char ch = s[i];
        if ((ch < L'A') || (ch > L'Z'))
            return false;
    }
    return true;
}


void splitFileName(const std::wstring &fileName, std::wstring &name,
        std::wstring &ext, std::wstring &lang, std::wstring &country)
{
    int pos = fileName.find_last_of(L'.');
    if (pos <= 0) {
        ext = L"";
        name = fileName;
    } else {
        name = fileName.substr(0, pos);
        ext = fileName.substr(pos + 1);
    }

    pos = name.find_last_of('_');
    if ((pos <= 0) || (name.length() - pos != 3)) {
        lang = L"";
        country = L"";
    } else {
        std::wstring l = name.substr(pos + 1);
        std::wstring s = name.substr(0, pos);
        if (isUpperCase(l)) { // country
            name = s;
            country = l;
            pos = name.find_last_of('_');
            if ((pos <= 0) || (name.length() - pos != 3))
                lang = L"";
            else {
                std::wstring l = name.substr(pos + 1);
                std::wstring s = name.substr(0, pos);
                if (isLowerCase(l)) { // language
                    name = s;
                    lang = l;
                } else // invalid
                    lang = L"";
            }
        } else if (isLowerCase(l))  { // language
            name = s;
            lang = l;
            country = L"";
        } else { // invalid
            lang = L"";
            country = L"";
        }
    }
}

int getScore(const std::wstring &lang, const std::wstring &country,
        const Locale &locale)
{
    if ((! country.length()) && (! lang.length()))   // locale independent
        return 1;

    int score = 0;
    if (locale.getCountry().length() && (locale.getCountry() == country))
        score += 2;
    if (locale.getLanguage().length() && (locale.getLanguage() == lang))
        score += 4;

    return score;
}

