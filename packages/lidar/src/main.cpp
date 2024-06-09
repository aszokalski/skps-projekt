#include <fcntl.h>
#include <mqueue.h>
#include <unistd.h>

#include <fstream>
#include <iostream>
#include <string>
#include <stdexcept>
#include <sstream>

#include "vl53l0x_api.h"
#include "vl53l0x_platform.h"

#define MAX_DEGREE 180
#define MIN_DEGREE 0
#define STEP 5

#define MIN_DUTY_CYCLE 1'000'000
#define MAX_DUTY_CYCLE 2'000'000

#define PWM_CHIP "/sys/class/pwm/pwmchip0"
#define PWM_CHANNEL "/sys/class/pwm/pwmchip0/pwm0"

// Sensor Time Of Flight
// https://github.com/cassou/VL53L0X_rasp/blob/master/examples/vl53l0x_SingleRanging_Long_Range_Example.c

class ToF {
public:
    ToF(VL53L0X_Dev_t &sensor): sensor(sensor) {
        sensor.I2cDevAddr = 0x29;

        // choose between i2c-0 and i2c-1
        char i2c[] = "/dev/i2c-1";
        std::cout << "Sensor: init"  << std::endl;
        sensor.fd = VL53L0X_i2c_init(i2c, sensor.I2cDevAddr);
        if (sensor.fd < 0) {
            error_status = VL53L0X_ERROR_CONTROL_INTERFACE;
            std::cerr << "Sensor: failed to initialize" << std::endl;
            check_sensor_error_status();
        }

        std::cout << "Sensor: VL53L0X_DataInit" << std::endl;
        error_status = VL53L0X_DataInit(&sensor);
        check_sensor_error_status();

        std::cout << "Sensor: VL53L0X_StaticInit" << std::endl;
        error_status = VL53L0X_StaticInit(&sensor);
        check_sensor_error_status();

        std::cout << "Sensor: VL53L0X_PerformRefCalibration" << std::endl;
        error_status = VL53L0X_PerformRefCalibration(&sensor, &vhv_settings, &phase_cal);
        check_sensor_error_status();


        std::cout << "Sensor: VL53L0X_PerformRefSpadManagement" << std::endl;
        error_status = VL53L0X_PerformRefSpadManagement(&sensor, &ref_spad_count, &is_aperture_spads);
        check_sensor_error_status();
        std::cout << "Sensor: ref_spad_count = " << ref_spad_count
                  << ", is_aperture_spads = " << is_aperture_spads << std::endl;


        // Setup in single ranging mode
        std::cout << "Sensor: VL53L0X_SetDeviceMode" << std::endl;
        error_status = VL53L0X_SetDeviceMode(&sensor, VL53L0X_DEVICEMODE_SINGLE_RANGING);
        check_sensor_error_status();


        std::cout << "Sensor: setting limit checks" << std::endl;
        error_status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE, 1);
        check_sensor_error_status();

        error_status = VL53L0X_SetLimitCheckEnable(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE, 1);
        check_sensor_error_status();

        error_status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGNAL_RATE_FINAL_RANGE,
                                            (FixPoint1616_t)(0.1 * 65536));
        check_sensor_error_status();

        error_status = VL53L0X_SetLimitCheckValue(&sensor, VL53L0X_CHECKENABLE_SIGMA_FINAL_RANGE,
                                            (FixPoint1616_t)(60 * 65536));
        check_sensor_error_status();


        std::cout << "Setting timing budget" << std::endl;
        error_status = VL53L0X_SetMeasurementTimingBudgetMicroSeconds(&sensor, 33000);
        check_sensor_error_status();


        std::cout << "Setting pulse periods" << std::endl;

        error_status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_PRE_RANGE, 18);
        check_sensor_error_status();

        error_status = VL53L0X_SetVcselPulsePeriod(&sensor, VL53L0X_VCSEL_PERIOD_FINAL_RANGE, 14);
        check_sensor_error_status();
    };

    ~ToF() {
        std::cout << "Sensor: closing comms" << std::endl;
        VL53L0X_i2c_close();
    }

    uint16_t measure_distance() {
        error_status = VL53L0X_PerformSingleRangingMeasurement(
                &sensor, &ranging_measurement_data);

        check_sensor_error_status();
        check_measurement_status();

        return ranging_measurement_data.RangeMilliMeter;
    }
private:
    VL53L0X_Dev_t &sensor;
    VL53L0X_Error error_status;
    uint32_t ref_spad_count;
    uint8_t is_aperture_spads;
    uint8_t vhv_settings;
    uint8_t phase_cal;
    VL53L0X_RangingMeasurementData_t ranging_measurement_data;

    void check_sensor_error_status() const {
        if (error_status != VL53L0X_ERROR_NONE) {
            char buffer[VL53L0X_MAX_STRING_LENGTH];
            VL53L0X_GetPalErrorString(error_status, buffer);
            throw std::runtime_error(
                    (std::ostringstream()
                        << "Sensor failed: "
                        << buffer
                        << "\n"
                    ).str()
                );
        }
    }

    void check_measurement_status() const {
        /*
         * New Range Status: data is valid when ranging_measurement_data.RangeStatus = 0
         */

        uint8_t range_status = ranging_measurement_data.RangeStatus;

        if (range_status != 0) {
            std::cerr << "Sensor: measurement failed" << std::endl;

            char buffer[VL53L0X_MAX_STRING_LENGTH];
            VL53L0X_GetRangeStatusString(range_status, buffer);
            std::cerr << "Sensor: range_status - " << range_status << " : " << buffer << std::endl;
        }
    }
};

// Servo

// https://jumpnowtek.com/rpi/Using-the-Raspberry-Pi-Hardware-PWM-timers.html
// https://www.cplusplus.com/doc/tutorial/files/

class Servo{
public:
    Servo(){
        std::cout<< "Exporting PWM" <<std::endl;
        write(PWM_CHIP "/export", "0");
    };
    ~Servo(){
        std::cout << "Unexporting PWM" << std::endl;
        write(PWM_CHIP "/unexport", "0");
    }
    void rotate(uint16_t angle) {
        uint32_t duty_cycle = MIN_DUTY_CYCLE + (MAX_DUTY_CYCLE - MIN_DUTY_CYCLE) *
                                               (angle - MIN_DEGREE) / (MAX_DEGREE - MIN_DEGREE);
        write(PWM_CHANNEL "/period", "20000000");
        write(PWM_CHANNEL "/duty_cycle", std::to_string(duty_cycle));
        write(PWM_CHANNEL "/enable", "1");

        usleep(250000);
        write(PWM_CHANNEL "/enable", "0");
    }
private:
    static void write(const std::string &file_path, const std::string &text) {
        std::ofstream file;
        file.open(file_path);
        file << text << std::endl;
        file.close();
    }

};


// Message Queue

// https://man7.org/linux/man-pages/man2/mq_open.2.html
// https://w3.cs.jmu.edu/kirkpams/OpenCSF/Books/csf/html/MQueues.html
class MessageQueue{
public:
    MessageQueue(){
        queue_data = mq_open("/data", O_WRONLY);
        if (queue_data < 0) {
            throw std::runtime_error(
                    (std::ostringstream()
                            << "MessageQueue data init failed"
                            << "\n"
                    ).str()
            );
        }
    }

    ~MessageQueue(){
        if (queue_data > 0) {
            mq_close(queue_data);
        }
    }

    void send_data(uint16_t angle, uint16_t distance) {
        std::cout<< "Sending data" << std::endl;
        std::cout << "angle = " << angle << std::endl;
        std::cout << "distance = " << distance << std::endl;
        std::cout << std::endl;

        std::string data = std::to_string(angle) + "," + std::to_string(distance);
        int sent = mq_send(queue_data, data.c_str(), data.size() + 1, 1);
        if (sent < 0) {
            throw std::runtime_error(
                    (std::ostringstream()
                            << "Queue data failed to send"
                            << "\n"
                    ).str()
            );
        }
    }

private:
    mqd_t volatile queue_data = 0;
};


int main() {
    std::cout << "Main starting" << std::endl;

    auto queue = MessageQueue();
    VL53L0X_Dev_t sensor;
    auto tof = ToF(sensor);
    auto servo = Servo();

    uint16_t angle = MIN_DEGREE;
    int direction = STEP;

    while (true) {
        servo.rotate(angle);
        auto distance = tof.measure_distance();
        queue.send_data(angle, distance);

        angle += direction;
        if (angle >= MAX_DEGREE) {
            angle = MAX_DEGREE;
            direction = -direction;
        } else if (angle <= MIN_DEGREE) {
            angle = MIN_DEGREE;
            direction = -direction;
        }
    }

    return 0;
}