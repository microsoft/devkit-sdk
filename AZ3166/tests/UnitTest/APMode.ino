test(APMode)
{
    byte mac[6];
    char ap_name[24];
  
    WiFi.macAddress(mac);
    sprintf(ap_name, "AZ_%02x%02x%02x%02x%02x%02x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    int ret = WiFi.beginAP(ap_name, "");
    assertEqual(ret, WL_CONNECTED);  
    assertEqual(ap_name, WiFi.SSID());
    if(SystemWiFiRSSI() == 0)
    {
        assertEqual(WL_CONNECTION_LOST, WiFi.status());
    }
    else
    {
        assertEqual(WL_CONNECTED, WiFi.status());
    }    
    
    ret = WiFi.disconnectAP();
    assertEqual(ret, 1);  
    assertEqual(WL_DISCONNECTED, WiFi.status());

    delay(LOOP_DELAY);
}
