//----------------------------------------------------------------------------------------------------------------------------------

#include "types.h"
#include "scope_functions.h"
#include "statemachine.h"
#include "timer.h"
#include "fpga_control.h"
#include "spi_control.h"
#include "sd_card_interface.h"
#include "display_lib.h"
#include "ff.h"
#include "user_interface_functions.h"
#include "usb_interface.h"
#include "variables.h"

#include "sin_cos_math.h"

#include <string.h>

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data processing functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_acquire_trace_data(void)
{
  uint32 data;

  //Check if running
  if(scopesettings.runstate == RUN_STATE_RUNNING)
  {
    //Show the user waiting for a trigger
    ui_display_waiting_triggered_text(0);
    
    //Set the trigger level
    fpga_set_trigger_level();

    //Write the time base setting to the FPGA
    fpga_set_time_base(scopesettings.timeperdiv);

    //Sampling with trigger circuit enabled
    scopesettings.samplemode = 1;

    //Start the conversion and wait until done or user input given
    fpga_do_conversion();

    //Check if cut short with user input
    if(toprocesscommand)
    {
      //If so skip the rest
      return;
    }
    
    //Check if in single mode
    if(scopesettings.triggermode == 1)
    {
      //Switch to stopped
      scopesettings.runstate = RUN_STATE_STOPPED;

      //Show this on the screen
      ui_display_run_stop_text();
    }

    //Get trigger point information
    //Later on used to send to the FPGA with command 0x1F
    data = fpga_prepare_for_transfer();
    
    
    //Check if there was a trigger and show it on the screen if so
    if(fpga_had_trigger())
    {
      ui_display_waiting_triggered_text(1);
    }
    
    //After 800 samples the FPGA starts checking on a trigger, and it can run past the end of the buffer back to the beginning
    //To determine the starting point of from where to read the data it is possible to extend it to this 800 samples as the before
    //and read 1600 samples per ADC

    //Just using the same calculation for every setting solves the frequency calculation error
    //The signal representation still is correct and the trigger point seems to be more valid also
    //The original uses time base dependent processing here, but this seems to do the trick on all ranges
    //The software needs to verify the trigger to make it more stable
    if(data < 750)
    {
      //Less then 750 make it bigger (4095 - 750 = 3345)
      //Reading is also circular. 4096 becomes 0.
      data = data + 3345;
    }
    else
    {
      //More then 750 make it smaller
      data = data - 750;
    }

    //Only need a single count variable for both channels, since they run on the same sample rate
    //This can be changed to a global define
    scopesettings.nofsamples  = SAMPLES_PER_ADC;
    scopesettings.samplecount = SAMPLE_COUNT;

    //Check if channel 1 is enabled
    if(scopesettings.channel1.enable)
    {
      //Get the samples for channel 1
      fpga_read_sample_data(&scopesettings.channel1, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && (scopesettings.triggerchannel == 0))
      {
        //Use the channel 1 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel1.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }

    //Check if channel 2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Get the samples for channel 2
      fpga_read_sample_data(&scopesettings.channel2, data);

      //Check if always 50% trigger is enabled and the trigger is on this channel
      if(scopesettings.alwaystrigger50 && scopesettings.triggerchannel)
      {
        //Use the channel 2 center value as trigger level
        scopesettings.triggerlevel = scopesettings.channel2.center;

        //Set the trigger vertical position position to match the new trigger level
        scope_calculate_trigger_vertical_position();
      }
    }


    //Need to improve on this for a more stable displaying. On the low sample rate settings it seems to flip between two positions.
    //Determine the trigger position based on the selected trigger channel
    scope_process_trigger(scopesettings.nofsamples);

  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_process_trigger(uint32 count)
{
  uint8  *buffer;
  uint32  index;
  uint32  level = scopesettings.triggerlevel;
  uint32  sample1;
  uint32  sample2;

  //Select the trace buffer to process based on the trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    //Channel 1 buffer
    buffer = (uint8 *)channel1tracebuffer;
  }
  else
  {
    //Channel 2 buffer
    buffer = (uint8 *)channel2tracebuffer;
  }

  disp_have_trigger = 0;

  //Set a starting point for checking on trigger
  //Count is half a sample buffer!!
  index = count - 10;
  count = 20;

  //Need a better check here, maybe over a wider range of samples

  while(count--)
  {
    sample1 = buffer[index];
    sample2 = buffer[index + 1];

    if(((scopesettings.triggeredge == 0) && (sample1 < level) && (sample2 >= level)) ||
       ((scopesettings.triggeredge == 1) && (sample1 >= level) && (sample2 < level)))
    {
      //Set the current index as trigger point
      disp_trigger_index = index;

      //Signal trigger has been found
      disp_have_trigger = 1;

      //Done with checking
      break;
    }

    //Select next sample to check
    index++;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_do_baseline_calibration(void)
{
  uint32 flag = 1;
  uint32 voltperdiv;

  //Disable the trigger circuit
  scopesettings.samplemode = 0;

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = SAMPLE_COUNT;
  scopesettings.nofsamples  = SAMPLES_PER_ADC;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Clear the compensation values before doing the calibration
  calibrationsettings.adc1compensation = 0;
  calibrationsettings.adc2compensation = 0;

  //Setup for channel 1 calibration
  calibrationsettings.voltperdivcommand = 0x33;
  calibrationsettings.offsetcommand     = 0x32;
  calibrationsettings.adc1command       = 0x20;
  calibrationsettings.adc2command       = 0x21;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel1tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel1.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel1.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel1.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Setup for channel 2 calibration
  calibrationsettings.voltperdivcommand = 0x36;
  calibrationsettings.offsetcommand     = 0x35;
  calibrationsettings.adc1command       = 0x22;
  calibrationsettings.adc2command       = 0x23;

  //Use the trace buffer of this channel
  calibrationsettings.tracebuffer = (uint8 *)channel2tracebuffer;

  //Calibrate this channel
  flag &= scope_do_channel_calibration();

  //Check if calibration was successful
  if(flag)
  {
    //Copy the results if so
    for(voltperdiv=0;voltperdiv<6;voltperdiv++)
    {
      //For each volt per division setting there is a DC calibration offset
      scopesettings.channel2.dc_calibration_offset[voltperdiv] = calibrationsettings.dc_calibration_offset[voltperdiv];
    }

    //The last one is also for the highest sensitivity setting
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];

    //Copy the ADC compensation values
    scopesettings.channel2.adc1compensation = calibrationsettings.adc1compensation;
    scopesettings.channel2.adc2compensation = calibrationsettings.adc2compensation;
  }

  //Load the normal operation settings back into the FPGA
  fpga_set_channel_voltperdiv(&scopesettings.channel1);
  fpga_set_channel_offset(&scopesettings.channel1);
  fpga_set_channel_voltperdiv(&scopesettings.channel2);
  fpga_set_channel_offset(&scopesettings.channel2);
  fpga_set_sample_rate(scopesettings.samplerate);

  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

#define HIGH_DC_OFFSET   500
#define LOW_DC_OFFSET   1200

uint32 scope_do_channel_calibration(void)
{
  uint32 flag = 1;
  int32  samplerate;
  int32  voltperdiv;
  int32  highaverage;
  int32  lowaverage;
  int32  dcoffsetstep;
  int32  compensationsum = 0;

  //Calibrate for the hardware sensitivity settings
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start with the first set of averages
    samplerateindex = 0;

    //Do the measurements on two sample rates. 200MSa/s and 50KSa/s
    for(samplerate=0;samplerate<18;samplerate+=11)
    {
      //Set the selected sample rate
      fpga_set_sample_rate(samplerate);

      //Set the matching time base
      fpga_set_time_base(sample_rate_time_per_div[samplerate]);

      //Set the high DC offset in the FPGA (Lower value returns higher ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = HIGH_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as one reading here. Only use ADC1 data
      highaverage = calibrationsettings.adc1rawaverage;

      //Set the low DC offset in the FPGA (Higher value returns lower ADC reading)
      calibrationsettings.dc_calibration_offset[voltperdiv] = LOW_DC_OFFSET;
      fpga_set_channel_offset(&calibrationsettings);

      //Wait 25ms before sampling
      timer0_delay(25);

      //Start the conversion and wait until done
      fpga_do_conversion();

      //Get the data from a sample run
      fpga_read_sample_data(&calibrationsettings, 100);

      //Need the average as another reading here. Only use ADC1 data
      lowaverage = calibrationsettings.adc1rawaverage;

      //Calculate the DC offset step for a single ADC bit change for this volt/div setting
      //Low DC offset minus high DC offset (1200 - 500) = 700. Scaled up for fixed point calculation ==> 700 << 20 = 734003200
      dcoffsetstep = 734003200 / (highaverage - lowaverage);

      //Calculate the average DC offset settings for both the low to center as the high to center readings
      highaverage = HIGH_DC_OFFSET + (((highaverage - 128) * dcoffsetstep) >> 20);
      lowaverage  = LOW_DC_OFFSET  - (((128 - lowaverage) * dcoffsetstep) >> 20);

      //Set the result for this sample rate and volt per division setting
      samplerateaverage[samplerateindex][voltperdiv] = (highaverage + lowaverage) / 2;

      //Save the dc offset step for final calibration after compensation data has been determined
      sampleratedcoffsetstep[samplerateindex][voltperdiv] = dcoffsetstep;

      //Select the next set of sample indexes
      samplerateindex++;
    }
  }

  //Set the sample rate to 2MSa/s
  fpga_set_sample_rate(6);

  //Set the matching time base
  fpga_set_time_base(sample_rate_time_per_div[6]);

  //Average and test the results on a third sample rate
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //Set the average of the two measurements as the DC offset to work with
    calibrationsettings.dc_calibration_offset[voltperdiv] = (samplerateaverage[0][voltperdiv] + samplerateaverage[1][voltperdiv]) / 2;

    //Set the to do sensitivity setting in the FPGA
    calibrationsettings.samplevoltperdiv = voltperdiv;
    fpga_set_channel_voltperdiv(&calibrationsettings);

    //Set the new DC channel offset in the FPGA
    fpga_set_channel_offset(&calibrationsettings);

    //Wait 50ms before sampling
    timer0_delay(50);

    //Start the conversion and wait until done
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(&calibrationsettings, 100);

    //Check if the average reading is outside allowed range
    if((calibrationsettings.adc1rawaverage < 125) || (calibrationsettings.adc1rawaverage > 131))
    {
      //When deviation is more then 3, signal it as a failure
      flag = 0;
    }

    //Sum the ADC differences
    compensationsum += (calibrationsettings.adc2rawaverage - calibrationsettings.adc1rawaverage);
  }

  //Calculate the average of the ADC difference
  compensationsum /= 6;

  //Split the difference on the two ADC's
  calibrationsettings.adc1compensation = compensationsum / 2;
  calibrationsettings.adc2compensation = -1 * (compensationsum - calibrationsettings.adc1compensation);

  //Check if the found result is within limits
  if((compensationsum < -20) || (compensationsum > 20))
  {
    //Not so clear the flag
    flag = 0;
  }

  //Adjust the center point DC offsets with the found compensation values
  for(voltperdiv=0;voltperdiv<6;voltperdiv++)
  {
    //The DC offset is based on the pre compensated ADC1 reading, so need to adjust with the average DC offset step times the ADC1 compensation value
    calibrationsettings.dc_calibration_offset[voltperdiv] = (int16)calibrationsettings.dc_calibration_offset[voltperdiv] + (((int32)calibrationsettings.adc1compensation * (((int32)sampleratedcoffsetstep[0][voltperdiv] + (int32)sampleratedcoffsetstep[1][voltperdiv]) / 2)) >> 20);
  }

  //Return the result of the tests. True if all tests passed
  return(flag);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_do_auto_setup(void)
{
  PCHANNELSETTINGS settings;

  int32  screentime;
  int32  timeperdiv;
  int32  samplerate;

  uint32 dochannel1 = scopesettings.channel1.enable;
  uint32 dochannel2 = scopesettings.channel2.enable;

  //No need to do auto setup if no channel is enabled
  if((dochannel1 == 0) && (dochannel2 == 0))
  {
    return;
  }

  //Disable the trigger circuit
  scopesettings.samplemode = 0;
  
  //Modification on 09-03-2022. Begin
  //Switch back to AUTO trigger mode to avoid lockup in NORMAL or SINGLE mode
  scopesettings.triggermode = 0;
  
  //Show it on the display
  ui_display_trigger_mode();
  
  //Also set the FPGA to the AUTO trigger mode
  fpga_set_trigger_mode();
  
  //To make sure the scope will be running after auto set, set the run mode to running
  scopesettings.runstate = RUN_STATE_RUNNING;
  
  //Show this on the screen
  ui_display_run_stop_text();
  //End. 09-03-2022

  //Set number of samples
  //This can be changed to a global define
  scopesettings.samplecount = SAMPLE_COUNT;
  scopesettings.nofsamples  = SAMPLES_PER_ADC;

  //Send the command for setting the trigger level to the FPGA
  fpga_write_cmd(0x17);
  fpga_write_byte(0);

  //Setup channel 1 if enabled
  if(dochannel1)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel1.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel1);
  }

  //Setup channel 2 if enabled
  if(dochannel2)
  {
    //Use the most sensitive hardware setting and set it in the FPGA
    scopesettings.channel2.samplevoltperdiv = 5;
    fpga_set_channel_voltperdiv(&scopesettings.channel2);
  }

  //Select the channel to work with
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  //Wait 50ms to allow the relays to settle
  timer0_delay(50);

  //Do the measurements on four sample rates. 200MSa/s, 2MSa/s, 20KSa/s and 1000Sa/s
  for(samplerate=0;samplerate<4;samplerate++)
  {
    //Set the selected sample rate
    fpga_set_sample_rate(samplerate_for_autosetup[samplerate]);

    //Set the matching time base
    fpga_set_time_base(sample_rate_time_per_div[samplerate_for_autosetup[samplerate]]);

    //Start the conversion and wait until done
    fpga_do_conversion();

    //Get the data from a sample run
    fpga_read_sample_data(settings, 100);

    //Check if there is a frequency reading and break the loop if so
    if(settings->frequencyvalid)
    {
      break;
    }
  }

  //When there is a frequency reading determine the needed time per division setting
  if(settings->frequencyvalid)
  {
    //Can't use the frequency here since it is based on the scopesettings.samplerate variable, which is not used here
    //Calculate the time in nanoseconds for getting three periods on the screen
    screentime = (float)settings->periodtime * sample_time_converters[samplerate_for_autosetup[samplerate]];

    //Match the found time to the nearest time per division setting
    for(timeperdiv=0;timeperdiv<24;timeperdiv++)
    {
      //When the found time is higher than the selected time per division quit the search
      if(screentime > time_per_div_matching[timeperdiv])
      {
        break;
      }
    }

    //Check if not on the first setting
    if(timeperdiv)
    {
      //If so take one of to get to the right one to use. Also ensures not selecting a non existing setting if none found
      timeperdiv--;
    }

    //Select the found time per division
    scopesettings.timeperdiv = timeperdiv;
    scopesettings.samplerate = time_per_div_sample_rate[timeperdiv];
  }
  else
  {
    //Set a default sample rate and time per division setting
    scopesettings.timeperdiv = 12;
    scopesettings.samplerate = time_per_div_sample_rate[12];
  }

  //Range the input sensitivity on the enabled channels
  //Check on which bottom check level needs to be used
  //When both channels are enabled and in normal display mode use separate sections of the screen for each channel.
  if(dochannel1 && dochannel2 && (scopesettings.xymodedisplay == 0))
  {
    //Both channels enabled then use a lower level. Smaller section of the display available per channel so lower value
    scopesettings.channel1.maxscreenspace = 1900;
    scopesettings.channel2.maxscreenspace = 1900;

    //Give both traces it's own location on screen
    scopesettings.channel1.traceposition = 300;
    scopesettings.channel2.traceposition = 100;

    //Get the sample data for the not trigger source channel when a valid frequency has been determined
    if(settings->frequencyvalid)
    {
      //Check which channel is the trigger source
      if(scopesettings.triggerchannel == 0)
      {
        //Channel 1 is trigger source so get channel 2 data
        fpga_read_sample_data(&scopesettings.channel2, 100);
      }
      else
      {
        //Channel 2 is trigger source so get channel 1 data
        fpga_read_sample_data(&scopesettings.channel1, 100);
      }
    }
  }
  else
  {
    //Only one channel enabled then more screen space available for it so higher value
    scopesettings.channel1.maxscreenspace = 3900;
    scopesettings.channel2.maxscreenspace = 3900;

    //Used channel will be set on the middle of the display
    scopesettings.channel1.traceposition = 200;
    scopesettings.channel2.traceposition = 200;
  }

  //Set the new sample rate in the FPGA
  fpga_set_sample_rate(scopesettings.samplerate);

  //Show the new settings
  ui_display_time_per_division();

  //Check the channels on valid voltage readings with the already done conversion, but only when valid frequency
  if(settings->frequencyvalid)
  {
    //Check if channel 1 is enabled and check its range if so
    if(dochannel1)
    {
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 is enabled and check its range if so
    if(dochannel2)
    {
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }
  }

  //If one or either channel not in range another conversion is needed
  if(dochannel1 || dochannel2)
  {
    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel1.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Use the 500mV/div setting and set it in the FPGA
      scopesettings.channel2.samplevoltperdiv = 3;
      fpga_set_channel_voltperdiv(&scopesettings.channel2);
    }

    //Wait 50ms to allow the relays to settle
    timer0_delay(50);

    //Start the conversion and wait until done
    fpga_do_conversion();

    //Check if channel 1 still needs to be done
    if(dochannel1)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel1, 100);

      //Check the range again
      dochannel1 = scope_check_channel_range(&scopesettings.channel1);
    }

    //Check if channel 2 still needs to be done
    if(dochannel2)
    {
      //Get the data from a sample run
      fpga_read_sample_data(&scopesettings.channel2, 100);

      //Check the range again
      dochannel2 = scope_check_channel_range(&scopesettings.channel2);
    }

    //If one or either channel not in range another conversion is needed
    if(dochannel1 || dochannel2)
    {
      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel1.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Use the 2.5V/div setting and set it in the FPGA
        scopesettings.channel2.samplevoltperdiv = 1;
        fpga_set_channel_voltperdiv(&scopesettings.channel2);
      }

      //Wait 50ms to allow the relays to settle
      timer0_delay(50);

      //Start the conversion and wait until done
      fpga_do_conversion();

      //Check if channel 1 still needs to be done
      if(dochannel1)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel1, 100);

        //Check the range again
        dochannel1 = scope_check_channel_range(&scopesettings.channel1);
      }

      //Check if channel 2 still needs to be done
      if(dochannel2)
      {
        //Get the data from a sample run
        fpga_read_sample_data(&scopesettings.channel2, 100);

        //Check the range again
        dochannel2 = scope_check_channel_range(&scopesettings.channel2);
      }

      //If one or either channel not in range the least sensitive setting needs to be used
      if(dochannel1 || dochannel2)
      {
        //Check if channel 1 still not done
        if(dochannel1)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel1.samplevoltperdiv = 0;
        }

        //Check if channel 2 still not done
        if(dochannel2)
        {
          //Set the lowest sensitivity for this channel
          scopesettings.channel2.samplevoltperdiv = 0;
        }
      }
    }
  }

  //Check if channel 1 is enabled and set the new settings if so
  if(scopesettings.channel1.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel1);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel1.displayvoltperdiv = scopesettings.channel1.samplevoltperdiv;
    
    //Update the display
    ui_display_channel_settings(&scopesettings.channel1);
  }

  //Check if channel 2 is enabled and set the new settings if so
  if(scopesettings.channel2.enable)
  {
    //Set the new setting in the FPGA
    fpga_set_channel_voltperdiv(&scopesettings.channel2);

    //Copy the found sample volt per division setting to the display setting
    scopesettings.channel2.displayvoltperdiv = scopesettings.channel2.samplevoltperdiv;
    
    //Update the display
    ui_display_channel_settings(&scopesettings.channel2);
  }

  //Adjust the trigger level to 50% setting
  sm_do_50_percent_trigger_setup();
}

//----------------------------------------------------------------------------------------------------------------------------------

uint32 scope_check_channel_range(PCHANNELSETTINGS settings)
{
  uint32 screenpixels;
  uint32 range;
  uint32 notdone = 0;

  //Convert the peak peak reading to screen pixels
  screenpixels = (settings->peakpeak * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Check if there is any signal at all to avoid divide by zero
  if(screenpixels)
  {
    //Calculate a fixed point range to determine the volts per division setting, based on the available screen space
    range = settings->maxscreenspace / screenpixels;

    //Check if signal is at least 10 times smaller then the max range
    if(range >= 100)
    {
      //If so go three sensitivity settings up. (e.g. 500mV/div ==> 50mV/div)
      settings->samplevoltperdiv = 3;
    }
    //Else check if signal is at least 5 times smaller then the max range
    else if(range >= 50)
    {
      //If so go two sensitivity settings up. (e.g. 500mV/div ==> 100mV/div)
      settings->samplevoltperdiv += 2;
    }
    //Else check if signal is at least 2.5 times smaller then the max range
    else if(range >= 25)
    {
      //If so go one sensitivity setting up. (e.g. 500mV/div ==> 200mV/div)
      settings->samplevoltperdiv += 1;
    }
    //Else check if signal is bigger then the max range
    else if(range < 10)
    {
      //Out of range so not done
      notdone = 1;
    }

    //Check if volt per division not out of range
    if(settings->samplevoltperdiv > 6)
    {
      //Keep it on max if so
      settings->samplevoltperdiv = 6;
    }
  }
  else
  {
    //When there is no signal use the most sensitive setting
    settings->samplevoltperdiv = 6;
  }

  return(notdone);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_calculate_trigger_vertical_position(void)
{
  PCHANNELSETTINGS settings;

  //Select the channel based on the current trigger channel
  if(scopesettings.triggerchannel == 0)
  {
    settings = &scopesettings.channel1;
  }
  else
  {
    settings = &scopesettings.channel2;
  }

  int32 position;

  //Center the trigger level around 0 point
  position = scopesettings.triggerlevel - 128;

  //Adjust it for the current volt per div setting
  position = (position * signal_adjusters[settings->displayvoltperdiv]) >> VOLTAGE_SHIFTER;

  //Add the trace center to it
  position = settings->traceposition + position;

  //Limit to extremes
  if(position < 0)
  {
    position = 0;
  }
  else if(position > 401)
  {
    position = 401;
  }

  //Set as new position
  scopesettings.triggerverticalposition = position;
}

//----------------------------------------------------------------------------------------------------------------------------------
// Signal data display functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_trace_data(void)
{
  //See if it is possible to rework this to fixed point. A 32 bit mantissa is not accurate enough though

  //On the scope the last pixel interleaving is not working properly. Don't know why.

  //Check out the sin x/x interpolation downloaded from keysight (Sine wave reproduction using Sinc interpolation - v.1.0.py)

  //This needs more code to skip when stopped and no changes are made. It seems to be ok without this, but what is not needed is not needed
  //In the display trace routine the display only needs to be redrawn when certain conditions apply
  //  The user changes the sample rate or the time per div setting. Might need to block the setting of the sample rate when stopped because that makes no change to the current sample buffer.
  //  The user moves pointers around. Trace up and down should work, and trigger left and right. Changing the trigger level should not do anything, or even be disabled



  //Need to compensate for the position being on the left side of the pointer
  uint32 triggerposition = scopesettings.triggerhorizontalposition + 7;

  //Check if a trigger position has been found
  if(disp_have_trigger == 0)
  {
    //When not use the center of the sample buffer
    disp_trigger_index = scopesettings.samplecount / 2;
  }

  //Make sure the two settings are in range of the tables!!!!
  //SKipp displaying if not????


  //The amount of x positions needed per sample is based on the number of pixels per division, the set time per division and the sample rate.
  disp_xpos_per_sample = (50.0 * frequency_per_div[scopesettings.timeperdiv]) / sample_rate[scopesettings.samplerate];

  //This gives the step size for going through the samples. Is also the linear interleaving step for the y direction
  disp_sample_step = 1.0 / disp_xpos_per_sample;

  //The displayable x range is based on the number of samples and the number of x positions needed per sample
  //Halved to allow trigger position to be in the center
  double xrange = (scopesettings.samplecount * disp_xpos_per_sample) / 2.0;

  //x range needs to be at least 1 pixel
  if(xrange < 1.0)
  {
    xrange = 1.0;
  }
  else if(xrange > (double)(TRACE_HORIZONTAL_END + 2))
  {
    //Limit on max screen pixels to avoid disp_xend becoming 0x80000000 due to overflow
    xrange = (double)(TRACE_HORIZONTAL_END + 2);
  }

  //Calculate the start and end x coordinates
  disp_xstart = triggerposition - xrange;
  disp_xend = triggerposition + xrange;

  //Limit on just before the start of trace display
  if(disp_xstart < (TRACE_HORIZONTAL_START - 2))
  {
    disp_xstart = TRACE_HORIZONTAL_START - 2;
  }

  //And limit slightly after the end of the trace display for better results
  if(disp_xend > (TRACE_HORIZONTAL_END + 2))
  {
    disp_xend = TRACE_HORIZONTAL_END + 2;
  }

  //Determine first sample to use based on a full screen worth of samples and the trigger position in relation to the number of pixels on the screen
  disp_first_sample = disp_trigger_index - ((((double)(TRACE_HORIZONTAL_END + 2) / disp_xpos_per_sample) * triggerposition) / (double)(TRACE_HORIZONTAL_END + 2)) - 1;

  //If below the first sample limit it on the first sample
  if(disp_first_sample < 0)
  {
    disp_first_sample = 0;
  }

  //This makes sure no reading outside the buffer can occur
  if(disp_sample_step > ((scopesettings.samplecount) / 2))
  {
    disp_sample_step = (scopesettings.samplecount) / 2;
  }




  //If samplestep > 1 might be an idea to draw the in between samples on the same x position to avoid aliasing
  //If sample step < 1 then skip drawing on x positions. The draw line function does the linear interpolation



  //Use a separate buffer to clear the screen. Also used as source to copy back to the screen
  display_set_screen_buffer(displaybuffer1);
  display_set_source_buffer(displaybuffer1);

  //Clear the trace portion of the screen
  display_set_fg_color(0x00000000);
  display_fill_rect(TRACE_HORIZONTAL_START, TRACE_VERTICAL_START, TRACE_MAX_WIDTH - 1, TRACE_MAX_HEIGHT - 1);

  //Check if not in waveform view mode with grid disabled
  if((scopesettings.waveviewmode == 0) || scopesettings.gridenable == 0)
  {
    //Draw the grid lines and dots based on the grid brightness setting
    ui_draw_grid();
  }

  //Check if scope is in normal display mode
  if(scopesettings.xymodedisplay == 0)
  {
    //The calculations done above need to go here??


    //Check if channel1 is enabled
    if(scopesettings.channel1.enable)
    {
      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel1);
    }

    //Check if channel2 is enabled
    if(scopesettings.channel2.enable)
    {
      //Go and do the actual trace drawing
      scope_display_channel_trace(&scopesettings.channel2);
    }

    //Displaying of FFT needs to be added here.

  }
  else
  {
    //Scope set to x y display mode
    //Set x-y mode display trace color
    display_set_fg_color(XYMODE_COLOR);

    uint32 index = disp_trigger_index - 315;
    uint32 last = index + 730;

    //Need two samples per channel
    uint32 x1,x2;
    uint32 y1,y2;

    //Get the samples for the first point
    x1 = scope_get_x_sample(&scopesettings.channel1, index);
    y1 = scope_get_y_sample(&scopesettings.channel2, index);
    
    //Handle all the needed samples
    for(;index<last;index++)
    {
      //Get the samples for the next point
      x2 = scope_get_x_sample(&scopesettings.channel1, index);
      y2 = scope_get_y_sample(&scopesettings.channel2, index);

      //Draw the line between these two points
      display_draw_line(x1, y1, x2, y2);

      //Swap the points for drawing the next part
      x1 = x2;
      y1 = y2;
    }
  }

  //Draw the cursors with their text and measurement display
  ui_draw_time_cursors();
  ui_draw_volt_cursors();
  ui_display_selected_text();
  ui_display_cursor_measurements();

  //Draw the signal center, trigger level and trigger position pointers
  ui_draw_pointers();

  //Update the measurements in the six slots on the screen
  ui_update_measurements();
  
  //To allow for grid brightness to be changed in the background of the slider menu draw it in when needed
  ui_show_open_slider();
  
  //Copy it to the actual screen buffer
  display_set_screen_buffer((uint16 *)maindisplaybuffer);
  display_copy_rect_to_screen(TRACE_HORIZONTAL_START, TRACE_VERTICAL_START, TRACE_MAX_WIDTH, TRACE_MAX_HEIGHT);

  //Check if in waveform view
  if(scopesettings.waveviewmode)
  {
    //Display the file name directly on the actual display because it is outside the trace window
    display_set_fg_color(FILE_NAME_HIGHLIGHT_COLOR);
    display_set_font(&font_0);
    display_text(VIEW_FILENAME_XPOS, VIEW_FILENAME_YPOS, viewfilename);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_x_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }

  //Limit the sample on max displayable
  if(sample > 401)
  {
    sample = 401;
  }

  //The x center position has an extra offset compared to the y trace position
  return(sample + 165);
}

//----------------------------------------------------------------------------------------------------------------------------------

int32 scope_get_y_sample(PCHANNELSETTINGS settings, int32 index)
{
  register int32 sample;

  //Center adjust the sample
  sample = (int32)settings->tracebuffer[index] - 128;

  //Get the sample and adjust the data for the correct voltage per div setting
  sample = (sample * signal_adjusters[settings->samplevoltperdiv]) >> VOLTAGE_SHIFTER;

  //Scale the sample based on the two volt per div settings when they differ
  if(settings->displayvoltperdiv != settings->samplevoltperdiv)
  {
    //Scaling factor is based on the two volts per division settings
    sample = (sample * vertical_scaling_factors[settings->displayvoltperdiv][settings->samplevoltperdiv]) / 10000;
  }
  
  //Offset the sample on the screen
  sample = settings->traceposition + sample;

  //Limit sample on min displayable
  if(sample < 0)
  {
    sample = 0;
  }
  //Limit the sample on max displayable plus one to cut it of when outside displayable range
  else if(sample > (TRACE_MAX_HEIGHT + 1))
  {
    sample = TRACE_MAX_HEIGHT + 1;
  }

  //Display y coordinates are inverted to signal orientation
  return(TRACE_VERTICAL_END - sample);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_display_channel_trace(PCHANNELSETTINGS settings)
{
  double inputindex;

  register int32 previousindex;
  register int32 currentindex;
  register int32 sample1 = 0;
  register int32 sample2 = 0;
  register uint32 lastx = disp_xstart;
  register uint32 xpos = disp_xstart + 1;

  register PDISPLAYPOINTS tracepoints = settings->tracepoints;

  //Set the trace color for the current channel
  display_set_fg_color(settings->color);

  //Get the processed sample
  sample1 = scope_get_y_sample(settings, disp_first_sample);

  //Store the first sample in the trace points buffer
  tracepoints->x = lastx;
  tracepoints->y = sample1;
  tracepoints++;

  //Start with one tracepoint
  settings->noftracepoints = 1;

  //Step to the next input index
  inputindex = disp_first_sample + disp_sample_step;

  //The previous index is the index of the first sample
  previousindex = disp_first_sample;

  //Process the sample data to screen data
  for(; xpos < disp_xend; inputindex += disp_sample_step, xpos++)
  {
    //Get the current integer index into the sample buffer
    currentindex = inputindex;

    //Check if linear approximation needs to be done. (Only when step < 1) pixels are skipped if so.
    if(currentindex != previousindex)
    {
      //Set new previous index
      previousindex = currentindex;

      //Get the processed sample
      sample2 = scope_get_y_sample(settings, currentindex);

      //Store the second sample in the screen buffer
      tracepoints->x = xpos;
      tracepoints->y = sample2;
      tracepoints++;

      //One more tracepoint
      settings->noftracepoints++;

      //Need to draw a line here
      display_draw_line(lastx, sample1, xpos, sample2);

      sample1 = sample2;

      lastx = xpos;
    }
  }

  //When step less then 1 the last pixel needs to be interpolated between current sample and next sample.
  if(disp_sample_step < 1.0)
  {
    //Calculate the scaler for the last y value based on the x distance from the last drawn position to the end of the screen
    //divided by the x distance it takes to where the next position should be drawn (Number of x steps per sample)
    double scaler =  ((double)(TRACE_HORIZONTAL_END + 2) - lastx) / disp_xpos_per_sample;    // (1 / samplestep);

    //Get the processed sample
    sample2 = scope_get_y_sample(settings, inputindex);

    sample2 = sample1 + ((double)((double)sample2 - (double)sample1) / scaler);

    //Store the last sample in the screen buffer
    tracepoints->x = xpos;
    tracepoints->y = sample2;
    tracepoints++;

    //One more tracepoint
    settings->noftracepoints++;

    //Draw the last line
    display_draw_line(lastx, sample1, xpos, sample2);
  }
}

//----------------------------------------------------------------------------------------------------------------------------------
// Configuration data functions
//----------------------------------------------------------------------------------------------------------------------------------

void scope_load_configuration_data(void)
{
  //Load the settings data from its sector on the SD card
  if(sd_card_read(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer) != SD_OK)
  {
    //Load a default set on failure
    scope_reset_config_data();
    
    //Save it to the SD card
    scope_save_configuration_data();
  }
  else
  {
    //Restore the settings from the loaded data
    scope_restore_config_data();
  }

  //Set the FPGA commands for channel 1
  scopesettings.channel1.enablecommand     = 0x02;
  scopesettings.channel1.couplingcommand   = 0x34;
  scopesettings.channel1.voltperdivcommand = 0x33;
  scopesettings.channel1.offsetcommand     = 0x32;
  scopesettings.channel1.adc1command       = 0x20;
  scopesettings.channel1.adc2command       = 0x21;

  //Set the menu data for channel 1
  scopesettings.channel1.color            = CHANNEL1_COLOR;
  scopesettings.channel1.highlightboxdata = &channel_1_highlight_box;
  scopesettings.channel1.boxdata          = &channel_1_box;
  scopesettings.channel1.boxtext          = &channel_1_box_text;
  scopesettings.channel1.infoxpos         = 517;
  scopesettings.channel1.infoypos         = 6;
  
  //Set the trace and display buffer pointers for channel 1
  scopesettings.channel1.tracebuffer  = (uint8 *)channel1tracebuffer;
  scopesettings.channel1.tracepoints = channel1pointsbuffer;

  //Set the FPGA commands for channel 2
  scopesettings.channel2.enablecommand     = 0x03;
  scopesettings.channel2.couplingcommand   = 0x37;
  scopesettings.channel2.voltperdivcommand = 0x36;
  scopesettings.channel2.offsetcommand     = 0x35;
  scopesettings.channel2.adc1command       = 0x22;
  scopesettings.channel2.adc2command       = 0x23;

  //Set the menu data for channel 2
  scopesettings.channel2.color            = CHANNEL2_COLOR;
  scopesettings.channel2.highlightboxdata = &channel_2_highlight_box;
  scopesettings.channel2.boxdata          = &channel_2_box;
  scopesettings.channel2.boxtext          = &channel_2_box_text;
  scopesettings.channel2.infoxpos         = 622;
  scopesettings.channel2.infoypos         = 6;

  //Set the trace and display buffer pointers for channel 2
  scopesettings.channel2.tracebuffer = (uint8 *)channel2tracebuffer;
  scopesettings.channel2.tracepoints = channel2pointsbuffer;
  
  //Start in running state
  scopesettings.runstate = RUN_STATE_RUNNING;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_configuration_data(void)
{
  //Save the settings for writing to the flash
  scope_save_config_data();

  //Write the data to its sector on the SD card
  sd_card_write(SETTINGS_SECTOR, 1, (uint8 *)settingsworkbuffer);
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_reset_config_data(void)
{
  uint32 index;
  
  //Load a default configuration in case of settings in flash being corrupted

  //Enable channel 1, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in top part of the screen
  scopesettings.channel1.enable            = 1;
  scopesettings.channel1.coupling          = 0;
  scopesettings.channel1.magnification     = 0;
  scopesettings.channel1.displayvoltperdiv = 0;
  scopesettings.channel1.samplevoltperdiv  = 0;
  scopesettings.channel1.fftenable         = 0;
  scopesettings.channel1.traceposition     = 300;

  //Enable channel 2, dc coupling, 1x magnification, 50V/div, fft disabled and trace line in bottom part of the screen
  scopesettings.channel2.enable            = 1;
  scopesettings.channel2.coupling          = 0;
  scopesettings.channel2.magnification     = 0;
  scopesettings.channel2.displayvoltperdiv = 0;
  scopesettings.channel2.samplevoltperdiv  = 0;
  scopesettings.channel2.fftenable         = 0;
  scopesettings.channel2.traceposition     = 100;

  //Set trigger mode to auto, trigger edge to rising, trigger channel to channel 1, trigger position and trigger screen offset to center of the screen
  scopesettings.triggermode     = 0;
  scopesettings.triggeredge     = 0;
  scopesettings.triggerchannel  = 0;
  scopesettings.triggerhorizontalposition = 362;
  scopesettings.triggerverticalposition   = 200;

  //Set move speed to fast
  scopesettings.movespeed = MOVE_SPEED_FAST;

  //Set time base to 20uS/div
  scopesettings.timeperdiv = 12;

  //Set the related acquisition speed which is 5MHz
  scopesettings.samplerate = 5;

  //Enable some default measurements
  scopesettings.measurementitems[0].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[0].channel         = 0;
  scopesettings.measurementitems[0].index           = 4;                            //Vpp

  scopesettings.measurementitems[1].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[1].channel         = 0;
  scopesettings.measurementitems[1].index           = 2;                            //Vavg

  scopesettings.measurementitems[2].channelsettings = &scopesettings.channel1;
  scopesettings.measurementitems[2].channel         = 0;
  scopesettings.measurementitems[2].index           = 6;                            //Freq
  
  scopesettings.measurementitems[3].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[3].channel         = 1;
  scopesettings.measurementitems[3].index           = 4;                            //Vpp

  scopesettings.measurementitems[4].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[4].channel         = 1;
  scopesettings.measurementitems[4].index           = 2;                            //Vavg

  scopesettings.measurementitems[5].channelsettings = &scopesettings.channel2;
  scopesettings.measurementitems[5].channel         = 1;
  scopesettings.measurementitems[5].index           = 6;                            //Freq

  //Select default cursor
  scopesettings.selectedcursor = CURSOR_TIME_LEFT;
  
  //Turn time cursor off and set some default positions
  scopesettings.timecursorsenable   = 0;
  scopesettings.timecursor1position = 183;
  scopesettings.timecursor2position = 547;

  //Turn volt cursor of and set some default positions
  scopesettings.voltcursorsenable   = 0;
  scopesettings.voltcursor1position = 167;
  scopesettings.voltcursor2position = 328;

  //Set screen brightness to high, grid brightness to low, always 50% trigger off, x-y display mode off and confirmation mode enabled
  scopesettings.screenbrightness = 100;
  scopesettings.gridbrightness   = 25;
  scopesettings.alwaystrigger50  = 0;
  scopesettings.xymodedisplay    = 0;
  scopesettings.confirmationmode = 1;
  
  //Set default channel calibration values
  for(index=0;index<7;index++)
  {
    //Set FPGA center level
    scopesettings.channel1.dc_calibration_offset[index] = 750;
    scopesettings.channel2.dc_calibration_offset[index] = 750;
  }
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_save_config_data(void)
{
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Set a version number for checking if the settings match the current firmware
  settingsworkbuffer[2] = SETTING_SECTOR_ID_HIGH;
  settingsworkbuffer[3] = SETTING_SECTOR_ID_LOW;
  settingsworkbuffer[4] = SETTING_SECTOR_VERSION_HIGH;
  settingsworkbuffer[5] = SETTING_SECTOR_VERSION_LOW;
  
  //Point to the channel 1 settings
  ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];
  
  //Save the channel 1 settings
  *ptr++ = scopesettings.channel1.enable;
  *ptr++ = scopesettings.channel1.coupling;
  *ptr++ = scopesettings.channel1.magnification;
  *ptr++ = scopesettings.channel1.displayvoltperdiv;
  *ptr++ = scopesettings.channel1.samplevoltperdiv;
  *ptr++ = scopesettings.channel1.fftenable;
  *ptr++ = scopesettings.channel1.traceposition;

  //Point to the channel 2 settings
  ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];
  
  //Save the channel 2 settings
  *ptr++ = scopesettings.channel2.enable;
  *ptr++ = scopesettings.channel2.coupling;
  *ptr++ = scopesettings.channel2.magnification;
  *ptr++ = scopesettings.channel2.displayvoltperdiv;
  *ptr++ = scopesettings.channel2.samplevoltperdiv;
  *ptr++ = scopesettings.channel2.fftenable;
  *ptr++ = scopesettings.channel2.traceposition;

  //Point to the trigger settings
  ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];
  
  //Save trigger settings
  *ptr++ = scopesettings.timeperdiv;
  *ptr++ = scopesettings.samplerate;
  *ptr++ = scopesettings.triggermode;
  *ptr++ = scopesettings.triggeredge;
  *ptr++ = scopesettings.triggerchannel;
  *ptr++ = scopesettings.triggerlevel;
  *ptr++ = scopesettings.triggerhorizontalposition;
  *ptr++ = scopesettings.triggerverticalposition;
  
  //Point to the other settings
  ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];
  
  //Save the other settings
  *ptr++ = scopesettings.movespeed;
  *ptr++ = scopesettings.confirmationmode;
  *ptr++ = scopesettings.screenbrightness;
  *ptr++ = scopesettings.gridbrightness;
  *ptr++ = scopesettings.alwaystrigger50;
  *ptr++ = scopesettings.xymodedisplay;

  //Point to the cursor settings
  ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];
  
  //Save the current selected cursor
  *ptr++ = scopesettings.selectedcursor;
  
  //Save the time cursor settings
  *ptr++ = scopesettings.timecursorsenable;
  *ptr++ = scopesettings.timecursor1position;
  *ptr++ = scopesettings.timecursor2position;

  //Save the volt cursor settings
  *ptr++ = scopesettings.voltcursorsenable;
  *ptr++ = scopesettings.voltcursor1position;
  *ptr++ = scopesettings.voltcursor2position;

  //Point to the first measurement enable setting
  ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

  //Save the measurement slots states
  for(index=0;index<6;index++)
  {
    //Copy the current measurement channel and index
    *ptr++ = scopesettings.measurementitems[index].channel;
    *ptr++ = scopesettings.measurementitems[index].index;
  }
  
  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

  //Copy the working set values to the saved values
  for(index=0;index<6;index++,ptr++)
  {
    //Copy the data for both channels
    ptr[0] = scopesettings.channel1.dc_calibration_offset[index];
    ptr[6] = scopesettings.channel2.dc_calibration_offset[index];
  }

  //Point to the calibration settings
  ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];
  
  //Save the ADC compensation values
  *ptr++ = scopesettings.channel1.adc1compensation;
  *ptr++ = scopesettings.channel1.adc2compensation;
  *ptr++ = scopesettings.channel2.adc1compensation;
  *ptr++ = scopesettings.channel2.adc2compensation;
  
  //Calculate a checksum over the settings data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Save the checksum
  settingsworkbuffer[0] = checksum >> 16;
  settingsworkbuffer[1] = checksum;
}

//----------------------------------------------------------------------------------------------------------------------------------

void scope_restore_config_data(void)
{
  uint32  index;
  uint32  checksum = 0;
  uint16 *ptr;

  //Calculate a checksum over the loaded data
  for(index=2;index<256;index++)
  {
    checksum += settingsworkbuffer[index];
  }
  
  //Check if the checksum is a match as well as the id and version number
  if((settingsworkbuffer[0] == (checksum >> 16))            &&
     (settingsworkbuffer[1] == (checksum & 0xFFFF))         &&
     (settingsworkbuffer[2] == SETTING_SECTOR_ID_HIGH)      &&
     (settingsworkbuffer[3] == SETTING_SECTOR_ID_LOW)       &&
     (settingsworkbuffer[4] == SETTING_SECTOR_VERSION_HIGH) &&
     (settingsworkbuffer[5] == SETTING_SECTOR_VERSION_LOW))
  {
    //Point to the channel 1 settings
    ptr = &settingsworkbuffer[CHANNEL1_SETTING_OFFSET];

    //Restore the channel 1 settings
    scopesettings.channel1.enable            = *ptr++;
    scopesettings.channel1.coupling          = *ptr++;
    scopesettings.channel1.magnification     = *ptr++;
    scopesettings.channel1.displayvoltperdiv = *ptr++;
    scopesettings.channel1.samplevoltperdiv  = *ptr++;
    scopesettings.channel1.fftenable         = *ptr++;
    scopesettings.channel1.traceposition     = *ptr++;

    //Point to the channel 2 settings
    ptr = &settingsworkbuffer[CHANNEL2_SETTING_OFFSET];

    //Restore the channel 2 settings
    scopesettings.channel2.enable            = *ptr++;
    scopesettings.channel2.coupling          = *ptr++;
    scopesettings.channel2.magnification     = *ptr++;
    scopesettings.channel2.displayvoltperdiv = *ptr++;
    scopesettings.channel2.samplevoltperdiv  = *ptr++;
    scopesettings.channel2.fftenable         = *ptr++;
    scopesettings.channel2.traceposition     = *ptr++;

    //Point to the trigger settings
    ptr = &settingsworkbuffer[TRIGGER_SETTING_OFFSET];

    //Restore trigger settings
    scopesettings.timeperdiv                = *ptr++;
    scopesettings.samplerate                = *ptr++;
    scopesettings.triggermode               = *ptr++;
    scopesettings.triggeredge               = *ptr++;
    scopesettings.triggerchannel            = *ptr++;
    scopesettings.triggerlevel              = *ptr++;
    scopesettings.triggerhorizontalposition = *ptr++;
    scopesettings.triggerverticalposition   = *ptr++;

    //Point to the other settings
    ptr = &settingsworkbuffer[OTHER_SETTING_OFFSET];

    //Restore the other settings
    scopesettings.movespeed        = *ptr++;
    scopesettings.confirmationmode = *ptr++;
    scopesettings.screenbrightness = *ptr++;
    scopesettings.gridbrightness   = *ptr++;
    scopesettings.alwaystrigger50  = *ptr++;
    scopesettings.xymodedisplay    = *ptr++;
    
    //Point to the cursor settings
    ptr = &settingsworkbuffer[CURSOR_SETTING_OFFSET];

    //Restore the current selected cursor
    scopesettings.selectedcursor = *ptr++;
    
    //Restore the time cursor settings
    scopesettings.timecursorsenable   = *ptr++;
    scopesettings.timecursor1position = *ptr++;
    scopesettings.timecursor2position = *ptr++;

    //Restore the volt cursor settings
    scopesettings.voltcursorsenable   = *ptr++;
    scopesettings.voltcursor1position = *ptr++;
    scopesettings.voltcursor2position = *ptr++;

    //Point to the first measurement enable setting
    ptr = &settingsworkbuffer[MEASUREMENT_SETTING_OFFSET];

    //Restore the measurement slots states
    for(index=0;index<6;index++)
    {
      //Copy the current measurement channel and index
      scopesettings.measurementitems[index].channel = *ptr++;
      scopesettings.measurementitems[index].index   = *ptr++;
      
      //Set the pointer to the actual channel data based on the selected channel
      if(scopesettings.measurementitems[index].channel == 0)
      {
        scopesettings.measurementitems[index].channelsettings = &scopesettings.channel1;
      }
      else
      {
        scopesettings.measurementitems[index].channelsettings = &scopesettings.channel2;
      }
    }
    
    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET];

    //Restore the working set values from the saved values
    for(index=0;index<6;index++,ptr++)
    {
      //Copy the data for both channels
      scopesettings.channel1.dc_calibration_offset[index] = ptr[0];
      scopesettings.channel2.dc_calibration_offset[index] = ptr[6];
    }

    //The last entry is a copy of the fore last value
    scopesettings.channel1.dc_calibration_offset[6] = scopesettings.channel1.dc_calibration_offset[5];
    scopesettings.channel2.dc_calibration_offset[6] = scopesettings.channel2.dc_calibration_offset[5];
    
    //Point to the calibration settings
    ptr = &settingsworkbuffer[CALIBRATION_SETTING_OFFSET + 20];

    //Restore the ADC compensation values
    scopesettings.channel1.adc1compensation = *ptr++;
    scopesettings.channel1.adc2compensation = *ptr++;
    scopesettings.channel2.adc1compensation = *ptr++;
    scopesettings.channel2.adc2compensation = *ptr++;
    
    //Checks needed on the validity of the settings????
    if((scopesettings.movespeed != MOVE_SPEED_FAST) && (scopesettings.movespeed != MOVE_SPEED_SLOW))
    {
      //Default to fast
      scopesettings.movespeed = MOVE_SPEED_FAST;
    }
  }
  else
  {
    //Load a default set on failure
    scope_reset_config_data();
    
    //Save it to the SD card
    scope_save_configuration_data();
  }
}


//----------------------------------------------------------------------------------------------------------------------------------
