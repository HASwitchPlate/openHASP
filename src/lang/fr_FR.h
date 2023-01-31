#ifndef HASP_LANG_FR_FR_H
#define HASP_LANG_FR_FR_H

#define D_ISO_LANG_CODE "fr-FR"

#define D_USERNAME "Utilisateur:"
#define D_PASSWORD "Mot de passe:"
#define D_SSID "Ssid:"
#define D_YES "Oui"
#define D_NO "Non"

#define D_ERROR_OUT_OF_MEMORY "Mémoire insuffisante "
#define D_ERROR_UNKNOWN "Erreur inconnue"

#define D_CONFIG_NOT_CHANGED "Paramètres pas modifiés"
#define D_CONFIG_CHANGED "Paramètres modifiés"
#define D_CONFIG_LOADED "Paramètres chargés"

#define D_FILE_LOADING "Charger %s"
#define D_FILE_LOADED "Chargé %s"
#define D_FILE_LOAD_FAILED "Échec du chargement %s"
#define D_FILE_SAVING "Enregistrer %s"
#define D_FILE_SAVED "Enregistré %s"
#define D_FILE_SAVE_FAILED "Échec de l'enregistrement %s"
#define D_FILE_NOT_FOUND "Fichier non trouvé"
#define D_FILE_SIZE_BYTES "octets"
#define D_FILE_SIZE_KILOBYTES "Kio"
#define D_FILE_SIZE_MEGABYTES "Mio"
#define D_FILE_SIZE_GIGABYTES "Gio"
#define D_FILE_SIZE_DIVIDER 1024 // kibi or kilo bytes
#define D_DECIMAL_POINT ","      // decimal comma or point

#define D_SERVICE_STARTING "Démarer..."
#define D_SERVICE_STARTED "Démaré"
#define D_SERVICE_START_FAILED "Échec du démarrage"
#define D_SERVICE_STOPPED "Arrêté"
#define D_SERVICE_DISABLED "Désactivé"
#define D_SERVICE_CONNECTED "Connecté"
#define D_SERVICE_DISCONNECTED "Débranché"

#define D_SETTING_ENABLED "Activé"
#define D_SETTING_DISABLED "Désactivé"
#define D_SETTING_DEFAULT "Défaut"

#define D_NETWORK_IP_ADDRESS_RECEIVED "Adresse IP reçue %s"
#define D_NETWORK_ONLINE "en ligne"
#define D_NETWORK_OFFLINE "déconnecté"
#define D_NETWORK_CONNECTION_FAILED "Échec de la connexion "
#define D_NETWORK_CONNECTION_UNAUTHORIZED "Échec de l'autorisation"

#define D_MQTT_DEFAULT_NAME "plaque_%s"
#define D_MQTT_CONNECTING "Connexion..."
#define D_MQTT_CONNECTED "Connecté au broker %s avec ID client %s"
#define D_MQTT_NOT_CONNECTED "Pas connecté ???"
#define D_MQTT_DISCONNECTING "Déconnexion..."
#define D_MQTT_DISCONNECTED "Débranché"
#define D_MQTT_RECONNECTING "Déconnecté du broker, reconnexion..."
#define D_MQTT_NOT_CONFIGURED "Broker non configuré"
#define D_MQTT_STARTED "Démarré: %d octets "
#define D_MQTT_FAILED "Manqué:"
#define D_MQTT_INVALID_TOPIC "Message avec sujet non valide "
#define D_MQTT_SUBSCRIBED "Abonné à %s"
#define D_MQTT_NOT_SUBSCRIBED "Échec de s'abonner à %s"
#define D_MQTT_HA_AUTO_DISCOVERY "Enregistrer la détection automatique HA"
#define D_MQTT_PAYLOAD_TOO_LONG "Charge utile trop long (%u octets) "

#define D_TELNET_CLOSING_CONNECTION "Clôture de la session %s"
#define D_TELNET_CLIENT_LOGIN_FROM "Connexion client depuis %s"
#define D_TELNET_CLIENT_CONNECT_FROM "Client connecté depuis %s"
#define D_TELNET_CLIENT_NOT_CONNECTED "Client NON connecté"
#define D_TELNET_INCORRECT_LOGIN_ATTEMPT "Tentative incorrecte de %s"
#define D_TELNET_STARTED "Console Telnet démarré"
#define D_TELNET_FAILED "Échec du démarrage de la console telnet"
#define D_TELNET_CLIENT_CONNECTED "Client connecté"
#define D_TELNET_CLIENT_REJECTED "Client rejeté"

#define D_HASP_INVALID_PAGE "Page non valide %u"
#define D_HASP_INVALID_LAYER "Impossible d'effacer la couche système"
#define D_HASP_CHANGE_PAGE "Changement de page %u"
#define D_HASP_CLEAR_PAGE "Effacement de la page %u"

#define D_OBJECT_DELETED "Objet supprimé"
#define D_OBJECT_UNKNOWN "Objet inconnu"
#define D_OBJECT_MISMATCH "Objets ne correspondent PAS!"
#define D_OBJECT_LOST "Objet perdu!"
#define D_OBJECT_CREATE_FAILED "Échec de la création d'objet %u"
#define D_OBJECT_PAGE_UNKNOWN "ID de page %u non défini"
#define D_OBJECT_EVENT_UNKNOWN "Inconnu Event %d"

#define D_ATTRIBUTE_UNKNOWN "Propriété %s inconnue"
// D_ATTRIBUTE_OBSOLETE D_ATTRIBUTE_INSTEAD can be used together or just D_ATTRIBUTE_OBSOLETE alone
#define D_ATTRIBUTE_OBSOLETE "%s est obsolète"
#define D_ATTRIBUTE_INSTEAD ", remplacé par %s"
#define D_ATTRIBUTE_READ_ONLY "%s is read-only"
#define D_ATTRIBUTE_PAGE_METHOD_INVALID "Unable to call %s on a page"
#define D_ATTRIBUTE_ALIGN_INVALID "Invalid align property: %s" // new
#define D_ATTRIBUTE_COLOR_INVALID "Invalid color property: %s" // new
#define D_ATTRIBUTE_LONG_MODE_INVALID "Invalid long mode: %s"  // new

#define D_OOBE_SSID_VALIDATED "SSID %s validated"      // new
#define D_OOBE_AUTO_CALIBRATE "Auto calibrate enabled" // new
#define D_OOBE_CALIBRATED "Already calibrated"         // new

#define D_DISPATCH_COMMAND_NOT_FOUND "Command '%s' not found" // new
#define D_DISPATCH_INVALID_PAGE "Invalid page %s"             // new
#define D_DISPATCH_REBOOT "Rebooting the MCU now!"            // new

#define D_JSON_FAILED "JSON parsing failed:"             // new
#define D_JSONL_FAILED "JSONL parsing failed at line %u" // new
#define D_JSONL_SUCCEEDED "Jsonl fully parsed"           // new

#define D_OTA_CHECK_UPDATE "Checking update URL: %s"       // new
#define D_OTA_CHECK_COMPLETE "Update check complete"       // new
#define D_OTA_CHECK_FAILED "Update check failed: %s"       // new
#define D_OTA_UPDATE_FIRMWARE "OTA Firmware Update"        // new
#define D_OTA_UPDATE_COMPLETE "OTA Update complete"        // new
#define D_OTA_UPDATE_APPLY "Applying Firmware & Reboot"    // new
#define D_OTA_UPDATE_FAILED "OTA Update failed"            // new
#define D_OTA_UPDATING_FIRMWARE "Updating firmware..."     // new
#define D_OTA_UPDATING_FILESYSTEM "Updating filesystem..." // new

#define D_HTTP_HASP_DESIGN "Conception HASP"
#define D_HTTP_INFORMATION "Information"
#define D_HTTP_HTTP_SETTINGS "Paramètres HTTP"
#define D_HTTP_FTP_SETTINGS "Paramètres FTP"
#define D_HTTP_WIFI_SETTINGS "Paramètres Wifi"
#define D_HTTP_MQTT_SETTINGS "Paramètres MQTT"
#define D_HTTP_GPIO_SETTINGS "Paramètres GPIO"
#define D_HTTP_MDNS_SETTINGS "Paramètres mDNS"
#define D_HTTP_TELNET_SETTINGS "Paramètres Telnet"
#define D_HTTP_DEBUG_SETTINGS "Paramètres de débogage"
#define D_HTTP_GUI_SETTINGS "Paramètres d'affichage"
#define D_HTTP_SAVE_SETTINGS "Enregistrer les paramètres"
#define D_HTTP_UPLOAD_FILE "Télécharger le fichier"
#define D_HTTP_ERASE_DEVICE "Réinitialiser tous les paramètres"
#define D_HTTP_ADD_GPIO "Ajouter une nouvelle épingles"
#define D_HTTP_BACK "Retour"
#define D_HTTP_REFRESH "Actualiser"
#define D_HTTP_PREV_PAGE "Page précédente"
#define D_HTTP_NEXT_PAGE "Page suivante"
#define D_HTTP_CALIBRATE "Calibrer"
#define D_HTTP_ANTIBURN "Anti-Brûle"
#define D_HTTP_SCREENSHOT "Capture d'écran"
#define D_HTTP_FILE_BROWSER "Éditeur de fichiers"
#define D_HTTP_FIRMWARE_UPGRADE "Mise à jour du micrologiciel"
#define D_HTTP_UPDATE_FIRMWARE "Mettre à jour le micrologiciel"
#define D_HTTP_FACTORY_RESET "Paramètres d'usine"
#define D_HTTP_MAIN_MENU "Menu principal"
#define D_HTTP_REBOOT "Redémarrer"
#define D_HTTP_CONFIGURATION "Configuration"
#define D_HTTP_CONFIG_CHANGED                                                                                            \
    "La configuration a changé, cliquez sur <a href='/reboot'>Redémarrer</a> pour enregistrer les modifications dans " \
    "le flash."
#define D_HTTP_SENDING_PAGE "La page %S envoyée à %s"
#define D_HTTP_FOOTER "par Francis Van Roie"

#define D_INFO_VERSION "Version"
#define D_INFO_BUILD_DATETIME "Date/heure de compilation"
#define D_INFO_ENVIRONMENT "Environnement"
#define D_INFO_UPTIME "Disponibilité"
#define D_INFO_FREE_HEAP "Tas libre"
#define D_INFO_FREE_BLOCK "Blocage libre"
#define D_INFO_DEVICE_MEMORY "Mémoire de l&#39;appareil"
#define D_INFO_LVGL_MEMORY "Mémoire LVGL"
#define D_INFO_TOTAL_MEMORY "Total"
#define D_INFO_FREE_MEMORY "Libre"
#define D_INFO_FRAGMENTATION "Fragmentation"
#define D_INFO_PSRAM_FREE "PSRam libre"
#define D_INFO_PSRAM_SIZE "Taille PSRam"
#define D_INFO_FLASH_SIZE "Taille du flash"
#define D_INFO_SKETCH_USED "Taille utilisée du programme"
#define D_INFO_SKETCH_FREE "Taille libre du programme"
#define D_INFO_FS_SIZE "Taille du système de fichiers"
#define D_INFO_FS_USED "Système de fichiers utilisé"
#define D_INFO_FS_FREE "Système de fichiers libre"
#define D_INFO_MODULE "Module"
#define D_INFO_MODEL "Modèle"
#define D_INFO_FREQUENCY "Fréquence"
#define D_INFO_CORE_VERSION "Version principale"
#define D_INFO_RESET_REASON "Raison de la réinitialisation"
#define D_INFO_STATUS "Statut"
#define D_INFO_SERVER "Serveur"
#define D_INFO_USERNAME "Nom d&#39;utilisateur"
#define D_INFO_CLIENTID "ID client"
// #define D_INFO_CONNECTED "Connecté"
// #define D_INFO_DISCONNECTED "Déconnecté"
#define D_INFO_RECEIVED "Reçu"
#define D_INFO_PUBLISHED "Publié"
#define D_INFO_FAILED "Échec"
#define D_INFO_ETHERNET "Ethernet"
#define D_INFO_WIFI "Wifi"
#define D_INFO_LINK_SPEED "Vitesse de liaison"
#define D_INFO_FULL_DUPLEX "Duplex intégral"
#define D_INFO_BSSID "BSSID"
#define D_INFO_SSID "SSID"
#define D_INFO_RSSI "Force du signal"
#define D_INFO_IP_ADDRESS "Adresse IP"
#define D_INFO_MAC_ADDRESS "Adresse MAC"
#define D_INFO_GATEWAY "Passerelle"
#define D_INFO_DNS_SERVER "Serveur DNS"

#define D_OOBE_MSG "Touchez l'écran pour configurer le WiFi ou branchez ce point d'accès:"
#define D_OOBE_SCAN_TO_CONNECT "Scanner pour se connecter"

#define D_WIFI_CONNECTING_TO "Connexion à %s"
#define D_WIFI_CONNECTED_TO "Connecté à %s, demande d'IP..."
#define D_WIFI_RSSI_EXCELLENT "Excellent"
#define D_WIFI_RSSI_GOOD "Bon"
#define D_WIFI_RSSI_FAIR "Juste"
#define D_WIFI_RSSI_WEAK "Faible"
#define D_WIFI_RSSI_BAD "Très mauvais"

#define D_GPIO_SWITCH "Interrupteur"
#define D_GPIO_BUTTON "Bouton"
#define D_GPIO_TOUCH "Touche Capacitive"
#define D_GPIO_LED "Led"
#define D_GPIO_LED_R "Humeur Rouge"
#define D_GPIO_LED_G "Humeur Vert"
#define D_GPIO_LED_B "Humeur Bleu"
#define D_GPIO_POWER_RELAY "Relais Electrique"
#define D_GPIO_LIGHT_RELAY "Relais de Lumière"
#define D_GPIO_PWM "PWM"
#define D_GPIO_DAC "DAC"
#define D_GPIO_SERIAL_DIMMER "Gradateur Série"
#define D_GPIO_UNKNOWN "Inconnu"
#define D_GPIO_PIN "Pin"
#define D_GPIO_GROUP "Groupe"
#define D_GPIO_GROUP_NONE "Aucun"
#define D_GPIO_STATE_NORMAL "Normal"
#define D_GPIO_STATE_INVERTED "Inverse"

#endif