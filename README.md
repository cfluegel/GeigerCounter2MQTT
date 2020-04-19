# GeigerCounter2MQTT Adapterboard

# Warum? 
Wochenendprojekt um die Zeit rum zu bekommen. Außerdem wollte ich mal wieder meine Erfahrungen mit Mikrocontrollerprogrammierung verbessern. 
Dank GPL hätte ich zwar die Originalfirmware modifizieren können, aber so kann diese Software jeder verwenden ohne Modifikationen am Originalbausatz

## Format der Ausgabe  
CPS, #####, CPM, #####, uSv/hr, ###.##, SLOW|FAST|INST

## Konfiguration 
Für die Nutzung muss noch die Datei "secrets.h" erstellt werden. 


> #ifndef SECRETES_H
>
> #define SECRETES_H
>
> // WIFI
> #define WLAN_SSID       "xxxxxxxxxxxxxxxxxxxxxxxxx"
>
> #define WLAN_PASS       "xxxxxxxxxxxxxxxxxxxxxxxxx"
>
> // Adafruit IO 
>
> #define AIO_USERNAME    "benutzername"
>
> #define AIO_KEY         "aio_xxxxxxxxxxxxxxxxxxxxx"
>
> #endif 


# Disclaimer 
Natürlich handelt es sich hierbei nicht um ein geeichtes Messgerät und sollte daher nicht in Umgebungen mit gefährlicher Strahlung genutzt werden. 