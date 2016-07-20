#include <FlashMemory.h>
#include <WiFi.h>
void srvr_or_ap(){
  pinMode(13, INPUT);
  delay(500);
  if(digitalRead(13) == LOW){
    // attempt to start AP:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to start AP with SSID: ");
      Serial.println(ssid);
      status = WiFi.apbegin(ssid, pass, channel);
      delay(10000);
    }
    //AP MODE already started:
    Serial.println("AP mode already started");
    Serial.println();
    server.begin();
  }
  else{
    Serial.println("Will start in STA... halting");
    while(true);
  }
  
  
}

void servePage(WiFiClient cl){
   char page[] = "<html><body><Form action=\"submitscript\" name =\"login\"><Input type=\"text\" name=\"txtssid\" /> <br /><Input type=\"password\" name=\"txtpass\" /><br /><Input type=\"submit\" name=\"btnSubmit\" value=\"Submit\" /></Form></body></html>";
   cl.println(page);
}

