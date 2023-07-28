#include <iostream>
#include <fstream>
#include <ctime>
#include <cstdlib>
#include <vector>
#include "PPMImage.h"
#include "Camera.h"
#include "Sphere.h"
#include "Box.h"
#include "PointLight.h"
#include "Matrix44f.h"
#include "Vector.h"
#include "Utility.h"

using namespace std;

vector<Object*>* objects = new vector<Object*>();
vector<Light*>* lights = new vector<Light*>();

int width = 400, height = 300;
int rgbMax = 255; // max scale for RGB, set to 255 for ease of use.

float plain_width = 5.0f;
float plain_length = 5.0f;
float plain_height = 0.5f;

Vector red = Vector(1.0f, 0.0f, 0.0f);
Vector blue = Vector(0.0f, 0.0f, 1.0f);
Vector green = Vector(0.0f, 1.0f, 0.0f);
Vector yellow = Vector(1.0f, 1.0f, 0.0f);
Vector white = Vector(1.0f, 1.0f, 1.0f);

void loadScenario1() {
    Sphere* sph = new Sphere(0.75f, Vector(0.0f, 0.0f, 0.0f));
    sph->setShadeType(kReflect);
    Sphere* small_sph_1 = new Sphere(0.5f, Vector(-2.0f, 0.0f, -4.0f)); // Mid-left, far
    Sphere* small_sph_2 = new Sphere(0.5f, Vector(-2.0f, 0.0f, -3.0f)); // Mid-left, middle
    Sphere* small_sph_3 = new Sphere(0.5f, Vector(-2.0f, 0.0f, -2.0f)); // Mid-left, close
    Sphere* small_sph_4 = new Sphere(0.5f, Vector(2.0f, 0.0f, -4.0f)); // Mid-right, far
    Sphere* small_sph_5 = new Sphere(0.5f, Vector(2.0f, 0.0f, -3.0f)); // Mid-right, middle
    Sphere* small_sph_6 = new Sphere(0.5f, Vector(2.0f, 0.0f, -2.0f)); // Mid-right, close


    Sphere* small_sph_7 = new Sphere(0.5f, Vector(-2.0f, 2.0f, -4.0f)); // Bottom-left, far
    Sphere* small_sph_8 = new Sphere(0.5f, Vector(-2.0f, 2.0f, -3.0f)); // Bottom-left, middle
    Sphere* small_sph_9 = new Sphere(0.5f, Vector(-2.0f, 2.0f, -2.0f)); // Bottom-left, close
    Sphere* small_sph_10 = new Sphere(0.5f, Vector(2.0f, 2.0f, -4.0f)); // Bottom-right, far
    Sphere* small_sph_11 = new Sphere(0.5f, Vector(2.0f, 2.0f, -3.0f)); // Bottom-right, middle
    Sphere* small_sph_12 = new Sphere(0.5f, Vector(2.0f, 2.0f, -2.0f)); // Bottom-right, close


    Sphere* small_sph_13 = new Sphere(0.5f, Vector(-2.0f, -2.0f, -4.0f)); // Top-left, far
    Sphere* small_sph_14 = new Sphere(0.5f, Vector(-2.0f, -2.0f, -3.0f)); // Top-left, middle
    Sphere* small_sph_15 = new Sphere(0.5f, Vector(-2.0f, -2.0f, -2.0f)); // Top-left, close
    Sphere* small_sph_16 = new Sphere(0.5f, Vector(2.0f, -2.0f, -4.0f)); // Top-right, far
    Sphere* small_sph_17 = new Sphere(0.5f, Vector(2.0f, -2.0f, -3.0f)); // Top-right, middle
    Sphere* small_sph_18 = new Sphere(0.5f, Vector(2.0f, -2.0f, -2.0f)); // Top-right, close


    Sphere* small_sph_19 = new Sphere(0.5f, Vector(0.0f, 2.0f, -4.0f)); // Top-mid, far
    Sphere* small_sph_20 = new Sphere(0.5f, Vector(0.0f, 2.0f, -3.0f)); // Top-mid, middle
    Sphere* small_sph_21 = new Sphere(0.5f, Vector(0.0f, 2.0f, -2.0f)); // Top-mid, close
    Sphere* small_sph_22 = new Sphere(0.5f, Vector(0.0f, -2.0f, -4.0f)); // Bottom-mid, far
    Sphere* small_sph_23 = new Sphere(0.5f, Vector(0.0f, -2.0f, -3.0f)); // Bottom-mid, middle
    Sphere* small_sph_24 = new Sphere(0.5f, Vector(0.0f, -2.0f, -2.0f)); // Bottom-mid, close


    Sphere* small_sph_25 = new Sphere(0.5f, Vector(-2.0f, 0.0f, -1.0f)); // Top-mid, close
    Sphere* small_sph_26 = new Sphere(0.5f, Vector(-2.0f, 0.0f, 0.0f)); // Top-mid, close

    Sphere* small_sph_27 = new Sphere(0.5f, Vector(2.0f, 0.0f, -1.0f)); // Top-mid, close
    Sphere* small_sph_28 = new Sphere(0.5f, Vector(2.0f, 0.0f, 0.0f)); // Top-mid, close
    
    Sphere* small_sph_29 = new Sphere(0.5f, Vector(0.0f, 2.0f, -1.0f)); // Top-mid, close
    Sphere* small_sph_30 = new Sphere(0.5f, Vector(0.0f, 2.0f, 0.0f)); // Top-mid, close
    
    Sphere* small_sph_31 = new Sphere(0.5f, Vector(0.0f, -2.0f, -1.0f)); // Top-mid, close
    Sphere* small_sph_32 = new Sphere(0.5f, Vector(0.0f, -2.0f, 0.0f)); // Top-mid, close

    //Box* plain = new Box(plain_width, plain_length, plain_height, Vector(0.0f, -3.0f, 0.0f));
    objects->push_back(sph);

    objects->push_back(small_sph_1);
    objects->push_back(small_sph_2);
    objects->push_back(small_sph_3);
    objects->push_back(small_sph_4);
    objects->push_back(small_sph_5);
    objects->push_back(small_sph_6);

    objects->push_back(small_sph_7);
    objects->push_back(small_sph_8);
    objects->push_back(small_sph_9);
    objects->push_back(small_sph_10);
    objects->push_back(small_sph_11);
    objects->push_back(small_sph_12);

    objects->push_back(small_sph_13);
    objects->push_back(small_sph_14);
    objects->push_back(small_sph_15);
    objects->push_back(small_sph_16);
    objects->push_back(small_sph_17);
    objects->push_back(small_sph_18);

    objects->push_back(small_sph_19);
    objects->push_back(small_sph_20);
    objects->push_back(small_sph_21);
    objects->push_back(small_sph_22);
    objects->push_back(small_sph_23);
    objects->push_back(small_sph_24);

    objects->push_back(small_sph_25);
    objects->push_back(small_sph_26);
    objects->push_back(small_sph_27);
    objects->push_back(small_sph_28);
    objects->push_back(small_sph_29);
    objects->push_back(small_sph_30);
    objects->push_back(small_sph_31);
    objects->push_back(small_sph_32);

    PointLight* pl = new PointLight(Vector(2.5f, 1.5f, 2.0f), red);
    pl->setIntensity(0.8f);
    PointLight* pl2 = new PointLight(Vector(-2.5f, 1.5f, 2.0f), blue);
    pl2->setIntensity(0.8f);
    PointLight* pl3 = new PointLight(Vector(2.5f, -1.5f, 2.0f), green);
    pl3->setIntensity(0.8f);
    PointLight* pl4 = new PointLight(Vector(-2.5f, -1.5f, 2.0f), yellow);
    pl4->setIntensity(0.8f);

    lights->push_back(pl);
    lights->push_back(pl2);
    lights->push_back(pl3);
    lights->push_back(pl4);
}

void testingRandomJunk() {

    Utility u;
    
    Matrix44f cameraToWorld(0.718762, 0.615033, -0.324214, 0, -0.393732, 0.744416, 0.539277, 0, 0.573024, -0.259959, 0.777216, 0, 0.526967, 1.254234, -2.53215, 1);
    Matrix44f worldToCamera = cameraToWorld.getInverse();
    Vector Pworld(-0.315792, 1.4489, -2.48901), Pcamera;
    Pcamera = worldToCamera.multVecMatrix(Pworld);
    std::cout << Pcamera.toString() << std::endl;

    Vector pRaster;
    float cw = 2.0f, ch = 2.0f;
    int iw = 512, ih = 512;
    if (u.computePixelCoordinates(Pworld, cameraToWorld, cw, ch, iw, ih, pRaster)) {
        std::cout << "visible\n";
    }
    else std::cout << "not visible \n";

    Matrix44f meh = Matrix44f();
    float fov = 90.0f;
    float near = 0.1f, far = 100.0f;
    u.setProjectionMatrix(fov, near, far, meh);
    std::cout << meh.toString() << std::endl;
}

/*int main()
{

    srand(time(0));

    PPMImage* img = new PPMImage(width, height);
    img->setImageType("P3");
    img->setColorScalar(rgbMax);
    
    // Outer loop does columns, inner does rows
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            img->setPixelColor(x, y, 0, 0, 0);
        }
    }

    img->writeImage("img.ppm");

    Camera* cam = new Camera();
    cam->assignImage(img);
    
    loadScenario1();

    cam->castRays(objects, lights);

    cam->getImage()->writeImage("img.ppm");

    //testingRandomJunk();
    
    img->~PPMImage();

    
    /*while (!objects->empty()) {
        Object* temp = objects->back();
        objects->pop_back();
        delete temp;
    }*/
    /*delete objects;
    delete lights;

    delete pl, pl2, pl3, pl4;
    delete sph;
    delete cam;
    delete img;*/

/*    std::cout << "Hello World!\n";
}*/