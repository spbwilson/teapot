Sean Wilson
s0831408

FILES INCLUDED IN FOLDER TEAPOT:

- README.txt 	= Simple file explaining code and how to run and compile
- Example.png 	= An example screenshot of the program running
- teapot.cc 	= The main teapot program file
- teapot.h 		= The teapot header file
- MIT_teapot.obj = The teapot coordinates
- teapot 		= A compiled version of the teapot program

//-------------------------------------------------------------------------------------------------
ABOUT TEAPOT.CC:

1)--------- FLOORING --------------
A floor has been added. The floor consists of two large triangles added to the end of the MIT_teapot.obj
Four extra vertices were also needed and added to the MIT_teapot.obj file



2)------- VECTOR NORMALS ----------
Each triangle has a normal vector calculated by taking the dot product of two of the edges. 
This is done as each triangle is read from the MIT_teapot.obj file. 
The results are stored in a two dimensional array.

The normal vector of a vertex is then calculated as the average of all normal vectors from all the 
triangles joined by that vertex. This is done in a separate function - 'setNormals'. For each vertex, the program 
loops through every triangle on record to check for connections to the current vertex. The results 
are stored in a two dimensional array.

Problems:
One problem over come when calculating the vectors of each vertex was forgetting to normalise the
vectors. This caused massive colour values and incorrect shading later on.
This problem was over come by using the algorithm below:

|a| = sqrt((ax * ax) + (ay * ay) + (az * az))
x = ax/|a|
y = ay/|a|
z = az/|a|

References:
[1] Vertex normals - msdn.microsoft.com/en-us/library/windows/desktop/bb324491(v=vs.85).aspx
[2] Normalising a vector -http://www.fundza.com/vectors/normalize/index.html



3)------- Phong Illumination --------
Lighting at each vertex is calculated using Phong Illumination Model. This combines ambient, diffuse
and specular lighting. There are coefficient variables for all of these which can be altered in order
to change the lighting of the surface of the teapot.
There are two coordinates that represent the location of the point light source and the viewing angle.

There are also variables that can change the roughness of the surface and light attenuation.

References:
[1] Illumination Algorithm - www.gamedev.net/page/resources/_/technical/graphics-programming-and-theory/phong-illumination-explained-r667
[2] Illumination - "Computer Graphics Principles and Practice, 2nd edition" By Foley, van Dam, Feiner, Hughes



4)------ Gouraud Shading ------------
For each triangle a bounding box is calculated, using the min and max of the vertices.
Each triangle is filled using a line fill alogorithm and Barycentric coordinates to check if a pixel
is within the triangle. The Barycentric coordinates are calculated using the three triangle vertices.
If the pixel lies within the triangle, the colour is then calculated using Gouraud Shading algorithm.
This uses the Barycentric coordinates calculated from before and the light values at each vertex to
interpolate the colour value of the current pixel:

colourValue = alpha * vertexOneValue + beta * vertexTwoValue + gamma * vertexThreeValue

Problems:
An example of the main disadvantage of using Gouraud shading over Phong Shading can be seen on the
flooring of the example image. As a speculation hits one vertex of a large triangle more than that
of the other floor triangle, the light intensity is very noticable between the two. This can be
improved by either increasing the amount of triangles used to represent the floor, or using Phong
Shading. Phong shading interploates a normal vector per pixel which increases the colour accuracy,
but is much more computationally expensive than Gouraud shading.

References:
[1] Colour interpolation - http://www.ugrad.cs.ubc.ca/~cs314/Vsep2008/notes-sept9.pdf

//-------------------------------------------------------------------------------------------------
COMPILING AND RUNNING TEAPOT:

Program can be compiled from the command line with command:

	g++ -o teapot teapot.cc -lglut -lGLU -lGL

Program can be run from command line in the correct directory with the command:

	./teapot MIT_teapot.obj

