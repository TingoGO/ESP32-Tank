# ESP32-Tank
A tank made using laser cutting, ESP32 and APP Inventor
https://youtu.be/GkZCpofhbtY

# Current status
Currently, I do not recommend anyone without relevant experience to try this project, unless you have a deep passion for tanks. The current version of the tank is rev1_1. I haven't tested it yet (the model part), and I'm not sure if I will have the time to test it.
In the current version, due to the lack of consideration for model compatibility, only the following materials can be used:
1. Toothpicks with a diameter of less than 1.45 mm
2. Steel BBs with a diameter of 5.5 to 6 mm
3. Wooden boards with a thickness close to 1/2 the diameter of the steel BBs for laser cutting
And it will take a long time for the tank to function properly.

# Timeline for this project
1. Make sure rev1_1 work properly.
2. Create tutorial about this project for people without relevant experience.
3. Release future revision.
My completion time depends on the feedback from this project and whether I have time, right now, I am jobless and two months before broken.

# Future revision
Rev1_2: In this version, I will change the location of the ESP32-CAM from the side of the barrel to the front body of the tank. This adjustment means that when moving forward in the app, the streaming view from the ESP32-CAM will also move forward. Consequently, when rotating the barrel, only the crosshair will move instead of the entire scene.

Rev1_3: Change the cross-section of the barrel from rectangular to inverted triangular, so the barrel should better accommodate different sizes of steel BBs and improve accuracy. The original rectangular design often caused deviations in bullet trajectories. Changing to an inverted triangular shape should help stabilize the bullets along the grooves under the influence of gravity.

Rev1_4: Add depth sensor like "VL53L5CX" on the barrel, so you can see how far the target from the tank in th APP. But this might make this project too hard for most people.

.

.

.

Rev10_1:  Gattling tank, needless to say.

# License
CC BY-NC-SA 4.0

