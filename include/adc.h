#ifndef _ADC_H_
#define _ADC_H_

int adc_setup(void); // Set up ADC drivers and inputs
int32_t adc_probe(void); // Read Moisture Probe value, returns mv
uint8_t adc_battery(void); // Read Battery voltage, returns percentage

#endif
