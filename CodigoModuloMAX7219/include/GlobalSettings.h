struct AlarmConfig {
    uint8_t hour;
    uint8_t minute;
    bool enabled;
    uint8_t tone;
};

class GlobalSettings {
public:
    static AlarmConfig alarms[3];
    static uint8_t alarmVolume;
    static bool wifiEnabled;
    static BrightnessMode brightnessMode;
    static uint8_t matrixBrightness;
};
