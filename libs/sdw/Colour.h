#include <iostream>

class Colour
{
  public:
    std::string name;
    int red;
    int green;
    int blue;

    Colour()
    {
    }

    Colour(uint32_t packedColour)
    {
      uint8_t r = packedColour >> 16;
      uint8_t g = packedColour >> 8;
      uint8_t b = packedColour;
      red = r;
      green = g;
      blue = b;
    }

    Colour(int r, int g, int b) {
      name = "";
      if (r>255) red = 255;
      else if (r<0) red = 0;
      else red = r;
      if (g>255) green = 255;
      else if (g<0) green = 0;
      else green = g;
      if (b>255) blue = 255;
      else if (b<0) blue = 0;
      else blue = b;
    }

    Colour(std::string n, int r, int g, int b) {
      name = n;
      if (r>255) red = 255;
      else if (r<0) red = 0;
      else red = r;
      if (g>255) green = 255;
      else if (g<0) green = 0;
      else green = g;
      if (b>255) blue = 255;
      else if (b<0) blue = 0;
      else blue = b;
    }

    void SetRed(int r) {
      if (r>255) red = 255;
      else if (r<0) red = 0;
      else red = r;
    }
    void SetGreen(int g) {
      if (g>255) green = 255;
      else if (g<0) green = 0;
      else green = g;
    }
    void SetBlue(int b) {
      if (b>255) blue = 255;
      else if (b<0) blue = 0;
      else blue = b;
    }

    uint32_t toUINT32_t() {
      return (255<<24) + (red<<16) + (green<<8) + blue; 
    }
    uint32_t toUINT32_t(float percentage) {
      return (255<<24) + (int(red*percentage)<<16) + (int(green*percentage)<<8) + (int(blue*percentage));
    }
    
};

std::ostream& operator<<(std::ostream& os, const Colour& colour)
{
    os << colour.name << " [" << colour.red << ", " << colour.green << ", " << colour.blue << "]" << std::endl;
    return os;
}
