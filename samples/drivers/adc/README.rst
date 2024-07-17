.. zephyr:code-sample:: adc12
	name: Analog-to-digital converter (ADC12)

	Read analog input from ADC12 channels
###########

Overview
********
This sample app demonstrates the usage of the Analog-to-Digital Conversion (ADC)
driver.It displays the temperature based on the board’s temperature, which is
converted into an analog signal and fed as input to ADC channel 6.

Note: The temperature will only be displayed if the digital output is in 12-bit format

Building and Running
********************

The application will build only for a target that has a devicetree entry with
:dt compatible:`alif,adc` as a compatible.

console Output
=============
	ALIF_ADC: Current temp 25.2 C
