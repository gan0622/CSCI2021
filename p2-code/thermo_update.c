#include <stdio.h>
#include "thermo.h"

// Uses the two global variables (ports) THERMO_SENSOR_PORT and
// THERMO_STATUS_PORT to set the temp structure. If THERMO_SENSOR_PORT
// is above its maximum trusted value, associated with +50.0 deg C,
// does not alter temp and returns 1.  Otherwise, sets fields of temp
// based on converting sensor value to degrees and checking whether
// Celsius or Fahrenheit display is in effect. Returns 0 on successful
// set. This function DOES NOT modify any global variables but may
// access global variables.
//
// CONSTRAINT: Uses only integer operations. No floating point
// operations are used as the target machine does not have a FPU.
int set_temp_from_ports(temp_t *temp){
    if(THERMO_SENSOR_PORT > 64000 || THERMO_SENSOR_PORT < 0){
        return 1;
    }
    // each value unit is 1/64 of tenth
    short tem = THERMO_SENSOR_PORT / 64;
    
    if(THERMO_SENSOR_PORT % 64 >= 32){
        tem += 1;
    }
    if(THERMO_STATUS_PORT & 0b1){
        temp->is_fahrenheit = 1;
        temp->tenths_degrees = ((tem - 500) * 9)/5 +320; //max 1220 min -580
    }else{
        temp->is_fahrenheit = 0;
        temp->tenths_degrees = tem - 500; //500 -500
    }
    return 0;
}


// Alters the bits of integer pointed to by display to reflect the
// temperature in struct arg temp.  If temp has a temperature value
// that is below minimum or above maximum temperature allowable or if
// an improper indication of celsius/fahrenheit is given, does nothing
// and returns 1. Otherwise, calculates each digit of the temperature
// and changes bits at display to show the temperature according to
// the pattern for each digit.  This function DOES NOT modify any
// global variables but may access global variables.
int set_display_from_temp(temp_t temp, int *display){
    //base case
    if(temp.is_fahrenheit != 1 && temp.is_fahrenheit != 0){
        return 1;
    }
    if(temp.is_fahrenheit == 1){
    	if(temp.tenths_degrees > 1220 || temp.tenths_degrees < -580){
    		return 1;
    	}
    }else if(temp.is_fahrenheit == 0){
    	if(temp.tenths_degrees > 500 || temp.tenths_degrees < -500){
    		return 1;
    	}
    }
              
    short bitmasks[12] = {0b1111110,0b0001100,0b110111,0b0011111,0b1001101,0b1011011,0b1111011,0b0001110,0b1111111,0b1011111,0b0000000,0b0000001};
    // Bits 0-6 control the tenths place
    // Bits 7-13 control the ones place
    // Bits 14-20 control the tens place
    // Bits 21-27 control the hundreds place
    // Bit 28 controls whether degrees Celsius is shown
    // Bit 29 controls whether degrees Fahrenheit is shown
    // Bits 30 and 31 are not used and should always be 0.
    

    int start = 0;
    //check 
    if(temp.tenths_degrees < 0){
        start |= bitmasks[11];
        start = start << 7;
        temp.tenths_degrees *= -1; // negative sign
    }
    short temp_tenths = temp.tenths_degrees % 10;
    short temp_ones = (temp.tenths_degrees / 10) % 10;
    short temp_tens = (temp.tenths_degrees / 100) % 10;
    short temp_hundreds = temp.tenths_degrees / 1000;
    
    //base case from left to right
    if(temp_hundreds != 0){
        start |= bitmasks[temp_hundreds];
        start = start << 7; // 21
    }
    if(temp_hundreds != 0 || temp_tens != 0){
        start |= bitmasks[temp_tens];
        start = start << 7; // 14
    }
    
    start |= bitmasks[temp_ones];
    start  = start << 7; //0
    start |= bitmasks[temp_tenths];
    if(temp.is_fahrenheit == 1){
        start |= 0b10 << 28;
    }else{
        start |= 0b01 << 28;
    }
    *display = start;
    return 0;
}

// Called to update the thermometer display.  Makes use of
// set_temp_from_ports() and set_display_from_temp() to access
// temperature sensor then set the display. Checks these functions and
// if they indicate an error, makes not changes to the display.
// Otherwise modifies THERMO_DISPLAY_PORT to set the display.
// 
// CONSTRAINT: Does not allocate any heap memory as malloc() is NOT
// available on the target microcontroller.  Uses stack and global
// memory only.
int thermo_update(){
    temp_t lst = {};
    int display = 0;
    int check_error = set_temp_from_ports(&lst);
    if(check_error == 1){
        return 1;
    }

    check_error = set_display_from_temp(lst, &display);
    if(check_error != 0){
        return 1;
    }
    THERMO_DISPLAY_PORT = display;
    return 0;
}
