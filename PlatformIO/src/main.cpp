#include <Arduino.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "lovense.h"
#include "LS.h"
#include "nvs_flash.h"

void setup() {
    Serial.begin(115200);

    // Preserve existing NVS data (including BLE bonds) on normal boots.
    esp_err_t nvsStatus = nvs_flash_init();

    // Erase only when NVS cannot be used because it is full or its
    // stored format is incompatible with the current firmware.
    if (nvsStatus == ESP_ERR_NVS_NO_FREE_PAGES ||
        nvsStatus == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        Serial.println("NVS needs recovery; erasing NVS partition.");
        esp_err_t eraseStatus = nvs_flash_erase();
        if (eraseStatus != ESP_OK) {
            Serial.printf("NVS erase failed: %s\\n", esp_err_to_name(eraseStatus));
            while (true) delay(1000);
        }

        nvsStatus = nvs_flash_init();
    }

    if (nvsStatus != ESP_OK) {
        Serial.printf("NVS initialization failed: %s\\n", esp_err_to_name(nvsStatus));
        while (true) delay(1000);
    }

    delay(100);

    NimBLEDevice::init(DEVICE_NAME);
    lovense_init();

    muse_init();
    muse_start();

    bluetooth_service_start();
}

void loop() {
    if (Serial.available()) {
        String line = Serial.readStringUntil('\\n');
        line.trim();

        if (line.length() > 0) {
            if (line.startsWith("Vibrate:") && line.endsWith(";")) {
                int colonPos = line.indexOf(':');
                int semicolonPos = line.indexOf(';');
                String valueStr = line.substring(colonPos + 1, semicolonPos);
                int level = valueStr.toInt();

                if (level < 0) level = 0;
                if (level > 20) level = 20;

                float intensity = level / 20.0f;
                muse_set_intensity(intensity);
            }
        }
    }

    delay(10);
}
