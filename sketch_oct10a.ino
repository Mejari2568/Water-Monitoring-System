/* Water level monitoring system with the New Blynk app
   https://srituhobby.com
*/
#define BLYNK_TEMPLATE_ID "TMPL3OPyXNklR"
#define BLYNK_TEMPLATE_NAME "WATER LEVEL MONITORING SYSTEM"

// Include the library files
#include <LiquidCrystal_I2C.h>
#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>

// Initialize the LCD display
LiquidCrystal_I2C lcd(0x27, 16, 2);

char auth[] = "htkBcAD2SNswxwQ_fFzjEQGFp_-RVfPA"; // Enter your Auth token
char ssid[] = "Mejari"; // Enter your WIFI name
char pass[] = "Mejari@123"; // Enter your WIFI password

BlynkTimer timer;

// Define the component pins
#define trig D7
#define echo D8
#define LED1 D0  // Low level indicator
#define LED2 D3  // Medium level indicator
#define LED3 D4  // High level indicator
#define relay 3   // Relay for controlling the motor or pump
#define buzzer D5  // Buzzer pin

// Enter your tank max value (CM)
int MaxLevel = 100;  // Assuming 100 cm as the max tank level

// Adjust the three levels
int LowLevel = (MaxLevel * 25) / 100;   // 25% of tank height
int MediumLevel = (MaxLevel * 65) / 100; // 65% of tank height (increased distance between Medium and High)
int HighLevel = (MaxLevel * 85) / 100;   // 85% of tank height (decreased High level range)

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();
  pinMode(trig, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  pinMode(buzzer, OUTPUT);  // Set the buzzer pin as output
  pinMode(relay, OUTPUT);   // Set the relay pin as output
  digitalWrite(relay, HIGH);  // Ensure relay is OFF initially (HIGH means off in most relay modules)
  digitalWrite(buzzer, LOW);  // Ensure buzzer is off initially
  Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);

  lcd.setCursor(0, 0);
  lcd.print("Water level");
  lcd.setCursor(4, 1);
  lcd.print("Monitoring");
  delay(4000);
  lcd.clear();

  // Call the functions
  timer.setInterval(100L, ultrasonic);
}

// Get the ultrasonic sensor values
void ultrasonic() {
  digitalWrite(trig, LOW);
  delayMicroseconds(4);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig, LOW);
  long t = pulseIn(echo, HIGH);
  int distance = t / 29 / 2;

  int waterLevel = MaxLevel - distance;  // Calculate water level based on distance
  int waterLevelRange = map(waterLevel, 0, MaxLevel, 1, 5); // Map the water level to a range of 1 to 5

  // Update the Blynk virtual pin V0 with the mapped range value
  if (waterLevel >= 0 && waterLevel <= MaxLevel) {
    Blynk.virtualWrite(V0, waterLevelRange);  // Send the range value (1 to 5) to Blynk V0
  } else {
    Blynk.virtualWrite(V0, 0);
  }

  lcd.setCursor(0, 0);
  lcd.print("WLevel:");

  // Determine the level based on water height and control the relay
  if (waterLevel < LowLevel) {
    lcd.setCursor(8, 0);
    lcd.print("Low ");
    digitalWrite(LED1, HIGH);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, LOW);
    digitalWrite(buzzer, LOW);  // Buzzer off
    digitalWrite(relay, LOW);  // Turn ON the relay (pump starts) when water level is low
  } else if (waterLevel >= LowLevel && waterLevel < MediumLevel) {
    lcd.setCursor(8, 0);
    lcd.print("Medium");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, HIGH);
    digitalWrite(LED3, LOW);
    digitalWrite(buzzer, LOW);  // Buzzer off
    digitalWrite(relay, HIGH);  // Turn OFF the relay (pump stops) when water level is medium
  } else if (waterLevel >= MediumLevel && waterLevel < HighLevel) {
    lcd.setCursor(8, 0);
    lcd.print("High  ");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    digitalWrite(buzzer, LOW);  // Buzzer on when level is high
    digitalWrite(relay, HIGH);  // Turn OFF the relay (pump stops) when water level is high
  } else {
    lcd.setCursor(8, 0);
    lcd.print("Overflow!");
    digitalWrite(LED1, LOW);
    digitalWrite(LED2, LOW);
    digitalWrite(LED3, HIGH);
    digitalWrite(buzzer, HIGH);  // Buzzer on during overflow
    digitalWrite(relay, HIGH);  // Turn OFF the relay (pump stops)
  }
}

// Get the button value
BLYNK_WRITE(V1) {
  bool Relay = param.asInt();
  if (Relay == 1) {
    digitalWrite(relay, LOW);  // Manually turn ON the relay
    lcd.setCursor(0, 1);
    lcd.println("Motor is ON ");
  } else {
    digitalWrite(relay, HIGH);  // Manually turn OFF the relay
    lcd.setCursor(0, 1);
    lcd.println("Motor is OFF");
  }
}

void loop() {
  Blynk.run();  // Run the Blynk library
  timer.run();  // Run the Blynk timer
}
