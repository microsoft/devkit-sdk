#include "AZ3166WiFi.h"
#include "SystemWiFi.h"
#include "app_httpd.h"

void setup() {
  // put your setup code here, to run once:

    printf( "start soft ap!\r\n" );

    char ap_name[24];
    sprintf(ap_name, "AZ3166_%s", WiFiInterface()->get_mac_address());
    int ret = WiFi.beginAP(ap_name, "12345678");
    if ( ret != WL_CONNECTED) {
        printf("Soft ap creation failed\r\n");
        return ;
    }
    printf("soft ap started\r\n");
    httpd_server_start();

    // you can connect to the board and set wifi at "http://192.168.0.1:999/" now
}

void loop() {
  // put your main code here, to run repeatedly:

}
