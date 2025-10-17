# Alif Zephyr port of Arm® Ethos™-U NPU Image Classification example

## Description of the Arm® example 
This use-case example solves the classical computer vision problem of image classification. The ML sample was developed using the MobileNet v2 model that was trained on the ImageNet dataset.

## The port
The port is Zephyr version of the Alif CMSIS based ML examples in https://github.com/alifsemi/alif_ml-embedded-evaluation-kit. Which in turn is based on Arm upstream repository.

The example runs ClassifyImageHandler() in a loop. They key steps are:
- Capturing and processing images. Most of the image pipeline processing is implemented with AIPL-module using Helium acceleration.
- Transfer captured image into LVGL buffer & draw using LVGL
- Running inference on the captured RGB888 images
- Update detections label with classified objects and their probability.
- Sending inference results to serial terminal

There is also separate thread which updates LVGL graphics.

## Supported hardware
Alif E7-DK HP & E8-DK HP & ARX3A0 serial camera & MW-405 display

## Building and running: E7-DK
```
west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ethos-u55-enable   samples/modules/tflite-micro/alif_img_class --   -DEXTRA_DTC_OVERLAY_FILE="boards/serial_camera_arx3a0.overlay"
```

## Building and running: E8-DK
```
west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable   samples/modules/tflite-micro/alif_img_class --   -DEXTRA_DTC_OVERLAY_FILE="boards/serial_camera_arx3a0_selfie.overlay"
```

## Expected output
Camera image on the screen and classifications with probability under the image. In serial:
```
...
[00:00:01.748,000] <inf> UseCaseHandler: Final results:
[00:00:01.748,000] <inf> UseCaseHandler: Total number of inferences: 1
[00:00:01.748,000] <inf> UseCaseHandler: 0) 620 (0.187500) -> lampshade, lamp shade
[00:00:01.748,000] <inf> UseCaseHandler: 1) 819 (0.082031) -> spotlight, spot
[00:00:01.748,000] <inf> UseCaseHandler: 2) 746 (0.074219) -> projector
[00:00:01.748,000] <inf> UseCaseHandler: 3) 795 (0.070312) -> shower curtain
[00:00:01.748,000] <inf> UseCaseHandler: 4) 847 (0.031250) -> table lamp
[00:00:01.948,000] <inf> UseCaseHandler: Final results:
[00:00:01.948,000] <inf> UseCaseHandler: Total number of inferences: 1
[00:00:01.948,000] <inf> UseCaseHandler: 0) 620 (0.160156) -> lampshade, lamp shade
[00:00:01.948,000] <inf> UseCaseHandler: 1) 746 (0.093750) -> projector
[00:00:01.948,000] <inf> UseCaseHandler: 2) 819 (0.078125) -> spotlight, spot
[00:00:01.948,000] <inf> UseCaseHandler: 3) 795 (0.066406) -> shower curtain
[00:00:01.948,000] <inf> UseCaseHandler: 4) 621 (0.035156) -> laptop, laptop computer
[00:00:02.148,000] <inf> UseCaseHandler: Final results:
[00:00:02.148,000] <inf> UseCaseHandler: Total number of inferences: 1
[00:00:02.148,000] <inf> UseCaseHandler: 0) 620 (0.187500) -> lampshade, lamp shade
[00:00:02.148,000] <inf> UseCaseHandler: 1) 746 (0.085937) -> projector
[00:00:02.148,000] <inf> UseCaseHandler: 2) 819 (0.078125) -> spotlight, spot
[00:00:02.148,000] <inf> UseCaseHandler: 3) 795 (0.031250) -> shower curtain
[00:00:02.148,000] <inf> UseCaseHandler: 4) 847 (0.027344) -> table lamp
[00:00:02.348,000] <inf> UseCaseHandler: Final results:
...
```