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
      else red = r;
      if (g>255) green = 255;
      else green = g;
      if (b>255) blue = 255;
      else blue = b;
    }

    Colour(std::string n, int r, int g, int b)
    {
      name = n;
      if (r>255) red = 255;
      else red = r;
      if (g>255) green = 255;
      else green = g;
      if (b>255) blue = 255;
      else blue = b;
    }

    void SetRed(int r) {
      if (r>255) red = 255;
      else red = r;
    }
    void SetGreen(int g) {
      if (g>255) green = 255;
      else green = g;
    }
    void SetBlue(int b) {
      if (b>255) blue = 255;
      else blue = b;
    }

    uint32_t toUINT32_t() {
      uint32_t col = (255<<24) + (red<<16) + (green<<8) + blue; 
      return col; 
    }
    
};

std::ostream& operator<<(std::ostream& os, const Colour& colour)
{
    os << colour.name << " [" << colour.red << ", " << colour.green << ", " << colour.blue << "]" << std::endl;
    return os;
}
