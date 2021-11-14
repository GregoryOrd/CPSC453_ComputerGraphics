README for Gregory Ord's CPSC 453 Assignment #3

This submission was built on Windows 10 in an environment using Microsoft Visual Studio 2019's CMake tools.
In this environment, the cmake generation identified the C and CXX compilers to be:

MSVC version 14.25.28610

To build this program in Visual Studio with Desktop C++ Development tools installed, you should just need
to use the "Open Folder" option in Visual Studio to open the folder with the CMakeLists.txt file. Visual Studio
should automatically start the CMake generation since the Desktop C++ Development tools are installed. Once 
the CMake generation is completed, select "453-skeleton.exe" as the "Startup Item", then click on the green arrow 
beside the "Startup Item" dropdown to compile and run the program.

I was also just using the x64-Debug configuration, which appeared as the default for my machine.

This submission was built as a modification from the provided 453-skeleton boilerplate code. To make this clear,
I left the source code folder and executable name as 453-skeleton.

Keyboard and mouse controls for the four scenes are as specified in the assignment with a few additional
controls. 


Switching between scenes:
-----------------------------
Press the "G" key to switch to to the 2D curve editor scene.
Press the "H" key to switch to to the 3D curve viewer scene.
Press the "J" key to switch to to the surface of revolution viewer scene.
Press the "G" key to switch to to the tensor product surfave viewer scene.
Note that switching between scenes will always reset the camera to its intial orientation.


2D Curve Editor Controls:
-----------------------------
Left click to add/select points.
Left click and drag to drag points.
Press the "D" key to delete the selected point (shown in blue).
Press the "R" key to delete all of the control points and reset the scene.
Press the UP arrow key to toggle between a bezier and b-spline curve.
Press the DOWN arrow key to toggle showing the control polygon (shown in green).
Press the RIGHT arrow key to toggle showing the control points (shown in red).


3D Curve Viewer Controls:
----------------------------
Note: a grid has been added to show the zx-plane and help with the 3D visualization.
Press the "F" key to toggle showing the zx-plane grid.
Press the "W" key to move the camera forward in the direction it is facing.
Press the "S" key to move the camera backward from the direction it is facing.
Press the "A" key to translate the camera to the left (orthogonal to the camera's up vector).
Press the "D" key to translate the camera to the right (orthogonal to the camera's up vector).
Press the UP arrow key to translate the camera up (in the direction of it's up vector).
Press the DOWN arrow key to translate the camera down (in the direction of it's up vector).
Left click and drag to rotate the camera and change the direction it is facing.


Surface of Revolution Viewer:
-------------------------------
Same 3D movement controls as the 3D Curve Viewer Controls.
Press the "T" key to toggle between wireframe and solid rendering.


Tensor Product Surface Viewer:
-------------------------------
Same 3D movement controls as the 3D Curve Viewer Controls.
Press the "T" key to toggle between wireframe and solid rendering.

