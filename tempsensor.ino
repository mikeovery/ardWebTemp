#include <DHT.h>

#define DHTPIN 2     // what pin we're connected to

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// Connect pin 2 of the sensor to whatever your DHTPIN is
// Connect pin 4 (on the right) of the sensor to GROUND
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

DHT dht(DHTPIN, DHTTYPE);

#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0x1C, 0xED, 0x00, 0x0D, 0xEC, 0xAF }; //MAC address found on the back of your ethernet shield.
IPAddress ip(10,0,0,112); // IP address dependent upon your network addresses.

// PIR Config
int ledPin = 12;                // choose the pin for the LED
int inputPin = 3;               // choose the input pin (for PIR sensor)
int pirState = LOW;             // we start, assuming no motion detected
int val = 0;                    // variable for reading the pin status

// Speaker
int speakerPin = 9;
int length = 3; // the number of notes
//char notes[] = "ccggaagffeeddc "; // a space represents a rest
char notes[] = "cg "; // a space represents a rest
char notes2[] = "gc "; // a space represents a rest
int beats[] = { 1, 1, 1, 1, 1, 1, 2, 1, 1, 1, 1, 1, 1, 2, 4 };
int tempo = 300;

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  pinMode(ledPin, OUTPUT);      // declare LED as output
  pinMode(inputPin, INPUT);     // declare sensor as input
  pinMode(speakerPin, OUTPUT);  // declare speaker as output
 
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  dht.begin();

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}


void loop() {
 
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = 0; //dht.readHumidity();
  float t = 0; //dht.readTemperature();
  int iPIR = PIR();

  // check if returns are valid, if they are NaN (not a number) then something went wrong!
  if (isnan(t) || isnan(h)) {
    Serial.println("Failed to read from DHT");
  } else {
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print("%\t");
    Serial.print("Temperature: ");
    Serial.print(t);
    Serial.print(" *C\t");
    Serial.print("Motion: ");
    Serial.print(iPIR);
    Serial.println("\t");
  }
 
  // listen for incoming clients
  EthernetClient client = server.available();
  if (client) {
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n' && currentLineIsBlank) {
          // send a standard http response header
          client.println("HTTP/1.1 200 OK");
          client.println("Content-Type: text/html");
          client.println("Connection: close");  // the connection will be closed after completion of the response
          client.println("Refresh: 10");  // refresh the page automatically every 5 sec
          client.println();
          client.println("<!DOCTYPE HTML>");
          client.println("<html>");
          // output the value of the DHT-11
            client.print("Humidity: ");
            if (isnan(h))
            {
               client.print("-");
            }
            else
            {
               client.print(h);              
               client.print(" %\t");
            }
            client.println("<br/>");
            client.print("Temperature: ");
            if (isnan(t))
            {
               client.print("-");
            }
            else
            {
               client.print(t);              
               client.print(" &#176;C");
            }
            client.println("<br/>");
            client.print("Motion: ");
            if (isnan(iPIR))
            {
               client.print("-");
            }
            else
            {
               if (iPIR == HIGH)
               {
                  client.print("Motion Detected");
               }
               else
               {
                  client.print("No Motion Detected");
               }
            }
            client.println("<br/>");
          client.println("</html>");
          break;
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        }
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disonnected");
  }
  delay(500);
}

int PIR()
{
  val = digitalRead(inputPin);  // read input value
  if (val == HIGH) 
  {            
    digitalWrite(ledPin, HIGH);  // turn LED ON
    if (pirState == LOW) 
    {
      // we have just turned on
      Serial.println("Motion detected!");
      playTune1();
      // We only want to print on the output change, not state
      pirState = HIGH;
    }
  } 
  else 
  {
    digitalWrite(ledPin, LOW); // turn LED OFF
    if (pirState == HIGH)
    {
      // we have just turned of
      Serial.println("Motion ended!");
      playTune2();
      // We only want to print on the output change, not state
      pirState = LOW;
    }
  }
  return val;
}

void playTone(int tone, int duration) 
{
  for (long i = 0; i < duration * 1000L; i += tone * 2) 
  {
    digitalWrite(speakerPin, HIGH);
    delayMicroseconds(tone);
    digitalWrite(speakerPin, LOW);
    delayMicroseconds(tone);
  }
}

void playNote(char note, int duration) 
{
  char names[] = { 'c', 'd', 'e', 'f', 'g', 'a', 'b', 'C' };
  int tones[] = { 1915, 1700, 1519, 1432, 1275, 1136, 1014, 956 };

  // play the tone corresponding to the note name
  for (int i = 0; i < 8; i++) 
  {
    if (names[i] == note) 
    {
      playTone(tones[i], duration);
    }
  }
}

void playTune1()
{
    for (int i = 0; i < length; i++) 
    {
      if (notes[i] == ' ') 
      {
        delay(beats[i] * tempo); // rest
      } 
      else 
      {
        playNote(notes[i], beats[i] * tempo);
      }
    }
    // pause between notes
    delay(tempo / 2); 
}

void playTune2()
{
    for (int i = 0; i < length; i++) 
    {
      if (notes2[i] == ' ') 
      {
        delay(beats[i] * tempo); // rest
      } 
      else 
      {
        playNote(notes2[i], beats[i] * tempo);
      }
    }
    // pause between notes
    delay(tempo / 2); 
}

