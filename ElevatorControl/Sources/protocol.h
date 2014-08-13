/* Elevator CAN node IDs */
#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#define MSCAN_CTL_ID    0x0001  // Controller node ID
#define MSCAN_CAR_ID    0x0002  // Elevator car node ID
#define MSCAN_FL1_ID    0x0004  // Floor 1 callbox node ID
#define MSCAN_FL2_ID    0x0008  // Floor 2 callbox node ID
#define MSCAN_FL3_ID    0x0010  // Floor 3 callbox node ID
#define MSCAN_USENSE_ID 0x0020  // Ultrasonic sensor node ID


//#define USE_LOOPBACK    // Will setup for loopback mode when defined

/*
 * Command structure
 *  Elevator location:           CMD_LOCATION    + FLOORx + DIRECTION_x
 *  Call button pressed:         CMD_BUTTON_CALL + FLOORx + BUTTON_x
 *  Elevator car button pressed: CMD_BUTTON_CAR  + BUTTON_x
 *  Append display character:    CMD_DISP_APPEND + ascii
 *  Error occurred:              CMD_ERROR       + ERROR_x
 */

// Command IDs
#define CMD_LOCATION        0x00    // From controller to car
#define CMD_BUTTON_CALL     0x01    // From callbox to controller
#define CMD_BUTTON_CAR      0x02    // From callbox to controller
#define CMD_DISP_APPEND     0x03    // From controller to car
#define CMD_DISTANCE        0x04    // From usense node to controller
#define CMD_ERROR           0xFF    // From any to any

// Floor IDs
#define FLOOR1              0x01
#define FLOOR2              0x02
#define FLOOR3              0x03

// Car direction
#define DIRECTION_UP        0x01
#define DIRECTION_DOWN      0x02
#define DIRECTION_STATIONARY    0x03

// Callbox button IDs
#define BUTTON_UP           0x01
#define BUTTON_DOWN         0x02

// Elevator car button IDs
#define BUTTON_FL1          0x01
#define BUTTON_FL2          0x02
#define BUTTON_FL3          0x03
#define BUTTON_DOOR_CLOSE   0x10
#define BUTTON_DOOR_OPEN    0x11
#define BUTTON_STOP         0xEE

// Error IDs
#define ERROR_CLR           0x00
#define ERROR_GENERAL       0x01

#endif // _PROTOCOL_H