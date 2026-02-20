# Alif Zephyr port of Arm® Ethos™-U NPU Object Detection example

## Description of the Arm® example 
Object Detection is a classical computer vision use case in which specific objects need to be identified and located within a full frame. In this specific example the model was trained for face detection. The ML sample was developed using the YOLO Fastest model. To adopt the model for low power / low memory systems the input images to the model are monochrome images. The model was trained on the Wider dataset (after conversion from RGB to monochrome) and on Emza Visual-Sense dataset <www.emza-vs.com>. The model makes detection faces in size of 20x20 pixels and above.

## The port
The port is Zephyr version of the Alif CMSIS based ML examples in https://github.com/alifsemi/alif_ml-embedded-evaluation-kit. Which in turn is based on Arm upstream repository.

The example runs ObjectDetectionHandler() in a loop. They key steps are:
- Capturing and processing images. Most of the image pipeline processing is implemented with AIPL-module using Helium acceleration.
- Transfer captured image into LVGL buffer & draw using LVGL
- Running inference on the captured RGB888 images
- Drawing bounding box on top of captured image if faces are detected. Update detections label.
- Sending inference results to serial terminal

There is also separate thread which updates LVGL graphics.

## Supported hardware
Alif E7-DK HP & E8-DK HP & ARX3A0 serial camera & MW-405 display

## Building and running: E7-DK
```
west build -b alif_e7_dk/ae722f80f55d5xx/rtss_hp -S ethos-u55-enable   samples/modules/tflite-micro/alif_object_detection --   -DEXTRA_DTC_OVERLAY_FILE="boards/serial_camera_arx3a0.overlay"
```

## Building and running: E8-DK
```
west build -b alif_e8_dk/ae822fa0e5597xx0/rtss_hp -S ethos-u55-enable   samples/modules/tflite-micro/alif_object_detection --   -DEXTRA_DTC_OVERLAY_FILE="boards/serial_camera_arx3a0_selfie.overlay"
```

## Expected output
Camera image on display with bounding box drawn on top when faces detected. In serial:
```
*** Booting Zephyr OS build 890faf75531c ***
...
[00:00:01.537,000] <inf> MainLoop: model.Init done
[00:00:01.537,000] <inf> MainLoop: Entering main loop
[00:00:04.837,000] <inf> UseCaseHandler: 0) (0.769478) -> Detection box: {x=112,y=44,w=64,h=77}
[00:00:05.037,000] <inf> UseCaseHandler: 0) (0.598941) -> Detection box: {x=121,y=37,w=61,h=74}
[00:00:05.237,000] <inf> UseCaseHandler: 0) (0.566333) -> Detection box: {x=118,y=37,w=61,h=74}
[00:00:05.437,000] <inf> UseCaseHandler: 0) (0.630695) -> Detection box: {x=123,y=37,w=61,h=74}
[00:00:05.637,000] <inf> UseCaseHandler: 0) (0.566406) -> Detection box: {x=138,y=42,w=53,h=64}
...
```