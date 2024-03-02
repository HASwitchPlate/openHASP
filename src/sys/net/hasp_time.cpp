/* MIT License - Copyright (c) 2019-2024 Francis Van Roie
   For full license information read the LICENSE file in the project folder */

#include <time.h>
#include <sys/time.h>

#include <Arduino.h>
#include "ArduinoLog.h"
#include "hasp_conf.h"
#include "hasp_time.h"

#include "hal/hasp_hal.h"
#include "hasp_debug.h"
#include "hasp_config.h"
#include "hasp/hasp_parser.h"

#if defined(ARDUINO_ARCH_ESP32)
#include "Preferences.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "esp_sntp.h"
#endif

#if defined(ARDUINO_ARCH_ESP32)
// These strings must be constant and kept in memory
String mytz((char*)0);
String ntp1((char*)0);
String ntp2((char*)0);
String ntp3((char*)0);

void timeSyncCallback(struct timeval* tv)
{
    LOG_VERBOSE(TAG_TIME, "NTP Synced: %s", ctime(&tv->tv_sec));
}
#endif

void timeSetup()
{
#if defined(ARDUINO_ARCH_ESP8266)
    LOG_WARNING(TAG_TIME, F("TIMEZONE: %s"), MYTZ);
    configTzTime(MYTZ, NTPSERVER1, NTPSERVER2, NTPSERVER3); // literal string
#endif
#if defined(ARDUINO_ARCH_ESP32)
    Preferences preferences;
    nvs_user_begin(preferences, "time", true);

    String zone((char*)0);
    zone = preferences.getString("zone", TIMEZONE);

    mytz        = time_zone_to_possix(zone.c_str());
    ntp1        = preferences.getString("ntp1", NTPSERVER1);
    ntp2        = preferences.getString("ntp2", NTPSERVER2);
    ntp3        = preferences.getString("ntp3", NTPSERVER3);
    bool enable = preferences.getBool("enable", true);
    bool dhcp   = preferences.getBool("dhcp", true);

    LOG_VERBOSE(TAG_TIME, F("%s => %s"), zone.c_str(), mytz.c_str());
    LOG_VERBOSE(TAG_TIME, F("NTP: %s %s %s"), ntp1.c_str(), ntp2.c_str(), ntp3.c_str());

    sntp_set_time_sync_notification_cb(timeSyncCallback);
    configTzTime(mytz.c_str(), ntp1.c_str(), ntp2.c_str(), ntp3.c_str());
    sntp_servermode_dhcp(enable && dhcp ? 1 : 0);
    preferences.end();
#endif
}

// https://github.com/nayarsystems/posix_tz_db
String time_zone_to_possix(const char* timezone)
{
#if defined(ARDUINO_ARCH_ESP32)
    uint16_t sdbm = 0;

    // sdbm doesn't parse numbers in the string; get the Etc/GMT hash from the offset to Greenwich
    if(timezone == strstr(timezone, "Etc/GMT")) { // startsWith Etc/GMT
        int offset           = atoi(timezone + 7);
        const uint16_t gmt[] = {TZ_ETC_GMT__14, TZ_ETC_GMT__13, TZ_ETC_GMT__12, TZ_ETC_GMT__11, TZ_ETC_GMT__10,
                                TZ_ETC_GMT__9,  TZ_ETC_GMT__8,  TZ_ETC_GMT__7,  TZ_ETC_GMT__6,  TZ_ETC_GMT__5,
                                TZ_ETC_GMT__4,  TZ_ETC_GMT__3,  TZ_ETC_GMT__2,  TZ_ETC_GMT__1,  TZ_ETC_GMT,
                                TZ_ETC_GMT1,    TZ_ETC_GMT2,    TZ_ETC_GMT3,    TZ_ETC_GMT4,    TZ_ETC_GMT5,
                                TZ_ETC_GMT6,    TZ_ETC_GMT7,    TZ_ETC_GMT8,    TZ_ETC_GMT9,    TZ_ETC_GMT10,
                                TZ_ETC_GMT11,   TZ_ETC_GMT12};
        int index            = offset + 14;
        if(index >= 0 && index < sizeof(gmt) / sizeof(gmt[0])) {
            sdbm = gmt[index];
            LOG_DEBUG(TAG_TIME, "Etc/GMT%d (%d)", offset, sdbm);
        } else {
            LOG_WARNING(TAG_TIME, "Invalid offset Etc/GMT%d", offset);
        }
    } else {
        sdbm = Parser::get_sdbm(timezone);
    }

    switch(sdbm) {
        case TZ_ANTARCTICA_TROLL:
            return "<+00>0<+02>-2,M3.5.0/1,M10.5.0/3";
        case TZ_AFRICA_CASABLANCA:
        case TZ_AFRICA_EL_AAIUN:
        case TZ_ETC_GMT__1:
            return "<+01>-1";
        case TZ_ETC_GMT__2:
            return "<+02>-2";
        case TZ_ANTARCTICA_SYOWA:
        case TZ_ASIA_ADEN:
        case TZ_ASIA_AMMAN:
        case TZ_ASIA_BAGHDAD:
        case TZ_ASIA_BAHRAIN:
        case TZ_ASIA_DAMASCUS:
        case TZ_ASIA_KUWAIT:
        case TZ_ASIA_QATAR:
        case TZ_ASIA_RIYADH:
        case TZ_ETC_GMT__3:
        case TZ_EUROPE_ISTANBUL:
        case TZ_EUROPE_MINSK:
            return "<+03>-3";
        case TZ_ASIA_TEHRAN:
            return "<+0330>-3:30";
        case TZ_ASIA_BAKU:
        case TZ_ASIA_DUBAI:
        case TZ_ASIA_MUSCAT:
        case TZ_ASIA_TBILISI:
        case TZ_ASIA_YEREVAN:
        case TZ_ETC_GMT__4:
        case TZ_EUROPE_ASTRAKHAN:
        case TZ_EUROPE_SAMARA:
        case TZ_EUROPE_SARATOV:
        case TZ_EUROPE_ULYANOVSK:
        case TZ_INDIAN_MAHE:
        case TZ_INDIAN_MAURITIUS:
        case TZ_INDIAN_REUNION:
            return "<+04>-4";
        case TZ_ASIA_KABUL:
            return "<+0430>-4:30";
        case TZ_ANTARCTICA_MAWSON:
        case TZ_ASIA_AQTAU:
        case TZ_ASIA_AQTOBE:
        case TZ_ASIA_ASHGABAT:
        case TZ_ASIA_ATYRAU:
        case TZ_ASIA_DUSHANBE:
        case TZ_ASIA_ORAL:
        case TZ_ASIA_QYZYLORDA:
        case TZ_ASIA_SAMARKAND:
        case TZ_ASIA_TASHKENT:
        case TZ_ASIA_YEKATERINBURG:
        case TZ_ETC_GMT__5:
        case TZ_INDIAN_KERGUELEN:
        case TZ_INDIAN_MALDIVES:
            return "<+05>-5";
        case TZ_ASIA_COLOMBO:
            return "<+0530>-5:30";
        case TZ_ASIA_KATHMANDU:
            return "<+0545>-5:45";
        case TZ_ANTARCTICA_VOSTOK:
        case TZ_ASIA_ALMATY:
        case TZ_ASIA_BISHKEK:
        case TZ_ASIA_DHAKA:
        case TZ_ASIA_OMSK:
        case TZ_ASIA_THIMPHU:
        case TZ_ASIA_URUMQI:
        case TZ_ETC_GMT__6:
        case TZ_INDIAN_CHAGOS:
            return "<+06>-6";
        case TZ_ASIA_YANGON:
        case TZ_INDIAN_COCOS:
            return "<+0630>-6:30";
        case TZ_ANTARCTICA_DAVIS:
        case TZ_ASIA_BANGKOK:
        case TZ_ASIA_BARNAUL:
        case TZ_ASIA_HO_CHI_MINH:
        case TZ_ASIA_HOVD:
        case TZ_ASIA_KRASNOYARSK:
        case TZ_ASIA_NOVOKUZNETSK:
        case TZ_ASIA_NOVOSIBIRSK:
        case TZ_ASIA_PHNOM_PENH:
        case TZ_ASIA_TOMSK:
        case TZ_ASIA_VIENTIANE:
        case TZ_ETC_GMT__7:
        case TZ_INDIAN_CHRISTMAS:
            return "<+07>-7";
        case TZ_ASIA_BRUNEI:
        case TZ_ASIA_CHOIBALSAN:
        case TZ_ASIA_IRKUTSK:
        case TZ_ASIA_KUALA_LUMPUR:
        case TZ_ASIA_KUCHING:
        case TZ_ASIA_SINGAPORE:
        case TZ_ASIA_ULAANBAATAR:
        case TZ_ETC_GMT__8:
            return "<+08>-8";
        case TZ_AUSTRALIA_EUCLA:
            return "<+0845>-8:45";
        case TZ_ASIA_CHITA:
        case TZ_ASIA_DILI:
        case TZ_ASIA_KHANDYGA:
        case TZ_ASIA_YAKUTSK:
        case TZ_ETC_GMT__9:
        case TZ_PACIFIC_PALAU:
            return "<+09>-9";
        case TZ_ANTARCTICA_DUMONTDURVILLE:
        case TZ_ASIA_UST__NERA:
        case TZ_ASIA_VLADIVOSTOK:
        case TZ_ETC_GMT__10:
        case TZ_PACIFIC_CHUUK:
        case TZ_PACIFIC_PORT_MORESBY:
            return "<+10>-10";
        case TZ_AUSTRALIA_LORD_HOWE:
            return "<+1030>-10:30<+11>-11,M10.1.0,M4.1.0";
        case TZ_ANTARCTICA_CASEY:
        case TZ_ASIA_MAGADAN:
        case TZ_ASIA_SAKHALIN:
        case TZ_ASIA_SREDNEKOLYMSK:
        case TZ_ETC_GMT__11:
        case TZ_PACIFIC_BOUGAINVILLE:
        case TZ_PACIFIC_EFATE:
        case TZ_PACIFIC_GUADALCANAL:
        case TZ_PACIFIC_KOSRAE:
        case TZ_PACIFIC_NOUMEA:
        case TZ_PACIFIC_POHNPEI:
            return "<+11>-11";
        case TZ_PACIFIC_NORFOLK:
            return "<+11>-11<+12>,M10.1.0,M4.1.0/3";
        case TZ_ASIA_ANADYR:
        case TZ_ASIA_KAMCHATKA:
        case TZ_ETC_GMT__12:
        case TZ_PACIFIC_FIJI:
        case TZ_PACIFIC_FUNAFUTI:
        case TZ_PACIFIC_KWAJALEIN:
        case TZ_PACIFIC_MAJURO:
        case TZ_PACIFIC_NAURU:
        case TZ_PACIFIC_TARAWA:
        case TZ_PACIFIC_WAKE:
        case TZ_PACIFIC_WALLIS:
            return "<+12>-12";
        case TZ_PACIFIC_CHATHAM:
            return "<+1245>-12:45<+1345>,M9.5.0/2:45,M4.1.0/3:45";
        case TZ_ETC_GMT__13:
        case TZ_PACIFIC_APIA:
        case TZ_PACIFIC_ENDERBURY:
        case TZ_PACIFIC_FAKAOFO:
        case TZ_PACIFIC_TONGATAPU:
            return "<+13>-13";
        case TZ_ETC_GMT__14:
        case TZ_PACIFIC_KIRITIMATI:
            return "<+14>-14";
        case TZ_ATLANTIC_CAPE_VERDE:
        case TZ_ETC_GMT1:
            return "<-01>1";
        case TZ_AMERICA_SCORESBYSUND:
        case TZ_ATLANTIC_AZORES:
            return "<-01>1<+00>,M3.5.0/0,M10.5.0/1";
        case TZ_AMERICA_NORONHA:
        case TZ_ATLANTIC_SOUTH_GEORGIA:
        case TZ_ETC_GMT2:
            return "<-02>2";
        case TZ_AMERICA_ARAGUAINA:
        case TZ_AMERICA_ARGENTINA_BUENOS_AIRES:
        case TZ_AMERICA_ARGENTINA_CATAMARCA:
        case TZ_AMERICA_ARGENTINA_CORDOBA:
        case TZ_AMERICA_ARGENTINA_JUJUY:
        case TZ_AMERICA_ARGENTINA_LA_RIOJA:
        case TZ_AMERICA_ARGENTINA_MENDOZA:
        case TZ_AMERICA_ARGENTINA_RIO_GALLEGOS:
        case TZ_AMERICA_ARGENTINA_SALTA:
        case TZ_AMERICA_ARGENTINA_SAN_JUAN:
        case TZ_AMERICA_ARGENTINA_SAN_LUIS:
        case TZ_AMERICA_ARGENTINA_TUCUMAN:
        case TZ_AMERICA_ARGENTINA_USHUAIA:
        case TZ_AMERICA_BAHIA:
        case TZ_AMERICA_BELEM:
        case TZ_AMERICA_CAYENNE:
        case TZ_AMERICA_FORTALEZA:
        case TZ_AMERICA_MACEIO:
        case TZ_AMERICA_MONTEVIDEO:
        case TZ_AMERICA_PARAMARIBO:
        case TZ_AMERICA_PUNTA_ARENAS:
        case TZ_AMERICA_RECIFE:
        case TZ_AMERICA_SANTAREM:
        case TZ_AMERICA_SAO_PAULO:
        case TZ_ANTARCTICA_PALMER:
        case TZ_ANTARCTICA_ROTHERA:
        case TZ_ATLANTIC_STANLEY:
        case TZ_ETC_GMT3:
            return "<-03>3";
        case TZ_AMERICA_MIQUELON:
            return "<-03>3<-02>,M3.2.0,M11.1.0";
        case TZ_AMERICA_NUUK:
        case TZ_AMERICA_GODTHAB:
            return "<-02>2<-01>,M3.5.0/-1,M10.5.0/0";
        case TZ_AMERICA_BOA_VISTA:
        case TZ_AMERICA_CAMPO_GRANDE:
        case TZ_AMERICA_CARACAS:
        case TZ_AMERICA_CUIABA:
        case TZ_AMERICA_GUYANA:
        case TZ_AMERICA_LA_PAZ:
        case TZ_AMERICA_MANAUS:
        case TZ_AMERICA_PORTO_VELHO:
        case TZ_ETC_GMT4:
            return "<-04>4";
        case TZ_AMERICA_ASUNCION:
            return "<-04>4<-03>,M10.1.0/0,M3.4.0/0";
        case TZ_AMERICA_SANTIAGO:
            return "<-04>4<-03>,M9.1.0/0,M4.1.0/0";
        case TZ_AMERICA_BOGOTA:
        case TZ_AMERICA_EIRUNEPE:
        case TZ_AMERICA_GUAYAQUIL:
        case TZ_AMERICA_LIMA:
        case TZ_AMERICA_RIO_BRANCO:
        case TZ_ETC_GMT5:
            return "<-05>5";
        case TZ_ETC_GMT6:
        case TZ_PACIFIC_GALAPAGOS:
            return "<-06>6";
        case TZ_PACIFIC_EASTER:
            return "<-06>6<-05>,M9.1.6/22,M4.1.6/22";
        case TZ_ETC_GMT7:
            return "<-07>7";
        case TZ_ETC_GMT8:
        case TZ_PACIFIC_PITCAIRN:
            return "<-08>8";
        case TZ_ETC_GMT9:
        case TZ_PACIFIC_GAMBIER:
            return "<-09>9";
        case TZ_PACIFIC_MARQUESAS:
            return "<-0930>9:30";
        case TZ_ETC_GMT10:
        case TZ_PACIFIC_RAROTONGA:
        case TZ_PACIFIC_TAHITI:
            return "<-10>10";
        case TZ_ETC_GMT11:
        case TZ_PACIFIC_NIUE:
            return "<-11>11";
        case TZ_ETC_GMT12:
            return "<-12>12";
        case TZ_AUSTRALIA_DARWIN:
            return "ACST-9:30";
        case TZ_AUSTRALIA_ADELAIDE:
        case TZ_AUSTRALIA_BROKEN_HILL:
            return "ACST-9:30ACDT,M10.1.0,M4.1.0/3";
        case TZ_AUSTRALIA_BRISBANE:
        case TZ_AUSTRALIA_LINDEMAN:
            return "AEST-10";
        case TZ_ANTARCTICA_MACQUARIE:
        case TZ_AUSTRALIA_CURRIE:
        case TZ_AUSTRALIA_HOBART:
        case TZ_AUSTRALIA_MELBOURNE:
        case TZ_AUSTRALIA_SYDNEY:
            return "AEST-10AEDT,M10.1.0,M4.1.0/3";
        case TZ_AMERICA_ANCHORAGE:
        case TZ_AMERICA_JUNEAU:
        case TZ_AMERICA_METLAKATLA:
        case TZ_AMERICA_NOME:
        case TZ_AMERICA_SITKA:
        case TZ_AMERICA_YAKUTAT:
            return "AKST9AKDT,M3.2.0,M11.1.0";
        case TZ_AMERICA_ANGUILLA:
        case TZ_AMERICA_ANTIGUA:
        case TZ_AMERICA_ARUBA:
        case TZ_AMERICA_BARBADOS:
        case TZ_AMERICA_BLANC__SABLON:
        case TZ_AMERICA_CURACAO:
        case TZ_AMERICA_DOMINICA:
        case TZ_AMERICA_GRENADA:
        case TZ_AMERICA_GUADELOUPE:
        case TZ_AMERICA_KRALENDIJK:
        case TZ_AMERICA_LOWER_PRINCES:
        case TZ_AMERICA_MARIGOT:
        case TZ_AMERICA_MARTINIQUE:
        case TZ_AMERICA_MONTSERRAT:
        case TZ_AMERICA_PORT_OF_SPAIN:
        case TZ_AMERICA_PUERTO_RICO:
        case TZ_AMERICA_SANTO_DOMINGO:
        case TZ_AMERICA_ST_BARTHELEMY:
        case TZ_AMERICA_ST_KITTS:
        case TZ_AMERICA_ST_LUCIA:
        case TZ_AMERICA_ST_THOMAS:
        case TZ_AMERICA_ST_VINCENT:
        case TZ_AMERICA_TORTOLA:
            return "AST4";
        case TZ_AMERICA_GLACE_BAY:
        case TZ_AMERICA_GOOSE_BAY:
        case TZ_AMERICA_HALIFAX:
        case TZ_AMERICA_MONCTON:
        case TZ_AMERICA_THULE:
        case TZ_ATLANTIC_BERMUDA:
            return "AST4ADT,M3.2.0,M11.1.0";
        case TZ_AUSTRALIA_PERTH:
            return "AWST-8";
        case TZ_AFRICA_BLANTYRE:
        case TZ_AFRICA_BUJUMBURA:
        case TZ_AFRICA_GABORONE:
        case TZ_AFRICA_HARARE:
        case TZ_AFRICA_JUBA:
        case TZ_AFRICA_KHARTOUM:
        case TZ_AFRICA_KIGALI:
        case TZ_AFRICA_LUBUMBASHI:
        case TZ_AFRICA_LUSAKA:
        case TZ_AFRICA_MAPUTO:
        case TZ_AFRICA_WINDHOEK:
            return "CAT-2";
        case TZ_AFRICA_ALGIERS:
        case TZ_AFRICA_TUNIS:
            return "CET-1";
        case TZ_AFRICA_CEUTA:
        case TZ_ARCTIC_LONGYEARBYEN:
        case TZ_EUROPE_AMSTERDAM:
        case TZ_EUROPE_ANDORRA:
        case TZ_EUROPE_BELGRADE:
        case TZ_EUROPE_BERLIN:
        case TZ_EUROPE_BRATISLAVA:
        case TZ_EUROPE_BRUSSELS:
        case TZ_EUROPE_BUDAPEST:
        case TZ_EUROPE_BUSINGEN:
        case TZ_EUROPE_COPENHAGEN:
        case TZ_EUROPE_GIBRALTAR:
        case TZ_EUROPE_LJUBLJANA:
        case TZ_EUROPE_LUXEMBOURG:
        case TZ_EUROPE_MADRID:
        case TZ_EUROPE_MALTA:
        case TZ_EUROPE_MONACO:
        case TZ_EUROPE_OSLO:
        case TZ_EUROPE_PARIS:
        case TZ_EUROPE_PODGORICA:
        case TZ_EUROPE_PRAGUE:
        case TZ_EUROPE_ROME:
        case TZ_EUROPE_SAN_MARINO:
        case TZ_EUROPE_SARAJEVO:
        case TZ_EUROPE_SKOPJE:
        case TZ_EUROPE_STOCKHOLM:
        case TZ_EUROPE_TIRANE:
        case TZ_EUROPE_VADUZ:
        case TZ_EUROPE_VATICAN:
        case TZ_EUROPE_VIENNA:
        case TZ_EUROPE_WARSAW:
        case TZ_EUROPE_ZAGREB:
        case TZ_EUROPE_ZURICH:
            return "CET-1CEST,M3.5.0,M10.5.0/3";
        case TZ_PACIFIC_GUAM:
        case TZ_PACIFIC_SAIPAN:
            return "ChST-10";
        case TZ_AMERICA_HAVANA:
            return "CST5CDT,M3.2.0/0,M11.1.0/1";
        case TZ_AMERICA_BAHIA_BANDERAS:
        case TZ_AMERICA_BELIZE:
        case TZ_AMERICA_CHIHUAHUA:
        case TZ_AMERICA_COSTA_RICA:
        case TZ_AMERICA_EL_SALVADOR:
        case TZ_AMERICA_GUATEMALA:
        case TZ_AMERICA_MANAGUA:
        case TZ_AMERICA_MERIDA:
        case TZ_AMERICA_MEXICO_CITY:
        case TZ_AMERICA_MONTERREY:
        case TZ_AMERICA_REGINA:
        case TZ_AMERICA_SWIFT_CURRENT:
        case TZ_AMERICA_TEGUCIGALPA:
            return "CST6";
        case TZ_AMERICA_CHICAGO:
        case TZ_AMERICA_INDIANA_KNOX:
        case TZ_AMERICA_INDIANA_TELL_CITY:
        case TZ_AMERICA_MATAMOROS:
        case TZ_AMERICA_MENOMINEE:
        case TZ_AMERICA_NORTH_DAKOTA_BEULAH:
        case TZ_AMERICA_NORTH_DAKOTA_CENTER:
        case TZ_AMERICA_NORTH_DAKOTA_NEW_SALEM:
        case TZ_AMERICA_OJINAGA:
        case TZ_AMERICA_RAINY_RIVER:
        case TZ_AMERICA_RANKIN_INLET:
        case TZ_AMERICA_RESOLUTE:
        case TZ_AMERICA_WINNIPEG:
            return "CST6CDT,M3.2.0,M11.1.0";
        case TZ_ASIA_MACAU:
        case TZ_ASIA_SHANGHAI:
        case TZ_ASIA_TAIPEI:
            return "CST-8";
        case TZ_AFRICA_ADDIS_ABABA:
        case TZ_AFRICA_ASMARA:
        case TZ_AFRICA_DAR_ES_SALAAM:
        case TZ_AFRICA_DJIBOUTI:
        case TZ_AFRICA_KAMPALA:
        case TZ_AFRICA_MOGADISHU:
        case TZ_AFRICA_NAIROBI:
        case TZ_INDIAN_ANTANANARIVO:
        case TZ_INDIAN_COMORO:
        case TZ_INDIAN_MAYOTTE:
            return "EAT-3";
        case TZ_AFRICA_TRIPOLI:
        case TZ_EUROPE_KALININGRAD:
            return "EET-2";
        case TZ_AFRICA_CAIRO:
            return "EET-2EEST,M4.5.5/0,M10.5.4/24";
        case TZ_EUROPE_CHISINAU:
            return "EET-2EEST,M3.5.0,M10.5.0/3";
        case TZ_ASIA_BEIRUT:
            return "EET-2EEST,M3.5.0/0,M10.5.0/0";
        case TZ_ASIA_FAMAGUSTA:
        case TZ_ASIA_NICOSIA:
        case TZ_EUROPE_ATHENS:
        case TZ_EUROPE_BUCHAREST:
        case TZ_EUROPE_HELSINKI:
        case TZ_EUROPE_KIEV:
        case TZ_EUROPE_MARIEHAMN:
        case TZ_EUROPE_RIGA:
        case TZ_EUROPE_SOFIA:
        case TZ_EUROPE_TALLINN:
        case TZ_EUROPE_UZHGOROD:
        case TZ_EUROPE_VILNIUS:
        case TZ_EUROPE_ZAPOROZHYE:
            return "EET-2EEST,M3.5.0/3,M10.5.0/4";
        case TZ_ASIA_GAZA:
        case TZ_ASIA_HEBRON:
            return "EET-2EEST,M3.4.4/50,M10.4.4/50";
        case TZ_AMERICA_ATIKOKAN:
        case TZ_AMERICA_CANCUN:
        case TZ_AMERICA_CAYMAN:
        case TZ_AMERICA_JAMAICA:
        case TZ_AMERICA_PANAMA:
            return "EST5";
        case TZ_AMERICA_DETROIT:
        case TZ_AMERICA_GRAND_TURK:
        case TZ_AMERICA_INDIANA_INDIANAPOLIS:
        case TZ_AMERICA_INDIANA_MARENGO:
        case TZ_AMERICA_INDIANA_PETERSBURG:
        case TZ_AMERICA_INDIANA_VEVAY:
        case TZ_AMERICA_INDIANA_VINCENNES:
        case TZ_AMERICA_INDIANA_WINAMAC:
        case TZ_AMERICA_IQALUIT:
        case TZ_AMERICA_KENTUCKY_LOUISVILLE:
        case TZ_AMERICA_KENTUCKY_MONTICELLO:
        case TZ_AMERICA_MONTREAL:
        case TZ_AMERICA_NASSAU:
        case TZ_AMERICA_NEW_YORK:
        case TZ_AMERICA_NIPIGON:
        case TZ_AMERICA_PANGNIRTUNG:
        case TZ_AMERICA_PORT__AU__PRINCE:
        case TZ_AMERICA_THUNDER_BAY:
        case TZ_AMERICA_TORONTO:
            return "EST5EDT,M3.2.0,M11.1.0";
        case TZ_AFRICA_ABIDJAN:
        case TZ_AFRICA_ACCRA:
        case TZ_AFRICA_BAMAKO:
        case TZ_AFRICA_BANJUL:
        case TZ_AFRICA_BISSAU:
        case TZ_AFRICA_CONAKRY:
        case TZ_AFRICA_DAKAR:
        case TZ_AFRICA_FREETOWN:
        case TZ_AFRICA_LOME:
        case TZ_AFRICA_MONROVIA:
        case TZ_AFRICA_NOUAKCHOTT:
        case TZ_AFRICA_OUAGADOUGOU:
        case TZ_AFRICA_SAO_TOME:
        case TZ_AMERICA_DANMARKSHAVN:
        case TZ_ATLANTIC_REYKJAVIK:
        case TZ_ATLANTIC_ST_HELENA:
        case TZ_ETC_GMT:
        case TZ_ETC_GMT0:
        case TZ_ETC_GMT_0:
        case TZ_ETC_GMT__0:
        case TZ_ETC_GREENWICH:
            return "GMT0";
        case TZ_EUROPE_GUERNSEY:
        case TZ_EUROPE_ISLE_OF_MAN:
        case TZ_EUROPE_JERSEY:
        case TZ_EUROPE_LONDON:
            return "GMT0BST,M3.5.0/1,M10.5.0";
        case TZ_EUROPE_DUBLIN:
            return "GMT0IST,M3.5.0/1,M10.5.0";
        case TZ_ASIA_HONG_KONG:
            return "HKT-8";
        case TZ_PACIFIC_HONOLULU:
            return "HST10";
        case TZ_AMERICA_ADAK:
            return "HST10HDT,M3.2.0,M11.1.0";
        case TZ_ASIA_JERUSALEM:
            return "IST-2IDT,M3.5.5,M10.5.0";
        case TZ_ASIA_KOLKATA:
            return "IST-5:30";
        case TZ_ASIA_TOKYO:
            return "JST-9";
        case TZ_ASIA_PYONGYANG:
        case TZ_ASIA_SEOUL:
            return "KST-9";
        case TZ_EUROPE_KIROV:
        case TZ_EUROPE_MOSCOW:
        case TZ_EUROPE_SIMFEROPOL:
        case TZ_EUROPE_VOLGOGRAD:
            return "MSK-3";
        case TZ_AMERICA_CRESTON:
        case TZ_AMERICA_DAWSON:
        case TZ_AMERICA_DAWSON_CREEK:
        case TZ_AMERICA_FORT_NELSON:
        case TZ_AMERICA_HERMOSILLO:
        case TZ_AMERICA_MAZATLAN:
        case TZ_AMERICA_PHOENIX:
        case TZ_AMERICA_WHITEHORSE:
            return "MST7";
        case TZ_AMERICA_BOISE:
        case TZ_AMERICA_CAMBRIDGE_BAY:
        case TZ_AMERICA_DENVER:
        case TZ_AMERICA_EDMONTON:
        case TZ_AMERICA_INUVIK:
        case TZ_AMERICA_YELLOWKNIFE:
            return "MST7MDT,M3.2.0,M11.1.0";
        case TZ_AMERICA_ST_JOHNS:
            return "NST3:30NDT,M3.2.0,M11.1.0";
        case TZ_ANTARCTICA_MCMURDO:
        case TZ_PACIFIC_AUCKLAND:
            return "NZST-12NZDT,M9.5.0,M4.1.0/3";
        case TZ_ASIA_KARACHI:
            return "PKT-5";
        case TZ_ASIA_MANILA:
            return "PST-8";
        case TZ_AMERICA_LOS_ANGELES:
        case TZ_AMERICA_TIJUANA:
        case TZ_AMERICA_VANCOUVER:
            return "PST8PDT,M3.2.0,M11.1.0";
        case TZ_AFRICA_JOHANNESBURG:
        case TZ_AFRICA_MASERU:
        case TZ_AFRICA_MBABANE:
            return "SAST-2";
        case TZ_PACIFIC_MIDWAY:
        case TZ_PACIFIC_PAGO_PAGO:
            return "SST11";
        case TZ_ETC_UCT:
        case TZ_ETC_UNIVERSAL:
        case TZ_ETC_UTC:
        case TZ_ETC_ZULU:
            return "UTC0";
        case TZ_AFRICA_BANGUI:
        case TZ_AFRICA_BRAZZAVILLE:
        case TZ_AFRICA_DOUALA:
        case TZ_AFRICA_KINSHASA:
        case TZ_AFRICA_LAGOS:
        case TZ_AFRICA_LIBREVILLE:
        case TZ_AFRICA_LUANDA:
        case TZ_AFRICA_MALABO:
        case TZ_AFRICA_NDJAMENA:
        case TZ_AFRICA_NIAMEY:
        case TZ_AFRICA_PORTO__NOVO:
            return "WAT-1";
        case TZ_ATLANTIC_CANARY:
        case TZ_ATLANTIC_FAROE:
        case TZ_ATLANTIC_MADEIRA:
        case TZ_EUROPE_LISBON:
            return "WET0WEST,M3.5.0/1,M10.5.0";
        case TZ_ASIA_JAKARTA:
        case TZ_ASIA_PONTIANAK:
            return "WIB-7";
        case TZ_ASIA_JAYAPURA:
            return "WIT-9";
        case TZ_ASIA_MAKASSAR:
            return "WITA-8";
    }
#endif
    LOG_WARNING(TAG_TIME, F("Timezone %s (%d) not found, using %s"), timezone, sdbm, TIMEZONE);
    return TIMEZONE;
}

/* ===== Read/Write Configuration ===== */
#if HASP_USE_CONFIG > 0
bool timeGetConfig(const JsonObject& settings)
{
    Preferences preferences;
    bool changed = false;

    nvs_user_begin(preferences, "time", true);
    settings["zone"]   = preferences.getString("zone", TIMEZONE);
    settings["region"] = preferences.getString("region", "etc");
    settings["ntp"][0] = preferences.getString("ntp1", NTPSERVER1);
    settings["ntp"][1] = preferences.getString("ntp2", NTPSERVER2);
    settings["ntp"][2] = preferences.getString("ntp3", NTPSERVER3);
    preferences.end();

#if ESP_ARDUINO_VERSION_MAJOR >= 2
    nvs_iterator_t it = nvs_entry_find("config", "time", NVS_TYPE_ANY);
    while(it != NULL) {
        nvs_entry_info_t info;
        nvs_entry_info(it, &info);
        it = nvs_entry_next(it);
        printf("key '%s', type '%d' \n", info.key, info.type);
    };
#endif

    if(changed) configOutput(settings, TAG_TIME);

    return changed;
}

bool timeSetConfig(const JsonObject& settings)
{
    Preferences preferences;
    nvs_user_begin(preferences, "time", false);

    configOutput(settings, TAG_TIME);
    bool changed = false;

    changed |= nvsUpdateString(preferences, "zone", settings["zone"]);
    changed |= nvsUpdateString(preferences, "region", settings["region"]);
    changed |= nvsUpdateString(preferences, "ntp1", settings["ntp"][0]);
    changed |= nvsUpdateString(preferences, "ntp2", settings["ntp"][1]);
    changed |= nvsUpdateString(preferences, "ntp3", settings["ntp"][2]);

    preferences.end();
    timeSetup();

    return changed;
}
#endif