#include <Arduino.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <lvgl.h>
#include "ui/screens.h"
#include "ui/ui.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* marketPair[] = {"BTC/IDR", "ETH/IDR", "SOL/IDR"}; 
unsigned int indexPair = 0;

TaskHandle_t DisplayTaskHandle = NULL;
TaskHandle_t WifiTaskHandle = NULL;
void TaskDisplay(void *pvParameters);
void TaskWifi(void *pvParameters);
SemaphoreHandle_t xGuiSemaphore;

unsigned long previous_price = 0;

#include <string.h>
#include "ui/vars.h"

bool network;
bool get_var_network() {
    return network;
}
void set_var_network(bool value) {
    network = value;
}


String formatprice(String input, String base="") {
  if (base!="") {
    int dotIndex = input.indexOf('.');
    String integerPart = "";
    String decimalPart = "";

    // Split the string if a decimal exists
    if (dotIndex != -1) {
      integerPart = input.substring(0, dotIndex);
      decimalPart = input.substring(dotIndex + 1);
    } else {
      integerPart = input;
      decimalPart = "00";
    }

    // Format the Integer Part
    String formattedInt = "";

    for (int i = 0; i < integerPart.length(); i++) {
      formattedInt += integerPart[i];
      int remainingDigits = integerPart.length() - 1 - i;
      if (remainingDigits % 3 == 0 && remainingDigits != 0) {
        formattedInt += ','; // Standard crypto thousands separator
      }
    }

    // Truncate or pad the Decimal Part to exactly 2 digits
    if (decimalPart.length() > 2) {
      decimalPart = decimalPart.substring(0, 2); 
    } else {
      while (decimalPart.length() < 2) {
        decimalPart += '0'; 
      }
    }
    base.toUpperCase();
    return formattedInt + "." + decimalPart +" " + base;
  }else {
    String result = "";
    
    for (int i = 0; i < input.length(); i++) {
      result += input[i]; 
      
      int remainingDigits = input.length() - 1 - i;
      if (remainingDigits % 3 == 0 && remainingDigits != 0) {
        result += '.'; // Add dot for thousands
      }
    }
    return "Rp " + result;
}
}

void fetch(String market){
    int index = market.indexOf('/');
    String base = market.substring(0, index);
    String quote = market.substring(index + 1);
    base.toLowerCase();
    quote.toLowerCase();
    HTTPClient http;
    http.begin("https://indodax.com/api/ticker/"+base+quote);

    int code = http.GET();

    if (code == HTTP_CODE_OK) {
        String payload = http.getString();

        StaticJsonDocument<512> doc;
        DeserializationError err = deserializeJson(doc, payload);
        Serial.println(payload);

        if (!err) {
            const char *last   = doc["ticker"]["last"];
            const char *buy    = doc["ticker"]["buy"];
            const char *sell   = doc["ticker"]["sell"];
            const char *high   = doc["ticker"]["high"];
            const char *low    = doc["ticker"]["low"];
            const char *vlbase = doc["ticker"]["vol_"+base]; 
            const char *vlquote = doc["ticker"]["vol_"+quote];
            
            String priceIDR = formatprice(last);
            String priceBuy = formatprice(buy);
            String priceSell = formatprice(sell);
            String priceHigh = formatprice(high);
            String priceLow = formatprice(low);
            String vQuote = formatprice(vlquote);
            String vBase = formatprice(vlbase,base);

            if (last)   set_var_price(priceIDR.c_str());
            if (buy)    set_var_buy(priceBuy.c_str());
            if (sell)   set_var_sell(priceSell.c_str());
            if (high)   set_var_high(priceHigh.c_str());
            if (low)    set_var_low(priceLow.c_str());
            if (vlbase) set_var_volbase(vBase.c_str());
            if (vlquote) set_var_volquote(vQuote.c_str());

            unsigned long current_price = atol(last);
            if (xSemaphoreTake(xGuiSemaphore, (TickType_t)100) == pdTRUE) {
                if (previous_price == 0 || current_price == previous_price){
                    lv_obj_set_style_bg_color(objects.obj0, lv_color_hex(0x808080), LV_PART_MAIN);
                    lv_obj_set_style_text_color(objects.obj0, lv_color_hex(0x000000), LV_PART_MAIN);
                }
                else if (current_price > previous_price) {
                    lv_obj_set_style_bg_color(objects.obj0, lv_color_hex(0x00FF58), LV_PART_MAIN);
                    lv_obj_set_style_text_color(objects.obj0, lv_color_hex(0x000000), LV_PART_MAIN);
                } 
                else if (current_price < previous_price) {
                    lv_obj_set_style_bg_color(objects.obj0, lv_color_hex(0xFC0303), LV_PART_MAIN);
                    lv_obj_set_style_text_color(objects.obj0, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
                }
                xSemaphoreGive(xGuiSemaphore);
            }
            previous_price = current_price; 
        } else {
            Serial.printf("[fetch] JSON error: %s\n", err.c_str());
        }
    } else {
        set_var_network(true);
        Serial.printf("[fetch] HTTP error: %d\n", code);
    }
    http.end();
}

char pair[100] = { 0 };
const char *get_var_pair() {
    return pair;
}
void set_var_pair(const char *value) {
    strncpy(pair, value, sizeof(pair) / sizeof(char));
    pair[sizeof(pair) / sizeof(char) - 1] = 0;
}

char price[100] = { 0 };
const char *get_var_price() {
    return price;
}
void set_var_price(const char *value) {
    strncpy(price, value, sizeof(price) / sizeof(char));
    price[sizeof(price) / sizeof(char) - 1] = 0;
}

char buy[100] = { 0 };
const char *get_var_buy() {
    return buy;
}
void set_var_buy(const char *value) {
    strncpy(buy, value, sizeof(buy) / sizeof(char));
    buy[sizeof(buy) / sizeof(char) - 1] = 0;
}

char sell[100] = { 0 };
const char *get_var_sell() {
    return sell;
}
void set_var_sell(const char *value) {
    strncpy(sell, value, sizeof(sell) / sizeof(char));
    sell[sizeof(sell) / sizeof(char) - 1] = 0;
}

char high[100] = { 0 };
const char *get_var_high() {
    return high;
}
void set_var_high(const char *value) {
    strncpy(high, value, sizeof(high) / sizeof(char));
    high[sizeof(high) / sizeof(char) - 1] = 0;
}

char low[100] = { 0 };
const char *get_var_low() {
    return low;
}
void set_var_low(const char *value) {
    strncpy(low, value, sizeof(low) / sizeof(char));
    low[sizeof(low) / sizeof(char) - 1] = 0;
}

char volbase[100] = { 0 };
const char *get_var_volbase() {
    return volbase;
}
void set_var_volbase(const char *value) {
    strncpy(volbase, value, sizeof(volbase) / sizeof(char));
    volbase[sizeof(volbase) / sizeof(char) - 1] = 0;
}

char volquote[100] = { 0 };
const char *get_var_volquote() {
    return volquote;
}
void set_var_volquote(const char *value) {
    strncpy(volquote, value, sizeof(volquote) / sizeof(char));
    volquote[sizeof(volquote) / sizeof(char) - 1] = 0;
}


#include "ui/actions.h"

void action_switch(lv_event_t *e) {
    const unsigned int totalMarket = sizeof(marketPair) / sizeof(marketPair[0]);
    
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_get_act());
    
    if(dir == LV_DIR_LEFT){
        indexPair = (indexPair + totalMarket - 1) % totalMarket;
        set_var_pair(marketPair[indexPair]);
        
    } else if(dir == LV_DIR_RIGHT){
        indexPair = (indexPair + 1) % totalMarket;
        set_var_pair(marketPair[indexPair]);
    }
    previous_price=0;
    set_var_price("");
    set_var_buy("");
    set_var_sell("");
    set_var_high("");
    set_var_low("");
    set_var_volbase("");
    set_var_volquote("");
}



#define XPT2046_CS  21
#define XPT2046_IRQ -1

#define TOUCH_X_MIN  200
#define TOUCH_X_MAX  3800
#define TOUCH_Y_MIN  200
#define TOUCH_Y_MAX  3800


static const uint16_t SCREEN_W = 480;
static const uint16_t SCREEN_H = 320;

static lv_disp_draw_buf_t draw_buf;
static lv_color_t        *buf1 = NULL;
static lv_color_t        *buf2 = NULL;

static TFT_eSPI           tft;
static XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = area->x2 - area->x1 + 1;
    uint32_t h = area->y2 - area->y1 + 1;

    tft.startWrite();
    tft.setAddrWindow(area->x1, area->y1, w, h);
    tft.pushColors((uint16_t *)color_p, w * h, true);
    tft.endWrite();

    lv_disp_flush_ready(drv);
}

static void touchpad_read(lv_indev_drv_t *drv, lv_indev_data_t *data)
{
    if (touch.tirqTouched() && touch.touched()) {
        TS_Point p = touch.getPoint();

        int16_t x = map(p.x, TOUCH_X_MIN, TOUCH_X_MAX, 0, SCREEN_W - 1);
        int16_t y = map(p.y, TOUCH_Y_MIN, TOUCH_Y_MAX, 0, SCREEN_H - 1);

        data->point.x = constrain(x, 0, SCREEN_W - 1);
        data->point.y = constrain(y, 0, SCREEN_H - 1);
        data->state   = LV_INDEV_STATE_PR;
    } else {
        data->state = LV_INDEV_STATE_REL;
    }
}


void setup()
{
    Serial.begin(9600);
    tft.init();
    tft.setRotation(1);
    touch.begin();
    touch.setRotation(3);

    xGuiSemaphore = xSemaphoreCreateMutex();

    lv_init();

    uint32_t buffer_pixels = SCREEN_W * (SCREEN_H / 10);
    buf1 = (lv_color_t *)malloc(buffer_pixels * sizeof(lv_color_t));
    buf2 = (lv_color_t *)malloc(buffer_pixels * sizeof(lv_color_t));

    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, buffer_pixels);

    static lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res  = SCREEN_W;
    disp_drv.ver_res  = SCREEN_H;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);

    static lv_indev_drv_t indev_drv;
    lv_indev_drv_init(&indev_drv);
    indev_drv.type    = LV_INDEV_TYPE_POINTER;
    indev_drv.read_cb = touchpad_read;
    lv_indev_drv_register(&indev_drv);

  xTaskCreate(TaskWifi, "Wifi", 6144, NULL, 2, &WifiTaskHandle);
  xTaskCreate(TaskDisplay, "Display", 3072, NULL, 1, &DisplayTaskHandle);
}

void loop(){
  Serial.println("--- Memory Monitor ---");
  
  UBaseType_t displayMem = uxTaskGetStackHighWaterMark(DisplayTaskHandle);
  Serial.printf("Display Task Free Memory: %d bytes\n", displayMem);

  UBaseType_t wifiMem = uxTaskGetStackHighWaterMark(WifiTaskHandle);
  Serial.printf("WiFi Task Free Memory: %d bytes\n", wifiMem);
  
  Serial.println("------------------------------");
  
  delay(5000);
}

void TaskDisplay(void *pvParameters) {
ui_init();
  for (;;) { 
    static uint32_t last_tick = 0;
    uint32_t current_tick = millis();
    uint32_t delta = current_tick - last_tick;
    last_tick = current_tick;

    lv_tick_inc(delta); 
    
    if (xSemaphoreTake(xGuiSemaphore, (TickType_t)10) == pdTRUE) {
            lv_task_handler();  
            ui_tick();
            xSemaphoreGive(xGuiSemaphore);
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
}

void TaskWifi(void *pvParameters) {
    set_var_pair("BTC/IDR");

    Serial.print("Connecting to Wi-Fi");
    WiFi.begin("ssid", "12345678");
    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        vTaskDelay(100 / portTICK_PERIOD_MS);
        set_var_network(true);
    }
  for (;;) { 
    if (WiFi.status() == WL_CONNECTED) {
        set_var_network(false);
        fetch(get_var_pair());
    } else {
        Serial.println("[fetch] WiFi disconnected — reconnecting...");
        set_var_network(true);
        WiFi.reconnect();
    }
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}