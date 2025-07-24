// Script Control Actions
#define SCRIPT_HALT                             0xFA
#define SCRIPT_DELAY                            0xFB
#define SCRIPT_JMP                              0xFC
#define SCRIPT_RESTART                          0xFD
#define CLEAR_PROFILE                    		0xFF

#define TEST_CONNECTION                         0x00

// INIT Actions
#define SET_SYSTEM                              0x01
#define SET_RTC                                 0x02
#define SET_NTC_CONTROL                         0x03
#define SET_TEMP_PROFILE                        0x04
#define START_TEMP_PROFILE                      0x05
#define STOP_TEMP_PROFILE                       0x06
#define SET_OVERRIDE_TEC_PROFILE                0x07
#define START_OVERRIDE_TEC_PROFILE              0x08
#define STOP_OVERRIDE_TEC_PROFILE               0x09
#define SET_PDA_PROFILE                         0x0A
#define SET_CAMERA_PROFILE                      0x0B

// DLS_ROUTINE Actions
#define SET_DLS_INTERVAL                        0x11
#define SET_LASER_INTENSITY                     0x12
#define SET_POSITION                            0x13
#define START_SAMPLING_CYCLE                    0x14
#define GET_SAMPLE                              0x15

// CAM_ROUTINE Actions
#define SET_CAMERA_INTERVAL                     0x21
#define SET_EXT_LASER_INTENSITY                 0x22
#define TURN_ON_EXT_LASER                       0x23
#define SET_CAMERA_POSITION                     0x24
#define TAKE_IMG_WITH_TIMEOUT                   0x25
#define TURN_OFF_EXT_LASER                      0x26


/*
halt
delay + Param: time(32bit)
jmp + Param: step id

test_connection + Param: value(32bit)
set_temp_profile + Param: ntc_index(8bit) tec_positions(8bit) heater_positions(8bit) tec_voltage(8bit) heater_duty_cycle(8bit) target_temperature(16bit)
start_temp_profile
stop_temp_profile
set_override_tec_profile + Param: override_tec_index(8bit) override_tec_vol(8bit)
start_override_tec_profile
stop_override_tec_profile
set_sampling_profile + Param: pre_duration(32bit) sample_duration(32bit) post_duration(32bit) sampling_rate(16bit)
set_laser_intensity + Param: intensity(8bit)
set_position + Param: position(8bit)
start_sampling_cycle
get_sample
set_ext_laser_profile + Param: intensity(8bit)
turn_on_ext_laser + Param: position(8bit)
turn_off_ext_laser
take_img + Param: position(8bit)

*/
