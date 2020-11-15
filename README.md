# STM32-based gesture recognition vision test table

## Introduction

Aiming at the current situation that vision detection is not automatic enough, a vision detection instrument based on gesture recognition is designed. Use AMG8833 thermal imaging module to obtain user gestures, deploy BP neural network on STM32H743 to perform inference to obtain gesture recognition results, send them to the display module via Bluetooth, perform character display and vision calculation, and have a voice broadcast function.

## Folder Structure

+ Gesture_Rec
  + AMG8833convert: PCB.doc which used to linking the AMG8833 to the board. + 
  + FreeRtosAndAI_20201018: The neural network C code generated with the STM32CubeAI tool.
  + PCModel: The trained neural network model saved as .h5 and keras source code, along with the train set and test set.
+ Visual_Chart
  + final_edition: Src codes used on the display mode.

## Environment for this project

+ STM32Cube IDE
+ STM32Cube MX
+ Altium Designer 20
+ PC with Python3.7, tensorflow and keras should be installed.
+ MDK 5.0
+ STM32H743 Nucleo-144



