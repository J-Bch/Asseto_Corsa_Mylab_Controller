# MIPS Asseto Corsa controller & dashboard

## Project description

This set of controllers is used to control a car in Asseto Corsa. One card is used as a steering wheel and the other one as a dashboard communicating with the computer, sending inputs to a python script on the host computer.

This project was carried out as students at HEPIA as part of the "MIPS" course.


Made by Bach Joachim & Da Silva Marques David.

### Architecture 

![architecture drawio](https://github.com/user-attachments/assets/c04697f2-c1f2-45e6-b04f-f88b6537fd67)




## Demo Video

https://github.com/user-attachments/assets/ae3563fd-089c-4aa5-98f8-40012db98897




## Controllers

![both_flat](https://github.com/user-attachments/assets/d53b588e-3de5-4c34-bdfd-3197c1a288be)

With the left controller, you can steer the car, accelerate, brake, see the speed trough the screen & see the RPM indicated by the leds.
With the right controller, you can see the accelerating and braking percentage, the lap time, increase and decrease Traction Control and ABS.




## Game install

In order for this project to work, you need to install Asseto Corsa on steam. This needs to have a compatibility layer added BEFORE THE GAME because this is a Windows native game.

### Proton GE compatibility install 

Here is the link to the github page : https://github.com/GloriousEggroll/proton-ge-custom

You can follow the manual installation instructions by just copy-pasting the "Terminal example based on Latest Release".

### Configure steam

In order to allow non-Windows game to use proton GE, you must enable compatibility in the steam setting and set them as : 

![alt text](Readme_Images/Steam_Config.png)

You can select proton GE here or set it to run only Asseto Corsa : (Right click on the game in the steam library -> properties)

![alt text](Readme_Images/Asseto_Corsa_config.png)


### Configure the game's controls 

In order for the game to recognize the controls sent by our Mylabs, we need to copy the `controller_config/MIPS_Custom.ini` file from this git into this folder (example from my computer) : `~/.steam/debian-installation/steamapps/compatdata/244210/pfx/drive_c/users/steamuser/Documents/Assetto Corsa/cfg/controllers/savedsetups`


In the game's settings (after a relaunch of the game), you can now load the MIPS_Custom controller preset : 

![alt text](Readme_Images/Asseto_Control_Config.png)



## Launch the telemetry tool

To display the telemetry onto the dashboard and to be able to send inputs from it, you must run the following python3 script : `scripts/asseto.py`.

You may need to install the following modules in your python environnment: uinput, pyserial

## Play the game

### Choose the right car

This tool has been developed for only one car : Lotus Exos 125 Stage 1.

In the menu, before launching the game, select the Lotus Exos 125 : 

![alt text](Readme_Images/Lotus_Exos.png)

Then mouse over it and click on the wrench in the bottom right, then click on it and choose the S1 variant : 

![alt text](Readme_Images/Lotus_Exos_S1.png)
