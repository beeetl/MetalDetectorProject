#include "stdbool.h"
#include "stdint.h"

#define portA *(unsigned volatile*)0x40000000
#define LED *(unsigned volatile*)0x40000008

#define ADC1 *(int volatile*)0x44A10258
#define ADC2 *(int volatile*)0x44A10278
#define timer_dur *(unsigned volatile*)0x44A00008
#define timer_state *(unsigned volatile*)0x44A00004

#define pmod_counter *(unsigned volatile*)0x44A20000

#define BTN *(unsigned volatile*)0x40010008
#define btnU_offset 0b1000
#define btnD_offset 0b0100
#define btnL_offset 0b0010
#define btnR_offset 0b0001

#define HEX_DATA 1
#define RAW_DATA 0


#define SW *(unsigned volatile*)0x40010000

// seven segment display (SSD) signals
    // HEX data, 0xFEEF will show FEEF on SSD
#define SSD_HEX *(unsigned volatile*)0x44A50000

    // RAW data top TWO bytes, 0xFE will show FE, 0b_ will show
    // the raw data allowing for non standard characters to be
    // printed (not 0 through F)
#define SSD_RAW_TOP *(unsigned volatile*)0x44A50004

    // bottom TWO bytes of raw segment data
#define SSD_RAW_BOT *(unsigned volatile*)0x44A50008

    // mode and display point position. set bit 4 to 1 to use hex
    // data mode, set bit 4 to 0 for raw data mode
    // set bits [3:0] to 1 to display a decimal point after
    // segment symbol i, where i is the bit being set to 1.
#define SSD_MODE_DP *(unsigned volatile*)0x44a5000C

#include "xil_printf.h"

// generic delay function for use in some functions where the
// addition of calling and returning to the function does not matter.
void delay_n_secs(uint8_t seconds)
{
	// a delay in terms of seconds
    timer_dur = 100 * 1000 * 1000 * seconds;

    // waiting until timer_counter has incremented appropriately
    while( (timer_state & 0b1) == 0){}

    return;
} // end of delay_n_secs

// generic delay function for use in some functions where the
// addition of calling and returning to the function does not matter.
void delay_n_msecs(uint16_t millis)
{
	// a delay in term of milliseconds
    timer_dur = 100 * 1000 * millis;

    // waiting until timer_counter has incremented appropriately
    while( (timer_state & 0b1) == 0){}

    return;
} // end of delay_n_msecs

void printSSD(_Bool is_hex, uint32_t whole_vector, uint8_t dp_vector)
{
	// checks whether the input data whole_vector is to be interpreted as hexadecimal data (4 bits per digit)
	// or raw segment data (7 bits per digit)
    if (is_hex)
    {
    	// sets up the input to seven segment display FSM for hex data
        SSD_MODE_DP = 0x10 | (dp_vector & 0xF);
        SSD_HEX = whole_vector & 0xFFFF;
    }
    else
    {
    	// sets up the input to seven segment display FSM for raw segment data
        SSD_MODE_DP = dp_vector & 0xF;
        SSD_RAW_TOP = (whole_vector >> 14) & 0x3FFF;
        SSD_RAW_BOT = whole_vector & 0x3FFF;
    }

    return;
} // end of printSSD

void update_SSD_data(_Bool detected_left, _Bool detected_right, _Bool is_close)
{
	// based on the boolean values of metal detection and whether or not it is close,
	// print a different word to the seven segment display.
    if (!detected_left && !detected_right)
    {	// no metal detected case
        // raw mode, vector says ndet, decimal point after n to imply two word output
        printSSD(RAW_DATA, 0b0101011010000100001100000111, 0b1000);
        return;
    }

    else if (detected_left && detected_right)
    {	// metal detected on both ADCs case
        // raw mode, vector says cntr, no decimal point.
        printSSD(RAW_DATA, 0b1000110010101100001110101111, 0b0000);
        return;
    }

    // cases for metal is detected to the side, not "not detected" or "close"
    if(!is_close)
    {	// case for if the metal is considered close

		if (detected_left && !detected_right)
		{// far left case
				// raw mode, vector says F.LFt
				printSSD(RAW_DATA, 0b0001110100011100011100000111, 0b1000);
				return;
		}

		else if (!detected_left && detected_right)
		{// far right case
			  printSSD(RAW_DATA, 0b0001110010111100100000000111, 0b1000);
			  return;
		}
    }

    // else, is_close = 1
    else
    {
        if (detected_left && !detected_right)
        {// far left case
                    // raw mode, vector says LEFt, no decimal point.
            printSSD(RAW_DATA, 0b1000111000011000011100000111, 0b1000);
            return;
        }

        else if (!detected_left && detected_right)
        {// far right case
            // raw mode, vector says rght, no decimal point.
        	printSSD(RAW_DATA, 0b0101111001000000010010000111, 0b0000);
            return;
        }
    }
} // end of update_SSD_data

void print_title()
{
	// prints a title to the seven segment display on startup
	// by shifting the appropriate characters in over time

	// MET
    printSSD(RAW_DATA, 0b1001000111100000001100000111, 0b0000);
    delay_n_msecs(500);

    // ETAL
    printSSD(RAW_DATA, 0b0000110000011100010001000111, 0b0000);
    delay_n_msecs(250);

    // TAL_
    printSSD(RAW_DATA, 0b0000111000100010001111110111, 0b0000);
    delay_n_msecs(250);

    // AL_D
    printSSD(RAW_DATA, 0b0001000100011111101110100001, 0b0000);
    delay_n_msecs(250);

    // L_DE
    printSSD(RAW_DATA, 0b1000111111011101000010000110, 0b0000);
    delay_n_msecs(250);

    // _DET
    printSSD(RAW_DATA, 0b1110111010000100001100000111, 0b0000);
    delay_n_msecs(250);

    // DETE
    printSSD(RAW_DATA, 0b0100001000011000001110000110, 0b0000);
    delay_n_msecs(250);

    // ETEC
    printSSD(RAW_DATA, 0b0000110000011100001101000110, 0b0000);
    delay_n_msecs(250);

    // TECT
    printSSD(RAW_DATA, 0b0000111000011010001100000111, 0b0000);
    delay_n_msecs(250);

    // ECTO
    printSSD(RAW_DATA, 0b0000110100011000001111000000, 0b0000);
    delay_n_msecs(250);

    // CTOR
    printSSD(RAW_DATA, 0b1000110000011110000000101111, 0b0000);
    delay_n_msecs(500);
    return;
} // end of print_title

_Bool btn_R_deb(void)
{
	// debounces the button on the right

    enum state {st_idle, st_is_pressed, st_is_debounced, st_reset, st_post_debounce};
    static enum state st = st_reset;
    static uint8_t times_pressed = 0;
    _Bool btn = BTN & btnR_offset;
    _Bool retval = false;

    switch (st)
    {
        default:
        case st_reset:
            // reset, we want count to go to zero
            times_pressed = 0;
            // go immediately to idle
            st = st_idle;
            // default retval to false.
            retval = false;
            break;

        case st_idle:
            // check if button has been pressed once
            if (btn)
            {
                // if so, we go to is_pressed state
                st = st_is_pressed;
            }
            break;

        case st_is_pressed:
            // if we are in this state for so long that times_pressed reaches 5, we go to is_debounced
            if (btn && ++times_pressed >= 5)
            {
                st = st_is_debounced;
            }
            break;

        case st_is_debounced:
            // we set retval to true, and leave immediately.
            st = st_post_debounce;
            retval = true;
            break;

        case st_post_debounce:
            // sit in here until button goes low, then return to idle.
            if (!btn)
            {
                st = st_idle;
            }
            break;
    }


    return retval;
} // end of btn_R_deb

_Bool btn_L_deb(void)
{
    enum state {st_idle, st_is_pressed, st_is_debounced, st_reset, st_post_debounce};
    static enum state st = st_reset;
    static uint8_t times_pressed = 0;
    _Bool btn = BTN & btnL_offset;
    _Bool retval = false;

    switch (st)
    {
        default:
        case st_reset:
            // reset, we want count to go to zero
            times_pressed = 0;
            // go immediately to idle
            st = st_idle;
            // default retval to false.
            retval = false;
            break;

        case st_idle:
            // check if button has been pressed once
            if (btn)
            {
                // if so, we go to is_pressed state
                st = st_is_pressed;
            }
            break;

        case st_is_pressed:
            // if we are in this state for so long that times_pressed reaches 5, we go to is_debounced
            if (btn && ++times_pressed >= 5)
            {
                st = st_is_debounced;
            }
            break;

        case st_is_debounced:
            // we set retval to true, and leave immediately.
            st = st_post_debounce;
            retval = true;
            break;

        case st_post_debounce:
            // sit in here until button goes low, then return to idle.
            if (!btn)
            {
                st = st_idle;
            }
            break;
    }


    return retval;
}

_Bool btn_U_deb(void)
{
    enum state {st_idle, st_is_pressed, st_is_debounced, st_reset, st_post_debounce};
    static enum state st = st_reset;
    static uint8_t times_pressed = 0;
    _Bool btn = BTN & btnU_offset;
    _Bool retval = false;

    switch (st)
    {
        default:
        case st_reset:
            // reset, we want count to go to zero
            times_pressed = 0;
            // go immediately to idle
            st = st_idle;
            // default retval to false.
            retval = false;
            break;

        case st_idle:
            // check if button has been pressed once
            if (btn)
            {
                // if so, we go to is_pressed state
                st = st_is_pressed;
            }
            break;

        case st_is_pressed:
            // if we are in this state for so long that times_pressed reaches 5, we go to is_debounced
            if (btn && ++times_pressed >= 5)
            {
                st = st_is_debounced;
            }
            break;

        case st_is_debounced:
            // we set retval to true, and leave immediately.
            st = st_post_debounce;
            retval = true;
            break;

        case st_post_debounce:
            // sit in here until button goes low, then return to idle.
            if (!btn)
            {
                st = st_idle;
            }
            break;
    }


    return retval;
}

_Bool btn_D_deb(void)
{
    enum state {st_idle, st_is_pressed, st_is_debounced, st_reset, st_post_debounce};
    static enum state st = st_reset;
    static uint8_t times_pressed = 0;
    _Bool btn = BTN & btnD_offset;
    _Bool retval = false;

    switch (st)
    {
        default:
        case st_reset:
            // reset, we want count to go to zero
            times_pressed = 0;
            // go immediately to idle
            st = st_idle;
            // default retval to false.
            retval = false;
            break;

        case st_idle:
            // check if button has been pressed once
            if (btn)
            {
                // if so, we go to is_pressed state
                st = st_is_pressed;
            }
            break;

        case st_is_pressed:
            // if we are in this state for so long that times_pressed reaches 5, we go to is_debounced
            if (btn && ++times_pressed >= 5)
            {
                st = st_is_debounced;
            }
            break;

        case st_is_debounced:
            // we set retval to true, and leave immediately.
            st = st_post_debounce;
            retval = true;
            break;

        case st_post_debounce:
            // sit in here until button goes low, then return to idle.
            if (!btn)
            {
                st = st_idle;
            }
            break;
    }


    return retval;
}

void calibration ( uint16_t* p_adc1_duration, uint16_t* p_adc2_duration)
{

	// set up the minimum observed adc value to be the highest the ints can store
    uint16_t adc1_min = UINT16_MAX;
    uint16_t adc2_min = UINT16_MAX;

    // temp values to store
    uint16_t adc1_temp = 0;
    uint16_t adc2_temp = 0;

    printSSD(0, 0b1000110000100010001110000011, 0b0001);
    delay_n_secs(1);

    for (int i = 0; i < 1000; i++)
    {
        timer_dur = 100;
        adc1_temp = ( (ADC1 >> 4) * 244 / 1000);
        if (adc1_min > adc1_temp)
        {
            adc1_min = adc1_temp;
        }
        while( (timer_state & 0b1) == 0){}

        timer_dur = 100;
        adc2_temp = ( (ADC2 >> 4) * 244 / 1000);
        if (adc2_min > adc2_temp)
        {
            adc2_min = adc2_temp;
        }
        while( (timer_state & 0b1) == 0){}
    }

    printSSD(RAW_DATA, 0b0001000010000110001101111001, 0b0000);
    delay_n_secs(1);
    printSSD(HEX_DATA, adc1_min, 0b0000);
    delay_n_secs(1);

    printSSD(RAW_DATA, 0b0001000010000110001100100100, 0b0000);
    delay_n_secs(1);
    printSSD(HEX_DATA, adc2_min, 0b0000);
    delay_n_secs(1);


    *p_adc1_duration = adc1_min;
    *p_adc2_duration = adc2_min;

    return;
}

// bools needed to test if a newly seen object is detected, and whether that applies
// to the close or far versions of non-centered positions
void print_num_objects_SSD(_Bool adc1, _Bool adc2, _Bool is_close, uint8_t mode_input)
{
    static uint8_t one_second_cntr = 0;

    static uint8_t f_left_cnt = 0;
    static uint8_t left_cnt = 0;
    static uint8_t center_cnt = 0;
    static uint8_t right_cnt = 0;
    static uint8_t f_right_cnt = 0;

    static uint8_t same_count = 0;

    enum current_display {title_fleft, value_fleft, title_left, value_left, title_center, value_center, title_right, value_right, title_fright, value_fright};

    static enum current_display state = title_left;

    enum counter_list {fleft, left, cntr, right, fright, reset};
    static enum counter_list previous_count = reset;
    // logic to update the seen_counts variable based on current and previous ADC digital signals

    _Bool is_right = !adc1 && adc2;

    _Bool is_left = adc1 && !adc2;

    _Bool is_center = adc1 && adc2;


    // if metal is detected on both sensors
    if (is_center)
    {
        // to not count the same object twice in center_cnt
        if (previous_count != cntr && same_count == 25)
        {
            center_cnt++;
            previous_count = cntr;
            same_count = 0;
        }
        else
        {
        	same_count++;
        }
    }

    // if metal is detected on left sensor only
    else if (is_left)
    {
        if (is_close)
        {
            if (previous_count != left && same_count == 25)
            {
                left_cnt++;
                previous_count = left;
                same_count = 0;
            }
            else
            {
            	same_count++;
            }
        }
        else
        {
            if (previous_count != fleft && same_count == 25)
            {
                f_left_cnt++;
                previous_count = fleft;
                same_count = 0;
            }
            else
            {
            	same_count++;
            }

        }
    }

    // if metal is detected on right sensor only
    else if (is_right)
    {
        if (is_close)
        {
            if (previous_count != right && same_count == 25)
            {
                right_cnt++;
                previous_count = right;
                same_count = 0;
            }
            else
            {
            	same_count++;
            }
        }
        else
        {
            if (previous_count != fright && same_count == 25)
            {
                f_right_cnt++;
                previous_count = fright;
                same_count = 0;
            }
            else
            {
            	same_count++;
            }
        }
    }
    else
    {
        previous_count = reset;
        same_count = 0;
    }

    if (mode_input == 2)
    {
        switch(state)
        {
        case title_fleft:
            // F.lft
            printSSD(RAW_DATA, 0b0001110100011100011100000111, 0b1000);
            break;

        case value_fleft:
            printSSD(HEX_DATA, f_left_cnt, 0b0000);
            break;

        case title_left:
            // LEFt
            printSSD(RAW_DATA, 0b1000111000011000011100000111, 0b0000);
            break;

        case value_left:
            printSSD(HEX_DATA, left_cnt, 0b0000);
            break;

        case title_center:
            // Cntr
            printSSD(RAW_DATA, 0b1000110010101100001110101111, 0b0000);
            break;

        case value_center:
            printSSD(HEX_DATA, center_cnt, 0b0000);
            break;

        case title_right:
            // rght
            printSSD(RAW_DATA, 0b0101111001000000010110000111, 0b0000);
            break;

        case value_right:
            printSSD(HEX_DATA, right_cnt, 0b0000);
            break;

        case title_fright:
            // f.rgt
            printSSD(RAW_DATA, 0b0001110010111100100000000111, 0b1000);
            break;

        case value_fright:
            printSSD(HEX_DATA, f_right_cnt, 0b0000);
            break;
        }
    }

    // 20 mS per main loop cycle, every 50 cycles is then 1 second, rotate state.
    if ( (one_second_cntr++) == 50)
    {
        one_second_cntr = 0;
        state = (state + 1) % 10;
    }

    return;
}

void set_detected_value(uint16_t *p_adc1_threshold, uint16_t *p_adc2_threshold)
{
    // this function can set the threshold value for adc1 and adc2 individually.
    // The threshold value is what the adc compares to against the default calibrated
    // adc value for a metal detection.

    // a detection is seen when the sampled adc value is less than the calibrated value
    // minus its threshold, so the threshold determines the drop in mV necessary to detect metal.

    // set
    printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
    delay_n_secs(1);

    //adc
    printSSD(RAW_DATA, 0b001000010000110001101111111, 0b0000);
    delay_n_secs(1);

    //trsh (threshold)
    printSSD(RAW_DATA, 0b0000111010111100100100001011, 0b0001);
    delay_n_secs(1);

    do
    {
        // 20 mS loop to update SSD display
        timer_dur = 100 * 1000 * 20;



        // if SW[0] is 0, set adc1 threshold value
        if ( (SW & 0b1) == 0)
        {
            // first segment indicates ADC chosen, last three indicate current threshold value.
            printSSD(HEX_DATA, (0b0001 << 12) | (pmod_counter & 0xFFF), 0b1000);
        }

        // otherwise we set adc2 threshold value
        else
        {
            // first segment indicates ADC chosen, last three indicate current threshold value.
            printSSD(HEX_DATA, (0b0010 << 12) | (pmod_counter & 0xFFF), 0b1000);
        }

        while ((timer_state & 0b1) == 0) {}
    }
    while (!btn_U_deb());

    // if SW[0] = 0 set adc1
    if ( (SW & 0b1) == 0)
    {
        *p_adc1_threshold = pmod_counter;
        printSSD(RAW_DATA, 0b0001000010000110001101001111, 0b0000);
        delay_n_secs(1);
        printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
        delay_n_secs(1);
    }

    // else set adc2
    else
    {
        *p_adc2_threshold = pmod_counter;
        printSSD(RAW_DATA, 0b0001000010000110001100100100, 0b0000);
        delay_n_secs(1);
        printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
        delay_n_secs(1);
    }
    // reset timer_dur to not get caught in main loop waiting for timer_state
    timer_dur = 1000;

    return;
}

void set_close_value(uint16_t* p_adc1_close_value, uint16_t* p_adc2_close_value)
{
    // this defines the value that an adc needs to see to be considered a close
    // this decides whether to display "far left" or "left" for example.

    // set
    printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
    delay_n_secs(1);

    //adc
    printSSD(RAW_DATA, 0b001000010000110001101111111, 0b0000);
    delay_n_secs(1);

    // clse. (close value)
    printSSD(RAW_DATA, 0b1000110100011100100100000110, 0b0001);
    delay_n_secs(1);

    do
    {
            // 20 mS loop to update SSD display
        timer_dur = 100 * 1000 * 20;


        // if SW[0] is 0, set adc1 close value
        if ( (SW & 0b1) == 0)
        {
            // first segment indicates ADC chosen, last three indicate current threshold value.
            printSSD(HEX_DATA, (0b0001 << 12) | (pmod_counter & 0xFFF), 0b1000);
        }

        // otherwise we set adc2 close value
        else
        {
            // first segment indicates ADC chosen, last three indicate current threshold value.
            printSSD(HEX_DATA, (0b0010 << 12) | (pmod_counter & 0xFFF), 0b1000);
        }

        while ((timer_state & 0b1) == 0) {}
    }
    while (!btn_D_deb());

    // update adc1 close value
    if ( (SW & 0b1) == 0)
    {
        *p_adc1_close_value = pmod_counter;

        // adc1
        printSSD(RAW_DATA, 0b0001000010000110001101001111, 0b0000);
        delay_n_secs(1);

        // set
        printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
        delay_n_secs(1);
    }

    // else update adc2 close value
    else
    {
        *p_adc2_close_value = pmod_counter;

        // adc2
        printSSD(RAW_DATA, 0b0001000010000110001100100100, 0b0000);
        delay_n_secs(1);

        // set
        printSSD(RAW_DATA, 0b0010010000011000001111111111, 0b0000);
        delay_n_secs(1);
    }

    timer_dur = 1000;

    return;
}

#define deadzone_spacing 30

int main()
{

    enum mode {position, strength, num_objects};

    enum mode current_mode = position;

    _Bool adc1_digital = false;
    _Bool adc2_digital = false;

    _Bool is_close = false;

    _Bool is_close_left = false;
    _Bool is_close_right = false;

    // these are the default voltage on the capacitors, set during calibration time when no metal is near the coils.
    uint16_t adc1_cal = 0;
    uint16_t adc2_cal = 0;

    // these are the constantly sampled ADC values that will be updated during the main program loop
    uint16_t adc1_val = 0;
    uint16_t adc2_val = 0;

    // default values for the threshold of seeing a left or right metal detected
    uint16_t threshold_voltage_adc1 = 50;
    uint16_t threshold_voltage_adc2 = 50;

    // default values for the threshold of seeing a close left or right metal detected
    uint16_t is_close_threshold_adc1 = 100;
    uint16_t is_close_threshold_adc2 = 100;

    // These are used for setting the LED strength meter, updated after calibration
    uint16_t default_total = 0;
    uint16_t LED_unit = 0;

    // prints "metal detector" on the seven segment display using rotation
    print_title();

    // calibrates adc1_cal and adc2_cal variables for use in main loop
    calibration(&adc1_cal, &adc2_cal);

    // the default value that indicates no metal observed
    default_total = adc1_cal + adc2_cal;

    // the drop in mV that corresponds to one of 16 LEDS, default_total normalized by 16
    LED_unit = default_total / 16;

    while(1)
    {
        // 20 mS
        timer_dur = 100 * 1000 * 20;

        // sampling on the ADC values in terms of mV
        adc1_val = ((ADC1 >> 4) * 244 ) / 1000;
        adc2_val = ((ADC2 >> 4) * 244 ) / 1000;

        // debounced left button that sets the current mode enum
        if (btn_L_deb())
        {
            if (current_mode) // mode >= 0 (can go left)
            {
                current_mode -= 1;
            }
        }

        // debounced right button that sets the current mode enum
        if (btn_R_deb())
        {
            // cap mode to 2, since mode enum is size 3.
            current_mode = (current_mode + 1) % 3;
        }

        // debounced up button that calls the set detected values function, this allows one to change
        // threshold voltage that is required for metal to be considered detected by either ADC
        if (btn_U_deb())
        {
            set_detected_value(&threshold_voltage_adc1, &threshold_voltage_adc2);
        }

        // debounced down button that calls the set close values function, this allows one to change
        // the threshold voltage that is required for metal to be considered detected, and close by either ADC
        if (btn_D_deb())
        {
            set_close_value(&is_close_threshold_adc1, &is_close_threshold_adc2);
        }

        is_close = is_close_left || is_close_right;


        // updating the boolean signal for detecting metal on adc1
        if (adc1_digital)
        { // metal is detected case for adc1

        	// metal is detected but far
        	if (!is_close_left)
        	{ // can either go to ndet or close

        		// this checks if the deadzone has been passed and we want to go to ndet
        		if (adc1_val > (adc1_cal - threshold_voltage_adc1 + deadzone_spacing / 2))
        		{
        			adc1_digital = false;
        		}

        		// this checks if the deadzone has been passed and we want to go to close right
        		else if (adc1_val < (adc1_cal - is_close_threshold_adc1 - deadzone_spacing / 2))
        		{
        			is_close_left = true;
        		}
        	}

        	// metal is detected and close
        	else
        	{
        		// this checks if the threshold above the dead zone has been reached
        		if (adc1_val > (adc1_cal - is_close_threshold_adc1 + deadzone_spacing / 2))
        		{
        			is_close_left = false;
        		}
        	}
        }

        else
        { // in not detected case for adc1.

        	// this checks if the drop past the dead zone has been reached
        	if (adc1_val < (adc1_cal - threshold_voltage_adc1 - deadzone_spacing / 2))
        	{
        		// set digital to true to get a far output.
        		adc1_digital = true;
        	}
        }

        //updating the boolean signal for detecting metal on adc2
        if (adc2_digital)
        { // metal is detected case for adc2

        	// metal is detected but far
        	if (!is_close_right)
        	{ // can either go to ndet or close

        		// this checks if the deadzone has been passed and we want to go to ndet
        		if (adc2_val > (adc2_cal - threshold_voltage_adc2 + deadzone_spacing / 2))
        		{
        			adc2_digital = false;
        		}

        		// this checks if the deadzone has been passed and we want to go to close right
        		else if (adc2_val < (adc2_cal - is_close_threshold_adc2 - deadzone_spacing / 2))
        		{
        			is_close_right = true;
        		}
        	}

        	// metal is detected and close
        	else
        	{
        		// this checks if the threshold above the dead zone has been reached
        		if (adc2_val > (adc2_cal - is_close_threshold_adc2 + deadzone_spacing / 2))
        		{
        			is_close_right = false;
        		}
        	}
        }

        else
        { // in not detected case for adc2.

        	// this checks if the drop past the dead zone has been reached
        	if (adc2_val < (adc2_cal - threshold_voltage_adc2 - deadzone_spacing / 2))
        	{
        		// set digital to true to get a far output.
        		adc2_digital = true;
        	}
        }

        // checks that the ADC difference is positive due to noise in ADC signal
        if (default_total - adc1_val - adc2_val > 0)
        {
        	// if the difference is positive, take the difference and normalize it by LED_unit
        	// and set LED such that each LED_unit of mV seen lights up an LED
            LED = 0xFFFF & ~(0xFFFF >> ((default_total - adc1_val - adc2_val) / LED_unit));
        }

        else
        {
        	// otherwise if ADC difference due to noise is negative just blank out LEDs
            LED = 0x0000;
        }

        // based on the current_mode enumeration the information displayed on the seven segment display differs.
        switch (current_mode)
        {
            case position:
                // position mode, prints F.LFt, LEFt, Cntr, rght, F.rgt
                // depending on digitalized adc value readings, and is_close bool.
                update_SSD_data(adc1_digital, adc2_digital, is_close);
            break;

            case strength:
                // strength mode, prints a numeric value in HEX of the ADC calibrated values
                // minus their current values (metallic object decreases observed voltage on coils)
            	// again an if is used to check that the difference is positive due to noise on ADC inputs
                if (default_total - (adc1_val + adc2_val) > 0)
                printSSD(HEX_DATA, (default_total - (adc1_val + adc2_val)), 0b0000);
                else
                // clips lower bound to 0 in case of overflow (will show 0xF... on SSD)
                printSSD(HEX_DATA, 0b0000, 0b0000);
            break;
            default:
                // we dont care what this does in current_mode = num_objects since the function is calculated at all times
            break;
        }
        // this counts number of objects seen, it must be calculated every cycle.
        // whether or not this actually outputs depends on the current_mode input
        print_num_objects_SSD(adc1_digital, adc2_digital, is_close, current_mode);

        // wait until timer is the correct value to ensure a main loop frequency of 50 Hz
        while((timer_state & 0b1) == 0){}
    }
    return 0;
}
