/* 
  Inspired by onigiri
  original video: https://youtu.be/n4zUgtDk95w?si=CIhfePrFjpKQVU7P
  github source code: https://github.com/ArtemOnigiri/Console3D
*/
#include <iostream>
#include <stdio.h>
#include <math.h>
#include <windows.h>

// vector for operation with our primitives
struct vec2
{
  float x, y;

  vec2(float value) : x(value), y(value) {} 
  vec2(float _x, float _y) : x(_x), y(_y) {} // creating constructor to init vector with 2 values

  vec2 operator+(vec2 const& other) { return vec2(x + other.x, y + other.y); } // add vector
  vec2 operator-(vec2 const& other) { return vec2(x - other.x, y - other.y); } // substract vector
  vec2 operator*(vec2 const& other) { return vec2(x * other.x, y * other.y); } // multiply vector
  vec2 operator/(vec2 const& other) { return vec2(x / other.x, y / other.y); } // divide vector
};
// vector for operation with 3D dimentional primitives
struct vec3
{
  float x, y, z; 

  vec3(float _value) : x(_value), y(_value), z(_value) {}; // creating constructor to init vector with 3 values
  vec3(float _x, vec2 const& v) : x(_x), y(v.x), z(v.y) {}; 
  vec3(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {};

  vec3 operator+(vec3 const& other) { return vec3(x + other.x, y + other.y, z + other.z); }
  vec3 operator-(vec3 const& other) { return vec3(x - other.x, y - other.y, z - other.z); }
  vec3 operator*(vec3 const& other) { return vec3(x * other.x, y * other.y, z * other.z); }
  vec3 operator/(vec3 const& other) { return vec3(x / other.x, y / other.y, z / other.z); }
  vec3 operator-() { return vec3(-x, -y, -z); } // returning our revers vector

};
// setting window to printout our 3d engine in console 
void SetWindow(int Width, int Height)
{
  _COORD coord;
  coord.X = Width;
  coord.Y = Height;
  _SMALL_RECT Rect;
  Rect.Top = 0;
  Rect.Left = 0;
  Rect.Bottom = Height - 1;
  Rect.Right = Width - 1;
  HANDLE Handle = GetStdHandle(STD_OUTPUT_HANDLE);
  SetConsoleScreenBufferSize(Handle, coord); // set buffer size to printout all frames correctly
  SetConsoleWindowInfo(Handle, TRUE, &Rect);
}
// functions for basic manipulations as rotating, moving etc.
float clamp(float value, float min, float max) { return fmax(fmin(value, max), min); } // clamp function 
double sign(double a) { return (0 < a) - (a < 0); } 
double step(double edge, double x) { return x > edge; }

float length(vec2 const& v) { return sqrt(v.x * v.x + v.y * v.y); }

float length(vec3 const& v) { return sqrt(v.x * v.x + v.y * v.y + v.z * v.z); }
vec3 norm(vec3 v) { return v / length(v); }
float dot(vec3 const& a, vec3 const& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
vec3 abs(vec3 const& v) { return vec3(fabs(v.x), fabs(v.y), fabs(v.z)); }
vec3 sign(vec3 const& v) { return vec3(sign(v.x), sign(v.y), sign(v.z)); }
vec3 step(vec3 const& edge, vec3 v) { return vec3(step(edge.x, v.x), step(edge.y, v.y), step(edge.z, v.z)); }
vec3 reflect(vec3 rd, vec3 n) { return rd - n * (2 * dot(n, rd)); }

// rotating our primitive by x-axis
vec3 rotateX(vec3 a, double angle)
{
    vec3 b = a;
    b.z = a.z * cos(angle) - a.y * sin(angle);
    b.y = a.z * sin(angle) + a.y * cos(angle);
    return b;
}

// rotating our primitive by y-axis
vec3 rotateY(vec3 a, double angle)
{
    vec3 b = a;
    b.x = a.x * cos(angle) - a.z * sin(angle);
    b.z = a.x * sin(angle) + a.z * cos(angle);
    return b;
}

// rotating our primitive by z-axis
vec3 rotateZ(vec3 a, double angle)
{
    vec3 b = a;
    b.x = a.x * cos(angle) - a.y * sin(angle);
    b.y = a.x * sin(angle) + a.y * cos(angle);
    return b;
}

// creating sphere primitive with all properties and attributes
vec2 sphere(vec3 ro, vec3 rd, float r) {
    float b = dot(ro, rd);
    float c = dot(ro, ro) - r * r;
    float h = b * b - c;
    if (h < 0.0) return vec2(-1.0);
    h = sqrt(h);
    return vec2(-b - h, -b + h);
}
// box primitive
vec2 box(vec3 ro, vec3 rd, vec3 boxSize, vec3& outNormal) {
    vec3 m = vec3(1.0) / rd; // mesh
    vec3 n = m * ro; // normals
    vec3 k = abs(m) * boxSize; // size
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = fmax(fmax(t1.x, t1.y), t1.z);
    float tF = fmin(fmin(t2.x, t2.y), t2.z);
    if (tN > tF || tF < 0.0) return vec2(-1.0);
    vec3 yzx = vec3(t1.y, t1.z, t1.x); // coordinates 
    vec3 zxy = vec3(t1.z, t1.x, t1.y); 
    outNormal = -sign(rd) * step(yzx, t1) * step(zxy, t1);
    return vec2(tN, tF);
}
// plane to handle our primitives
float plane(vec3 ro, vec3 rd, vec3 p, float w) {
    return -(dot(ro, p) + w) / dot(rd, p); 
}

int main() {
  int width = 120 * 2; // setting window size to cmd as default
  int height = 30 * 2;
  SetWindow(width, height);
  float aspect = (float)width / height;
  float pixelAspect = 11.0f / 24.0f;
  char gradient[] = " .:!/r(l1Z4H9W8$@"; // gradient characters zero brightess to 255 brightess
  int gradientSize = std::size(gradient) - 2;

  wchar_t* screen = new wchar_t[width * height];
  HANDLE hConsole = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE, 0, NULL, CONSOLE_TEXTMODE_BUFFER, NULL); // buffer
  SetConsoleActiveScreenBuffer(hConsole);
  DWORD dwBytesWritten = 0;

  // printing out result to scene
  for (int t = 0; t < 10000; t++) {
    vec3 light = norm(vec3(-0.5, 0.5, -1.0));
    vec3 spherePos = vec3(0, 3, 0);
    for (int i = 0; i < width; i++) {
      for (int j = 0; j < height; j++) {
        vec2 uv = vec2(i, j) / vec2(width, height) * 2.0f - 1.0f;
        uv.x *= aspect * pixelAspect;
        vec3 ro = vec3(-6, 0, 0);
        vec3 rd = norm(vec3(2, uv)); // normalize vectors
        ro = rotateY(ro, 0.25); // rotating in scene by coordinates
        rd = rotateY(rd, 0.25);
        ro = rotateZ(ro, t * 0.01);
        rd = rotateZ(rd, t * 0.01);
        float diff = 1; 
        for (int k = 0; k < 5; k++) {
          float minIt = 99999;
          vec2 intersection = sphere(ro - spherePos, rd, 1);
          vec3 n = 0;
          float albedo = 1;
          if (intersection.x > 0) {
            vec3 itPoint = ro - spherePos + rd * intersection.x;
            minIt = intersection.x;
            n = norm(itPoint);
          }
          vec3 boxN = 0;
          intersection = box(ro, rd, 1, boxN);
          if (intersection.x > 0 && intersection.x < minIt) {
            minIt = intersection.x;
            n = boxN;
          }
          intersection = plane(ro, rd, vec3(0, 0, -1), 1);
          if (intersection.x > 0 && intersection.x < minIt) {
            minIt = intersection.x;
            n = vec3(0, 0, -1);
            albedo = 0.5;
          }
          if (minIt < 99999) {
            diff *= (dot(n, light) * 0.5 + 0.5) * albedo;
            ro = ro + rd * (minIt - 0.01);
            rd = reflect(rd, n);
          }
          else break;
        }
        int color = (int)(diff * 20); // color manipulation
        color = clamp(color, 0, gradientSize);
        char pixel = gradient[color];
        screen[i + j * width] = pixel;
      }
    }
    screen[width * height - 1] = '\0';
    WriteConsoleOutputCharacter(hConsole, screen, width * height, { 0, 0 }, &dwBytesWritten); // final output
  }
}