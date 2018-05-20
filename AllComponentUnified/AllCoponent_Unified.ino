#include <SPI.h>                                         // Library used in this arduino sketch
#include <arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "DHT.h"                                         // Externe library used in this arduino sketch
#include "RGBdriver.h"

#define OLED_RESET 4                                     // Attach OLED screen reset to Arduino Digital Pin 4 
Adafruit_SSD1306 display(OLED_RESET);

#define WATER_SENSOR 8                                   // Attach Water sensor to Arduino Digital Pin 8

#define LED_ARDUINO 13                                   // Attach Arduino LED to Arduino Digital Pin 13

#define PELTIER 6                                        // Attach Peltier to Arduino Digital Pin 6
#define LED_CONTROL 7                                    // Attach Control LED to Arduino Digital Pin 7

#define DHTPIN A0                                        // Attach Humidity and Temperature sensor to Arduino Analog Pin A0
#define DHTTYPE DHT11                                    // Set sensor type to DHT11 
DHT dht(DHTPIN, DHTTYPE);

#define WATER_ATOMIZER A6                                // Attach Water atomizer to Arduino Analog Pin A6

#define LED_DRIVER 2                                     // Attach LED Driver to Arduino Digital Pin 2  
#define DIO 3                                            // Attach LED Controler to Arduino Digital Pin 8
RGBdriver Driver(LED_DRIVER,DIO);

#define MOISTURE_SENSOR A4                               // Attach Moisture sensor to Arduino Analog Pin A4

#define LIGHT_SENSOR A2                                  // Attach Light sensor to Arduino Analog Pin A2

#define PIC_PKT_LEN    128                               // Data length of each read, this not should be too big because ram is limited
#define PIC_FMT_VGA    7
#define PIC_FMT_CIF    5
#define PIC_FMT_OCIF   3
#define CAM_ADDR       0
#define CAM_SERIAL     Serial1                           // Attach Camera to Arduino Serial Port 1    
#define PIC_FMT        PIC_FMT_VGA

char receive;                                            // Variable used to run TakePicture() function
char val = 0;
char initialisation = 0;
const byte cameraAddr = (CAM_ADDR << 5);                 // Camera address
unsigned long picTotalLen = 0;                           // Picture length
int picNameNum = 0;                                      // Picture Name

bool takePicture = false;                                 // Variable used to trigger component function
bool startCooler = false;
bool startLed = false;
bool startSprinkler = false;
bool getData = false;
bool getHumidity = true;
bool getHygrometry = true;
bool getTemperature = true;
bool getLight = false;
bool getWaterLevel = true;

int instruction[8] = {0};
int componentData[4] = {0};

void setup()
{   
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);           // Initialize with the I2C addr 0x3D (for the 128x64) 
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Arduino");
    display.setCursor(0,10);
    display.println("Configuration");
    display.display();
    delay(100);
    display.clearDisplay();
    
    Serial.begin(115200);                                // Set Baud Rate which is the speed of data transmission throught the Serial port
    Serial1.begin(115200);
    
    pinMode(LED_ARDUINO, OUTPUT);                        // Set LED_Arduino pin as OUTPUT
    digitalWrite(LED_ARDUINO, LOW);

    pinMode(WATER_SENSOR, INPUT);                        // Set Water sensor pin as INPUT
   
    pinMode(PELTIER, OUTPUT);                            // Set Peltier pin as OUTPUT
    digitalWrite(PELTIER, LOW );

    pinMode(LED_CONTROL, OUTPUT);                        // Set Control LED pin as OUTPUT
    digitalWrite(LED_CONTROL, LOW);

    InitializeCamera();                                  // Camera initialization, it's a handshake between the serial camera component and the borad Arduino
    InitializeArduino();                                 // Arduino initialization it's a handshake between the Arduino board and the Processing program
}

void loop()
{
    WaitingProcessingInstruction();
    for(int i = 0; i < 8; i++)
    {
       Serial.print(instruction[i]-48);
    }
    Serial.println(" ");
    InterpreteInstruction();
    if (takePicture == true)
    {
        Driver.begin();
        Driver.SetColor(255, 255, 255);
        Driver.end();  
        StartLed();
        TakeSendPicture();
        if (startLed == true)
        {
            Driver.begin();
            Driver.SetColor(62, 6, 148);
            Driver.end();  
            StartLed();            
        }
        else 
        {
            Driver.begin();
            Driver.SetColor(0, 0, 0);
            Driver.end();  
            StopLed();
        }
    }
    if (getData == true)
    {
        componentData[0] = 0;
        if (getHumidity == true)
        {
            componentData[1] = GetHumidity();
        }
        if (getHygrometry == true)
        {
            componentData[2] = GetHygrometry();
        }
        if (getTemperature == true)
        {
            componentData[3] = GetTemperature();
        }
        getData = false;
        SendDataToProcessing();
    }
    if (getLight == true)
    {
        bool lightValue = GetLight();
        getLight = false;
    }
    if (getWaterLevel == true)
    {
        bool waterLevel = GetWaterLevel();
    }
    if (startLed == true)
    {
        Driver.begin();
        Driver.SetColor(62, 6, 148);
        Driver.end();    
        StartLed();
    }
    else 
    {
        StopLed();
        Driver.begin();
        Driver.SetColor(0, 0, 0);
        Driver.end();    
        startLed = false;
    }
    if (startCooler == true)
    {
        StartCooler();
        digitalWrite(PELTIER, HIGH );
        digitalWrite(LED_CONTROL, HIGH); 
    }
    else 
    {
        StopCooler();
        digitalWrite(PELTIER, LOW );
        digitalWrite(LED_CONTROL, LOW);
        startCooler = false;
    }
    if (startSprinkler == true)
    {
        StartSprinkler();
        digitalWrite(WATER_ATOMIZER, HIGH );
        digitalWrite(LED_CONTROL, HIGH);    
    }
    else 
    {
        StopSprinkler();    
        digitalWrite(WATER_ATOMIZER, LOW );
        digitalWrite(LED_CONTROL, LOW);  
        startSprinkler = false;
    }
    delay(1000);
}
/*********************************************************************/
void InterpreteInstruction()
{  
    if(instruction[0] == '1')
    {
        getData = true;
    }
    else
    {
        getData = false;
    }
    if(instruction[1] == '1')
    {
        takePicture = true;
    }
    else
    {
        takePicture = false;
    }
    if(instruction[2] == '1')
    {
        startCooler = true;
    }
    else
    {
        startCooler = false;
    }
    if(instruction[3] == '1')
    {
        startSprinkler = true;
    }
    else
    {
        startSprinkler = false;
    }
    if(instruction[4] == '1')
    {
        startLed = true;
    }
    else
    {
        startLed = false;
    }
    if(instruction[5] == '1')
    {
        getLight = true;
    }
    else
    {
        getLight = false;
    }
    if(instruction[6] == '1')
    {
        getWaterLevel = true;
    }
    else
    {
        getWaterLevel = false;
    }    
}
/*********************************************************************/
void WaitingProcessingInstruction()
{
    while(true)
    {        
        for(int i = 0; i < 8; i++)
        {
            instruction[i] = 0;
        }   
        while(Serial.available())
        {                     
            for(int i = 0; i < 8; i++)
            {
                instruction[i] = Serial.read();
            }           
        }
        if(instruction[7] == '1')
        {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.println("Processing");
            display.setCursor(0,10);
            display.println("Instruction");
            display.setCursor(0,20);
            display.println("Received");
            display.display();
            delay(1000);
            break;
        }
        else
        {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.println("Waiting");
            display.setCursor(0,10);
            display.println("Processing");
            display.setCursor(0,20);
            display.println("Instruction ....");
            display.display();
        }
    }
}
/*********************************************************************/
void SendDataToProcessing()
{
    for(int i = 0; i < 4; i++)
    {
        Serial.println(componentData[i]);
    }
}
/*********************************************************************/
void StartSprinkler()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Sprinkler is");
    display.setCursor(0,10);
    display.println("ON");
    display.display();
    delay(500);
}
/*********************************************************************/
void StopSprinkler()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Sprinkler is");
    display.setCursor(0,10);
    display.println("OFF");
    display.display();  
    delay(500);
}
/*********************************************************************/
void StartCooler()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Peletier is");
    display.setCursor(0,10);
    display.println("ON");
    display.display();
    delay(500);
}
/*********************************************************************/
void StopCooler()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Peletier is");
    display.setCursor(0,10);
    display.println("OFF");
    display.display();
    delay(500);
}
/*********************************************************************/
void StopLed()
{
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Led are");
    display.setCursor(0,10);
    display.println("OFF");
    display.display(); 
    delay(500);
}
/*********************************************************************/
void StartLed()
{   
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Led are");
    display.setCursor(0,10);
    display.println("ON");
    display.display();
    delay(500);
}
/*********************************************************************/
int GetWaterLevel()
{
    if(digitalRead(WATER_SENSOR) == LOW)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Contact with");
        display.setCursor(0,10);
        display.println("water");
        display.display();
        delay(500);        
        return(1);
    }
    else
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("No contact");
        display.setCursor(0,10);
        display.println("with water");
        display.display();
        delay(500);        
        return(0);
    }
}
/*********************************************************************/
int GetLight()
{
    int value = analogRead(A2);
    value = map(value, 0, 800, 0, 10);

    if(value <= 3)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Light value :");
        display.setCursor(0,10);
        display.println(value);
        display.setCursor(0,20);
        display.println("Light is OFF");
        display.display();
        delay(500);        
        return(0);
    }
    else
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Light value :");
        display.setCursor(0,10);
        display.println(value);
        display.setCursor(0,20);
        display.println("Light is ON");
        display.display();
        delay(500);        
        return(1);
    }
}
/*********************************************************************/
int GetHumidity()
{
    int sensorValue = analogRead(MOISTURE_SENSOR);
    sensorValue = map(sensorValue, 0, 950, 0, 100);

    if(sensorValue <= 50)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Humidity value :");
        display.setCursor(0,10);
        display.println(sensorValue);
        display.setCursor(0,20);
        display.println("Soil is DRY");
        display.display();
        delay(500);        
    }
    else if(sensorValue <= 70)
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Humidity value :");
        display.setCursor(0,10);
        display.println(sensorValue);
        display.setCursor(0,20);
        display.println("Soil is HUMID");
        display.display();
        delay(500);
    }
    else
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Humidity value :");
        display.setCursor(0,10);
        display.println(sensorValue);
        display.setCursor(0,20);
        display.println("Soil is FLOOD");
        display.display();
        delay(500);
    }
    return(sensorValue);
}
/*********************************************************************/
int GetHygrometry()
{
    float h = dht.readHumidity();
    
    if (isnan(h)) 
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Faild to read");
        display.setCursor(0,10);
        display.println("from DHT");
        display.display();
        delay(5000);              
    }
    else
    {
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.print("Humidity:    ");
        display.print(h);
        display.println(" %");
        delay(5000);    
        return((int) h);
    }
}
/*********************************************************************/
int GetTemperature()
{
    float t = dht.readTemperature();
    
    if (isnan(t)) 
    {
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.println("Faild to read");
        display.setCursor(0,10);
        display.println("from DHT");
        display.display();
        delay(500);        
    }
    else
    {      
        display.clearDisplay();
        display.setTextSize(1);
        display.setTextColor(WHITE);
        display.setCursor(0,0);
        display.print("Temperature: ");
        display.print(t);
        display.println(" C");
        display.display();
        delay(500);   
        return((int) t);
    }
}
/*********************************************************************/
void TakeSendPicture()
{
    PreCapture();
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println("Camera ready");
    display.display();
    delay(200);
    Capture();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,10);
    display.println("Photo captured");
    display.display();
    delay(200);
    GetData();
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,20);
    display.println("Photo sent");
    display.display();
    delay(1000);
    takePicture = false;
}
/*********************************************************************/
void InitializeArduino()
{
    while( initialisation != '1' )
    {                                                   
        initialisation = Serial.read();
        if(initialisation == '1')
        {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.println("Initialisation");
            display.setCursor(0,10);
            display.println("Processing :");
            display.setCursor(0,20);
            display.println("   Done");
            display.display();
            delay(1000);
        }
        else
        {
            display.clearDisplay();
            display.setTextSize(1);
            display.setTextColor(WHITE);
            display.setCursor(0,0);
            display.println("Initialisation");
            display.setCursor(0,10);
            display.println("Processing :");
            display.setCursor(0,20);
            display.println("   ....");
            display.display();
        }
    }
}
/*********************************************************************/
void ClearRxBuf()
{
    while (Serial1.available())
    {
        Serial1.read();
    }
}
/*********************************************************************/
void SendCmd(char cmd[], int cmd_len)
{
    for (char i = 0; i < cmd_len; i++) Serial1.print(cmd[i]);
}
/*********************************************************************/
void InitializeCamera()
{
    char cmd[] = {0xaa,0x0d|cameraAddr,0x00,0x00,0x00,0x00} ;
    unsigned char resp[6];

    Serial1.setTimeout(500);
    while (1)
    {
        //ClearRxBuf();
        SendCmd(cmd,6);
        if (Serial1.readBytes((char *)resp, 6) != 6)
        {
            continue;
        }
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x0d && resp[4] == 0 && resp[5] == 0)
        {
            if (Serial1.readBytes((char *)resp, 6) != 6) continue;
            if (resp[0] == 0xaa && resp[1] == (0x0d | cameraAddr) && resp[2] == 0 && resp[3] == 0 && resp[4] == 0 && resp[5] == 0) break;
        }
    }
    cmd[1] = 0x0e | cameraAddr;
    cmd[2] = 0x0d;
    SendCmd(cmd, 6);       
}
/*********************************************************************/
void PreCapture()
{
    char cmd[] = { 0xaa, 0x01 | cameraAddr, 0x00, 0x07, 0x00, PIC_FMT }; // Initial Commande
    unsigned char resp[6];

    Serial1.setTimeout(100);
    while (1)
    {
        ClearRxBuf();
        SendCmd(cmd, 6);
        if (Serial1.readBytes((char *)resp, 6) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x01 && resp[4] == 0 && resp[5] == 0) break;
    }
}
/*********************************************************************/
void Capture()
{
    char cmd[] = { 0xaa, 0x06 | cameraAddr, 0x08, PIC_PKT_LEN & 0xff, (PIC_PKT_LEN>>8) & 0xff ,0}; // Commande SetPackageSize
    unsigned char resp[6];

    Serial1.setTimeout(100);
    while (1)
    {
        ClearRxBuf();
        SendCmd(cmd, 6);
        if (Serial1.readBytes((char *)resp, 6) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x06 && resp[4] == 0 && resp[5] == 0) break;
    }
    cmd[1] = 0x05 | cameraAddr;
    cmd[2] = 0;
    cmd[3] = 0;
    cmd[4] = 0;
    cmd[5] = 0;
    while (1)
    {
        ClearRxBuf();
        SendCmd(cmd, 6);// Commande snapshot
        if (Serial1.readBytes((char *)resp, 6) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x05 && resp[4] == 0 && resp[5] == 0) break;
    }
    cmd[1] = 0x04 | cameraAddr;
    cmd[2] = 0x1; //cmd[2] = 0x01;
    while (1)
    {
        ClearRxBuf();
        SendCmd(cmd, 6); // Commande GetPicture
        if (Serial1.readBytes((char *)resp, 6) != 6) continue;
        if (resp[0] == 0xaa && resp[1] == (0x0e | cameraAddr) && resp[2] == 0x04 && resp[4] == 0 && resp[5] == 0)
        {
            Serial1.setTimeout(1000);
            if (Serial1.readBytes((char *)resp, 6) != 6)
            {
                continue;
            }
            if (resp[0] == 0xaa && resp[1] == (0x0a | cameraAddr) && resp[2] == 0x01)
            {
                picTotalLen = (resp[3]) | (resp[4] << 8) | (resp[5] << 16);
                //Serial.print("picTotalLen: ");
                //Serial.print("0");
                Serial.println(picTotalLen);
                break;
            }
        }
    }
}
/*********************************************************************/
void GetData()
{
  delay(1000);
    unsigned int pktCnt = (picTotalLen) / (PIC_PKT_LEN - 6);
    if ((picTotalLen % (PIC_PKT_LEN-6)) != 0) pktCnt += 1;

    char cmd[] = { 0xaa, 0x0e | cameraAddr, 0x00, 0x00, 0x00, 0x00 };
    unsigned char pkt[PIC_PKT_LEN];

    char picName[] = "pic00.jpg";
    picName[3] = picNameNum/10 + '0';
    picName[4] = picNameNum%10 + '0';

    Serial1.setTimeout(1000);
    for (unsigned int i = 0; i < pktCnt; i++)
    {
        cmd[4] = i & 0xff;
        cmd[5] = (i >> 8) & 0xff;

        int retry_cnt = 0;
        retry:
        delay(10);
        ClearRxBuf();
        SendCmd(cmd, 6);
        uint16_t cnt = Serial1.readBytes((char *)pkt, PIC_PKT_LEN);

        unsigned char sum = 0;
        for (int y = 0; y < cnt - 2; y++)
        {
            sum += pkt[y];
        }
        if (sum != pkt[cnt-2])
        {
            if (++retry_cnt < 100) goto retry;
            else break;
        }

        Serial.write((const uint8_t *)&pkt[4], cnt-6);
        //if (cnt != PIC_PKT_LEN) break;
    }
    cmd[4] = 0xf0;
    cmd[5] = 0xf0;
    SendCmd(cmd, 6);
    picNameNum ++;
}
