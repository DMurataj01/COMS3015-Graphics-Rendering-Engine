#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

#ifndef VECTOR_H
  #define VECTOR_H
  #include <vector>
#endif

#include "Materials.h"

class Object {
  public:
    std::vector<ModelTriangle> faces; // stores the faces of the object
    bool hasBoundingBox; // true if a bounding box has been created for this object
    std::vector<ModelTriangle> boxFaces; // if a bounding box has been created, this stores the faces of it
    MATERIAL material;

    Object() {
      hasBoundingBox = false;
    }

    Object(std::vector<ModelTriangle> inputFaces) {
      faces = inputFaces;
      hasBoundingBox = false;
    }

    void Clear() {
      faces.clear();
      hasBoundingBox = false;
      boxFaces.clear();
    }

    void ApplyMaterial(MATERIAL mat) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).material = mat;
      }
      material = mat;
    }

    void ApplyColour(Colour colour, bool resetMaterial) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).colour = colour;
        if (resetMaterial) faces.at(i).material = NONE;
        material = NONE;
      }
    }

    glm::vec3 GetCentre() {
      glm::vec3 sum(0,0,0);
      for (int i=0; i< faces.size(); i++) {
        sum += ((faces.at(i).vertices[0] + faces.at(i).vertices[1] + faces.at(i).vertices[2])/(float)3);
      }
      return sum/(float) faces.size();
    }
};
