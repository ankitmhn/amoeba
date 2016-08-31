#include <FlashMemory.h>
#include <WiFi.h>

// Number of milliseconds to wait without receiving any data before we give up
const int kNetworkTimeout = 30*1000;
// Number of milliseconds to wait if no data is available before trying again
const int kNetworkDelay = 1000;

const char kHostname[] = "www.ncdc.noaa.gov";

const char kPath[] = "/cdo-web/api/v2/data?datasetid=GHCND&locationid=ZIP:28801&startdate=2016-08-04&enddate=2016-08-05";

const byte profile[10][4] =
{
  {5, 10 , 15, 20},
  {1, 2 , 3 ,4},
  {2, 3, 4, 5},
  {1, 3, 6, 9},
  {2, 4, 6, 8},
  {5, 15, 20, 25},
  {10, 15, 20, 30},
  {7, 15, 22, 28},
  {8, 12, 16, 20},
  {4, 8, 12, 16}
  
};

void setSchedule(){
  //TODO: Create function to select appropriate profile based on time
  
  for(byte i = 0 ; i < 4; i++ ) schedule[i] = profile[1][i];

  Serial.println("Schedule is set.");
}

void setZones(){
  //TODO: Init zones at powerup
  for(byte i = 0; i < 4; i++) {    
    toggle_tme_zones[i]  = millis();
  }
}

void toggleZones(){

  //check time elapsed since last toggle and compare with schedule
  for(byte i = 0; i < 4; i++){

    //convert min to millisec
    unsigned long period = schedule[i] * 60 * 1000; 
    unsigned long now = millis();

    for(byte i = 0; i < 4; i++){
      
      if( (now - toggle_tme_zones[i])  > schedule[i] )
        digitalWrite(i+1, !digitalRead(i+1));
    }

    
  }
}
void GetNOAA(){
  int err =0;
  
  WiFiClient c;
  HttpClient http(c);
  bool success = true;
  String response = "";
  
  err = http.get(kHostname, kPath);
  if (err == 0)
  {
    Serial.println("startedRequest ok");

    err = http.responseStatusCode();
    if (err >= 0)
    {
      Serial.print("Got status code: ");
      Serial.println(err);

      // Usually you'd check that the response code is 200 or a
      // similar "success" code (200-299) before carrying on,
      // but we'll print out whatever response we get

      err = http.skipResponseHeaders();
      
      if (err >= 0)
      {
        int bodyLen = http.contentLength();
        Serial.print("Content length is: ");
        Serial.println(bodyLen);
        Serial.println();
        Serial.println("Body returned follows:");
      
        // Now we've got to the body, so we can print it out
        unsigned long timeoutStart = millis();
        char c;
        // Whilst we haven't timed out & haven't reached the end of the body
        while ( (http.connected() || http.available()) &&
               ((millis() - timeoutStart) < kNetworkTimeout) )
        {
            if (http.available())
            {
                c = http.read();
                response = response + c;
                // Print out this character
                Serial.print(c);
               
                bodyLen--;
                // We read something, reset the timeout counter
                timeoutStart = millis();
            }
            else
            {
                // We haven't got any data, so let's pause to allow some to
                // arrive
                delay(kNetworkDelay);
            }
        }
      }
      else
      {
        Serial.print("Failed to skip response headers: ");
        Serial.println(err);
        success = false;
      }
    }
    else
    {    
      Serial.print("Getting response failed: ");
      Serial.println(err);
      success = false;
    }
  }
  else
  {
    Serial.print("Connect failed: ");
    Serial.println(err);    
    success = false;
  }
  http.stop();
  
  // And just stop, now that we've tried a download
  //while(1);
  
  if(success) parseResponse(response);
}
void parseResponse(String noaa){
  //Algorithm to parse the NOAA data goes here
}

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

