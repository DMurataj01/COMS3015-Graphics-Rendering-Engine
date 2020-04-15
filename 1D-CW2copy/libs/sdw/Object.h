//#include <ModelTriangle.h>

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
