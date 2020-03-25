#include "OBJStream.cpp"
#include "TriangleDrawing.cpp"
#include <assert.h>

using namespace std;
using namespace glm;

#define WIDTH 600
#define HEIGHT 600

/*Pre-defined Functions*/
void update();
void handleEvent(SDL_Event event);

/* Custom Functions */
void drawAALine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr);
void drawStrokedTriangle(CanvasTriangle triangle);
void drawTexturedTriangle(ImageFile imageFile, CanvasTriangle triangle);


DrawingWindow window = DrawingWindow(WIDTH, HEIGHT, false);

int main(int argc, char* argv[]) {
  SDL_Event event;
  window.clearPixels();

  const ImageFile textureImageFile = readPPMImage("texture.ppm");
  
  //** Draw a stroked triangle.
  CanvasTriangle tri_out (CanvasPoint(160, 10), CanvasPoint(300, 230), CanvasPoint(10, 150), Colour(0, 255, 255)); 
  tri_out.vertices[0].texturePoint = TexturePoint(195,5);
  tri_out.vertices[1].texturePoint = TexturePoint(395, 380);
  tri_out.vertices[2].texturePoint = TexturePoint(65, 330);

  drawTexturedTriangle(textureImageFile, tri_out);
  drawStrokedTriangle(tri_out);

  while(true) {
    // We MUST poll for events - otherwise the window will freeze !
    if(window.pollForInputEvents(&event)) handleEvent(event);
    update();
    // Need to render the frame at the end, or nothing actually gets shown on the screen !
    window.renderFrame();
  }
}

void drawAALine(CanvasPoint ptStart, CanvasPoint ptEnd, Colour ptClr) {
  float diffX = (ptEnd.x - ptStart.x);
  float diffY = (ptEnd.y - ptStart.y);
  
  int x1 = ptStart.x;
  int x2 = ptEnd.x;
  int y1 = ptStart.y;
  int y2 = ptEnd.y;

  if (abs(diffX) > abs(diffY)) {
    // horizontal line.
    if(x2 < x1) { swap(x1, x2); swap(y1, y2); }

    float gradient = diffY / diffX;

    float xend = float(round(x1));
    float yend = y1 + gradient * (xend - x1);
    float xgap = rfpart(x1 + 0.5f);
    
    int xpxl1 = int(xend);
    int ypxl1 = floor(yend);
    
    // Add the first endpoint.
    window.setPixelColour(xpxl1, ypxl1, getColour(ptClr, rfpart(yend) * xgap));
    window.setPixelColour(xpxl1, ypxl1 + 1, getColour(ptClr, fpart(yend) * xgap));
    
    float intery = yend + gradient;

    xend = float(round(x2));
    yend = y2 + gradient * (xend - x2);
    xgap = fpart(x2 + 0.5f);
    
    int xpxl2 = int(xend);
    int ypxl2 = floor(yend);
    
    // Add the second endpoint
    window.setPixelColour(xpxl2, ypxl2, getColour(ptClr, rfpart(yend) * xgap));
    window.setPixelColour(xpxl2, ypxl2 + 1, getColour(ptClr, fpart(yend) * xgap));
    
    // Add all the points between the endpoints
    for(int x = xpxl1 + 1; x <= xpxl2 - 1; ++x){
      int i_intery = floor(intery);
      
      window.setPixelColour(x, i_intery, getColour(ptClr, rfpart(intery)));
      window.setPixelColour(x, i_intery + 1, getColour(ptClr, fpart(intery)));

      intery += gradient;
    }

  }   
  else {
    if(y2 < y1) { swap(x1, x2); swap(y1, y2); }

    float gradient = diffX / diffY;
    float yend = float(round(y1));
    float xend = x1 + gradient * (yend - y1);
    float ygap = rfpart(y1 + 0.5f);
    
    int ypxl1 = int(yend);
    int xpxl1 = floor(xend);

    // Add the first endpoint
    window.setPixelColour( xpxl1, ypxl1, getColour(ptClr, rfpart(xend) * ygap));
    window.setPixelColour( xpxl1, ypxl1 + 1, getColour(ptClr, fpart(xend) * ygap));
    
    float interx = xend + gradient;

    yend = float(round(y2));
    xend = x2 + gradient * (yend - y2);
    ygap = fpart(y2 + 0.5f);
    
    int ypxl2 = int(yend);
    int xpxl2 = floor(xend);
    
    // Add the second endpoint
    window.setPixelColour( xpxl2, ypxl2, getColour(ptClr, rfpart(xend) * ygap));
    window.setPixelColour( xpxl2, ypxl2 + 1, getColour(ptClr, fpart(xend) * ygap));
    
    // Add all the points between the endpoints
    for(int y = ypxl1 + 1; y <= ypxl2 - 1; ++y){

      int i_interx = floor(interx);

      window.setPixelColour( i_interx, y, getColour(ptClr, rfpart(interx)));
      window.setPixelColour( i_interx + 1, y, getColour(ptClr, fpart(interx)));
      interx += gradient;
    }
  }
}

CanvasTriangle sortTriangleVertices(CanvasTriangle tri) {
  //** Unrolled 'Bubble Sort' for 3 items.

  if (tri.vertices[1].y < tri.vertices[0].y)
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        swap(tri.vertices[0], tri.vertices[1]);

  if (tri.vertices[2].y < tri.vertices[1].y)
    swap(tri.vertices[1], tri.vertices[2]);
  else if (tri.vertices[2].y == tri.vertices[1].y)
      if (tri.vertices[2].x < tri.vertices[1].x)
        swap(tri.vertices[1], tri.vertices[2]);

  if (tri.vertices[1].y < tri.vertices[0].y)
    swap(tri.vertices[0], tri.vertices[1]);
  else if (tri.vertices[1].y == tri.vertices[0].y)
      if (tri.vertices[1].x < tri.vertices[0].x)
        swap(tri.vertices[0], tri.vertices[1]);

  return tri;
}

void textureFlatBottomTriangle(ImageFile imageFile, CanvasTriangle triangle) {
  //Assumption: last two vertices represent the flat bottom.
  //Assumption: last two y values are both the same.
  assert( triangle.vertices[1].y == triangle.vertices[2].y);

  if (triangle.vertices[2].x < triangle.vertices[1].x) 
    swap(triangle.vertices[1], triangle.vertices[2]);
  
  int noOfRows = triangle.vertices[1].y - triangle.vertices[0].y;

  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[1], noOfRows);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows);

  vector<TexturePoint> interpLeftTexture = interpolate(triangle.vertices[0].texturePoint, triangle.vertices[1].texturePoint, noOfRows);
  vector<TexturePoint> interpRightTexture = interpolate(triangle.vertices[0].texturePoint, triangle.vertices[2].texturePoint, noOfRows);

  //** Do line by line interp.
  for (int i=0; i< noOfRows; i++) {
    CanvasPoint xy_start (interpLeft[i].x, triangle.vertices[0].y + i);
    CanvasPoint xy_end (interpRight[i].x, triangle.vertices[0].y + i);

    int noOfRowValues = xy_end.x - xy_start.x;

    vector<TexturePoint> interpTexLine = interpolate(interpLeftTexture[i], interpRightTexture[i], noOfRowValues);

    for (int j = 0; j<noOfRowValues; j++) {
      int x = xy_start.x + j;
      int y = xy_start.y;
      window.setPixelColour( x, y, getColour(getPixelColour(imageFile, interpTexLine[j].x, interpTexLine[j].y)));
    }
  }
  return;
}

void textureFlatTopTriangle(ImageFile imageFile, CanvasTriangle triangle) {
  //Assumption: last two vertices represent the flat bottom.
  //Assumption: last two y values are both the same.
  assert( triangle.vertices[0].y == triangle.vertices[1].y);

  if (triangle.vertices[1].x < triangle.vertices[0].x)
    swap(triangle.vertices[0], triangle.vertices[1]);
  
  int noOfRows = triangle.vertices[2].y - triangle.vertices[0].y;

  vector<CanvasPoint> interpLeft = interpolate(triangle.vertices[0], triangle.vertices[2], noOfRows);
  vector<CanvasPoint> interpRight = interpolate(triangle.vertices[1], triangle.vertices[2], noOfRows);

  vector<TexturePoint> interpLeftTexture = interpolate(triangle.vertices[0].texturePoint, triangle.vertices[2].texturePoint, noOfRows);
  vector<TexturePoint> interpRightTexture = interpolate(triangle.vertices[1].texturePoint, triangle.vertices[2].texturePoint, noOfRows);

  //** Do line by line interp.
  for (int i=0; i< noOfRows; i++) {
    CanvasPoint xy_start (interpLeft[i].x, triangle.vertices[0].y + i);
    CanvasPoint xy_end (interpRight[i].x, triangle.vertices[0].y + i);

    int noOfRowValues = xy_end.x - xy_start.x;

    vector<TexturePoint> interpTexLine = interpolate(interpLeftTexture[i], interpRightTexture[i], noOfRowValues);

    for (int j = 0; j<noOfRowValues; j++) {
      int x = xy_start.x + j;
      int y = xy_start.y;
      window.setPixelColour( x, y, getColour(getPixelColour(imageFile, interpTexLine[j].x, interpTexLine[j].y)));
    }
  }

  return;
}

void drawTexturedTriangle(ImageFile imageFile, CanvasTriangle triangle){
  // 1) Sort Vertices.  
  triangle = sortTriangleVertices(triangle);

  // 2) Get difference between top vertex & bottom vertex.
  const float xDiff = glm::abs(triangle.vertices[2].x - triangle.vertices[0].x);
  const float yDiff = glm::abs(triangle.vertices[2].y - triangle.vertices[0].y);

  if (yDiff == 0.f) {
    cout << "Y diff is zero.\n\n"; //** TODO: Draw a line.
  } 
  else {
    float cutX;

    if (triangle.vertices[0].x <= triangle.vertices[2].x) {
      const float minorYDiff = triangle.vertices[1].y - triangle.vertices[0].y;
      cutX = triangle.vertices[0].x + (minorYDiff * xDiff/yDiff);
    }
    else {
      const float minorYDiff = triangle.vertices[2].y - triangle.vertices[1].y; 
      cutX = triangle.vertices[2].x + (minorYDiff * xDiff/yDiff);
    }

    CanvasPoint cutPoint = CanvasPoint(cutX, triangle.vertices[1].y);
    
    // Calculate the proportion -> how far down the line the cut point is.
    const float distFromTop = sqrtf ( pow(triangle.vertices[0].x - cutPoint.x, 2) + pow(triangle.vertices[0].y - cutPoint.y, 2));
    const float distLine = sqrtf ( pow(triangle.vertices[0].x - triangle.vertices[2].x, 2) + pow(triangle.vertices[0].y - triangle.vertices[2].y, 2));
    const float p = distFromTop/distLine;
    //Get texture cut point using proportion & SOHCAHTOA.
    const float distLineTex = sqrtf ( pow(triangle.vertices[0].texturePoint.x - triangle.vertices[2].texturePoint.x, 2) + pow(triangle.vertices[0].texturePoint.y - triangle.vertices[2].texturePoint.y, 2));
    const float distFromTopTex = p*distLineTex;

    if (triangle.vertices[0].x <= triangle.vertices[2].x) {
      float cutTextureX = triangle.vertices[0].texturePoint.x + (distFromTopTex * xDiff/distLine);
      float cutTextureY = triangle.vertices[0].texturePoint.y + (distFromTopTex * yDiff/distLine);
      cutPoint.texturePoint = TexturePoint(cutTextureX, cutTextureY);
    } else {
      float cutTextureX = triangle.vertices[0].texturePoint.x - (distFromTopTex * xDiff/distLine);
      float cutTextureY = triangle.vertices[0].texturePoint.y + (distFromTopTex * yDiff/distLine);
      cutPoint.texturePoint = TexturePoint(cutTextureX, cutTextureY);
    }

    /*
    cout << "\n\nTriangle Vertices\n";
    cout << "Start Point:  x: " << triangle.vertices[0].x << " y: " << triangle.vertices[0].y << "\n"; 
    cout << "Middle Point:  x: " << triangle.vertices[1].x << " y: " << triangle.vertices[1].y << "\n"; 
    cout << "End Point:  x: " << triangle.vertices[2].x << " y: " << triangle.vertices[2].y << "\n"; 
    cout << "Cutting Point:  x: " << cutPoint.x << " y: " << cutPoint.y << "\n"; 

    cout << "\n\nTexture Point Vertices\n";
    print(triangle.vertices[0].texturePoint);
    print(triangle.vertices[1].texturePoint);
    print(triangle.vertices[2].texturePoint);
    print(cutPoint.texturePoint);
    */
    
    //** 4.1. Fill the Top Flat Bottom Triangle.
    textureFlatBottomTriangle(imageFile, CanvasTriangle(triangle.vertices[0], cutPoint, triangle.vertices[1], triangle.colour));
    //** 4.2. Fill the Bottom Flat Top Triangle.
    textureFlatTopTriangle(imageFile, CanvasTriangle(triangle.vertices[1], cutPoint, triangle.vertices[2], triangle.colour));
  }
}

void drawStrokedTriangle(CanvasTriangle triangle){
  /* Vertex sorting NOT required */
  CanvasPoint pt0 = triangle.vertices[0];
  CanvasPoint pt1 = triangle.vertices[1];
  CanvasPoint pt2 = triangle.vertices[2];

  drawAALine(pt0, pt1, triangle.colour);
  drawAALine(pt2, pt0, triangle.colour);
  drawAALine(pt1, pt2, triangle.colour);
}

void update() {
  // Function for performing animation (shifting artifacts or moving the camera)
}

void handleEvent(SDL_Event event) {
  if(event.type == SDL_KEYDOWN) {
    if(event.key.keysym.sym == SDLK_LEFT) cout << "LEFT" << endl;
    else if(event.key.keysym.sym == SDLK_RIGHT) cout << "RIGHT" << endl;
    else if(event.key.keysym.sym == SDLK_UP) cout << "UP" << endl;
    else if(event.key.keysym.sym == SDLK_DOWN) cout << "DOWN" << endl;
  }
  else if(event.type == SDL_MOUSEBUTTONDOWN) cout << "MOUSE CLICKED" << endl;
}
