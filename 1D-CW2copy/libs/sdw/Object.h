#ifndef MODELTRIANGLE_H
  #define MODELTRIANGLE_H
  #include <ModelTriangle.h>
#endif

class Object
{
  public:
    std::vector<ModelTriangle> faces; // stores the faces of the object
    bool hasBoundingBox; // true if a bounding box has been created for this object
    std::vector<ModelTriangle> boxFaces; // if a bounding box has bee created, this stores the faces of it

    Object()
    {
      hasBoundingBox = false;
    }
};
