#include <iostream>
#include <fstream>
#include <cstring>
#include <math.h>
#include <GL/glut.h>
#include "teapot.h"

using namespace std;

int nRows = 640;
int nCols = 480; 

//----Phong Illumination Values-----
float lx[3];				//Result colour
float Lx[3] = {1,1,1};		//Light colour
float Ax[3] = {0,0.1,0.5};	//Ambient colour
float Dx[3] = {0,0.2,0.8};	//Diffuse colour (surface colour)
float Sx[3] = {1,1,1};		//Specular colour

float Ka = 0.2;				//Ambient coefficient (how much ambient light effects object)
float Kd = 0.6;				//Diffuse coefficient (how much source light effects object)
float Ks = 1.0;				//Specular coefficient (how bright speculation is)
float Att = 2.0;			//Attenuation coefficient (atmospheric attenuation - depth cueing)
float n = 5.0;				//Roughness

float N[3];					//Surface normal
float R[3];					//Reflection vector oposit to light vector
float L[3];					//Light vector (directed light source)
float V[3];					//View vector

float Vcoord[3] = {0,240.0,240.0};
float Lcoord[3] = {-1800.0,2400.0,-440.0};

float lr, lg, lb; 			//Current pixel colour
float NdotL;
float RdotV;

//----------------------------------

Vector3f av;
TriangleMesh trig;
float range;
float vector1[3], vector2[3];

//Hold all the vetices
float vertex[3648][3];

//Will hold the normal vectors
float triangleNormal[6323][3];
float vertexNormal[3648][3];

//Will hold the triangle vertice colours
float v1colour[3];
float v2colour[3];
float v3colour[3];

//----------------------------------------------------
//---------------------EXTRACT------------------------

void TriangleMesh::loadFile(char * filename)
{
	ifstream f(filename);

	//Check file exists
	if (f == NULL) {
		cerr << "failed reading polygon data file " << filename << endl;
		exit(1);
	}

	char buf[1024];
	char header[100];
	float x,y,z;
	float xmax,ymax,zmax,xmin,ymin,zmin;
	int v,t;
	int v1, v2, v3;

	xmax =-10000; ymax =-10000; zmax =-10000;
	xmin =10000; ymin =10000; zmin =10000;
	av[0] = av[1] = av[2] = 0.f;

	//While not end of file..
	v = 0;
	t = 0;
	
	while (!f.eof()) {
		    f.getline(buf, sizeof(buf));
		    sscanf(buf, "%s", header);  

			//-----------Store Vertices---------------
		    if (strcmp(header, "v") == 0) {
				sscanf(buf, "%s %f %f %f", header, &x, &y, &z);  
				//	x *= 1000; y *= 1000; z *= 1000;
				_v.push_back(Vector3f(x,y,z));
				
				vertex[v][0] = x;
				vertex[v][1] = y;
				vertex[v][2] = z;

				av[0] += x; av[1] += y; av[2] += z;

				if (x > xmax) xmax = x;
				if (y > ymax) ymax = y;
				if (z > zmax) zmax = z;

				if (x < xmin) xmin = x;
				if (y < ymin) ymin = y;
				if (z < zmin) zmin = z;
				
				v++;
		    }
		    //----------Store Triangles---------------
		    else if (strcmp(header, "f") == 0) {
		    	
				sscanf(buf, "%s %d %d %d", header, &v1, &v2, &v3);			
				Triangle trig(v1-1, v2-1, v3-1);
				_trig.push_back(trig);
				
				//Get the normal vector of the triangle
				vector1[0] = vertex[v2-1][0] - vertex[v1-1][0];
				vector1[1] = vertex[v2-1][1] - vertex[v1-1][1];
				vector1[2] = vertex[v2-1][2] - vertex[v1-1][2];
				
				vector2[0] = vertex[v3-1][0] - vertex[v1-1][0];
				vector2[1] = vertex[v3-1][1] - vertex[v1-1][1];
				vector2[2] = vertex[v3-1][2] - vertex[v1-1][2];
				
				triangleNormal[t][0] = vector1[1]*vector2[2] - vector1[2]*vector2[1];
				triangleNormal[t][1] = vector1[2]*vector2[0] - vector1[0]*vector2[2];
				triangleNormal[t][2] = vector1[0]*vector2[1] - vector1[1]*vector2[0];

				t++;
		    }
 	}

	_xmin = xmin; _ymin = ymin; _zmin = zmin;
	_xmax = xmax; _ymax = ymax; _zmax = zmax;
 
	if (xmax-xmin > ymax-ymin) range = xmax-xmin;
	else range = ymax-ymin;

	for (int j = 0; j < 3; j++) av[j] /= _v.size();

	for (int i = 0; i < _v.size(); i++) {
		for (int j = 0; j < 3; j++) {
			_v[i][j] = (_v[i][j]-av[j])/range*400;
			vertex[i][j] = (vertex[i][j]-av[j])/range*400;
		}
	}
	
	for (int i = 0; i < 6323; i++) {
		for (int j = 0; j < 3; j++) {
			triangleNormal[i][j] = triangleNormal[i][j] -av[j]/range*400;
		}
	}
	
	cout << "trig " << _trig.size() << " vertices " << _v.size() << endl;
	f.close();
};

//------------------VECTOR NORMALS--------------------

void setNormals()
{
	Vector3f v1,v2,v3;
	int triangleLocs[6139];
	int x = 0; int y = 0; int z = 0;
	int index;

	//For each vertex
	for (int i = 0; i < 3648; i++) {
		index = x = y = z = 0;

		//Check if triangle contains vertex
		for (int i2 = 0; i2 < 6323; i2++) {
			trig.getTriangleVertices(i2, v1,v2,v3);			
			
			if (vertex[i][0] == v1[0] && vertex[i][1] == v1[1] && vertex[i][2] == v1[2]) {
				//vertex in triangle
				triangleLocs[index] = i2;
				index++;
			}
			else if (vertex[i][0] == v2[0] && vertex[i][1] == v2[1] && vertex[i][2] == v2[2]) {
				//vertex in triangle
				triangleLocs[index] = i2;
				index++;
			}
			else if (vertex[i][0] == v3[0] && vertex[i][1] == v3[1] && vertex[i][2] == v3[2]) {
				//vertex in triangle
				triangleLocs[index] = i2;
				index++;
			}
		}
		
		//Calculate the normal and store
		for (int i2 = 0; i2 < index; i2++) {
			x += triangleNormal[triangleLocs[i2]][0];
			y += triangleNormal[triangleLocs[i2]][1];
			z += triangleNormal[triangleLocs[i2]][2];			
		}
		vertexNormal[i][0] = x/index;
		vertexNormal[i][1] = y/index;
		vertexNormal[i][2] = z/index;
	}
}

//----------------------------------------------------
//---------------PHONG ILLUMINATION-------------------

void getColour(float x, float y, float z, int vertex)
{
	int point[3];
	point[0] = x; point[1] = y; point[2] = z;	
	
	//Calculate the vectors of light and view to surface
	for (int i = 0; i < 3; i++) {
		V[i] = point[i] - Vcoord[i];
		L[i] = point[i] - Lcoord[i]; 
	}
	
	//-----------Normalise vectors--------------
	float llen = sqrt((L[0]*L[0]) + (L[1]*L[1]) + (L[2]*L[2]));
	float nlen = sqrt((N[0]*N[0]) + (N[1]*N[1]) + (N[2]*N[2]));
	
	for (int i = 0; i < 3; i++) {
		L[i] = L[i]/llen;
		N[i] = N[i]/nlen;
	}
	//-----------------------------------------		
	NdotL = N[0]*L[0] + N[1]*L[1] + N[2]*L[2];
	
	R[0] = 2*N[0]*NdotL - L[0];
	R[1] = 2*N[1]*NdotL - L[1];
	R[2] = 2*N[2]*NdotL - L[2];
	
	//-----------Normalise vectors--------------
	float vlen = sqrt((V[0]*V[0]) + (V[1]*V[1]) + (V[2]*V[2]));
	float rlen = sqrt((R[0]*R[0]) + (R[1]*R[1]) + (R[2]*R[2]));
	
	for (int i = 0; i < 3; i++) {
		V[i] = V[i]/vlen;
		R[i] = R[i]/rlen;
	}
	//-----------------------------------------	
	RdotV = R[0]*V[0] + R[1]*V[1] + R[2]*V[2];
	//printf("RdotV %f\n", RdotV);	
	
	//Phong Illuminate	
	lr = (Ax[0]*Ka*Dx[0]) + Att*Lx[0] * ((Kd*Dx[0]*NdotL) + (Ks*Sx[0]*pow(RdotV,n)));
	lg = (Ax[1]*Ka*Dx[1]) + Att*Lx[1] * ((Kd*Dx[1]*NdotL) + (Ks*Sx[1]*pow(RdotV,n)));
	lb = (Ax[2]*Ka*Dx[2]) + Att*Lx[2] * ((Kd*Dx[2]*NdotL) + (Ks*Sx[2]*pow(RdotV,n)));
	
	//Ensure boundaries
	if (lr < 0) lr = 0;
	else if (lr > 1) lr = 1;	
	if (lg < 0) lg = 0;
	else if (lg > 1) lg = 1;		
	if (lb < 0) lb = 0;
	else if (lb > 1) lb = 1;
	
	//Save colour if for vetrex
	if (vertex == 1) { v1colour[0] = lr; v1colour[1] = lg; v1colour[2] = lb; }
	else if (vertex == 2) { v2colour[0] = lr; v2colour[1] = lg; v2colour[2] = lb; }
	else if (vertex == 3) { v3colour[0] = lr; v3colour[1] = lg; v3colour[2] = lb; }
	
	//printf("Colour: (R = %f, G = %f, B = %f)\n",lr,lg,lb);
	
	glColor3f(lr, lg, lb);
}

//----------------------------------------------------
//--------------------DDA-----------------------------

void DDAline(int x1, int y1, int x2, int y2)
{
	float x, y;
	int dx = x2-x1, dy = y2-y1;
	int n = max(abs(dx), abs(dy));
	float dt = n, dxdt = dx/dt, dydt = dy/dt;
	
	x = x1;
	y = y1;

	while(n--) {
		glVertex2i(round(x), round(y));
		x += dxdt;
		y += dydt;
	}
}
//----------------------------------------------------

void myDisplay()
{
	float xmin, ymin, xmax, ymax;
	float px, py, pz; //Current pixel

	glClear(GL_COLOR_BUFFER_BIT); // Clear OpenGL Window

	int trignum = trig.trigNum();
	Vector3f v1,v2, v3;

	//Find the vetex normals
	setNormals();

	// for all the triangles, get the location of the vertices,
	// project them on the xy plane, fill with colour

	for (int i = 0 ; i < trignum; i++)  
	{
			
		trig.getTriangleVertices(i, v1,v2,v3); //getting the vertices of the triangle i
		N[0] = triangleNormal[i][0];
		N[1] = triangleNormal[i][1];
		N[2] = triangleNormal[i][2];
 
		//Begin colour process
		glBegin(GL_POINTS);	
		
			//Colour the vertices of the triangle
			getColour(v1[0],v1[1],v1[2],1);
			glVertex2i((int)v1[0],(int)v1[1]);
			
			getColour(v2[0],v2[1],v2[2],2);
			glVertex2i((int)v2[0],(int)v2[1]);
			
			getColour(v3[0],v3[1],v3[2],3);
			glVertex2i((int)v3[0],(int)v3[1]);
		
			//Draw line pixels using DDA
			//DDAline(v1[0], v1[1], v2[0], v2[1], 1, 2);
			//DDAline(v2[0], v2[1], v3[0], v3[1], 2, 3);
			//DDAline(v1[0], v1[1], v3[0], v3[1], 1, 3);

			
			//Find the boundaries of the triangle plane
			xmin = v1[0]; xmax = v1[0]; ymin = v1[1]; ymax = v1[1];
			
			if (v2[0] < xmin) xmin = v2[0];
			else if (v2[0] > xmax) xmax = v2[0];
			if (v3[0] < xmin) xmin = v3[0];
			else if (v3[0] > xmax) xmax = v3[0];
			
			if (v2[1] < ymin) ymin = v2[1];
			else if (v2[1] > ymax) ymax = v2[1];
			if (v3[1] < ymin) ymin = v3[1];
			else if (v3[1] > ymax) ymax = v3[1];
			
			//Fill the triangles with ambient colour using scan line
			for (int i2 = ymax; i2 >= ymin; i2--) {		//i2 = current y
				for (int i3 = xmin; i3 < xmax; i3++) {	//i3 = current x
				
					//Use Baricentric to calculate pixels to colour
					px = (((v2[1]-v3[1])*i3) + ((v3[0]-v2[0])*i2) + (v2[0]*v3[1]) - (v3[0]*v2[1]))
						/ (((v2[1]-v3[1])*v1[0]) + ((v3[0]-v2[0])*v1[1]) + (v2[0]*v3[1]) - (v3[0]*v2[1]));
					py = (((v3[1]-v1[1])*i3) + ((v1[0]-v3[0])*i2) + (v3[0]*v1[1]) - (v1[0]*v3[1]))
						/ (((v3[1]-v1[1])*v2[0]) + ((v1[0]-v3[0])*v2[1]) + (v3[0]*v1[1]) - (v1[0]*v3[1]));
					pz = (((v1[1]-v2[1])*i3) + ((v2[0]-v1[0])*i2) + (v1[0]*v2[1]) - (v2[0]*v1[1]))
						/ (((v1[1]-v2[1])*v3[0]) + ((v2[0]-v1[0])*v3[1]) + (v1[0]*v2[1]) - (v2[0]*v1[1]));
					
					//If in triangle, colour
					if ((px < 1 && px > 0) && (py < 1 && py > 0) && (pz < 1 && pz > 0)) {
						
						//Use Gouraud shading to colour
						float cx = px*v1colour[0] + py*v2colour[0] + pz*v3colour[0];
						float cy = px*v1colour[1] + py*v2colour[1] + pz*v3colour[1];
						float cz = px*v1colour[2] + py*v2colour[2] + pz*v3colour[2];
						
						glColor3f(cx,cy,cz);
						glVertex2i(round(i3),round(i2));
					}
				}
			}		
		glEnd();		
	}

	glFlush();// Output everything
}	

int main(int argc, char **argv)
{
	if (argc >  1)  {
		trig.loadFile(argv[1]);
	}
	else {
		cerr << argv[0] << " <filename> " << endl;
		exit(1);
	}

	int width, height;
	glutInit(&argc, argv);
	glutInitWindowSize(nRows, nCols);
	glutCreateWindow("SimpleExample");
	gluOrtho2D(-nRows/2, nRows/2, -(float)nCols/2,  (float)nCols/2);
	glutDisplayFunc(myDisplay);// Callback function
	glutMainLoop();// Display everything and wait
}
