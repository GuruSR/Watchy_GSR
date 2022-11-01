#ifndef Locale_GSR_H
#define Locale_GSR_H
/* This code is designed to allow for multi-lingual setup to happen with Watchy GSR.
 */

class LocaleGSR {
    public:
    LocaleGSR() {};

/*
 *  MaxLangID holds the maximum ID for language, done when more are added.
 */

    virtual uint8_t MaxLangID() final { return 0; };

/*
 *  LangString:
 *  
 *  Values starting with 0 are encased in {% %} so they look like {%0%} and are replaced
 *  with the language ID's id, calls for LangString will replace those with the values of
 *  the stored strings, use the StartID and LastID to minimize the workload on specific
 *  strings.
 */

    virtual String LangString(String Input, bool Web, uint8_t LangID = 0, uint8_t StartID = 0, uint8_t LastID = 0) final{
        String S = Input;
        S.replace("{%LANG%}",GetWebLang(LangID));
        if (LastID < StartID || (LastID + StartID) == 0) LastID = LocaleGSR::MaxLangID();
        for (int I = StartID; I <= LastID; I++) S.replace("{%" + String(I) + "%}",(Web ? LocaleGSR::GetWebID(LangID,I) : LocaleGSR::GetID(LangID,I)));
        return S;
    };

/* 
 *  GetFormatID is used to retrieve format information for specific watch face fields
 *  by regional changes.
 */

    String GetFormatID(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "{W}"; // Weekday.
                   case 1:
                      return "{M} {D}";  // Month & Date
                }
            }
            break;
            case 1:{
                switch(ID){
                   case 0:
                      return "{W}"; // Weekday.
                   case 1:
                      return "{D} {M}";  // Month & Date
                }
            }
        }
        return "None";
    };

/*
 *  GetID, GetWebID are used by the LangString to retrieve those language changes per entry.
 *  WebIDs are different from normal IDs as they're used in the OTA Website and are not limited
 *  to the font limitations in the system.
 */

    String GetID(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "Classic GSR"; // Watchface 0.
                   case 1:
                      return "--";          // Placeholder
                   case 2:
                      return "Tomorrow";
                   case 3:
                      return "Reset Today";
                   case 4:
                      return "Steps";
                   case 5:
                      return "Alarms";
                   case 6:
                      return "Timers";
                   case 7:
                      return "Options";
                   case 8:
                      return "Alarm 1 Time";
                   case 9:
                      return "A1 Tone Repeats";
                   case 10:
                      return "Alarm 1 Options";
                   case 11:
                      return "Alarm 1";
                   case 12:
                      return "Alarm 2 Time";
                   case 13:
                      return "A2 Tone Repeats";
                   case 14:
                      return "Alarm 2 Options";
                   case 15:
                      return "Alarm 2";
                   case 16:
                      return "Alarm 3 Time";
                   case 17:
                      return "A3 Tone Repeats";
                   case 18:
                      return "Alarm 3 Options";
                   case 19:
                      return "Alarm 3";
                   case 20:
                      return "Alarm 4 Time";
                   case 21:
                      return "A4 Tone Repeats";
                   case 22:
                      return "Alarm 4 Options";
                   case 23:
                      return "Alarm 4";
                   case 24:
                      return "Tone Repeats";
                   case 25:
                      return "Countdown Timer";
                   case 26:
                      return "Elapsed Timer";
                   case 27:
                      return "Watch Style";
                   case 28:
                      return "Display Style";
                   case 29:
                      return "Language";
                   case 30:
                      return "Border Mode";
                   case 31:
                      return "Dexterity";
                   case 32:
                      return "Menu & Back";
                   case 33:
                      return "Orientation";
                   case 34:
                      return "Time Mode";
                   case 35:
                      return "Feedback";
                   case 36:
                      return "Turbo Time";
                   case 37:
                      return "Screen Off";
                   case 38:
                      return "Screen Blanking";
                   case 39:
                      return "Screen Auto-Off";
                   case 40:
                      return "Sleeping Begins";
                   case 41:
                      return "Sleeping Ends";
                   case 42:
                      return "Orientation";
                   case 43:
                      return "Performance";
                   case 44:
                      return "WiFi Tx Power";
                   case 45:
                      return "Information";
                   case 46:
                      return "Troubleshoot";
                   case 47:
                      return "Sync Watchy";
                   case 48:
                      return "Watchy Connect";
                   case 49:
                      return "Upload Firmware";
                   case 50:
                      return "OTA Update";
                   case 51:
                      return "Visit Website";
                   case 52:
                      return "OTA Website";
                   case 53:
                      return "Reset Screen";
                   case 54:
                      return "Watchy Reboot";
                   case 55:
                      return "Return to RTC";
                   case 56:
                      return "Detect Travel";
                   case 57:
                      return "Storage Settings";
                   case 58:
                      return "Change Storage";
                   case 59:
                      return "Delete and Reboot";
                   case 60:
                      return "MENU to Reset";
                   case 61:
                      return "MENU to Select";
                   case 62:
                      return "MENU to Edit";
                   case 63:
                      return "Repeat";
                   case 64:
                      return "Active";
                   case 65:
                      return "repeats";
                   case 66:
                      return "MENU to Select";
                   case 67:
                      return "MENU to Change";
                   case 68:
                      return "Off";
                   case 69:
                      return "On";
                   case 70:
                      return "MENU to Enter";
                   case 71:
                      return "Light";
                   case 72:
                      return "Dark";
                   case 73:
                      return "Left-handed";
                   case 74:
                      return "Right-handed";
                   case 75:
                      return "Swapped";
                   case 76:
                      return "Normal";
                   case 77:
                      return "Watchy UP";
                   case 78:
                      return "Ignore";
                   case 79:
                      return "24 Hour";
                   case 80:
                      return "AM/PM";
                   case 81:
                      return "Enabled";
                   case 82:
                      return "Locked";
                   case 83:
                      return "Disabled";
                   case 84:
                      return "Always";
                   case 85:
                      return "Bed Time";
                   case 86:
                      return "Double Tap On";
                   case 87:
                      return "Double Tap Only";
                   case 88:
                      return "to";
                   case 89:
                      return "Version";
                   case 90:
                      return "Battery";
                   case 91:
                      return "Battery Saving";
                   case 92:
                      return "Turbo";
                   case 93:
                      return "MENU to Start";
                   case 94:
                      return "Time";
                   case 95:
                      return "TimeZone";
                   case 96:
                      return "TimeZone & Time";
                   case 97:
                      return "MENU to Begin";
                   case 98:
                      return "Starting AP";
                   case 99:
                      return "BACK to End";
                   case 100:
                      return "MENU to Connect";
                   case 101:
                      return "Connecting...";
                   case 102:
                      return "MENU to Agree";
                   case 103:
                      return "MENU to Reboot";
                   case 104:
                      return "Time Sync";
                   case 105:
                      return "Calculating";
                   case 106:
                      return "Bad RTC";
                   case 107:
                      return "No Drift";
                   case 108:
                      return "Menu to Disable";
                   case 109:
                      return "Menu to Enable";
                   case 110:
                      return "MENU to Keep";
                   case 111:
                      return "Delete and Reboot";
                   case 112:
                      return "seconds";
                   case 113:
                      return "second";
                   case 114:
                      return "PM";
                   case 115:
                      return "AM";
                   case 116:
                      return "Full";
                   case 117:
                      return "Once";
                   case 118:
                      return "Repeat";
                   case 119:
                      return "Countdown Options";
                }
                break;
                case 1:{
                switch(ID){
                   case 0:
                      return "GSR Classique"; // Watchface 0.
                   case 1:
                      return "--";          // Placeholder
                   case 2:
                      return "Demain";
                   case 3:
                      return "Reset Demain";
                   case 4:
                      return "Pas";
                   case 5:
                      return "Alarmes";
                   case 6:
                      return "Minuteries";
                   case 7:
                      return "Options";
                   case 8:
                      return "Heure de l'alarme 1";
                   case 9:
                      return "A1 repet. bip";
                   case 10:
                      return "Options de l'Alarme 1";
                   case 11:
                      return "Alarme 1";
                   case 12:
                      return "Heure de l'alarme 2";
                   case 13:
                      return "A2 repet. bip";
                   case 14:
                      return "Options de l'alarme 2";
                   case 15:
                      return "Alarme 2";
                   case 16:
                      return "Heure de l'alarme 3";
                   case 17:
                      return "A3 repet. bip";
                   case 18:
                      return "Options de l'alarme 3";
                   case 19:
                      return "Alarme 3";
                   case 20:
                      return "Heure de l'alarme 4";
                   case 21:
                      return "A4 repet. bip";
                   case 22:
                      return "Options de l'alarme 4";
                   case 23:
                      return "Alarme 4";
                   case 24:
                      return "Nb. Bip";
                   case 25:
                      return "Décompte";
                   case 26:
                      return "Chrono";
                   case 27:
                      return "Style de montre";
                   case 28:
                      return "Couleur";
                   case 29:
                      return "Langue";
                   case 30:
                      return "Bordure";
                   case 31:
                      return "Main dominante";
                   case 32:
                      return "Menu & retour";
                   case 33:
                      return "Orientation";
                   case 34:
                      return "Mode holorge";
                   case 35:
                      return "Vibration";
                   case 36:
                      return "Temps turbo";
                   case 37:
                      return "Écran éteint";
                   case 38:
                      return "Effacement de l'écran";
                   case 39:
                      return "Écran s'éteint";
                   case 40:
                      return "Heure du coucher";
                   case 41:
                      return "Heure du lever";
                   case 42:
                      return "Orientation";
                   case 43:
                      return "Performance";
                   case 44:
                      return "Puissance Tx Wifi";
                   case 45:
                      return "Information";
                   case 46:
                      return "Dépannage";
                   case 47:
                      return "Synchro Watchy";
                   case 48:
                      return "Connecter à Watchy";
                   case 49:
                      return "Téléverser Firmware";
                   case 50:
                      return "Mise à jour OTA";
                   case 51:
                      return "Visiter le site web";
                   case 52:
                      return "Site web OTA";
                   case 53:
                      return "Remise à zéro écran";
                   case 54:
                      return "Redémarrer Watchy";
                   case 55:
                      return "Retourner à RTC";
                   case 56:
                      return "Detecter déviations RTC";
                   case 57:
                      return "Paramètres de stockage";
                   case 58:
                      return "Changer le stockage";
                   case 59:
                      return "Effacer et redémarrer";
                   case 60:
                      return "MENU pour reset";
                   case 61:
                      return "MENU pour choisir";
                   case 62:
                      return "MENU to changer";
                   case 63:
                      return "Répéter";
                   case 64:
                      return "Actif";
                   case 65:
                      return "se répète";
                   case 66:
                      return "MENU pour choisir";
                   case 67:
                      return "MENU pour changer";
                   case 68:
                      return "éteint";
                   case 69:
                      return "allumé";
                   case 70:
                      return "MENU pour confirmer";
                   case 71:
                      return "Clair";
                   case 72:
                      return "Foncé";
                   case 73:
                      return "Gaucher";
                   case 74:
                      return "Droitier";
                   case 75:
                      return "Inverse";
                   case 76:
                      return "Normal";
                   case 77:
                      return "Watchy à l'endroit";
                   case 78:
                      return "Ignorer";
                   case 79:
                      return "24 Heures";
                   case 80:
                      return "AM/PM";
                   case 81:
                      return "Actif";
                   case 82:
                      return "Verrouillé";
                   case 83:
                      return "Inactif";
                   case 84:
                      return "Toujours";
                   case 85:
                      return "Heure du coucher";
                   case 86:
                      return "Double clic On";
                   case 87:
                      return "Double clic seul.";
                   case 88:
                      return "à";
                   case 89:
                      return "Version";
                   case 90:
                      return "Batterie";
                   case 91:
                      return "Écono Battery";
                   case 92:
                      return "Turbo";
                   case 93:
                      return "MENU pour débuter";
                   case 94:
                      return "Heure";
                   case 95:
                      return "Fuseau horaire";
                   case 96:
                      return "Fuseau horaire et heure";
                   case 97:
                      return "MENU pour commencer";
                   case 98:
                      return "Démarrage point accès";
                   case 99:
                      return "BACK pour terminer";
                   case 100:
                      return "MENU pour connecter";
                   case 101:
                      return "Connection...";
                   case 102:
                      return "MENU pour accepter";
                   case 103:
                      return "MENU pour redémarrer";
                   case 104:
                      return "Synchro du temps";
                   case 105:
                      return "En Calcul";
                   case 106:
                      return "Mauvais RTC";
                   case 107:
                      return "Pas de déviation";
                   case 108:
                      return "Menu pour inactiver";
                   case 109:
                      return "Menu pour activer";
                   case 110:
                      return "MENU pour conserver";
                   case 111:
                      return "Effacer et redémarrer";
                   case 112:
                      return "secondes";
                   case 113:
                      return "seconde";
                   case 114:
                      return "PM";
                   case 115:
                      return "AM";
                   case 116:
                      return "Plein";
                   case 117:
                      return "Unique";
                   case 118:
                      return "Repeater";
                   case 119:
                      return "Options de décompte";
                }
            }
        }
        return "None";
    };

    String GetWebID(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "Watchy WiFi TX Power";  // Why did I do this?
                   case 1:
                      return "Watchy Connect Options";  // 1 to 4 Index page
                   case 2:
                      return "Backup & Restore Settings";
                   case 3:
                      return "Edit Additional WiFi Access Points";
                   case 4:
                      return "Upload New Firmware";
                   case 5:
                      return "Watchy Backup & Restore Settings";  // 5 to 6 Settings page
                   case 6:
                      return "Store"; // WiFi D
                   case 7:
                      return "Watchy settings restored!  OTA Website Off"; // 5 to 7 for Settings Done
                   case 8:
                      return "Watchy Upload New Firmware"; // 8 to 13 Upgrade
                   case 9:
                      return "Upload";
                   case 10:
                      return "Waiting";
                   case 11:
                      return "You must select a firmware [bin] to upload!";
                   case 12:
                      return "Progress";
                   case 13:
                      return "Success";
                   case 14:
                      return "Watchy Additional WiFi Access Point Settings"; // WiFi A
                   case 15:
                      return "PASS";  // WiFi C
                   case 16:
                      return "Watchy Additional APs updated!  OTA Website Off";
                }
            }
            break;
            case 1:{
                switch(ID){
                   case 0:
                      return "Watchy Puissance Tx WiFi";  // Why did I do this?
                   case 1:
                      return "Watchy Options de connection";  // 1 to 4 Index page
                   case 2:
                      return "Enregistrer et restaurer les paramètres";
                   case 3:
                      return "Liste des points d'accès WiFi";
                   case 4:
                      return "Déployer un nouveau microprogramme";
                   case 5:
                      return "Enregistrer et restaurer les paramètres de Watchy";  // 5 to 6 Settings page
                   case 6:
                      return "Sauvegarder"; // WiFi D
                   case 7:
                      return "Paramètres de Watchy restaurés! Site web OTA Inactivé"; // 5 to 7 for Settings Done
                   case 8:
                      return "Watchy téléverser microprogramme"; // 8 to 13 Upgrade
                   case 9:
                      return "Téléverser";
                   case 10:
                      return "En attente";
                   case 11:
                      return "Vous devez sélectionner un micropgrogramme [bin] à téléverser!";
                   case 12:
                      return "Progrès";
                   case 13:
                      return "Succès";
                   case 14:
                      return "Autres points d'accès Wifi Watchy"; // WiFi A
                   case 15:
                      return "PASS";  // WiFi C
                   case 16:
                      return "Points d'accès mis à jour! Site web OTA Inactif";
                }
            }
        }
        return "None";
    };

/*
 *  GetWebLang retrieves the current language for the OTA website (2 letters usually).
 */

    String GetWebLang(uint8_t LangID){
        switch(LangID){
           case 0:
              return "en";
           case 1:
              return "fr";
           case 2:
              return "None";
        }
        return "None";
    };

/*
 *  GetLangName retrieves the current language (typically written in that language).
 */

    String GetLangName(uint8_t LangID){
        switch(LangID){
           case 0:
              return "English";
           case 1:
              return "Français";
           case 2:
              return "None";
        }
        return "None";
    };

/*
 *  GetWeekday retrieves the current day of week in language (0 to 6 input starting from
 *  Sunday)..
 */

    String GetWeekday(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "Sunday";
                   case 1:
                      return "Monday";
                   case 2:
                      return "Tuesday";
                   case 3:
                      return "Wednesday";
                   case 4:
                      return "Thursday";
                   case 5:
                      return "Friday";
                   case 6:
                      return "Saturday";
                }
            }
            break;
            case 1:{
                switch(ID){
                   case 0:
                      return "Dimanche";
                   case 1:
                      return "Lundi";
                   case 2:
                      return "Mardi";
                   case 3:
                      return "Mercredi";
                   case 4:
                      return "Jeudi";
                   case 5:
                      return "Vendredi";
                   case 6:
                      return "Samedi";
                }
            }
        }
        return "None";
    };

/*
 *  GetShortWeekday retrieves the current day of week in language (0 to 6 input starting from
 *  Sunday)..
 */

    String GetShortWeekday(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "Sun";
                   case 1:
                      return "Mon";
                   case 2:
                      return "Tue";
                   case 3:
                      return "Wed";
                   case 4:
                      return "Thu";
                   case 5:
                      return "Fri";
                   case 6:
                      return "Sat";
                }
            }
            break;
            case 1:{
                switch(ID){
                   case 0:
                      return "Dim";
                   case 1:
                      return "Lun";
                   case 2:
                      return "Mar";
                   case 3:
                      return "Mer";
                   case 4:
                      return "Jeu";
                   case 5:
                      return "Ven";
                   case 6:
                      return "Sam";
                }
            }
        }
        return "Non";
    };

/*
 *  GetMonth retrieves the current month in the language (0 to 11 input starting from the
 *  first month January).
 */

    String GetMonth(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "January";
                   case 1:
                      return "February";
                   case 2:
                      return "March";
                   case 3:
                      return "April";
                   case 4:
                      return "May";
                   case 5:
                      return "June";
                   case 6:
                      return "July";
                   case 7:
                      return "August";
                   case 8:
                      return "September";
                   case 9:
                      return "October";
                   case 10:
                      return "November";
                   case 11:
                      return "December";
                }
            }
            break;
            case 1:{
                switch(ID){
                   case 0:
                      return "Janvier";
                   case 1:
                      return "Février";
                   case 2:
                      return "Mars";
                   case 3:
                      return "Avril";
                   case 4:
                      return "Mai";
                   case 5:
                      return "Juin";
                   case 6:
                      return "Juillet";
                   case 7:
                      return "Août";
                   case 8:
                      return "Septembre";
                   case 9:
                      return "Octobre";
                   case 10:
                      return "Novembre";
                   case 11:
                      return "Décembre";
                }
            }
        }
        return "None";
    };

/*
 *  GetShortMonth retrieves the current month in the language (0 to 11 input starting from the
 *  first month January).
 */

    String GetShortMonth(uint8_t LangID, uint8_t ID){
        switch(LangID){
            case 0:{
                switch(ID){
                   case 0:
                      return "Jan";
                   case 1:
                      return "Feb";
                   case 2:
                      return "Mar";
                   case 3:
                      return "Apr";
                   case 4:
                      return "May";
                   case 5:
                      return "Jun";
                   case 6:
                      return "Jul";
                   case 7:
                      return "Aug";
                   case 8:
                      return "Sep";
                   case 9:
                      return "Oct";
                   case 10:
                      return "Nov";
                   case 11:
                      return "Dec";
                }
            }
            break;
            case 0:{
                switch(ID){
                   case 0:
                      return "Jan";
                   case 1:
                      return "Fév";
                   case 2:
                      return "Mar";
                   case 3:
                      return "Avr";
                   case 4:
                      return "Mai";
                   case 5:
                      return "Jun";
                   case 6:
                      return "Jul";
                   case 7:
                      return "Août";
                   case 8:
                      return "Sep";
                   case 9:
                      return "Oct";
                   case 10:
                      return "Nov";
                   case 11:
                      return "Déc";
                }
            }
        }
        return "Non";
    };
};
#endif
