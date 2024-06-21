# Collision-Avoidance-Autonomous-Tank-with-AI-Recognition <br>
_**Our product, named "AI-Enhanced Collision-Avoidance Remote-Controlled Autonomous Vehicle," integrates advanced AI technology to provide collision avoidance, remote control, and AI image recognition capabilities. It's designed to offer a versatile and intelligent solution for various applications.**_ <br>

## 主要功能
1. 防撞功能：搭載飛時感測器，能夠察覺前方障礙物並自動停止並後退防撞，以確保行駛安全。<br>
2. 遠端遙控：通過手機App，實現對自走車的遠端操控，包括前進、後退、轉彎等功能。<br>
3. 拍照AI辨識：搭載鏡頭和AI辨識系統，能夠拍攝環境圖片並進行物體或場景的辨識甚至延伸，這取決於你對AI下達甚麼指令，也可作為識別交通標誌、行人。<br>
--------------------------------------------------------------
## Development Board Introduciton
### AMB82-MINI overview
<p><img width="50%" height="50%" src="https://www.amebaiot.com/wp-content/uploads/2023/03/amb82_mini.png"></p>

[RTL8735B](https://www.amebaiot.com/en/amebapro2/): 32-bit Arm v8M, up to 500MHz, 768KB ROM, 512KB RAM, 16MB Flash (MCM embedded DDR2/DDR3L up to 128MB) 802.11 a/b/g/n WiFi 2.4GHz/5GHz, BLE 5.1, NN Engine 0.4 TOPS, Crypto Engine, Audo Codec, …

* [Ameba Arduino](https://www.amebaiot.com/en/ameba-arduino-summary/)

* [Amebapro2 AMB82-mini Arduino Example Guides](https://www.amebaiot.com/en/amebapro2-amb82-mini-arduino-peripherals-examples)

* [Amebapro2 AMB82-mini Arduino getting started](https://www.amebaiot.com/en/amebapro2-amb82-mini-arduino-getting-started/) <br>
## System Diagram
<p><img width="50%" height="50%" src="https://github.com/wang930510/Collision-Avoidance-Autonomous-Tank-with-AI-Recognition/assets/136217880/d0a45b2b-73d9-43c2-9a99-4ab14358ad4e"></p>

<p><img width="50%" height="50%" src="https://github.com/wang930510/Collision-Avoidance-Autonomous-Tank-with-AI-Recognition/assets/136217880/c926af5f-4200-4ffe-888b-fdd930b07bfd"></p>

## Product Features
### 馬達藍牙遙控
> #### **BLE-V7RC APP**
APP 提供了兩個 Channel 與四個 Channel 控制的版本，分別可以控制兩個伺服馬達或是四個伺服馬達。
可相容於大部分的遙控車。您可以透過設定校正油門與方向的伺服馬達設定
<p><img width="50%" height="50%" src="https://github.com/wang930510/portable-ChatGPT/assets/136217880/4232c339-5cee-47f0-96c5-441aa4103621"></p>

**我們使用的模型是坦克:** <br>
> 範例字串：SRT1000200015001500# 起始字串：SRT 上限：2000 (10進位) 中央：1500 (10進位) 下限：1000 (10進位)<br>
> 圖形排列： 第一組4位數為左側水平控制，第二組4位數為左側垂直控制 第三組4位數為右側垂直控制，第四組4位數為右側水平控制
> 
------------------------------------------
#### **使用example:**

#### [BLE - V7RC Car With Video Streaming](https://www.amebaiot.com/en/amebapro2-arduino-ble-v7rc/)
-----------------------------------------------------------------------

##### **程式中透過手機搖桿控制數值，利用bleReciveData回傳各數值(value1,value2)去做邏輯判斷:**
```
value1絕對值-1500及value2絕對值-1500皆小於100 (手沒做任何動作) 執行函式BrakeAll() //煞車 <br>
若未滿足第一點條件且value1大於value2，在value1絕對值大於1500時 (左搖桿右滑) 執行函式turnRight() //右轉 <br>
若未滿足第一點條件且value1大於value2，在value1絕對值小於1500時 (左搖桿左滑) 執行函式turnLeft() //左轉 <br>
若未滿足第一點條件且value1小於value2，在value2絕對值於1500時 (左搖桿上滑) 執行函式forward() //前進 <br>
若未滿足第一點條件且value1小於value2，在value2絕對值小於1500時 (左搖桿下滑) 執行函式backward() //後退 <br>
```

```
 //BrakeAll()函式     
 digitalWrite(MotoA_1A, 0);
digitalWrite(MotoB_1A, 0);

 //turnRight()函式 
 digitalWrite(MotoA_1A, 0);
analogWrite(MotoA_1B, 250);

 //turnLeft()函式 
 digitalWrite(MotoB_1A, 0);
analogWrite(MotoB_1B, 250);

 //forward()函式 
 digitalWrite(MotoA_1A, 0);
analogWrite(MotoA_1B, 250);
digitalWrite(MotoB_1A, 0);
analogWrite(MotoB_1B, 250);

 //backward()函式 <
 digitalWrite(MotoA_1A, 1);
analogWrite(MotoA_1B, 5);
digitalWrite(MotoB_1A, 1);
analogWrite(MotoB_1B, 5);
```
--------------------------------------------------------------------------------------------------------
### 飛時測距防撞
>在本自走車專案中，我們在車體前方安裝了一個VL53L0X紅外線測距器，以實現防撞功能。該元件能夠持續偵測前方與障礙物的距離。如果VL53L0X測距器偵測到物體距離小於30公分，車輛將自動執行後退（backward）程式，直至與障礙物的距離超過30厘米。此後，車輛停止並等待人類下達其他指令。
#### **使用example:**
[IR_VL53L0X](https://github.com/rkuo2000/Arduino/tree/master/examples/AMB82-MINI/IR_VL53L0X) <br>
>在程式的 loop 函數中，使用 int distance = sensor.readRangeContinuousMillimeters(); 這行程式碼來讀取 VL53L0X 的測距值，該值以毫米為單位。<br>
然後，通過 Serial.print(distance / 10); 將距離轉換為公分並打印出來，最後使用 Serial.println(" cm"); 輸出結果並附加單位 " cm"。這段程式碼實現了即時距離測量並提供了易於理解的輸出結果，對於機器人避障、自動導航等應用非常實用。之後可以針對(distance/10)變數進行進一步的應用，例如根據測得的距離來決定機器人的行動路徑，如果距離過近則觸發避障機制，或者根據距離來調整自走車的速度和方向，從而實現更加智能化的控制。<br>
```
 int distance = sensor.readRangeContinuousMillimeters();
 Serial.print(distance/10);
 Serial.println(" cm");
if((distance/10)<30){ 
    backward();
    delay(50);
    BrakeAll();
    }
```
-------------------------------------------------------
### 遙控觸發AI鏡頭辨識

>_**我們使用的VLM的AI模型是AmebaPro2_Whisper_LlavaNext_server。**_<br>
>在本專題中，我們實現了遠端控制與AI辨識功能。該系統能夠在程式碼中預先設定要詢問的問題。例如，在本次專題中，我們選擇執行「what do you see,give answer in a few words.」的指令。後端伺服器接收到鏡頭的資訊後，由AI進行分析並給出回答。
>原本的範例程式碼需要按住按鈕兩秒鐘來觸發AI辨識功能，且連結方式是有線的。我們對此進行了改進，實現了無線啟動功能。<br>
>具體來說，我們將按鈕啟動改為通過V7RC應用程式上的Value3來啟動，從而達到遠端控制的效果。這樣的改進不僅提升了操作的便利性，也使得系統更加靈活和高效。

#### **使用example:**
#### [HTTP_Post_TextImage](https://github.com/rkuo2000/Arduino/blob/master/examples/AMB82-MINI/HTTP_Post_TextImage/HTTP_Post_TextImage.ino)
----------------------------------------------------------------------
```
if ((abs(bleReciveData[value3].ReciveValue) - 1500) > 100) {
Serial.println("nice");
buttonPressedFor2Seconds = true};
else{
buttonPressedFor2Seconds = false};
```

* 這段程式碼原本是用來檢測按鈕是否被按下長達兩秒，並根據這個檢測結果執行不同的操作。然而，經過修改後，現在改為根據 value3 對應的值來決定是否進行特定操作。<br>
* 具體來說，這段程式碼會讀取 bleReciveData 陣列中索引為 value3 的 ReciveValue 值，並計算該值的絕對值減去 1500。如果結果大於 100，也就是手機遙控的右邊搖桿有做變化，則程式會輸出 "nice"，這個nice只是方便我們確認是否有執行近來這段，並將 buttonPressedFor2Seconds 變數設置為 true，也就是執行拍照並進行AI辨識的功能；否則，將 buttonPressedFor2Seconds 設置為 false。<br>
* **這種改變使得程式的條件判斷不再依賴按鈕的實際按下時間，而是可以遠端無線的方式改變 value3 的值來進行判斷**，從而實現了無線啟動AI辨識的功能，更靈活的觸發條件設定。<br>

## Implementation
### Server: 
code:<br>
[AmebaPro2_Whisper_LlavaNext_server.py](https://github.com/rkuo2000/Arduino/blob/7b27c19c8d924dda0ef92c7febe82ba65bbf08fa/examples/AMB82-MINI/src/AmebaPro2_Whisper_LlavaNext_server.py#L4)
``` @app.post("/asr")
async def post_asr(data: Base64audio):
    try:
        # Save the decoded audio data to a MP4 file
        decoded_data = base64.b64decode(data.base64_string)
        with open("speech.mp4", "wb") as f:
            f.write(decoded_data)

        # Whisper transcribe
        result = ASR.transcribe("speech.mp4",fp16=False)
        header1 = "ASR:"
        print(header1+result["text"])
        return Response(header1+result["text"])
    except Exception as e:
        raise HTTPException(status_code=500, detail=str(e))
```

## Demo Video
### [藍芽遙控+AI辨識](https://www.youtube.com/watch?app=desktop&si=WUV3qTxEbcg9R8pC&v=CAePZU45b6w&feature=youtu.be) <br>
<p><img width="50%" height="50%" src="https://github.com/wang930510/Collision-Avoidance-Autonomous-Tank-with-AI-Recognition/assets/136217880/0beb0b52-e42e-4d7f-88df-8242f30d5b5e"></p>

### [飛時感測防撞](https://www.youtube.com/watch?v=PPgx732bsN8)<br>
<p><img width="50%" height="50%" src="https://github.com/wang930510/Collision-Avoidance-Autonomous-Tank-with-AI-Recognition/assets/136217880/582ab8c6-3c15-49b3-a4f1-5403a086a6c5"></p>

<br>
