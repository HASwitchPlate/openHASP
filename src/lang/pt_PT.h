// This translation file is maintained on https://crowdin.com/project/openhasp/
// Do not edit directly!

#ifndef HASP_LANG_PT_PT_H
#define HASP_LANG_PT_PT_H

#define D_ISO_LANG_CODE "pt-PT"

#define D_USERNAME "Utilizador:"
#define D_PASSWORD "Palavra-passe:"
#define D_SSID "SSID:"
#define D_YES "Sim"
#define D_NO "Não"

#define D_ERROR_OUT_OF_MEMORY "Memória Cheia"
#define D_ERROR_UNKNOWN "Erro desconhecido"

#define D_CONFIG_NOT_CHANGED "Sem alterações na configuração"
#define D_CONFIG_CHANGED "Configuração alterada"
#define D_CONFIG_LOADED "Configuração carregada"

#define D_FILE_LOADING "A carregar %s"
#define D_FILE_LOADED "%s carregado"
#define D_FILE_LOAD_FAILED "Não foi possível carregar %s"
#define D_FILE_SAVING "A guardar %s"
#define D_FILE_SAVED "%s guardado"
#define D_FILE_SAVE_FAILED "Não foi possível guardar %s"
#define D_FILE_NOT_FOUND "Ficheiro não encontrado"
#define D_FILE_SIZE_BYTES "bytes"   // new
#define D_FILE_SIZE_KILOBYTES "KiB" // new
#define D_FILE_SIZE_MEGABYTES "MiB" // new
#define D_FILE_SIZE_GIGABYTES "GiB" // new
#define D_FILE_SIZE_TERABYTES "TiB"
#define D_FILE_SIZE_DIVIDER 1024    // new, kibi or kilo bytes
#define D_DECIMAL_POINT "."         // new, decimal comma or point

#define D_SERVICE_STARTING "A inicializar..."
#define D_SERVICE_STARTED "Iniciado"
#define D_SERVICE_START_FAILED "Não foi possível iniciar"
#define D_SERVICE_STOPPED "Parado"
#define D_SERVICE_DISABLED "Desativado"
#define D_SERVICE_CONNECTED "Ligado"
#define D_SERVICE_DISCONNECTED "Desligado"

#define D_SETTING_ENABLED "Ativado"
#define D_SETTING_DISABLED "Desativado"
#define D_SETTING_DEFAULT "Default" // New

#define D_NETWORK_IP_ADDRESS_RECEIVED "Foi atribuído endereço IP: %s"
#define D_NETWORK_ONLINE "Online"
#define D_NETWORK_OFFLINE "Offline"
#define D_NETWORK_CONNECTION_FAILED "Falhou a ligação"
#define D_NETWORK_CONNECTION_UNAUTHORIZED "Falhou a autorização"

#define D_MQTT_DEFAULT_NAME "placa_%s"
#define D_MQTT_CONNECTING "A ligar..."
#define D_MQTT_CONNECTED "Ligado ao broker %s com clientID %s"
#define D_MQTT_NOT_CONNECTED "Não há ligação ???"
#define D_MQTT_DISCONNECTING "A desligar..."
#define D_MQTT_DISCONNECTED "Desligado"
#define D_MQTT_RECONNECTING "Desligado do broker, a religar..."
#define D_MQTT_NOT_CONFIGURED "O Broker não foi configurado"
#define D_MQTT_STARTED "A iniciar: %d bytes"
#define D_MQTT_FAILED "Falhou:"
#define D_MQTT_INVALID_TOPIC "A mensagem tem um tópico inválido"
#define D_MQTT_SUBSCRIBED "Subscrito a %s"
#define D_MQTT_NOT_SUBSCRIBED "Não foi possível subscrever %s"
#define D_MQTT_HA_AUTO_DISCOVERY "A registar auto-descoberta no HA"
#define D_MQTT_PAYLOAD_TOO_LONG "Os dados são demasiado grandes(%u bytes)"

#define D_TELNET_CLOSING_CONNECTION "A fechar a ligação de %s"
#define D_TELNET_CLIENT_LOGIN_FROM "Foi feito login ao cliente %s"
#define D_TELNET_CLIENT_CONNECT_FROM "Foi conectado ao cliente %s"
#define D_TELNET_CLIENT_NOT_CONNECTED "Cliente não ligado"
#define D_TELNET_INCORRECT_LOGIN_ATTEMPT "Tentativa de ligação incorreta desde %s"
#define D_TELNET_STARTED "Consola inicializada"
#define D_TELNET_FAILED "Falhou inicialização da consola"
#define D_TELNET_CLIENT_CONNECTED "Cliente ligado"
#define D_TELNET_CLIENT_REJECTED "Cliente rejeitado"

#define D_HASP_INVALID_PAGE "Página inválida %u"
#define D_HASP_INVALID_LAYER "Não foi possível eliminar a camada de sistema"
#define D_HASP_CHANGE_PAGE "A alterar a página %u"
#define D_HASP_CLEAR_PAGE "A limpar página %u"

#define D_OBJECT_DELETED "Objeto eliminado"
#define D_OBJECT_UNKNOWN "Objeto desconhecido"
#define D_OBJECT_MISMATCH "Os objetos não são iguais!"
#define D_OBJECT_LOST "Objeto perdido!"
#define D_OBJECT_CREATE_FAILED "Não foi possível criar o objeto %u"
#define D_OBJECT_PAGE_UNKNOWN "A página ID %u não está definida"
#define D_OBJECT_EVENT_UNKNOWN "Não se conhece o evento %d "

#define D_ATTRIBUTE_UNKNOWN "Propriedade %s desconhecida"
// D_ATTRIBUTE_OBSOLETE D_ATTRIBUTE_INSTEAD can be used together or just D_ATTRIBUTE_OBSOLETE alone
#define D_ATTRIBUTE_OBSOLETE "%s is obsolete"  // new
#define D_ATTRIBUTE_INSTEAD ", use %s instead" // new
#define D_ATTRIBUTE_READ_ONLY "%s é de leitura apenas"
#define D_ATTRIBUTE_PAGE_METHOD_INVALID "Não foi possível chamar %s numa página"
#define D_ATTRIBUTE_ALIGN_INVALID "Invalid align property: %s" // new
#define D_ATTRIBUTE_COLOR_INVALID "Invalid color property: %s" // new
#define D_ATTRIBUTE_LONG_MODE_INVALID "Invalid long mode: %s"  // new

#define D_OOBE_SSID_VALIDATED "SSID %s válido"
#define D_OOBE_AUTO_CALIBRATE "Auto calibração ativada"
#define D_OOBE_CALIBRATED "já foi calibrado"

#define D_DISPATCH_COMMAND_NOT_FOUND "Não se encontrou o comando '%s'"
#define D_DISPATCH_INVALID_PAGE "Página inválida %s"
#define D_DISPATCH_REBOOT "A reiniciar dispositivo!"

#define D_JSON_FAILED "Não foi possível analisar o JSON:"
#define D_JSONL_FAILED "A análise do JSONL falhou na linha %u"
#define D_JSONL_SUCCEEDED "JSONL analisado"

#define D_OTA_CHECK_UPDATE "A procurar por atualização no URL: %s"
#define D_OTA_CHECK_COMPLETE "Verificação da atualização completa"
#define D_OTA_CHECK_FAILED "Falhou a verificação da atualização: %s"
#define D_OTA_UPDATE_FIRMWARE "Atualização de firmware OTA"
#define D_OTA_UPDATE_COMPLETE "Atualização OTA completa"
#define D_OTA_UPDATE_APPLY "A aplicar o novo firmware e reiniciar"
#define D_OTA_UPDATE_FAILED "A atualização OTA falhou"
#define D_OTA_UPDATING_FIRMWARE "A atualizar o firmware..."
#define D_OTA_UPDATING_FILESYSTEM "A atualizar o sistema de ficheiros..."

#define D_HTTP_HASP_DESIGN "Design do HASP"
#define D_HTTP_INFORMATION "Informação"
#define D_HTTP_HTTP_SETTINGS "Configurar HTTP"
#define D_HTTP_FTP_SETTINGS "Configurar FTP"
#define D_HTTP_WIFI_SETTINGS "Configurar Wifi"
#define D_HTTP_WIREGUARD_SETTINGS "Configurar WireGuard"
#define D_HTTP_MQTT_SETTINGS "Configurar MQTT"
#define D_HTTP_GPIO_SETTINGS "Configurar GPIO"
#define D_HTTP_MDNS_SETTINGS "Configurar mDNS"
#define D_HTTP_TELNET_SETTINGS "Configurar Telnet"
#define D_HTTP_DEBUG_SETTINGS "Configurar debug"
#define D_HTTP_GUI_SETTINGS "Configurar GUI"
#define D_HTTP_SAVE_SETTINGS "Guardar Configuração"
#define D_HTTP_UPLOAD_FILE "Carregar ficheiro"
#define D_HTTP_ERASE_DEVICE "Eliminar Configuração"
#define D_HTTP_ADD_GPIO "Adicionar novo pino"
#define D_HTTP_BACK "Retroceder"
#define D_HTTP_REFRESH "Atualizar"
#define D_HTTP_PREV_PAGE "Página Anterior"
#define D_HTTP_NEXT_PAGE "Página Seguinte"
#define D_HTTP_CALIBRATE "Calibrar"
#define D_HTTP_ANTIBURN "Run Anti Burn-in" // New
#define D_HTTP_SCREENSHOT "Screenshot"
#define D_HTTP_FILE_BROWSER "Editor de ficheiros"
#define D_HTTP_FIRMWARE_UPGRADE "Atualização do firmware"
#define D_HTTP_UPDATE_FIRMWARE "Atualizar o firmware"
#define D_HTTP_FACTORY_RESET "Repor configuração de fábrica"
#define D_HTTP_MAIN_MENU "Menu Principal"
#define D_HTTP_REBOOT "Reiniciar"
#define D_HTTP_CONFIGURATION "Configuração"
#define D_HTTP_CONFIG_CHANGED                                                                                          \
    "Configuration has changed, please click <a href='/reboot'>Restart</a> to save changes to flash." // New
#define D_HTTP_SENDING_PAGE "Foi enviado página %S a %s"
#define D_HTTP_FOOTER "por Francis Van Roie"

#define D_INFO_VERSION "Versão"
#define D_INFO_BUILD_DATETIME "Compilado a"
#define D_INFO_ENVIRONMENT "Environment" // new
#define D_INFO_UPTIME "Tempo ativo"
#define D_INFO_FREE_HEAP "Heap livre"
#define D_INFO_FREE_BLOCK "Blocos livres"
#define D_INFO_DEVICE_MEMORY "Memória do dispositivo"
#define D_INFO_LVGL_MEMORY "Memória LVGL"
#define D_INFO_TOTAL_MEMORY "Total"
#define D_INFO_FREE_MEMORY "Livre"
#define D_INFO_FRAGMENTATION "Fragmentação"
#define D_INFO_PSRAM_FREE "PSRam livre"
#define D_INFO_PSRAM_SIZE "Tamanho PSRam "
#define D_INFO_FLASH_SIZE "Tamanho Flash"
#define D_INFO_SKETCH_USED "Memória programa usada"
#define D_INFO_SKETCH_FREE "Memória programa livre"
#define D_INFO_FS_SIZE "Filesystem Size"
#define D_INFO_FS_USED "Filesystem Used"
#define D_INFO_FS_FREE "Filesystem Free"
#define D_INFO_MODULE "Módulo"
#define D_INFO_MODEL "Modelo"
#define D_INFO_FREQUENCY "Frequência"
#define D_INFO_CORE_VERSION "Versão do core"
#define D_INFO_RESET_REASON "Razão do último Reset"
#define D_INFO_STATUS "Estado"
#define D_INFO_SERVER "Servidor"
#define D_INFO_USERNAME "Nome do utilizador"
#define D_INFO_CLIENTID "ID do Cliente"
// #define D_INFO_CONNECTED "Ligado"
// #define D_INFO_DISCONNECTED "Desligado"
#define D_INFO_RECEIVED "Recebido"
#define D_INFO_PUBLISHED "Publicado"
#define D_INFO_FAILED "Em falha"
#define D_INFO_ETHERNET "Ethernet"
#define D_INFO_WIFI "Wifi"
#define D_INFO_WIREGUARD "WireGuard"
#define D_INFO_LINK_SPEED "Link Speed"
#define D_INFO_FULL_DUPLEX "Full Duplex"
#define D_INFO_BSSID "BSSID"
#define D_INFO_SSID "SSID"
#define D_INFO_RSSI "Potência do sinal"
#define D_INFO_IP_ADDRESS "Endereço IP"
#define D_INFO_MAC_ADDRESS "Endereço MAC"
#define D_INFO_GATEWAY "Gateway"
#define D_INFO_DNS_SERVER "Servidor DNS"
#define D_INFO_ENDPOINT_IP "Endpoint IP"
#define D_INFO_ENDPOINT_PORT "Endpoint Port"

#define D_OOBE_MSG "Toque no ecrã para configurar WiFi ou para se ligar a um access point"
#define D_OOBE_SCAN_TO_CONNECT "Procurar rede"

#define D_WIFI_CONNECTING_TO "A ligar a %s"
#define D_WIFI_CONNECTED_TO "Ligado a %s, a pedir IP..."
#define D_WIFI_RSSI_EXCELLENT "Excelente"
#define D_WIFI_RSSI_GOOD "Bom"
#define D_WIFI_RSSI_WEAK "Fraco"
#define D_WIFI_RSSI_BAD "Muito baixo"
#define D_WIFI_RSSI_FAIR "Decente"

#define D_WG_INITIALIZED "Inicializado"
#define D_WG_BAD_CONFIG "Configuração ausente ou ruim"

#define D_GPIO_SWITCH "Interruptor"
#define D_GPIO_BUTTON "Botão"
#define D_GPIO_TOUCH "Capacitive Touch" // new
#define D_GPIO_LED "LED"
#define D_GPIO_LED_R "LED Red"
#define D_GPIO_LED_G "LED Green"
#define D_GPIO_LED_B "LED Blue"
#define D_GPIO_POWER_RELAY "Power Relé" // new
#define D_GPIO_LIGHT_RELAY "Light Relé" // new
#define D_GPIO_PWM "PWM"
#define D_GPIO_DAC "DAC"
#define D_GPIO_SERIAL_DIMMER "Dimmer serial"
#define D_GPIO_UNKNOWN "Desconhecido"
#define D_GPIO_PIN "Pin"
#define D_GPIO_GROUP "Grupo"
#define D_GPIO_GROUP_NONE "Nenhum"
#define D_GPIO_STATE_NORMAL "Normal"     // new
#define D_GPIO_STATE_INVERTED "Inverted" // new

#endif