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
    
  }
  else{
    Serial.println("Will start in STA... ");
    if(readCredNVM()){
      while (status != WL_CONNECTED){
        Serial.print("Attempting to connect to ssid: "); Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(5000);
      }
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
  Serial.println("Reading from NVM: ");
  FlashMemory.read();
  byte len = FlashMemory.buf[0];
  byte plen = FlashMemory.buf[len+1];
  ssid[len] = '\0';
  pass[plen] = '\0';

  //read next i char beginning at pos
  byte pos = 1;
  byte i = len;
  while(i > 0) {
    ssid[pos - 1] = FlashMemory.buf[pos];
    pos++;
    i--;
  }

  pos = len + 2;
  i = plen;
  while(i > 0){
    pass[plen-i] = FlashMemory.buf[pos];
    i--;
    pos++;
  }
  
  Serial.print("SSID: ");
  Serial.println(ssid);
  Serial.print("pass: ");
  Serial.print(pass);
  return true;
}

bool writeCredNVM(){
  FlashMemory.read();

  int len = strlen(ssid);
  FlashMemory.buf[0] = len;

  Serial.print("FlashMemory.buf[0] = ");
  Serial.println(len);
  
  
  for(int i = 0; i < len; i++) {
    FlashMemory.buf[i+1] = ssid[i];
    Serial.print("FlashMemory.buf[");
    Serial.print(i+1);
    Serial.print("] = ");
    Serial.println(ssid[i]);
    
  }

  int pw_len = strlen(pass);
  FlashMemory.buf[len+1] = pw_len;
  Serial.print("pw_len = ");
  Serial.print(pw_len);
  Serial.print("stored at: ");
  Serial.println(len + 1);
  
  for(int i = 0; i < pw_len; i++) {
    FlashMemory.buf[i+len+2] = pass[i];
    Serial.print("FlashMemory.buf[");
    Serial.print(i+len+2);
    Serial.print("] = ");
    Serial.println(pass[i]);
  }
  Serial.println("Writing to NVM");
  FlashMemory.update();

  FlashMemory.read();
  Serial.print("SSID len: "); Serial.println(FlashMemory.buf[0]);
  Serial.print("Pass len: "); Serial.println(FlashMemory.buf[(FlashMemory.buf[0] + 1)]);
  
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

