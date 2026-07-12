# Crypto Price Tracker ESP32+EEZ-Studio

A real-time cryptocurrency ticker built with an ESP32, a TFT touchscreen display, and the LVGL graphics library. This project fetches live market data directly from the [Indodax API](https://indodax.com/) and features a touch-enabled graphical user interface that changes dynamically based on market movements.

## ⚙️ Configuration & Setup

### Wi-Fi Setup
Open the `main.cpp` file and update the network credentials with your Wi-Fi details:
```cpp
const char *networkSSID = "<Your SSID>";
const char *networkPASS = "Your PASS";
```

## Adding Custom Pairs
Simply add the new pair to the `marketPair` array at the top of your main sketch. The code automatically calculates the array size, so no other changes are needed!
```cpp
const char *marketPair[] = {"BTC/IDR", "ETH/IDR", "SOL/IDR", "BNB/IDR", "DOGE/IDR"};
```
