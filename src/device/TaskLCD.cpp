#include "TaskLCD.h"

LiquidCrystal_I2C lcd(33, 16, 2);
int adcPins[] = {1, 2, 3, 4};

void TaskLCD(void *paParameters){

    Sensordata data;

    lcd.init(MY_SDA, MY_SCL);
	lcd.backlight();

    while (true) {
        if (xQueueReceive(lcdQueue, &data, portMAX_DELAY)) {
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print("T:");
        lcd.print(data.temp);
        lcd.print("C H:");
        lcd.print(data.humi);
        lcd.print("%");

        lcd.setCursor(0, 1);
        if (data.temp >= 40 && data.humi >= 80)
            lcd.print("CRITICAL");
        else if (data.temp >= 35 || data.humi >= 70)
            lcd.print("WARNING");
        else
            lcd.print("NORMAL");
        }
    }
}