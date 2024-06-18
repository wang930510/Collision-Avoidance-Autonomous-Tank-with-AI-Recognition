#define ARDUINOJSON_STRING_LENGTH_SIZE 4
#include "StreamIO.h"
#include "WiFi.h"
#include "AmebaFatFS.h"
#include "Base64.h"
#include "ArduinoJson.h"
#include "BLEDevice.h"
#include "VideoStream.h"
#include "RTSP.h"
//red
#include <Wire.h>
#include <VL53L0X_IR_libraries/VL53L0X.h>
VL53L0X sensor;

#define CHANNEL 1
#define AUDIOFILENAME "TestRecordingAudioOnly.mp4"
#define UART_SERVICE_UUID "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define STRING_BUF_SIZE 100
#define MaxNumValue 3  //將2改為3，得以使用value3
#define value1 0
#define value2 1
#define value3 2
#define MotoA_1A 16
#define MotoA_1B 7
#define MotoB_1A 17
#define MotoB_1B 8

VideoSetting config(VIDEO_FHD, CAM_FPS, VIDEO_JPEG, 1);
RTSP rtsp1;
RTSP rtsp2;
StreamIO videoStreamer(1, 2);

char ssid[] = "bbbb87cry";
char pass[] = "00000000";
int status = WL_IDLE_STATUS;

char server[] = "123.195.32.57";
#define PORT 5000

typedef struct {
    bool reciveCMDFlag;
    int ReciveValue;
} _rCMD;

BLEService UartService(UART_SERVICE_UUID);
BLECharacteristic Rx(CHARACTERISTIC_UUID_RX);
BLECharacteristic Tx(CHARACTERISTIC_UUID_TX);
BLEAdvertData advdata;
BLEAdvertData scndata;
bool notify = false;
uint8_t Count;

String CMDRefer[5] = {"SS2", "SS4", "SRT", "SR2", "SRV"};
_rCMD bleReciveData[MaxNumValue];

AmebaFatFS fs;
WiFiClient wifiClient;

//問題內容(可調整)
char prompt[] = "what do you see,give answer in a few words.";

int encodedImageLen;
char *encodedImageData;

uint32_t img_addr = 0;
uint32_t img_len = 0;

void forward() {
    
    digitalWrite(MotoA_1A, 0);
    analogWrite(MotoA_1B, 250);
    digitalWrite(MotoB_1A, 0);
    analogWrite(MotoB_1B, 250);

    delay(50);
}

void backward() {

    digitalWrite(MotoA_1A, 1);
    analogWrite(MotoA_1B, 5);
    digitalWrite(MotoB_1A, 1);
    analogWrite(MotoB_1B, 5);

    delay(50);
}
//原程式對兩個馬達皆有寫入程式，但原程式會造成馬達轉向不如預期，因而調整部分程式碼。
void turnLeft() {

    digitalWrite(MotoB_1A, 0);
    analogWrite(MotoB_1B, 250);
    //刪除MotoA程式防止出錯
    delay(50);
}

void turnRight() {

    digitalWrite(MotoA_1A, 0);
    analogWrite(MotoA_1B, 250);
    //刪除MotoB程式防止出錯
    delay(50);
}

void BrakeAll() {

    digitalWrite(MotoA_1A, 0);
    analogWrite(MotoA_1B, 0);
    digitalWrite(MotoB_1A, 0);
    analogWrite(MotoB_1B, 0);

    delay(50);
}

void readCB(BLECharacteristic* chr, uint8_t connID) {
    printf("Characteristic %s read by connection %d \n", chr->getUUID().str(), connID);
}

void writeCB(BLECharacteristic* chr, uint8_t connID) {
    if (chr->getDataLen() > 0) {
        ParseCMDString(chr->readString());
    }
}

void notifCB(BLECharacteristic* chr, uint8_t connID, uint16_t cccd) {
    if (cccd & GATT_CLIENT_CHAR_CONFIG_NOTIFY) {
        printf("Notifications enabled on Characteristic %s for connection %d \n", chr->getUUID().str(), connID);
        notify = true;
    } else {
        printf("Notifications disabled on Characteristic %s for connection %d \n", chr->getUUID().str(), connID);
        notify = false;
    }
}

void ParseCMDString(String cmd) {
    int comdLength = cmd.length();
    int chkx;
    int CMDMaxNUM = sizeof(CMDRefer) / sizeof(String);

    for (chkx = 0; chkx < CMDMaxNUM; chkx++) {
        if (cmd.indexOf(CMDRefer[chkx].c_str()) > -1) {
            break;
        }
    }

    if (chkx >= CMDMaxNUM && cmd.charAt(comdLength - 1) != '#') {
        return;
    }
//在ParseCMDString函數中，x的最終值為3，而循環條件x < (comdLength - 1)可能會導致訪問越界cmd。新增條件cmd.length() > 4
    if (cmd.indexOf("SRT") > -1 && cmd.length() > 4) {
        int x = 3;
        int ValueIndex = 0;

        while (x < (comdLength - 1)) {
            if ((x + 3) < comdLength) {
                String _NumString = cmd.substring(x, (x + 4));
                if (ValueIndex < MaxNumValue) {
                    if (bleReciveData[ValueIndex].ReciveValue != _NumString.toInt()) {
                        bleReciveData[ValueIndex].ReciveValue = _NumString.toInt();
                        bleReciveData[ValueIndex].reciveCMDFlag = true;
                    }
                }
            }
            ValueIndex++;
            x += 4;
        }
    }
}

void printInfo(void) {
    Serial.println("------------------------------");
    Serial.println("- Summary of Streaming -");
    Serial.println("------------------------------");
    Camera.printInfo();

    IPAddress ip = WiFi.localIP();

    Serial.println("- RTSP -");
    Serial.print("rtsp://");
    Serial.print(ip);
    Serial.print(":");
    rtsp1.printInfo();

    Serial.print("rtsp://");
    Serial.print(ip);
    Serial.print(":");
    rtsp2.printInfo();
}

void setup() {
//red
Wire.begin();
if (!sensor.init()) {
        Serial.println("Failed to detect and initialize sensor!");
        while (1) {
        }
    }
    sensor.startContinuous();
//red end    

    Serial.begin(115200);
    buttonState = digitalRead(buttonPin);
    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();
    Camera.channelBegin(CHANNEL);

    while (status != WL_CONNECTED) {
        Serial.print("Attempting to connect to WPA SSID: ");
        Serial.println(ssid);
        status = WiFi.begin(ssid, pass);
        delay(2000);
    }

    fs.begin();
    pinMode(buttonPin, INPUT);
    pinMode(LED_BUILTIN, OUTPUT);
    pinMode(LED_G, OUTPUT);

    advdata.addFlags(GAP_ADTYPE_FLAGS_LIMITED | GAP_ADTYPE_FLAGS_BREDR_NOT_SUPPORTED);
    advdata.addCompleteName("AMB82-OCTO");
    scndata.addCompleteServices(BLEUUID(UART_SERVICE_UUID));

    Rx.setWriteNRProperty(true);
    Rx.setWritePermissions(GATT_PERM_WRITE);
    Rx.setWriteCallback(writeCB);
    Rx.setBufferLen(STRING_BUF_SIZE);

    Tx.setReadProperty(true);
    Tx.setReadPermissions(GATT_PERM_READ);
    Tx.setReadCallback(readCB);
    Tx.setNotifyProperty(true);
    Tx.setCCCDCallback(notifCB);
    Tx.setBufferLen(STRING_BUF_SIZE);

    UartService.addCharacteristic(Rx);
    UartService.addCharacteristic(Tx);

    BLE.init();
    BLE.configAdvert()->setAdvData(advdata);
    BLE.configAdvert()->setScanRspData(scndata);
    BLE.configServer(1);
    BLE.addService(UartService);
    BLE.beginPeripheral();

    Camera.configVideoChannel(CHANNEL, config);
    Camera.videoInit();

    rtsp1.configVideo(config);
    rtsp1.begin();
    rtsp2.configVideo(config);
    rtsp2.begin();

    videoStreamer.registerInput(Camera.getStream(CHANNEL));
    videoStreamer.registerOutput1(rtsp1);
    videoStreamer.registerOutput2(rtsp2);
    if (videoStreamer.begin() != 0) {
        Serial.println("StreamIO link start failed");
    }

    delay(1000);
    printInfo();

    pinMode(MotoA_1A, OUTPUT);
    pinMode(MotoA_1B, OUTPUT);
    pinMode(MotoB_1A, OUTPUT);
    pinMode(MotoB_1B, OUTPUT);

    digitalWrite(MotoA_1A, 0);
    digitalWrite(MotoA_1B, 0);
    digitalWrite(MotoB_1A, 0);
    digitalWrite(MotoB_1B, 0);
    
}

void loop() {
  int i=0;
  //red
  int distance = sensor.readRangeContinuousMillimeters();
    Serial.print(distance/10);
    Serial.println(" cm");
    
    if (sensor.timeoutOccurred()) {
        Serial.println(" TIMEOUT");
    }
    //若距離過近(<30cm)，執行函式backward() //後退
    //直到超過30cm，才會執行後續指令
    if((distance/10)<30){      
    backward();
    delay(50);
    BrakeAll();
    }
    
    //red end
//修改按鈕程式碼，觸發功能從有線改為無線
//偵測value3的值，當其絕對值-1500大於100，在serial monitor輸出'nice'(just for test)
//並將 buttonPressedFor2Seconds 變數設置為 true，也就是執行拍照並進行AI辨識的功能；否則，將 buttonPressedFor2Seconds 設置為 false。
    if ((abs(bleReciveData[value3].ReciveValue) - 1500) > 100) {
        Serial.println("nice");
        buttonPressedFor2Seconds = true;
    }
    else{
              buttonPressedFor2Seconds = false;
    }
    while (Count < MaxNumValue) {
        if (bleReciveData[Count].reciveCMDFlag) {
            bleReciveData[Count].reciveCMDFlag = false;
            if (abs(bleReciveData[value1].ReciveValue - 1500) < 100 && abs(bleReciveData[value2].ReciveValue - 1500) < 100) {
              //value1絕對值-1500及value2絕對值-1500皆小於100 (手沒做任何動作) 執行函式BrakeAll()
                BrakeAll();
            } else if (abs(bleReciveData[value1].ReciveValue - 1500) > abs(bleReciveData[value2].ReciveValue - 1500)) {
                if (bleReciveData[value1].ReciveValue > 1500) {
                  //value1絕對值大於1500 (左搖桿右滑) 執行函式turnRight() //右轉
                    turnRight();
                    Serial.println("R");
                } else {
                  //在value1絕對值小於1500時 (左搖桿左滑) 執行函式turnLeft() //左轉
                    turnLeft();
                    Serial.println("L");
                }
            } else {
                if (bleReciveData[value2].ReciveValue > 1500) {
                  //若未滿足第一點條件且value1小於value2，在value2絕對值於1500時 (左搖桿上滑) 執行函式forward() //前進
                    forward();
                    Serial.println("forward");
                } else {
                  //若未滿足第一點條件且value1小於value2，在value2絕對值小於1500時 (左搖桿下滑) 執行函式backward() //後退
                    backward();
                    Serial.println("backward");
                }
            }
        }
        Count++;
    }
    Count = 0;
    delay(1);

    if (buttonPressedFor2Seconds) {
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("Image Capturing...");
        Camera.getImage(CHANNEL, &img_addr, &img_len);
        encodeImageandsendHttpPostRequest();
        digitalWrite(LED_BUILTIN, LOW);
    }

    while (wifiClient.available()) {
        char c = wifiClient.read();
        Serial.write(c);
    }

    delay(10);
}

void encodeImageandsendHttpPostRequest() {
    encodedImageLen = base64_enc_len(img_len);
    encodedImageData = (char *)malloc(encodedImageLen);
    base64_encode(encodedImageData, (char *)img_addr, img_len);

    Serial.println(prompt);
    DynamicJsonDocument doc(1024);
    doc["text_base64_string"] = prompt;
    doc["image_base64_string"] = encodedImageData;

    String jsonString;
    serializeJson(doc, jsonString);

    if (wifiClient.connect(server, PORT)) {
        wifiClient.println("POST /imgtxt HTTP/1.1");
        wifiClient.println("Host: " + String(server));
        wifiClient.println("Content-Type: application/json");
        wifiClient.println("Content-Length: " + String(jsonString.length()));
        wifiClient.println("Connection: keep-alive");
        wifiClient.println();
        wifiClient.print(jsonString);
        Serial.println("Binary sent");
    }
    free(encodedImageData);
}
