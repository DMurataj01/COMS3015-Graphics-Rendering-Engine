#ifndef PPM_H
#define PPM_H

#ifndef SSTREAM_H
#define SSTREAM_H
#include <sstream> 
#endif

#ifndef DRAWINGWINDOW_H
  #define DRAWINGWINDOW_H
  #include <DrawingWindow.h>
#endif

/* STRUCTURE - ImageFile */
struct ImageFile {
  std::vector<Colour> vecPixelList;
  int width;
  int height;  
};

/* ........... */
/* ........... */
/* PPM SECTION */
/* ........... */
/* ........... */

std::string removeLeadingWhitespace(std::string s){
  s.erase(0, s.find_first_not_of(" "));
  return s;
}

ImageFile importPPM(std::string fileName) {
  std::ifstream ifs;
  ifs.open (fileName, std::ifstream::in);

  /* Parse Header */

  //Check if header is a P6 file.
  std::string headerInput = "";
  getline(ifs, headerInput);

  if (headerInput != "P6") {
    //cout << "Error - Header file is invalid";
    ifs.close();
    throw 1;
  }

  int width = -1;
  int height = -1;
  int maxvalue = -1;

  /* Following Specification: http://netpbm.sourceforge.net/doc/ppm.html */ 
  // 1) Check if header is a P6 file.
  // 2) Ignore Comments.
  // 3) Parse Width + whitespace + Height.
  // 4) Parse Max value

  while (true || !ifs.eof()) {
    std::string inputLine = "";
    getline(ifs,inputLine);
    inputLine = removeLeadingWhitespace(inputLine);
    if (inputLine[0] == '#'){
      //This is a comment line -> ignore them.
    }
    else {
      //Parse Width + Height.
      std::stringstream ss_wh(inputLine);
      ss_wh >> width >> height;
      
      //Read new line -> Parse Max value: // 0<val<65536.
      getline(ifs,inputLine);
      inputLine = removeLeadingWhitespace(inputLine);
      
      std::stringstream ss_mv(inputLine);
      ss_mv >> maxvalue;
      break;
    }
  }
  
  /* Body RGB Parse */

  std::vector<Colour> vecPixelList; //RGB storage.

  while (ifs.peek()!= EOF) {
    vecPixelList.push_back(Colour ({ifs.get(), ifs.get(), ifs.get()})); //Create a Pixel element with three consecutive byte reads.
  }
  //cout << "\nBody Parse -- Number of elements: " << vecPixelList.size() << "\n";
  ifs.close();
  ImageFile outputImageFile = ImageFile ({vecPixelList, width, height});
  return outputImageFile;
}

Colour getPixelColour(ImageFile *imageFile, int x, int y) {
  int index = (imageFile->width * y) + x;

  try {
    return imageFile->vecPixelList.at(index);      // vector::at throws an out-of-range
  }
  catch (const std::out_of_range& oor) {
    //std::cerr << "Texture File Out Of Range error: [ " << x << ", "<< y << "]\n";
    return Colour(255,255, 255);
  }
}

ImageFile CreateImageFileFromWindow(DrawingWindow window, int width, int height) {
  vector<Colour> vecPixelList;

  for (int jj=0; jj<height; jj++) {
    for (int ii=0; ii<width; ii++) {
      uint32_t packedColour = window.getPixelColour(ii, jj);
      Colour colour(packedColour);
      vecPixelList.push_back(colour);
    }
  }

  ImageFile imageFileOut = ImageFile({vecPixelList, width, height});
  return imageFileOut;
}

bool exportToPPM(std::string fileName, ImageFile imageFile) {
    std::ofstream outfile (fileName,std::ofstream::binary);
    
    /* Following Specification: http://netpbm.sourceforge.net/doc/ppm.html */ 

    // 1) Write "P6" Header file.
    outfile.write("P6\n", 3);

    // 2) Ignore Comments.
    outfile.write("#Comment Ignore.\n", 17);

    // 3) Write Width + whitespace + Height.
    std::string width  = std::to_string(imageFile.width);
    std::string height = std::to_string(imageFile.height);
    outfile.write(width.c_str(), width.length());
    outfile.write(" ", 1);
    outfile.write(height.c_str(), height.length());
    outfile.write("\n", 1);

    // 4) Write Max value
    outfile.write("255\n", 4);

    // 5) Write Tuples.
    for (int i = 0 ; i < imageFile.vecPixelList.size() ; i++) {
        Colour temp_clr = imageFile.vecPixelList[i];
        char buffer[3];
        buffer[0] = temp_clr.red;
        buffer[1] = temp_clr.green;
        buffer[2] = temp_clr.blue;
        outfile.write(buffer, 3);
    }

    // 6) Close File.
    outfile.close();

    // 7) Check for errors.

    return true;
}

#endif