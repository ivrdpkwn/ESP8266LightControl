#define BLINKER_WIFI
#include <Servo.h>
#include <Blinker.h>
#include <EEPROM.h>

/*---------------------- 全局变量区 ------------------*/

// ===== WiFi / Blinker =====
char auth[] = "456***";    // 设备密钥
char ssid[] = "Xia***";    // WIFI名
char pswd[] = "123***";    // WIFI密码

// ===== UI =====
BlinkerButton ButtonOpen("btn_k");
BlinkerButton ButtonClose("btn_g");

BlinkerSlider SliderMax("ran_max1");
BlinkerSlider SliderMin("ran_min1");
BlinkerSlider SliderMid("ran_mid1");

// ===== 硬件 =====
Servo myservo;

// ===== 参数（会保存到Flash）=====
struct ServoConfig {
    int max;
    int min;
    int mid;
};

ServoConfig servo_cfg = {120, 60, 90};

// ===== 状态机 =====
enum ServoState {
    SERVO_IDLE,
    SERVO_OPEN,
    SERVO_CLOSE
};

volatile ServoState servo_state = SERVO_IDLE;


// ===== EEPROM配置 =====
#define EEPROM_SIZE 64
#define EEPROM_ADDR 0


/*---------------------- EEPROM函数区 ------------------*/

// 保存配置到Flash
void save_config()
{
    EEPROM.put(EEPROM_ADDR, servo_cfg);
    EEPROM.commit();
    // BLINKER_LOG("EEPROM saved");
}

// 从Flash读取配置
void load_config()
{
    EEPROM.get(EEPROM_ADDR, servo_cfg);

    // 简单保护（防止第一次烧录是垃圾值）
    if (servo_cfg.max <= 0 || servo_cfg.max > 180)
    {
        servo_cfg = {120, 60, 90};
    }

    // BLINKER_LOG("EEPROM loaded");
}

/*---------------------- 控制函数区 ------------------*/

// 舵机动作
void servo_run_open()
{
    myservo.write(servo_cfg.max);
    Blinker.vibrate();
    Blinker.delay(800);
    myservo.write(servo_cfg.mid);
}

void servo_run_close()
{
    myservo.write(servo_cfg.min);
    Blinker.vibrate();
    Blinker.delay(800);
    myservo.write(servo_cfg.mid);
}

/*---------------------- 状态机执行 ------------------*/

void servo_execute()
{
    switch (servo_state)
    {
        case SERVO_OPEN:
            servo_run_open();
            break;

        case SERVO_CLOSE:
            servo_run_close();
            break;

        default:
            break;
    }

    servo_state = SERVO_IDLE;
}


/*---------------------- 回调函数区 ------------------*/

// 开
void ButtonOpen_callback(const String & state)
{
    // BLINKER_LOG("OPEN:", state);
    servo_state = SERVO_OPEN;
}

// 关
void ButtonClose_callback(const String & state)
{
    // BLINKER_LOG("CLOSE:", state);
    servo_state = SERVO_CLOSE;
}


// ===== 滑块（修改 + 保存）=====
void updateConfig(int &target, int value)
{
    target = value;
    save_config();
    syncUI();
}

void SliderMax_callback(int32_t value)
{
    updateConfig(servo_cfg.max, value);
    myservo.write(value);
}

void SliderMin_callback(int32_t value)
{
    updateConfig(servo_cfg.min, value);
    myservo.write(value);
}

void SliderMid_callback(int32_t value)
{
    updateConfig(servo_cfg.mid, value);
    myservo.write(value);
}

// 数据接收
void dataRead(const String & data)
{
    syncUI();
    // BLINKER_LOG("DATA:", data);
    Blinker.vibrate();

    unsigned long t = millis();
    // Blinker.print("time", t);
}

// 刷新UI
void syncUI()
{
    SliderMax.print(servo_cfg.max);
    SliderMin.print(servo_cfg.min);
    SliderMid.print(servo_cfg.mid);
}

/*---------------------- 初始化 ------------------*/

void setup()
{
    Serial.begin(115200);

    EEPROM.begin(EEPROM_SIZE);   

    load_config();             

    Blinker.begin(auth, ssid, pswd);

    // UI回调绑定

    ButtonOpen.attach(ButtonOpen_callback);
    ButtonClose.attach(ButtonClose_callback);

    SliderMax.attach(SliderMax_callback);
    SliderMin.attach(SliderMin_callback);
    SliderMid.attach(SliderMid_callback);

    Blinker.attachData(dataRead);

    // 舵机
    myservo.attach(0);
    myservo.write(servo_cfg.mid);

    delay(500);
    syncUI();
}

/*---------------------- 主循环 ------------------*/

void loop()
{
    Blinker.run();

    if (servo_state != SERVO_IDLE)
    {
        servo_execute();
    }
}