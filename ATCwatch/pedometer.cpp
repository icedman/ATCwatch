#include "pedometer.h"
#include <TimeLib.h>
#include <math.h>

#define STEP_INTERVAL_MIN 500  // Shortest interval to walk one step (mS)
#define STEP_INTERVAL_MAX 2000 // Longest interval to walk one step (mS)
#define WINDOW_INTERVAL 1000   // How often to reset window min/max range (mS)
#define FILTER_SIZE_PEDO 12    // Number of accelerometer readings to average
static float mag;
static float window_min;
static float window_max;
static float threshold;
static float sample_old;
static float sample_new;
static long time;
static long last_step_time;
static long last_window_time;
static float filter_buf[FILTER_SIZE_PEDO];
static float filter_sum = 0.0;      // Initial average value
static int filter_index = 0;        // Current position in sample-averaging buffer
static int steps = 0;
static float precision = 1.25;      // Lower numbers = more sensitive to steps
int time_since_last_step;

static float lowpassFilter(float mag){
    float sample_result;
    
    // Low-pass filter: average the last FILTER_SIZE_PEDO magnitude readings
    filter_sum -= filter_buf[filter_index];   // Subtract old value from sum
    filter_buf[filter_index] = mag;           // Store new value in buffer
    filter_sum += mag;                        // Add new value to sum
    filter_index += 1;                        // Increment position in buffer
    if (filter_index >= FILTER_SIZE_PEDO)          // and wrap around to start
    {
        filter_index = 0;
    }
    sample_result = filter_sum / FILTER_SIZE_PEDO; // Average buffer value
    
    return sample_result;
}


int get_steps()
{
    return steps;
}

void reset_steps()
{
    steps = 0;
}

void pedometer_init()
{
    float xaccl, yaccl, zaccl;
    int x, y, z;
    
    // Read initial accelerometer state and assign to various things to start
    accl_data_struct accl_data = get_accl_data();

    xaccl=float(accl_data.x);
    yaccl=float(accl_data.y);
    zaccl=float(accl_data.z);
    //mag = squareRoot(xaccl * xaccl + yaccl * yaccl + zaccl * zaccl); // 3space magnitude
    mag = sqrt(xaccl * xaccl + yaccl * yaccl + zaccl * zaccl); // 3space magnitude

    window_min = mag; // Minimum reading from accel in last WINDOW_INTERVAL seconds
    window_max = mag; // Maximum reading from accel in last WINDOW_INTERVAL seconds
    threshold = mag;  // Midpoint of WINDOW_MIN, WINDOW_MAX
    sample_old = mag;
    sample_new = mag;
    last_step_time = millis();
    last_window_time = last_step_time;
}

// void pedometer_calculatePedo(int x, int y, int z)
void pedometer_calculate()
{
    float sample_result, xaccl, yaccl, zaccl;

    time = millis();

    accl_data_struct accl_data = get_accl_data();

    xaccl=float(accl_data.x)/100;
    yaccl=float(accl_data.y)/100;
    zaccl=float(accl_data.z)/100;

    // xaccl=float(x)/100.0;
    // yaccl=float(y)/100.0;
    // zaccl=float(z)/100.0;
    //mag = squareRoot(xaccl * xaccl + yaccl * yaccl + zaccl * zaccl); // 3space magnitude
    mag = sqrt(xaccl * xaccl + yaccl * yaccl + zaccl * zaccl); // 3space magnitude

    sample_result = lowpassFilter(mag);

    // Every WINDOW_INTERVAL seconds, calc new THRESHOLD, reset min and max
    if ((time - last_window_time) >= WINDOW_INTERVAL) // Time for new window?
    {
        threshold = lowpassFilter((window_min + window_max) / 2);  // Average of min & max

        window_min = sample_result;                 // Reset min and max to
        window_max = sample_result;                 // the last value read
        last_window_time = time;                    // Note time of reset
    }
    else                                            // Not WINDOW_INTERVAL yet,
    {
        if (sample_result < window_min)             // keep adjusting min and
        {
            window_min = sample_result;             // max to accel data.
        }
        if (sample_result > window_max)
        {
            window_max = sample_result;
        }
    }
    
    // Watch for sufficiently large changes in accelerometer readings...
    sample_old = sample_new;
    if (abs(sample_result - sample_old) > precision)
    {
        sample_new = sample_result;
        // If crossing the threshold in the + direction...
        if ((sample_old <= threshold) && (threshold <= sample_new))
        {
            // And if within reasonable time window for another step...
            time_since_last_step = time - last_step_time;
            if (STEP_INTERVAL_MIN <= time_since_last_step)
            {
                if (time_since_last_step <= STEP_INTERVAL_MAX)
                {
                    // It's a step!
                    steps = steps + 1;
                }
                last_step_time = time;
            }
        }
    }

    return;
}