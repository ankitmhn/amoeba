#include <FlashMemory.h>
#include <WiFi.h>
void srvr_or_ap(){
  pinMode(13, INPUT);
  delay(500);
  if(digitalRead(13) == HIGH){
    // attempt to start AP:
    while (status != WL_CONNECTED) {
      Serial.print("Attempting to start AP with SSID: ");
      Serial.println(ssid);
      status = WiFi.apbegin(ssid, pass, channel);
      delay(10000);
    }
    //AP MODE already started:
    Serial.println("AP mode started");
    Serial.println();
    server.begin();
  }
  else{
    Serial.println("Will start in STA... ");
    if(readCredNVM()){
      //WiFi.begin(ssid, pass);
      isSTA = true;
      Serial.print("Connected to: ");
      Serial.println(ssid);
      return;
    }
    Serial.println("Could not connect... halting");
    while(true);
  }
  
}

bool readCredNVM(){
  FlashMemory.read();
  int len = FlashMemory.buf[0];
  ssid[len] = '\0';
  for(int i = 0; i < len; i++) ssid[i] = FlashMemory.buf[i+1];

  int pw_len = FlashMemory.buf[len+1];
  pass[pw_len] = '\0';
  for(int i = 0; i < pw_len; i++) pass[i] = FlashMemory.buf[i+len+1];
  Serial.println("Reading from NVM: ");
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("pass: ");
  Serial.print(pass);
}

bool writeCredNVM(){
  FlashMemory.read();
  FlashMemory.buf[0] = strlen(ssid);
  
  int len = strlen(ssid);
  for(int i = 0; i < len; i++) FlashMemory.buf[i+1] = ssid[i];

  int pw_len = strlen(pass);
  FlashMemory.buf[len+1] = pw_len;
  
  for(int i = 0; i < pw_len; i++) FlashMemory.buf[i+len+1] = pass[i];
  Serial.println("Writing to NVM: ");
  FlashMemory.update();
  return true;
}
void servePage(WiFiClient cl){
   char page[] = "<html><body><Form action=\"submitscript\" name =\"login\"><Input type=\"text\" name=\"txtssid\" /> <br /><Input type=\"password\" name=\"txtpass\" /><br /><Input type=\"submit\" name=\"btnSubmit\" value=\"Submit\" /></Form></body></html>";
   cl.println(page);
}

int find_text(String needle, String haystack) {
  int foundpos = -1;
  for (int i = 0; i <= haystack.length() - needle.length(); i++) {
    if (haystack.substring(i,needle.length()+i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}

bool getCreds(String str){
  int pos = find_text("ssid=", str);
  if(pos == -1){
    Serial.println("SSID not found");
    return false;
  }
  //ssid = str.substring(pos+5, str.indexOf('&')).c_str();
  strcpy(ssid, str.substring(pos+5, str.indexOf('&')).c_str());
  Serial.print("SSID entered: ");
  Serial.println(ssid);

  pos = find_text("pass=", str);  
  if(pos == -1){
    Serial.println("PASS not found");
    return false;
  }
  //pass = str.substring(pos+5, (find_text("&btn", str))).c_str();
  strcpy(pass, str.substring(pos+5, (find_text("&btn", str))).c_str());
  Serial.print("PASS entered: ");
  Serial.println(pass);
  return true;
}

