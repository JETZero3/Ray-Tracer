# Ray Tracer

##### About
This little program was made for a university project, and was used to demonstrate Phong illumination and ray tracing in action. The program asks for an input to name the output file, and asks if you want a 0% reflective plane or a 50% reflective plane. The program then outputs a .bmp file containing the scene, with objects defined in the main file. 

The results were cool in that the scenes created were quite surreal and cool to look at. One could also change how the scene looked, providing myself with a little creative outlet. 

##### Dependencies
The ray tracer works with Visual Studio in Windows. In Linux, make is used in Terminal to compile the program. 

The ray tracer requires some libraries to run properly. 
- OpenGL Mathematics (GLM)
- OpenGL Utility Toolkit (GLUT)
- OpenGL Extension Wrangler Toolkit (GLEW)
- GLFW

In Visual Studio, you'll want to add these libraries by: 
- Right-clicking the project name, and clicking "Properties".
- Under C/C++ -> General, go to "Additional Include Properties"
- Add these: (of course, if these folders aren't in the C: drive, replace the locations accordingly.)
	- C:\glfw-3.1.bin.WIN32\include
	- C:\glew-1.12.0\include
	- C:\glut-3.7.6-bin

##### Code Reuse:
	- Plane intersection: https://www.cs.princeton.edu/courses/archive/fall00/cs426/lectures/raycast/sld017.htm
	- Triangle intersection: https://courses.cs.washington.edu/courses/csep557/10au/lectures/triangle_intersection.pdf
	
##### Data Structures:
	- Scene objects used: Sphere, Plane, Triangle (all subclasses of the SceneObject class)
	- Lights used
	- Ray data structure used (to simplify code)

##### Algorithm:
	1. Set up (create objects, position camera, place pixel screen)
	2. Fire a ray for each pixel, and check for intersection with objects
	3. If intersection occurs, trace the ray through reflections
	4. Obtain correct colour based on the traced ray
	5. Colour each pixel
	6. Create the image
	
##### Image Descriptions:
	- 1.bmp:
		- Contains the eight primitives required
		- Has the 100% opaque floor
		- Shows shadows (without attenuation)
		- Has the illumination requirements
		- Shows the required spheres and triangles for reflections
	- 2.bmp:
		- Contains the eight primitives required
		- Has the 50% reflective floor
		- Shows shadows (without attenuation)
		- Has the illumination requirements
		- Shows the required spheres and triangles for reflections
	- 3.bmp:
		- Same parameters as 1.bmp, except the camera position is 1 unit distance higher. 