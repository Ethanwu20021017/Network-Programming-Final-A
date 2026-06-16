# Final A - Arduino WiFi Weather Display

## 一、作品簡介

本專案是 Final 作業第 A 題的 Arduino 程式，主要目標是讓 Arduino 透過 WiFi 連線到指定的無線網路，並使用 HTTP POST 向老師提供的天氣伺服器送出查詢請求。伺服器回傳資料後，Arduino 會解析目前的時間、溫度與濕度，最後將結果顯示在 Arduino LED Matrix 上。

這份程式的核心意義在於把「網路通訊」和「嵌入式顯示」結合起來。Arduino 不只是單純顯示固定文字，而是會主動連上網路、向遠端伺服器要求即時資料，再把取得的內容轉成適合 LED Matrix 顯示的格式。整體流程包含 WiFi 連線、HTTP POST 傳送、HTTP 回應解析、字串處理、LED Matrix 捲動顯示，以及依照溫度變化顯示不同圖示。

除了完成老師指定的功能之外，我也額外加入了幾個 Bonus 設計，例如 WiFi 掃描、錯誤警告圖示、天氣狀態圖示，以及自製 LED Matrix 字型與捲動文字效果，讓整體呈現不只是能運作，也更接近一個完整的小型天氣顯示系統。

---

## 二、使用的硬體與環境

本專案使用 Arduino 內建的 WiFi 與 LED Matrix 功能，因此適合用在支援 `WiFiS3.h` 與 `Arduino_LED_Matrix.h` 的開發板上，例如 Arduino UNO R4 WiFi。

### 使用硬體

* Arduino UNO R4 WiFi
* Arduino 內建 LED Matrix
* 可連線的 WiFi AP

### 使用函式庫

```cpp
#include <WiFiS3.h>
#include "Arduino_LED_Matrix.h"
#include "arduino_secrets.h"
```

其中：

* `WiFiS3.h`：負責 WiFi 連線與 TCP Client 通訊。
* `Arduino_LED_Matrix.h`：負責控制 Arduino 內建的 8x12 LED Matrix。
* `arduino_secrets.h`：存放 WiFi SSID 與密碼，避免直接把密碼寫死在主程式中。

---

## 三、檔案結構

本專案主要包含兩個檔案：

```text
Final_A/
├── Final_A.ino
└── arduino_secrets.h
```

### `Final_A.ino`

這是主要程式，負責：

1. 初始化 Serial Monitor 與 LED Matrix。
2. 掃描附近 WiFi。
3. 連線到指定 WiFi AP。
4. 向天氣伺服器送出 HTTP POST。
5. 接收並解析伺服器回傳的天氣資料。
6. 將時間、溫度與濕度顯示在 LED Matrix。
7. 根據溫度顯示太陽或雪花圖示。
8. 發生錯誤時顯示警告圖示。

### `arduino_secrets.h`

這個檔案用來存放 WiFi 名稱與密碼：

```cpp
#define SECRET_SSID "MSI 8955"
#define SECRET_PASS "你的 WiFi 密碼"
```

這樣做的好處是可以把 WiFi 資訊和主程式分開管理。如果未來要換 WiFi，只需要修改 `arduino_secrets.h`，不用動到主要程式邏輯。

---

## 四、程式主要流程

整體程式的執行流程如下：

```text
程式啟動
   ↓
初始化 Serial Monitor 與 LED Matrix
   ↓
掃描附近 WiFi AP，並印出 SSID
   ↓
顯示警告圖示，表示系統正在準備連線
   ↓
連線到 WiFi
   ↓
進入 loop()
   ↓
送出 HTTP POST 到天氣伺服器
   ↓
接收伺服器回傳資料
   ↓
解析 time、temperature、humidity
   ↓
將資料組合成顯示文字
   ↓
在 LED Matrix 上捲動顯示
   ↓
依照溫度顯示太陽或雪花圖示
   ↓
等待 10 秒後再次更新
```

---

## 五、指定功能完成情況

以下是本作業的自我檢核表，整理本程式有完成哪些老師指定功能，以及我額外加入的 Bonus 功能。

| 檢核項目                                                                        | 是否完成 | 說明                                                                                            |
| --------------------------------------------------------------------------- | ---: | --------------------------------------------------------------------------------------------- |
| Your Arduino can connect to WiFi AP.                                        | ✅ 完成 | 程式會使用 `WiFi.begin(ssid, pass)` 連線到 `arduino_secrets.h` 中設定的 WiFi AP，並在 Serial Monitor 顯示連線狀態。 |
| Your Arduino can send out HTTP POST to the server.                          | ✅ 完成 | 程式會使用 `WiFiClient` 連線到 `nycu.waynewolf.tw`，並對 `/weather/weather.do` 送出 HTTP POST。             |
| You can download the returned data, and displayed it on Arduino LED Matrix. | ✅ 完成 | 程式會讀取伺服器回傳的資料，解析時間、溫度與濕度，並透過 LED Matrix 捲動顯示。                                                 |
| Bonus: Your own creativity.                                                 | ✅ 完成 | 額外加入 WiFi 掃描、錯誤警告圖示、太陽圖示、雪花圖示、自製字型與捲動文字顯示。                                                    |

---

## 六、WiFi 連線功能

程式一開始會先進行 WiFi 掃描：

```cpp
int n = WiFi.scanNetworks();
Serial.print("Found ");
Serial.print(n);
Serial.println(" networks");

for (int i = 0; i < n; i++) {
  Serial.println(WiFi.SSID(i));
}
```

這段程式會把附近掃描到的 WiFi SSID 印在 Serial Monitor 上。這個功能雖然不是老師指定的基本需求，但在實際測試時很有幫助，因為可以確認 Arduino 是否真的有偵測到目標 WiFi。

接著程式會透過以下程式碼連線到 WiFi：

```cpp
WiFi.begin(ssid, pass);

while (WiFi.status() != WL_CONNECTED) {
  Serial.print("WiFi Status = ");
  Serial.println(WiFi.status());
  delay(1000);
}
```

只要還沒有連上 WiFi，Arduino 就會每秒印出目前的 WiFi 狀態。連線成功後，Serial Monitor 會顯示：

```text
WiFi connected
```

這代表 Arduino 已成功連上指定的 WiFi AP，可以開始和伺服器進行通訊。

---

## 七、HTTP POST 功能

本作業要求 Arduino 要能送出 HTTP POST，本程式是在 `getWeatherData()` 函式中完成這個功能。

伺服器資訊設定如下：

```cpp
const char HOST[] = "nycu.waynewolf.tw";
const char PATH[] = "/weather/weather.do";
const int PORT = 80;
const char LOCATION[] = "taipei";
```

POST 的 body 內容為：

```cpp
time=now&location=taipei
```

完整的 HTTP POST request 會由以下程式送出：

```cpp
client.print("POST ");
client.print(PATH);
client.println(" HTTP/1.1");
client.print("Host: ");
client.println(HOST);
client.println("Content-Type: application/x-www-form-urlencoded");
client.print("Content-Length: ");
client.println(strlen(body));
client.println("Connection: close");
client.println();
client.print(body);
```

這段程式手動組出 HTTP request，包含：

* Request method：`POST`
* Request path：`/weather/weather.do`
* Host：`nycu.waynewolf.tw`
* Content-Type：`application/x-www-form-urlencoded`
* Content-Length：根據 body 長度自動計算
* Connection：`close`

這樣 Arduino 就可以向老師指定的天氣伺服器送出查詢要求。

---

## 八、伺服器資料接收與解析

伺服器回傳的資料會被存入 `weatherData` 陣列中：

```cpp
char weatherData[80];
```

程式會先等待伺服器有資料可以讀取，如果超過 5 秒沒有收到資料，就會視為失敗：

```cpp
unsigned long t0 = millis();

while (client.connected() && !client.available()) {
  if (millis() - t0 > 5000) {
    client.stop();
    return 0;
  }
}
```

收到 HTTP 回應後，程式會先略過 HTTP Header，只保留真正的 body。判斷 Header 結束的方式是偵測：

```text
\r\n\r\n
```

也就是 HTTP Header 和 Body 中間的空行。

```cpp
if (last4[0] == '\r' && last4[1] == '\n' && last4[2] == '\r' && last4[3] == '\n') {
  bodyStarted = 1;
}
```

當 `bodyStarted` 變成 1 之後，後續讀到的字元就會被存入 `out`：

```cpp
if (index < outSize - 1) {
  out[index++] = c;
}
```

最後再使用 `trimText(out)` 去除資料前後多餘的空白、換行或 `\r`。

---

## 九、天氣資料格式處理

本程式預期伺服器回傳的資料格式為：

```text
time,temperature,humidity
```

例如可能會像這樣：

```text
14:30,27.50,76
```

程式會使用逗號 `,` 來切割資料：

```cpp
char *p1 = strchr(weatherData, ',');
char *p2 = NULL;
if (p1 != NULL) p2 = strchr(p1 + 1, ',');
```

如果資料中沒有兩個逗號，代表格式不符合預期，程式會顯示錯誤訊息並在 LED Matrix 上顯示警告圖示：

```cpp
if (p1 == NULL || p2 == NULL) {
  Serial.println("Data format error.");
  showIcon(warnIcon);
  delay(3000);
  return;
}
```

資料格式正確時，程式會把字串切成三個部分：

```cpp
*p1 = '\0';
*p2 = '\0';

char *timeText = weatherData;
float temp = atof(p1 + 1);
float humid = atof(p2 + 1);
```

其中：

* `timeText`：時間
* `temp`：溫度
* `humid`：濕度

接著再組成要顯示在 LED Matrix 上的文字：

```cpp
snprintf(showMsg, sizeof(showMsg), "Time:%s Temp. %.2f C Humid. %.0f%%", timeText, temp, humid);
```

顯示格式大致如下：

```text
Time:14:30 Temp. 27.50 C Humid. 76%
```

---

## 十、LED Matrix 文字顯示

由於 Arduino LED Matrix 的寬度有限，無法一次顯示完整的時間、溫度與濕度，因此本程式使用捲動文字的方式呈現。

主要顯示函式為：

```cpp
scrollText(showMsg);
```

`scrollText()` 會根據文字長度計算需要捲動的寬度：

```cpp
int textLen = strlen(text);
int width = textLen * 6;
```

每個字元使用 5 欄像素表示，並保留 1 欄作為字元間距，因此一個字元約佔 6 欄。

接著透過 `shift` 控制畫面從右到左移動：

```cpp
for (int shift = -12; shift < width; shift++) {
  clearFrame(frame);

  for (int y = 0; y < 7; y++) {
    for (int x = 0; x < 12; x++) {
      int realX = x + shift;
      if (getTextPixel(text, realX, y)) frame[y][x] = 1;
    }
  }

  matrix.renderBitmap(frame, 8, 12);
  delay(70);
}
```

這段程式會不斷更新 LED Matrix 的畫面，讓文字產生水平捲動效果。每次畫面更新間隔為 70 毫秒，因此捲動速度不會太快，也比較容易閱讀。

---

## 十一、自製 LED Matrix 字型

為了讓 LED Matrix 能顯示英文字母、數字與符號，我在程式中自己建立了簡單的 5x7 字型資料。

主要函式是：

```cpp
byte getCharColumn(char c, int col)
```

這個函式會根據目前要顯示的字元，回傳該字元在指定欄位的像素資料。例如數字 `0` 的字型如下：

```cpp
case '0': {
  byte f[5] = {62, 81, 73, 69, 62};
  memcpy(font, f, 5);
  break;
}
```

每個字元使用 5 個 byte 表示 5 欄，每個 byte 中的 bit 代表垂直方向的像素是否亮起。這樣就可以把普通文字轉成 LED Matrix 可以顯示的像素圖案。

本程式目前支援顯示所需的數字、英文字母與符號，例如：

* 數字：`0` 到 `9`
* 英文字母：`T`, `C`, `H`, `W`, `a`, `e`, `i`, `m`, `p`, `t`, `u`, `d` 等
* 符號：`:`, `.`, `%`, `-`, 空白

這些字元已足夠顯示：

```text
Time
Temp
Humid
C
%
```

---

## 十二、天氣圖示 Bonus 功能

除了文字顯示之外，我也另外設計了三種 LED Matrix 圖示：

1. 太陽圖示 `sunIcon`
2. 雪花圖示 `snowIcon`
3. 警告圖示 `warnIcon`

### 太陽圖示

當溫度大於或等於 25°C 時，LED Matrix 會顯示太陽圖示：

```cpp
if (temp >= 25.0) {
  showIcon(sunIcon);
  delay(2000);
}
```

這代表目前天氣偏熱或溫暖。

### 雪花圖示

當溫度小於或等於 24°C 時，LED Matrix 會顯示雪花圖示：

```cpp
else if (temp <= 24.0) {
  showIcon(snowIcon);
  delay(2000);
}
```

這代表目前天氣相對較涼。

### 警告圖示

當 WiFi 或天氣伺服器發生問題時，LED Matrix 會顯示警告圖示：

```cpp
showIcon(warnIcon);
```

例如以下情況會出現警告圖示：

* 無法成功取得天氣資料
* 伺服器回傳資料格式錯誤
* HTTP 回應為空
* 連線等待超時

這樣即使不看 Serial Monitor，也可以從 LED Matrix 上知道目前系統可能發生錯誤。

---

## 十三、錯誤處理設計

本程式有加入基本的錯誤處理，避免 Arduino 在資料異常時直接卡住。

### 1. 天氣伺服器連線失敗

如果無法連上伺服器：

```cpp
if (!client.connect(HOST, PORT)) return 0;
```

主程式會印出：

```text
Weather service failed.
```

並顯示警告圖示。

### 2. 伺服器回應逾時

如果超過 5 秒仍然沒有資料可以讀取：

```cpp
if (millis() - t0 > 5000) {
  client.stop();
  return 0;
}
```

程式會停止 client，避免一直等待而卡住。

### 3. 回傳資料為空

如果最後取得的 body 長度為 0：

```cpp
if (strlen(out) == 0) return 0;
```

也會視為取得資料失敗。

### 4. 資料格式錯誤

如果回傳資料中沒有正確的逗號分隔格式：

```cpp
if (p1 == NULL || p2 == NULL) {
  Serial.println("Data format error.");
  showIcon(warnIcon);
  delay(3000);
  return;
}
```

程式不會繼續解析錯誤資料，而是直接提示錯誤。

---

## 十四、Serial Monitor 輔助觀察

為了方便測試與除錯，程式在 Serial Monitor 中會印出許多重要資訊。

例如一開始會顯示掃描到的 WiFi：

```text
Scan start
Found 5 networks
MSI 8955
...
```

連線過程中會顯示 WiFi 狀態：

```text
WiFi Status = ...
WiFi connected
```

成功取得資料後，會印出原始資料：

```text
Raw weather data: 14:30,27.50,76
```

也會印出實際要顯示在 LED Matrix 上的文字：

```text
Display: Time:14:30 Temp. 27.50 C Humid. 76%
```

這些輸出可以幫助我確認 Arduino 是否有完成每一個步驟，也方便在展示時向老師說明程式的執行狀況。

---

## 十五、更新頻率

每次成功取得並顯示天氣資料後，程式會等待 10 秒再重新向伺服器要求資料：

```cpp
delay(10000);
```

也就是說，Arduino 會持續週期性更新天氣資料，而不是只執行一次。這讓作品比較接近實際的即時天氣看板。

需要注意的是，這個 10 秒是指每一輪顯示流程結束後，再額外等待 10 秒。因為前面還包含 HTTP 連線、資料解析、文字捲動與圖示顯示，所以實際兩次 POST 之間的總時間會比 10 秒再多一些。

---

## 十六、Bonus 功能整理

除了老師指定功能之外，本專案額外完成以下 Bonus 設計：

| Bonus 功能            | 說明                                                |
| ------------------- | ------------------------------------------------- |
| WiFi 掃描功能           | 程式啟動時會掃描附近 WiFi，並印出 SSID，方便確認 Arduino 是否偵測到目標 AP。 |
| Serial Monitor 除錯資訊 | 會印出 WiFi 狀態、原始天氣資料與顯示文字，方便觀察程式流程。                 |
| HTTP Header 解析      | 程式會判斷 `\r\n\r\n`，自動略過 HTTP Header，只保留 body 資料。    |
| 資料格式檢查              | 若伺服器回傳格式錯誤，程式會顯示錯誤並避免繼續解析。                        |
| 連線逾時處理              | 若伺服器超過 5 秒沒有回應，程式會停止等待，避免卡住。                      |
| LED Matrix 捲動文字     | 將時間、溫度與濕度組合成文字，在 8x12 LED Matrix 上水平捲動顯示。         |
| 自製 5x7 字型           | 自行定義數字、英文字母與符號的像素資料，讓 LED Matrix 可以顯示完整訊息。        |
| 太陽圖示                | 溫度大於或等於 25°C 時顯示太陽，讓使用者能快速感受天氣狀態。                 |
| 雪花圖示                | 溫度小於或等於 24°C 時顯示雪花，表示天氣相對較涼。                      |
| 警告圖示                | 發生連線或資料錯誤時顯示警告圖示，讓使用者不用看電腦也能知道異常。                 |

---

## 十七、如何執行

### Step 1：建立 `arduino_secrets.h`

在 `Final_A.ino` 同一個資料夾中建立 `arduino_secrets.h`，內容如下：

```cpp
#define SECRET_SSID "你的 WiFi 名稱"
#define SECRET_PASS "你的 WiFi 密碼"
```

例如：

```cpp
#define SECRET_SSID "MSI 8955"
#define SECRET_PASS "xxxxxxxx"
```

### Step 2：開啟 Arduino IDE

使用 Arduino IDE 開啟：

```text
Final_A.ino
```

### Step 3：選擇開發板與連接埠

在 Arduino IDE 中選擇對應的開發板，例如：

```text
Arduino UNO R4 WiFi
```

並選擇正確的 Serial Port。

### Step 4：上傳程式

按下 Upload，將程式燒錄到 Arduino。

### Step 5：開啟 Serial Monitor

將 Serial Monitor 設定為：

```text
9600 baud
```

即可看到 WiFi 掃描、WiFi 連線狀態、伺服器回傳資料，以及 LED Matrix 顯示內容。

---

## 十八、展示時可以觀察的結果

程式成功執行時，可以觀察到以下現象：

1. Serial Monitor 會先印出附近 WiFi 清單。
2. Arduino 會連線到指定 WiFi AP。
3. 連線成功後，Arduino 會送出 HTTP POST 到天氣伺服器。
4. Serial Monitor 會顯示伺服器回傳的 raw data。
5. LED Matrix 會捲動顯示目前時間、溫度與濕度。
6. 若溫度大於或等於 25°C，會顯示太陽圖示。
7. 若溫度小於或等於 24°C，會顯示雪花圖示。
8. 若資料取得失敗或格式錯誤，會顯示警告圖示。
