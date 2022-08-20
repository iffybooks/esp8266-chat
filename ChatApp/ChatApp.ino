//Source: https://devcraze.com/tutorials/internet-of-things/create-awesome-wifi-chat-with-captive-portal-using-nodemcu-esp8266/
//Chat messages are located at http://10.10.10.1/readMessages

//Include wifi library, filesystem and DNSServer for our Captive Portal
#include <ESP8266WiFi.h>
#include "./DNSServer.h"
#include <ESP8266WebServer.h>
#include "FS.h"

const byte DNS_PORT = 53;
const String messagesFile = "/messages.txt";  //The messages will be stores here. 
const String chatFile = "/chat.html"; //This is our HTML file with javascript
const char *wifiName = "Chat With Me";  //This is your wireless AP name. 
String chatHtml;

//You can type this IP in your Browser. 
IPAddress apIP(10, 10, 10, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);
void setup()
{
  Serial.begin(115200);
  SPIFFS.begin();
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(wifiName);
  dnsServer.start(DNS_PORT, "*", apIP);
  chatHtml = fileRead(chatFile);  //Read our HTML file that will be served
  webServer.begin();  //Start our web server
  setupAppHandlers();
  handleSendMessage();
  showChatPage();
}
//Check if the request has data with a message name. 
//Write to our messages.txt file. 
//Send a response back to our request. Access-Control-Allow-Origin *is added to allow Cross-Origin Resource Sharing
void handleSendMessage()
{
  if (webServer.hasArg("message"))
  {
    String message = webServer.arg("message");
    fileWrite(messagesFile, message + "\n", "a+");
    webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
    webServer.sendHeader("Access-Control-Allow-Origin", "*");
    webServer.send(200, "text/plain", "Message Sent");
  }
}
//Delete the messages.txt file in our file system and send back a response to our request. 
void handleClearMessages()
{
  SPIFFS.remove(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", "File Deleted");
}
//Display our chat page to our users
void showChatPage()
{
  webServer.send(200, "text/html", chatHtml);
}
//read messages.txt and send it back to users. 
void showMessages()
{
  String messages = fileRead(messagesFile);
  webServer.sendHeader("Access-Control-Allow-Methods", "POST,GET,OPTIONS");
  webServer.sendHeader("Access-Control-Allow-Origin", "*");
  webServer.send(200, "text/plain", messages);
}
void setupAppHandlers()
{
  webServer.onNotFound([]()
  {
    //ON 404 page not found, show our chat page. 
    showChatPage();
  });
  //When our browser send request to this url, it will trigger the action defined in our method. 
  webServer.on("10.10.10.1", showChatPage);
  webServer.on("/sendMessage", handleSendMessage);
  webServer.on("/readMessages", showMessages);
  webServer.on("/clearMessages", handleClearMessages);
}
//Method that reads the file stored in SPIFFS and return a string
String fileRead(String name)
{
  String contents;
  int i;
  File file = SPIFFS.open(name, "a+");
  for (i = 0; i < file.size(); i++)
  {
    contents += (char) file.read();
  }
  file.close();
  return contents;
}
//Method to write text in a text file
void fileWrite(String name, String content, String mode)
{
  File file = SPIFFS.open(name.c_str(), mode.c_str());
  file.write((uint8_t*) content.c_str(), content.length());
  file.close();
}
//Handle requests
void loop()
{
  dnsServer.processNextRequest();
  webServer.handleClient();
}
