#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

#ifndef VECTOR_H
  #define VECTOR_H
  #include <vector>
#endif

class Object {
  public:
    std::vector<ModelTriangle> faces; // stores the faces of the object
    bool hasBoundingBox; // true if a bounding box has been created for this object
    std::vector<ModelTriangle> boxFaces; // if a bounding box has been created, this stores the faces of it

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
    void ApplyMaterial(std::string texture) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).texture = texture;
      }
    }
    void ApplyColour(Colour colour, bool resetTexture) {
      for(int i= 0; i< faces.size(); i++) {
        faces.at(i).colour = colour;
        if (resetTexture) faces.at(i).texture = "";
      }
    }
};
