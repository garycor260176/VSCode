#pragma once
#include <Arduino.h>

#define SQRT_VARIANT sqrt3   // выбор основной функции для вычисления квадратного корня sqrtf или sqrt3 для ускорения
#define trackingOBJECT_MAX_COUNT                         (100U)  // максимальное количество отслеживаемых объектов (очень влияет на расход памяти)
#define enlargedOBJECT_MAX_COUNT                     (WIDTH * 2) // максимальное количество сложных отслеживаемых объектов (меньше, чем trackingOBJECT_MAX_COUNT)
#define bballsGRAVITY           (-9.81)              // Downward (negative) acceleration of gravity in m/s^2
#define bballsH0                (1)                  // Starting height, in meters, of the ball (strip length)

uint8_t enlargedObjectNUM;                                       // используемое в эффекте количество объектов
uint8_t trackingObjectHue[trackingOBJECT_MAX_COUNT];
uint8_t trackingObjectState[trackingOBJECT_MAX_COUNT];
long    enlargedObjectTime[enlargedOBJECT_MAX_COUNT];
float   trackingObjectPosX[trackingOBJECT_MAX_COUNT];
float   trackingObjectPosY[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedX[trackingOBJECT_MAX_COUNT];
float   trackingObjectSpeedY[trackingOBJECT_MAX_COUNT];
float   trackingObjectShift[trackingOBJECT_MAX_COUNT];
bool    trackingObjectIsShift[trackingOBJECT_MAX_COUNT];
float bballsVImpact0 = SQRT_VARIANT( -2 * bballsGRAVITY * bballsH0 );  // Impact velocity of the ball when it hits the ground if "dropped" from the top of the strip
uint8_t deltaValue;                                // просто повторно используемая переменная
uint8_t deltaHue, deltaHue2;                       // ещё пара таких же, когда нужно много

extern const TProgmemRGBPalette16 WaterfallColors_p FL_PROGMEM = {0x000000, 0x060707, 0x101110, 0x151717, 0x1C1D22, 0x242A28, 0x363B3A, 0x313634, 0x505552, 0x6B6C70, 0x98A4A1, 0xC1C2C1, 0xCACECF, 0xCDDEDD, 0xDEDFE0, 0xB2BAB9};

const TProgmemRGBPalette16 *palette_arr[] = {
    &PartyColors_p,
    &OceanColors_p, 
    &LavaColors_p, 
    &HeatColors_p, 
    &WaterfallColors_p, 
    &CloudColors_p, 
    &ForestColors_p, 
    &RainbowColors_p, 
    &RainbowStripeColors_p};
const TProgmemRGBPalette16 *curPalette = palette_arr[0];

uint8_t spirocount = 1;
byte spirotheta1 = 0;
byte spirotheta2 = 0;
uint8_t spirooffset = 256 / spirocount;
const uint8_t spiroradiusx = WIDTH / 4;// - 1;
const uint8_t spiroradiusy = HEIGHT / 4;// - 1;
const uint8_t spirocenterX = WIDTH / 2;
const uint8_t spirocenterY = HEIGHT / 2;
const uint8_t spirominx = spirocenterX - spiroradiusx;
const uint8_t spiromaxx = spirocenterX + spiroradiusx - (WIDTH%2 == 0 ? 1:0);//+ 1;
const uint8_t spirominy = spirocenterY - spiroradiusy;
const uint8_t spiromaxy = spirocenterY + spiroradiusy - (HEIGHT%2 == 0 ? 1:0);//+ 1;
boolean spirohandledChange = false;
boolean spiroincrement = false;

float speedfactor;                                 // регулятор скорости в эффектах реального времени

#define NUM_LAYERSMAX 2
uint8_t noise3d[NUM_LAYERSMAX][WIDTH][HEIGHT];     // двухслойная маска или хранилище свойств в размер всей матрицы
uint8_t shiftValue[HEIGHT];                        // свойство пикселей в размер столбца матрицы ещё одно
#define FLAME_MAX_DY        256 // максимальная вертикальная скорость перемещения языков пламени за кадр.  имеется в виду 256/256 =   1 пиксель за кадр
#define FLAME_MIN_DY        128 // минимальная вертикальная скорость перемещения языков пламени за кадр.   имеется в виду 128/256 = 0.5 пикселя за кадр
#define FLAME_MAX_DX         32 // максимальная горизонтальная скорость перемещения языков пламени за кадр. имеется в виду 32/256 = 0.125 пикселя за кадр
#define FLAME_MIN_DX       (-FLAME_MAX_DX)
#define FLAME_MAX_VALUE     255 // максимальная начальная яркость языка пламени
#define FLAME_MIN_VALUE     176 // минимальная начальная яркость языка пламени

extern const TProgmemRGBPalette16 WoodFireColors_p FL_PROGMEM = {CRGB::Black, 0x330e00, 0x661c00, 0x992900, 0xcc3700, CRGB::OrangeRed, 0xff5800, 0xff6b00, 0xff7f00, 0xff9200, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold};             //* Orange
extern const TProgmemRGBPalette16 NormalFire_p FL_PROGMEM = {CRGB::Black, 0x330000, 0x660000, 0x990000, 0xcc0000, CRGB::Red, 0xff0c00, 0xff1800, 0xff2400, 0xff3000, 0xff3c00, 0xff4800, 0xff5400, 0xff6000, 0xff6c00, 0xff7800};                             // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 NormalFire2_p FL_PROGMEM = {CRGB::Black, 0x560000, 0x6b0000, 0x820000, 0x9a0011, CRGB::FireBrick, 0xc22520, 0xd12a1c, 0xe12f17, 0xf0350f, 0xff3c00, 0xff6400, 0xff8300, 0xffa000, 0xffba00, 0xffd400};                      // пытаюсь сделать что-то более приличное
extern const TProgmemRGBPalette16 LithiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x240707, 0x470e0e, 0x6b1414, 0x8e1b1b, CRGB::FireBrick, 0xc14244, 0xd16166, 0xe08187, 0xf0a0a9, CRGB::Pink, 0xff9ec0, 0xff7bb5, 0xff59a9, 0xff369e, CRGB::DeepPink};        //* Red
extern const TProgmemRGBPalette16 SodiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x332100, 0x664200, 0x996300, 0xcc8400, CRGB::Orange, 0xffaf00, 0xffb900, 0xffc300, 0xffcd00, CRGB::Gold, 0xf8cd06, 0xf0c30d, 0xe9b913, 0xe1af1a, CRGB::Goldenrod};           //* Yellow
extern const TProgmemRGBPalette16 CopperFireColors_p FL_PROGMEM = {CRGB::Black, 0x001a00, 0x003300, 0x004d00, 0x006600, CRGB::Green, 0x239909, 0x45b313, 0x68cc1c, 0x8ae626, CRGB::GreenYellow, 0x94f530, 0x7ceb30, 0x63e131, 0x4bd731, CRGB::LimeGreen};     //* Green
extern const TProgmemRGBPalette16 AlcoholFireColors_p FL_PROGMEM = {CRGB::Black, 0x000033, 0x000066, 0x000099, 0x0000cc, CRGB::Blue, 0x0026ff, 0x004cff, 0x0073ff, 0x0099ff, CRGB::DeepSkyBlue, 0x1bc2fe, 0x36c5fd, 0x51c8fc, 0x6ccbfb, CRGB::LightSkyBlue};  //* Blue
extern const TProgmemRGBPalette16 RubidiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, CRGB::Indigo, 0x3c0084, 0x2d0086, 0x1e0087, 0x0f0089, CRGB::DarkBlue};        //* Indigo
extern const TProgmemRGBPalette16 PotassiumFireColors_p FL_PROGMEM = {CRGB::Black, 0x0f001a, 0x1e0034, 0x2d004e, 0x3c0068, CRGB::Indigo, 0x591694, 0x682da6, 0x7643b7, 0x855ac9, CRGB::MediumPurple, 0xa95ecd, 0xbe4bbe, 0xd439b0, 0xe926a1, CRGB::DeepPink}; //* Violet
const TProgmemRGBPalette16 *firePalettes[] = {
//    &HeatColors_p, // эта палитра уже есть в основном наборе. если в эффекте подключены оба набора палитр, тогда копия не нужна
    &WoodFireColors_p,
    &NormalFire_p,
    &NormalFire2_p,
    &LithiumFireColors_p,
    &SodiumFireColors_p,
    &CopperFireColors_p,
    &AlcoholFireColors_p,
    &RubidiumFireColors_p,
    &PotassiumFireColors_p};
uint16_t ff_x, ff_y, ff_z;                         // большие счётчики
#define FIXED_SCALE_FOR_Y 4U // менять нельзя. корректировка скорости ff_x =... подогнана под него
uint8_t step;                                      // какой-нибудь счётчик кадров или последовательностей операций
float emitterX, emitterY;                          // какие-то динамичные координаты
CRGBPalette16 myPal;
#define AURORA_COLOR_RANGE 10 // (+/-10 единиц оттенка) диапазон, в котором плавает цвет сияния относительно выбранного оттенка 
#define AURORA_COLOR_PERIOD 2 // (2 раза в минуту) частота, с которой происходит колебание выбранного оттенка в разрешённом диапазоне
unsigned long polarTimer;
static const uint8_t MBAuroraColors_arr[5][4] PROGMEM = // палитра в формате CHSV
{//№, цвет, насыщенность, яркость
  {0  , 0 , 255,   0},// black
  {80 , 0 , 255, 255},
  {130, 25, 220, 255},
  {180, 25, 185, 255},
  {255, 25, 155, 255} //245
};

uint8_t razmerX, razmerY; // размеры ячеек по горизонтали / вертикали
uint8_t shtukX, shtukY; // количество ячеек по горизонтали / вертикали
uint8_t poleX, poleY; // размер всего поля по горизонтали / вертикали (в том числе 1 дополнительная пустая дорожка-разделитель с какой-то из сторон)
bool seamlessX; // получилось ли сделать поле по Х бесшовным
int8_t globalShiftX, globalShiftY; // нужно ли сдвинуть всё поле по окончаии цикла и в каком из направлений (-1, 0, +1)
bool krutimVertikalno; // направление вращения в данный момент
#define PAUSE_MAX 7 // пропустить 7 кадров после завершения анимации сдвига ячеек

uint8_t shiftHue[HEIGHT];                          // свойство пикселей в размер столбца матрицы

CRGB solidRainColor = CRGB(60,80,90);

#define BALLS_AMOUNT          (3U)                          // количество "шариков"
#define CLEAR_PATH            (1U)                          // очищать путь
#define BALL_TRACK            (1U)                          // (0 / 1) - вкл/выкл следы шариков
#define TRACK_STEP            (70U)                         // длина хвоста шарика (чем больше цифра, тем хвост короче)
int16_t coord[BALLS_AMOUNT][2U];
CRGB ballColors[BALLS_AMOUNT];
int8_t vector[BALLS_AMOUNT][2U];

#define e_sns_DENSE (32U) // плотность снега - меньше = плотнее

#define DIMSPEED (254U - 500U / WIDTH / HEIGHT)

#define BORDERTHICKNESS (1U) // глубина бордюра для размытия яркой частицы: 0U - без границы (резкие края); 1U - 1 пиксель (среднее размытие) ; 2U - 2 пикселя (глубокое размытие)
const uint8_t paintWidth = WIDTH - BORDERTHICKNESS * 2;
const uint8_t paintHeight = HEIGHT - BORDERTHICKNESS * 2;

byte waveRotation = 0;
uint8_t waveScale = 256 / WIDTH;
uint8_t waveCount = 1;
byte waveTheta = 0;
byte waveThetaUpdate = 0;
byte waveThetaUpdateFrequency = 0;
byte hueUpdate = 0;
byte hueUpdateFrequency = 0;

uint32_t noise32_x[NUM_LAYERSMAX];
uint32_t noise32_y[NUM_LAYERSMAX];
uint32_t noise32_z[NUM_LAYERSMAX];
uint32_t scale32_x[NUM_LAYERSMAX];
uint32_t scale32_y[NUM_LAYERSMAX];
const uint8_t CENTER_X_MINOR =  (WIDTH / 2) -  ((WIDTH - 1) & 0x01); // центр матрицы по ИКСУ, сдвинутый в меньшую сторону, если ширина чётная
const uint8_t CENTER_Y_MINOR = (HEIGHT / 2) - ((HEIGHT - 1) & 0x01); // центр матрицы по ИГРЕКУ, сдвинутый в меньшую сторону, если высота чётная
uint8_t noisesmooth;
CRGB ledsbuff[NUM_LEDS];                           // копия массива leds[] целиком
int8_t zD;
int8_t zF;

template <class T>
class Vector2 {
public:
    T x, y;

    Vector2() :x(0), y(0) {}
    Vector2(T x, T y) : x(x), y(y) {}
    Vector2(const Vector2& v) : x(v.x), y(v.y) {}

    Vector2& operator=(const Vector2& v) {
        x = v.x;
        y = v.y;
        return *this;
    }
    
    bool isEmpty() {
        return x == 0 && y == 0;
    }

    bool operator==(Vector2& v) {
        return x == v.x && y == v.y;
    }

    bool operator!=(Vector2& v) {
        return !(x == y);
    }

    Vector2 operator+(Vector2& v) {
        return Vector2(x + v.x, y + v.y);
    }
    Vector2 operator-(Vector2& v) {
        return Vector2(x - v.x, y - v.y);
    }

    Vector2& operator+=(Vector2& v) {
        x += v.x;
        y += v.y;
        return *this;
    }
    Vector2& operator-=(Vector2& v) {
        x -= v.x;
        y -= v.y;
        return *this;
    }

    Vector2 operator+(double s) {
        return Vector2(x + s, y + s);
    }
    Vector2 operator-(double s) {
        return Vector2(x - s, y - s);
    }
    Vector2 operator*(double s) {
        return Vector2(x * s, y * s);
    }
    Vector2 operator/(double s) {
        return Vector2(x / s, y / s);
    }
    
    Vector2& operator+=(double s) {
        x += s;
        y += s;
        return *this;
    }
    Vector2& operator-=(double s) {
        x -= s;
        y -= s;
        return *this;
    }
    Vector2& operator*=(double s) {
        x *= s;
        y *= s;
        return *this;
    }
    Vector2& operator/=(double s) {
        x /= s;
        y /= s;
        return *this;
    }

    void set(T x, T y) {
        this->x = x;
        this->y = y;
    }

    void rotate(double deg) {
        double theta = deg / 180.0 * M_PI;
        double c = cos(theta);
        double s = sin(theta);
        double tx = x * c - y * s;
        double ty = x * s + y * c;
        x = tx;
        y = ty;
    }

    Vector2& normalize() {
        if (length() == 0) return *this;
        *this *= (1.0 / length());
        return *this;
    }

    float dist(Vector2 v) const {
        Vector2 d(v.x - x, v.y - y);
        return d.length();
    }
    float length() const {
        //return SQRT_VARIANT(x * x + y * y); некорректно работает через sqrt3, нужно sqrt
        return sqrt(x * x + y * y);
    }

    float mag() const {
        return length();
    }

    float magSq() {
        return (x * x + y * y);
    }

    void truncate(double length) {
        double angle = atan2f(y, x);
        x = length * cos(angle);
        y = length * sin(angle);
    }

    Vector2 ortho() const {
        return Vector2(y, -x);
    }

    static float dot(Vector2 v1, Vector2 v2) {
        return v1.x * v2.x + v1.y * v2.y;
    }
    static float cross(Vector2 v1, Vector2 v2) {
        return (v1.x * v2.y) - (v1.y * v2.x);
    }

    void limit(float max) {
        if (magSq() > max*max) {
            normalize();
            *this *= max;
        }
    }
};

typedef Vector2<float> PVector;

class Boid {
  public:

    PVector location;
    PVector velocity;
    PVector acceleration;
    float maxforce;    // Maximum steering force
    float maxspeed;    // Maximum speed

    float desiredseparation = 4;
    float neighbordist = 8;
    byte colorIndex = 0;
    float mass;

    boolean enabled = true;

    Boid() {}

    Boid(float x, float y) {
      acceleration = PVector(0, 0);
      velocity = PVector(randomf(), randomf());
      location = PVector(x, y);
      maxspeed = 1.5;
      maxforce = 0.05;
    }

    static float randomf() {
      return mapfloat(random(0, 255), 0, 255, -.5, .5);
    }

    static float mapfloat(float x, float in_min, float in_max, float out_min, float out_max) {
      return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
    }

    void run(Boid boids [], uint8_t boidCount) {
      flock(boids, boidCount);
      update();
      // wrapAroundBorders();
      // render();
    }

    // Method to update location
    void update() {
      // Update velocity
      velocity += acceleration;
      // Limit speed
      velocity.limit(maxspeed);
      location += velocity;
      // Reset acceleration to 0 each cycle
      acceleration *= 0;
    }

    void applyForce(PVector force) {
      // We could add mass here if we want A = F / M
      acceleration += force;
    }

    void repelForce(PVector obstacle, float radius) {
      //Force that drives boid away from obstacle.

      PVector futPos = location + velocity; //Calculate future position for more effective behavior.
      PVector dist = obstacle - futPos;
      float d = dist.mag();

      if (d <= radius) {
        PVector repelVec = location - obstacle;
        repelVec.normalize();
        if (d != 0) { //Don't divide by zero.
          // float scale = 1.0 / d; //The closer to the obstacle, the stronger the force.
          repelVec.normalize();
          repelVec *= (maxforce * 7);
          if (repelVec.mag() < 0) { //Don't let the boids turn around to avoid the obstacle.
            repelVec.y = 0;
          }
        }
        applyForce(repelVec);
      }
    }

    // We accumulate a new acceleration each time based on three rules
    void flock(Boid boids [], uint8_t boidCount) {
      PVector sep = separate(boids, boidCount);   // Separation
      PVector ali = align(boids, boidCount);      // Alignment
      PVector coh = cohesion(boids, boidCount);   // Cohesion
      // Arbitrarily weight these forces
      sep *= 1.5;
      ali *= 1.0;
      coh *= 1.0;
      // Add the force vectors to acceleration
      applyForce(sep);
      applyForce(ali);
      applyForce(coh);
    }

    // Separation
    // Method checks for nearby boids and steers away
    PVector separate(Boid boids [], uint8_t boidCount) {
      PVector steer = PVector(0, 0);
      int count = 0;
      // For every boid in the system, check if it's too close
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        // If the distance is greater than 0 and less than an arbitrary amount (0 when you are yourself)
        if ((d > 0) && (d < desiredseparation)) {
          // Calculate vector pointing away from neighbor
          PVector diff = location - other.location;
          diff.normalize();
          diff /= d;        // Weight by distance
          steer += diff;
          count++;            // Keep track of how many
        }
      }
      // Average -- divide by how many
      if (count > 0) {
        steer /= (float) count;
      }

      // As long as the vector is greater than 0
      if (steer.mag() > 0) {
        // Implement Reynolds: Steering = Desired - Velocity
        steer.normalize();
        steer *= maxspeed;
        steer -= velocity;
        steer.limit(maxforce);
      }
      return steer;
    }

    // Alignment
    // For every nearby boid in the system, calculate the average velocity
    PVector align(Boid boids [], uint8_t boidCount) {
      PVector sum = PVector(0, 0);
      int count = 0;
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist)) {
          sum += other.velocity;
          count++;
        }
      }
      if (count > 0) {
        sum /= (float) count;
        sum.normalize();
        sum *= maxspeed;
        PVector steer = sum - velocity;
        steer.limit(maxforce);
        return steer;
      }
      else {
        return PVector(0, 0);
      }
    }

    // Cohesion
    // For the average location (i.e. center) of all nearby boids, calculate steering vector towards that location
    PVector cohesion(Boid boids [], uint8_t boidCount) {
      PVector sum = PVector(0, 0);   // Start with empty vector to accumulate all locations
      int count = 0;
      for (int i = 0; i < boidCount; i++) {
        Boid other = boids[i];
        if (!other.enabled)
          continue;
        float d = location.dist(other.location);
        if ((d > 0) && (d < neighbordist)) {
          sum += other.location; // Add location
          count++;
        }
      }
      if (count > 0) {
        sum /= count;
        return seek(sum);  // Steer towards the location
      }
      else {
        return PVector(0, 0);
      }
    }

    // A method that calculates and applies a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    PVector seek(PVector target) {
      PVector desired = target - location;  // A vector pointing from the location to the target
      // Normalize desired and scale to maximum speed
      desired.normalize();
      desired *= maxspeed;
      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      return steer;
    }

    // A method that calculates a steering force towards a target
    // STEER = DESIRED MINUS VELOCITY
    void arrive(PVector target) {
      PVector desired = target - location;  // A vector pointing from the location to the target
      float d = desired.mag();
      // Normalize desired and scale with arbitrary damping within 100 pixels
      desired.normalize();
      if (d < 4) {
        float m = map(d, 0, 100, 0, maxspeed);
        desired *= m;
      }
      else {
        desired *= maxspeed;
      }

      // Steering = Desired minus Velocity
      PVector steer = desired - velocity;
      steer.limit(maxforce);  // Limit to maximum steering force
      applyForce(steer);
      //Serial.println(d);
    }

    void wrapAroundBorders() {
      if (location.x < 0) location.x = WIDTH - 1;
      if (location.y < 0) location.y = HEIGHT - 1;
      if (location.x >= WIDTH) location.x = 0;
      if (location.y >= HEIGHT) location.y = 0;
    }

    void avoidBorders() {
      PVector desired = velocity;

      if (location.x < 8) desired = PVector(maxspeed, velocity.y);
      if (location.x >= WIDTH - 8) desired = PVector(-maxspeed, velocity.y);
      if (location.y < 8) desired = PVector(velocity.x, maxspeed);
      if (location.y >= HEIGHT - 8) desired = PVector(velocity.x, -maxspeed);

      if (desired != velocity) {
        PVector steer = desired - velocity;
        steer.limit(maxforce);
        applyForce(steer);
      }

      if (location.x < 0) location.x = 0;
      if (location.y < 0) location.y = 0;
      if (location.x >= WIDTH) location.x = WIDTH - 1;
      if (location.y >= HEIGHT) location.y = HEIGHT - 1;
    }

    bool bounceOffBorders(float bounce) {
      bool bounced = false;

      if (location.x >= WIDTH) {
        location.x = WIDTH - 1;
        velocity.x *= -bounce;
        bounced = true;
      }
      else if (location.x < 0) {
        location.x = 0;
        velocity.x *= -bounce;
        bounced = true;
      }

      if (location.y >= HEIGHT) {
        location.y = HEIGHT - 1;
        velocity.y *= -bounce;
        bounced = true;
      }
      else if (location.y < 0) {
        location.y = 0;
        velocity.y *= -bounce;
        bounced = true;
      }

      return bounced;
    }

    void render() {
      // // Draw a triangle rotated in the direction of velocity
      // float theta = velocity.heading2D() + radians(90);
      // fill(175);
      // stroke(0);
      // pushMatrix();
      // translate(location.x,location.y);
      // rotate(theta);
      // beginShape(TRIANGLES);
      // vertex(0, -r*2);
      // vertex(-r, r*2);
      // vertex(r, r*2);
      // endShape();
      // popMatrix();
      // backgroundLayer.drawPixel(location.x, location.y, CRGB::Blue);
    }
};
static const uint8_t AVAILABLE_BOID_COUNT = 20U;
Boid boids[AVAILABLE_BOID_COUNT]; 
#define FAIRY_BEHAVIOR //типа сложное поведение

#define SNAKES_LENGTH (8U) // длина червяка от 2 до 15 (+ 1 пиксель голова/хвостик), ограничена размером переменной для хранения трактории тела червяка

unsigned liquidLampTR[enlargedOBJECT_MAX_COUNT];        
unsigned liquidLampMX[enlargedOBJECT_MAX_COUNT];        
unsigned liquidLampSC[enlargedOBJECT_MAX_COUNT];        
float    liquidLampSpf[enlargedOBJECT_MAX_COUNT];        
float    liquidLampHot[enlargedOBJECT_MAX_COUNT];        
unsigned MASS_MIN = 10;
unsigned MASS_MAX = 50;
static const uint8_t MBVioletColors_arr[5][4] PROGMEM = // та же палитра, но в формате CHSV
{
  {0  , 0  , 255, 255}, //  0, 255,   0,   0, // red
//  {1  , 108, 161, 122}, //  1,  46, 123,  87, // seaBlue
  {1  , 155, 209, 255}, //  1,  46, 124, 255, // сделал поярче цвет воды
  {80 , 170, 255, 140}, // 80,   0,   0, 139, // DarkBlue
  {150, 213, 255, 128}, //150, 128,   0, 128, // purple
  {255, 0  , 255, 255}  //255, 255,   0,   0  // red again
};

void dimAll(uint8_t value, CRGB *LEDarray = leds);
void setCurrentPalette(s_mode mode);
void blurScreen(fract8 blur_amount, CRGB *LEDarray = leds);
uint8_t mapsin8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255);
uint8_t mapcos8(uint8_t theta, uint8_t lowest = 0, uint8_t highest = 255);
uint8_t myScale8(uint8_t x);
void wu_pixel_maxV(int16_t item);
void wu_pixel(uint32_t x, uint32_t y, CRGB * col);
void fillMyPal16_2(uint8_t hue, bool isInvert = false);
void LeapersRestart_leaper(uint8_t l);
void LeapersMove_leaper(uint8_t l);
void rain(byte backgroundDepth, byte maxBrightness, byte spawnFreq, byte tailLength, CRGB rainColor, bool splashes, bool clouds, bool storm);
uint8_t wrapX(int8_t x);
uint8_t wrapY(int8_t y);
void PicassoRoutine();
void PicassoRoutine2();
void PicassoRoutine3();
void PicassoGenerate(bool reset);
void PicassoPosition();
void FillNoise(int8_t layer);
void FillNoise(int8_t layer);
void MoveFractionalNoiseX(int8_t amplitude = 1, float shift = 0);
void MoveFractionalNoiseY(int8_t amplitude = 1, float shift = 0);
void fairyEmit(uint8_t i);
void particlesUpdate2(uint8_t i);
void nexusReset(uint8_t i);
void LiquidLampPosition();
void LiquidLampPhysic();
void fillMyPal16(uint8_t hue, bool isInvert = false);

void LiquidLampRoutine(s_mode mode, bool isColored){
    if (loadingFlag)
    {
        loadingFlag = false;
        speedfactor = mode.speed / 64.0 + 0.1; // 127 БЫЛО
        
        if (isColored){
            fillMyPal16((mode.scale - 1U) * 2.55, !(mode.scale & 0x01));
            enlargedObjectNUM = enlargedOBJECT_MAX_COUNT / 2U - 2U; //14U;
        }
        else{
            enlargedObjectNUM = (mode.scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
            hue = random8();
            deltaHue = random8(2U);
            fillMyPal16(hue, deltaHue);
        }
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
        else if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

        double minSpeed = 0.2, maxSpeed = 0.8;
        
        for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) { 
            trackingObjectPosX[i] = random8(WIDTH);
            trackingObjectPosY[i] = 0; //random8(HEIGHT);
            trackingObjectState[i] = random(MASS_MIN, MASS_MAX);
            liquidLampSpf[i] = fmap(trackingObjectState[i], MASS_MIN, MASS_MAX, 0.0015, 0.0005);
            trackingObjectShift[i] = fmap(trackingObjectState[i], MASS_MIN, MASS_MAX, 2, 3);
            liquidLampMX[i] = map(trackingObjectState[i], MASS_MIN, MASS_MAX, 60, 80); // сила возмущения
            liquidLampSC[i] = map(trackingObjectState[i], MASS_MIN, MASS_MAX, 6, 10); // радиус возмущения
            liquidLampTR[i] = liquidLampSC[i]  * 2 / 3; // отсечка расчетов (оптимизация скорости)
        }
    }
    
    LiquidLampPosition();
    LiquidLampPhysic;

    if (!isColored){
        hue2++;
        if (hue2 % 0x10 == 0U){
            hue++;
            fillMyPal16(hue, deltaHue);
        }
    }

    for (uint8_t x = 0; x < WIDTH; x++) {
        for (uint8_t y = 0; y < HEIGHT; y++) {
            float sum = 0;
            //for (unsigned i = 0; i < numParticles; i++) {
            for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
                if (abs(x - trackingObjectPosX[i]) > liquidLampTR[i] || abs(y - trackingObjectPosY[i]) > liquidLampTR[i]) continue;
                float dx =  min((float)fabs(trackingObjectPosX[i] - (float)x), (float)WIDTH + trackingObjectPosX[i] - (float)x); //по идее бесшовный икс
                float dy =  fabs(trackingObjectPosY[i] - (float)y);
                float d = SQRT_VARIANT((dx * dx) + (dy * dy));
                
                if (d < trackingObjectShift[i]) {
                    sum += mapcurve(d, 0, trackingObjectShift[i], 255, liquidLampMX[i], InQuad);
                }   
                else if (d < liquidLampSC[i]){
                    sum += mapcurve(d, trackingObjectShift[i], liquidLampSC[i], liquidLampMX[i], 0, OutQuart);
                }
                if (sum >= 255) { sum = 255; break; }
            }
            if (sum < 16) sum = 16;// отрезаем смазанный кусок палитры из-за отсутствия параметра NOBLEND
            CRGB color = ColorFromPalette(myPal, sum); // ,255, NOBLEND
            drawPixelXY(x, y, color);
        }
    }
}

void Sinusoid3Routine(s_mode mode)
{
    if (loadingFlag) {
      loadingFlag = false;
      deltaValue = (mode.speed - 1U) % 9U; // количество режимов
      emitterX = WIDTH * 0.5;
      emitterY = HEIGHT * 0.5;
      speedfactor = 0.00145 * mode.speed + 0.015;
    }
    float e_s3_size = 3. * mode.scale / 100.0 + 2;    // amplitude of the curves

    uint32_t time_shift = millis() & 0xFFFFFF; // overflow protection

    uint16_t _scale = (((mode.scale - 1U) % 9U) * 10U + 80U) << 7U; // = fmap(scale, 1, 255, 0.1, 3);
    float _scale2 = (float)((mode.scale - 1U) % 9U) * 0.2 + 0.4; // для спиралей на sinf
    uint16_t _scale3 = ((mode.scale - 1U) % 9U) * 1638U + 3276U; // для спиралей на sin16

    CRGB color;

    float center1x = float(e_s3_size * sin16(speedfactor * 72.0874 * time_shift)) / 0x7FFF - emitterX;
    float center1y = float(e_s3_size * cos16(speedfactor * 98.301  * time_shift)) / 0x7FFF - emitterY;
    float center2x = float(e_s3_size * sin16(speedfactor * 68.8107 * time_shift)) / 0x7FFF - emitterX;
    float center2y = float(e_s3_size * cos16(speedfactor * 65.534  * time_shift)) / 0x7FFF - emitterY;
    float center3x = float(e_s3_size * sin16(speedfactor * 134.3447 * time_shift)) / 0x7FFF - emitterX;
    float center3y = float(e_s3_size * cos16(speedfactor * 170.3884 * time_shift)) / 0x7FFF - emitterY;
  
    switch (deltaValue) {
        case 0://Sinusoid I
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.r = v;
                    cx = x + center3x;
                    cy = y + center3y;
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.b = v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 1: //Sinusoid II ???
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    //int8_t v = 127 * (0.001 * time_shift * speedfactor + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 32767.0);
                    uint8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.r = v;
                    
                    cx = x + center2x;
                    cy = y + center2y;
                    //v = 127 * (float(0.001 * time_shift * speedfactor) + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 32767.0);
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    //color.g = (uint8_t)v >> 1;
                    color.g = (v - (min(v, color.r) >> 1)) >> 1;
                    //color.b = (uint8_t)v >> 2;
                    color.b = color.g >> 2;
                    color.r = max(v, color.r);
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 2://Sinusoid III
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.r = v;

                    cx = x + center2x;
                    cy = y + center2y;
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.b = v;

                    cx = x + center3x;
                    cy = y + center3y;
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.g = v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 3: //Sinusoid IV
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
                    color.r = ~v;

                    cx = x + center2x;
                    cy = y + center2y;
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
                    color.g = ~v;

                    cx = x + center3x;
                    cy = y + center3y;
                    v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 100)) / 0x7FFF);
                    color.b = ~v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 4: //changed by stepko //colored sinusoid
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    int8_t v = 127 * (1 + float(sin16(_scale * (beatsin16(2,1000,1750)/2550.) * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);// + time_shift * speedfactor * 5 // mass colors plus by SottNick
                    color.r = v;

                    //v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 7)) / 0x7FFF);
                    //v = 127 * (1 + sinf (_scale2 * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 0.001 * time_shift * speedfactor));
                    v = 127 * (1 + float(sin16(_scale * (beatsin16(1,570,1050)/2250.) * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 13 * time_shift * speedfactor)) / 0x7FFF); // вместо beatsin сперва ставил просто * 0.41
                    color.b = v;

                    //v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 19)) / 0x7FFF);
                    //v = 127 * (1 + sinf (_scale2 * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 0.0025 * time_shift * speedfactor));
                    v = 127 * (1 + float(cos16(_scale * (beatsin16(3,1900,2550)/2550.) * SQRT_VARIANT(((cx * cx) + (cy * cy)))  + 41 * time_shift * speedfactor)) / 0x7FFF); // вместо beatsin сперва ставил просто * 0.53
                    color.g = v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 5: //changed by stepko //sinusoid in net
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    int8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy) + time_shift * speedfactor * 5)) / 0x7FFF);
                    color.g = ~v;

                    //v = 127 * (1 + float(sin16(_scale * x) + 0.01 * time_shift * speedfactor) / 0x7FFF);
                    v = 127 * (1 + float(sin16(_scale * (x + 0.005 * time_shift * speedfactor))) / 0x7FFF); // proper by SottNick
                    
                    color.b = ~v;

                    //v = 127 * (1 + float(sin16(_scale * y * 127 + float(0.011 * time_shift * speedfactor))) / 0x7FFF);
                    v = 127 * (1 + float(sin16(_scale * (y + 0.0055 * time_shift * speedfactor))) / 0x7FFF); // proper by SottNick
                    color.r = ~v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 6: //changed by stepko //spiral
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    uint8_t v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    //uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали - proper by SottNick
                    float d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.r = v;

                    cx = x + center2x;
                    cy = y + center2y;
                    //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    //v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали
                    d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.b = v;

                    cx = x + center3x;
                    cy = y + center3y;
                    //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали
                    d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.g = v;
                    drawPixelXY(x, y, color);
                }
            }
            break;
        case 7: //variant by SottNick
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    //uint8_t v = 127 * (1 + float(sin16(3* atan2(cy, cx) + _scale *  hypot(cy, cx) + time_shift * speedfactor * 5)) / 0x7FFF);
                    //uint8_t v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали
                    float d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.g = v;

                    cx = x + center3x;
                    cy = y + center3y;
                    //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали
                    d = SQRT_VARIANT(cx * cx + cy * cy) / 10.; // 10 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.r = v;

                    drawPixelXY(x, y, color);
                    //nblend(leds[XY(x, y)], color, 150);
                }
            }
            break;
        case 8: //variant by SottNick
            for (uint8_t y = 0; y < HEIGHT; y++) {
                for (uint8_t x = 0; x < WIDTH; x++) {
                    float cx = x + center1x;
                    float cy = y + center1y;
                    //uint8_t v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    //uint8_t v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    //uint8_t v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    uint8_t v = 127 * (1 + float(sin16(_scale * SQRT_VARIANT(cx * cx + cy * cy))) / 0x7FFF);
                    color.g = v;

                    cx = x + center2x;
                    cy = y + center2y;
                    //v = 127 * (1 + float(sin16(_scale * (2 * atan2(cy, cx) + hypot(cy, cx)) + time_shift * speedfactor * 5)) / 0x7FFF);
                    //v = 127 * (1 + sinf (3* atan2(cy, cx)  + _scale2 *  hypot(cy, cx))); // proper by SottNick
                    v = 127 * (1 + float(sin16(atan2(cy, cx) * 31255  + _scale3 *  hypot(cy, cx))) / 0x7FFF); // proper by SottNick
                    //вырезаем центр спирали
                    float d = SQRT_VARIANT(cx * cx + cy * cy) / 16.; // 16 - это радиус вырезаемого центра в каких-то условных величинах. 10 = 1 пиксель, 20 = 2 пикселя. как-то так
                    if (d < 0.06) d = 0.06;
                    if (d < 1) // просто для ускорения расчётов
                        v = constrain(v - int16_t(1/d/d), 0, 255);
                    //вырезали
                    color.g = max(v, color.g);
                    color.b = v;// >> 1;
                    //color.r = v >> 1;

                    drawPixelXY(x, y, color);
                    //nblend(leds[XY(x, y)], color, 150);
                }
            }
            break;
    }
}

void snakesRoutine(s_mode mode){
    if (loadingFlag) {
        loadingFlag = false;
        speedfactor = (float)mode.speed / 555.0f + 0.001f;
    
        enlargedObjectNUM = (mode.scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
        for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
            enlargedObjectTime[i] = 0;
            trackingObjectPosX[i] = random8(WIDTH);
            trackingObjectPosY[i] = random8(HEIGHT);
            trackingObjectSpeedX[i] = (255. + random8()) / 255.;
            trackingObjectSpeedY[i] = 0;
            trackingObjectHue[i] = random8();
            trackingObjectState[i] = random8(4);//     B00           направление головы змейки
                                                // B10     B11
                                                //     B01
        }
    }

    FastLED.clear();

    int8_t dx, dy;
    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
        trackingObjectSpeedY[i] += trackingObjectSpeedX[i] * speedfactor;
        if (trackingObjectSpeedY[i] >= 1) {
            trackingObjectSpeedY[i] = trackingObjectSpeedY[i] - (int)trackingObjectSpeedY[i];
            if (random8(9U) == 0U) // вероятность поворота
                if (random8(2U)) { // <- поворот налево
                    enlargedObjectTime[i] = (enlargedObjectTime[i] << 2) | B01; // младший бит = поворот
                    switch (trackingObjectState[i]) {
                        case B10:
                            trackingObjectState[i] = B01;
                            if (trackingObjectPosY[i] == 0U)
                            trackingObjectPosY[i] = HEIGHT - 1U;
                            else
                            trackingObjectPosY[i]--;
                            break;
                        case B11:
                            trackingObjectState[i] = B00;
                            if (trackingObjectPosY[i] >= HEIGHT - 1U)
                            trackingObjectPosY[i] = 0U;
                            else
                            trackingObjectPosY[i]++;
                            break;
                        case B00:
                            trackingObjectState[i] = B10;
                            if (trackingObjectPosX[i] == 0U)
                            trackingObjectPosX[i] = WIDTH - 1U;
                            else
                            trackingObjectPosX[i]--;
                            break;
                        case B01:
                            trackingObjectState[i] = B11;
                            if (trackingObjectPosX[i] >= WIDTH - 1U)
                            trackingObjectPosX[i] = 0U;
                            else
                            trackingObjectPosX[i]++;
                            break;
                    }
                } else { // -> поворот направо
                    enlargedObjectTime[i] = (enlargedObjectTime[i] << 2) | B11; // младший бит = поворот, старший = направо
                    switch (trackingObjectState[i]) {
                        case B11:
                            trackingObjectState[i] = B01;
                            if (trackingObjectPosY[i] == 0U)
                            trackingObjectPosY[i] = HEIGHT - 1U;
                            else
                            trackingObjectPosY[i]--;
                            break;
                        case B10:
                            trackingObjectState[i] = B00;
                            if (trackingObjectPosY[i] >= HEIGHT - 1U)
                            trackingObjectPosY[i] = 0U;
                            else
                            trackingObjectPosY[i]++;
                            break;
                        case B01:
                            trackingObjectState[i] = B10;
                            if (trackingObjectPosX[i] == 0U)
                            trackingObjectPosX[i] = WIDTH - 1U;
                            else
                            trackingObjectPosX[i]--;
                            break;
                        case B00:
                            trackingObjectState[i] = B11;
                            if (trackingObjectPosX[i] >= WIDTH - 1U)
                            trackingObjectPosX[i] = 0U;
                            else
                            trackingObjectPosX[i]++;
                            break;
                    }
                }
            else { // двигаем без поворота
                enlargedObjectTime[i] = (enlargedObjectTime[i] << 2);
                switch (trackingObjectState[i]) {
                    case B01:
                        if (trackingObjectPosY[i] == 0U)
                        trackingObjectPosY[i] = HEIGHT - 1U;
                        else
                        trackingObjectPosY[i]--;
                        break;
                    case B00:
                        if (trackingObjectPosY[i] >= HEIGHT - 1U)
                        trackingObjectPosY[i] = 0U;
                        else
                        trackingObjectPosY[i]++;
                        break;
                    case B10:
                        if (trackingObjectPosX[i] == 0U)
                        trackingObjectPosX[i] = WIDTH - 1U;
                        else
                        trackingObjectPosX[i]--;
                        break;
                    case B11:
                        if (trackingObjectPosX[i] >= WIDTH - 1U)
                        trackingObjectPosX[i] = 0U;
                        else
                        trackingObjectPosX[i]++;
                        break;
                }
      
            }
        }
   
        switch (trackingObjectState[i]) {
            case B01:
                dy = 1;
                dx = 0;
                break;
            case B00:
                dy = -1;
                dx = 0;
                break;
            case B10:
                dy = 0;
                dx = 1;
                break;
            case B11:
                dy = 0;
                dx = -1;
                break;
        }

        long temp = enlargedObjectTime[i];
        uint8_t x = trackingObjectPosX[i];
        uint8_t y = trackingObjectPosY[i];
        leds[XY(x,y)] += CHSV(trackingObjectHue[i], 255U, trackingObjectSpeedY[i] * 255); // тут рисуется голова

        for (uint8_t m = 0; m < SNAKES_LENGTH; m++) { // 16 бит распаковываем, 14 ещё остаётся без дела в запасе, 2 на хвостик
            x = (WIDTH + x + dx) % WIDTH;
            y = (HEIGHT + y + dy) % HEIGHT;
            leds[XY(x,y)] += CHSV(trackingObjectHue[i] + (m + trackingObjectSpeedY[i])*4U, 255U, 255U); // тут рисуется тело
        
            if (temp & B01) { // младший бит = поворот, старший = направо
                temp = temp >> 1;
                if (temp & B01){ // старший бит = направо
                    if (dx == 0) {
                        dx = 0 - dy;
                        dy = 0;
                    } else {
                        dy = dx;
                        dx = 0;
                    }
                } else { // иначе налево
                    if (dx == 0) {
                        dx = dy;
                        dy = 0;
                    } else{
                        dy = 0 - dx;
                        dx = 0;
                    }
                }
                temp = temp >> 1;
            } else { // если без поворота
                temp = temp >> 2;
            }
        }
        x = (WIDTH + x + dx) % WIDTH;
        y = (HEIGHT + y + dy) % HEIGHT;
        leds[XY(x,y)] += CHSV(trackingObjectHue[i] + (SNAKES_LENGTH + trackingObjectSpeedY[i])*4U, 255U, (1 - trackingObjectSpeedY[i]) * 255); // хвостик
    } 
}

void nexusRoutine(s_mode mode){
    if (loadingFlag)
    {
        loadingFlag = false;
        speedfactor = fmap(mode.speed, 1, 255, 0.1, .33);//(float)modes[currentMode].Speed / 555.0f + 0.001f;
        
        enlargedObjectNUM = (mode.scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
        for (uint8_t i = 0; i < enlargedObjectNUM; i++){
        trackingObjectPosX[i] = random8(WIDTH);
        trackingObjectPosY[i] = random8(HEIGHT);
        trackingObjectSpeedX[i] = (float)random8(5,11) / 70 + speedfactor; // делаем частицам немного разное ускорение и сразу пересчитываем под общую скорость
        trackingObjectHue[i] = random8();
        trackingObjectState[i] = random8(4);//     B00           // задаем направление
                                            // B10     B11
                                            //     B01
        }
        deltaValue = 255U - map(mode.speed, 1, 255, 11, 33);
        
    }
    dimAll(deltaValue);

    for (uint8_t i = 0; i < enlargedObjectNUM; i++){
            switch (trackingObjectState[i]) {
            case B01:
                trackingObjectPosY[i] -= trackingObjectSpeedX[i];
                if (trackingObjectPosY[i] <= -1)
                nexusReset(i);
                break;
            case B00:
                trackingObjectPosY[i] += trackingObjectSpeedX[i];
                if (trackingObjectPosY[i] >= HEIGHT)
                nexusReset(i);
                break;
            case B10:
                trackingObjectPosX[i] -= trackingObjectSpeedX[i];
                if (trackingObjectPosX[i] <= -1)
                nexusReset(i);
                break;
            case B11:
                trackingObjectPosX[i] += trackingObjectSpeedX[i];
                if (trackingObjectPosX[i] >= WIDTH)
                nexusReset(i);
                break;
            }
        drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i],  CHSV(trackingObjectHue[i], 255U, 255));
    }
}

void magmaRoutine(s_mode mode){
    //unsigned num = map(scale, 0U, 255U, 6U, sizeof(boids) / sizeof(*boids));
    if (loadingFlag)
    {
        loadingFlag = false;
        deltaValue = mode.scale * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
        if (deltaValue == 3U ||deltaValue == 4U)
        curPalette =  palette_arr[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F))];
        else
        curPalette = firePalettes[deltaValue]; // (uint8_t)(modes[currentMode].Scale/100.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
        deltaValue = 12U;
        deltaHue = 10U;// map(deltaValue, 8U, 168U, 8U, 84U); // высота языков пламени должна уменьшаться не так быстро, как ширина
        for (uint8_t j = 0; j < HEIGHT; j++) {
        shiftHue[j] = (HEIGHT - 1 - j) * 255 / (HEIGHT - 1); // init colorfade table
        }
        
        enlargedObjectNUM = (mode.scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;

        for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
        trackingObjectPosX[i] = random8(WIDTH);
        trackingObjectPosY[i] = random8(HEIGHT);

        trackingObjectHue[i] = 50U;random8();
        }
    }
    dimAll(181);

    for (uint8_t i = 0; i < WIDTH; i++) {
        for (uint8_t j = 0; j < HEIGHT; j++) {
        drawPixelXYF(i,HEIGHT-1U-j,ColorFromPalette(*curPalette, qsub8(inoise8(i * deltaValue, (j+ff_y+random8(2)) * deltaHue, ff_z), shiftHue[j]), 255U));
        } 
    }

    for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
        LeapersMove_leaper(i);
        if (trackingObjectPosY[i] >= HEIGHT/4U)
        drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
    };

    ff_y++;
    if (ff_y & 0x01)
        ff_z++;
}

void LLandRoutine(s_mode mode){
    if (loadingFlag) {
        loadingFlag = false;
        setCurrentPalette(mode);
        deltaValue = 10U * ((mode.scale - 1U) % 11U + 1U);// значения от 1 до 11 
    }
    hue2 += 32U;
    if (hue2 < 32U)
        hue++;
    ff_y += 16U;
    
    for (uint8_t y = 0; y < HEIGHT; y++)
        for (uint16_t x = 0; x < WIDTH; x++)
        drawPixelXY(x, y, ColorFromPalette (*curPalette, map(inoise8(x * deltaValue, y * deltaValue - ff_y, ff_z) - y * 255 / (HEIGHT - 1), 0, 255, 205, 255) + hue, 255));
    ff_z++;      
}

void fairyRoutine(s_mode mode){
    if (loadingFlag)
    {
        loadingFlag = false;
        deltaValue = 10; // количество зарождающихся частиц за 1 цикл //perCycle = 1;
        enlargedObjectNUM = (mode.scale - 1U) / 99.0 * (trackingOBJECT_MAX_COUNT - 1U) + 1U;
        if (enlargedObjectNUM > trackingOBJECT_MAX_COUNT) enlargedObjectNUM = trackingOBJECT_MAX_COUNT;
        for(int i = 0; i<enlargedObjectNUM; i++)
        trackingObjectIsShift[i] = false; // particle->isAlive

        // лень было придумывать алгоритм для траектории феи, поэтому это будет нулевой "бойд" из эффекта Притяжение
        boids[0] = Boid(random8(WIDTH), random8(HEIGHT));//WIDTH - 1, HEIGHT - 1);
        boids[0].mass = 0.5;//((float)random8(33U, 134U)) / 100.; // random(0.1, 2); // сюда можно поставить регулятор разлёта. чем меньше число, тем дальше от центра будет вылет
        boids[0].velocity.x = ((float) random8(46U, 100U)) / 500.0;
        if (random8(2U)) boids[0].velocity.x = -boids[0].velocity.x;
        boids[0].velocity.y = 0;
        hue = random8();//boids[0].colorIndex = 
        #ifdef FAIRY_BEHAVIOR
            deltaHue2 = 1U;
        #endif;
    }
    step = deltaValue; //счётчик количества частиц в очереди на зарождение в этом цикле
    
    #ifdef FAIRY_BEHAVIOR
    if (!deltaHue && deltaHue2 && fabs(boids[0].velocity.x) + fabs(boids[0].velocity.y) < 0.15){ 
        deltaHue2 = 0U;
        
        boids[1].velocity.x = ((float)random8()+255.) / 4080.;
        boids[1].velocity.y = ((float)random8()+255.) / 2040.;
        if (boids[0].location.x > WIDTH * 0.5) boids[1].velocity.x = -boids[1].velocity.x;
        if (boids[0].location.y > HEIGHT * 0.5) boids[1].velocity.y = -boids[1].velocity.y;
    }
    if (!deltaHue2){
        step = 1U;
        
        boids[0].location.x += boids[1].velocity.x;
        boids[0].location.y += boids[1].velocity.y;
        deltaHue2 = (boids[0].location.x <= 0 || boids[0].location.x >= WIDTH-1 || boids[0].location.y <= 0 || boids[0].location.y >= HEIGHT-1);
    }
    else
    #endif // FAIRY_BEHAVIOR
    {  
        PVector attractLocation = PVector(WIDTH * 0.5, HEIGHT * 0.5);
        //float attractMass = 10;
        //float attractG = .5;
        // перемножаем и получаем 5.
        Boid boid = boids[0];
        PVector force = attractLocation - boid.location;      // Calculate direction of force
        float d = force.mag();                                // Distance between objects
        d = constrain(d, 5.0f, HEIGHT);//видео снято на 5.0f  // Limiting the distance to eliminate "extreme" results for very close or very far objects
    //d = constrain(d, modes[currentMode].Scale / 10.0, HEIGHT);

        force.normalize();                                    // Normalize vector (distance doesn't matter here, we just want this vector for direction)
        float strength = (5. * boid.mass) / (d * d);          // Calculate gravitional force magnitude 5.=attractG*attractMass
    //float attractMass = (modes[currentMode].Scale) / 10.0 * .5;
    //strength = (attractMass * boid.mass) / (d * d);
        force *= strength;                                    // Get force vector --> magnitude * direction
        boid.applyForce(force);
        boid.update();
        
        if (boid.location.x <= -1) boid.location.x = -boid.location.x;
        else if (boid.location.x >= WIDTH) boid.location.x = -boid.location.x+WIDTH+WIDTH;
        if (boid.location.y <= -1) boid.location.y = -boid.location.y;
        else if (boid.location.y >= HEIGHT) boid.location.y = -boid.location.y+HEIGHT+HEIGHT;
        boids[0] = boid;

        //EVERY_N_SECONDS(20)
        if (!deltaHue){
        if (random8(3U)){
            d = ((random8(2U)) ? boids[0].velocity.x : boids[0].velocity.y) * ((random8(2U)) ? .2 : -.2);
            boids[0].velocity.x += d;
            boids[0].velocity.y -= d;
        }
        else {
            if (fabs(boids[0].velocity.x) < 0.02)
            boids[0].velocity.x = -boids[0].velocity.x;
            else if (fabs(boids[0].velocity.y) < 0.02)
            boids[0].velocity.y = -boids[0].velocity.y;
        }
        }
    }

    //renderer.fade(leds); = fadeToBlackBy(128); = dimAll(255-128)
    //dimAll(255-128/.25*speedfactor); очередной эффект, к которому нужно будет "подобрать коэффициенты"
    //if (modes[currentMode].Speed & 0x01)
        dimAll(127);
    //else FastLED.clear();    

    //go over particles and update matrix cells on the way
    for(int i = 0; i<enlargedObjectNUM; i++) {
        if (!trackingObjectIsShift[i] && step) {
        //emitter->emit(&particles[i], this->g);
        fairyEmit(i);
        step--;
        }
        if (trackingObjectIsShift[i]){ // particle->isAlive
        //particles[i].update(this->g);
        if (mode.scale & 0x01 && trackingObjectSpeedY[i] > -1) trackingObjectSpeedY[i] -= 0.05; //apply acceleration
        particlesUpdate2(i);

        //generate RGB values for particle
        CRGB baseRGB = CHSV(trackingObjectHue[i], 255,255); // particles[i].hue

        //baseRGB.fadeToBlackBy(255-trackingObjectState[i]);
        baseRGB.nscale8(trackingObjectState[i]);//эквивалент
        drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], baseRGB);
        }
    }
    drawPixelXYF(boids[0].location.x, boids[0].location.y, CHSV(hue, 160U, 255U));//boid.colorIndex + hue
}

void MultipleStream8(s_mode mode) { // Windows ))
    if (loadingFlag){
        loadingFlag = false;
        if (mode.scale > 1U)
        hue = (mode.scale - 2U) * 2.6;
        else
        hue = random8();
    }
    if (mode.scale <= 1U)
        hue++;
    dimAll(96); // < -- затухание эффекта для последующего кадра на 96/255*100=37%
    for (uint8_t y = 2; y < HEIGHT-1; y += 5) {
        for (uint8_t x = 2; x < WIDTH-1; x += 5) {
        leds[XY(x, y)]  += CHSV(x * y + hue, 255, 255);
        leds[XY(x + 1, y)] += CHSV((x + 4) * y + hue, 255, 255);
        leds[XY(x, y + 1)] += CHSV(x * (y + 4) + hue, 255, 255);
        leds[XY(x + 1, y + 1)] += CHSV((x + 4) * (y + 4) + hue, 255, 255);
        }
    }
    // Noise
    noise32_x[0] += 3000;
    noise32_y[0] += 3000;
    noise32_z[0] += 3000;
    scale32_x[0] = 8000;
    scale32_y[0] = 8000;
    FillNoise(0);
    
    MoveFractionalNoiseX(3);
    MoveFractionalNoiseY(3);
}

void MultipleStreamSmoke(s_mode mode, boolean isColored){
    if (loadingFlag)
    {
        loadingFlag = false;
        hue2 = 0U;
    }
    dimAll(254U);//(255U - modes[currentMode].Scale * 2);

    deltaHue++;
    CRGB color;//, color2;
    if (isColored)
    {
        if (hue2 == mode.scale)
        {
            hue2 = 0U;
            hue = random8();
        }
        if (deltaHue & 0x01)//((deltaHue >> 2U) == 0U) // какой-то умножитель охота подключить к задержке смены цвета, но хз какой...
        hue2++;

        hsv2rgb_spectrum(CHSV(hue, 255U, 127U), color);
    }
    else {
        hsv2rgb_spectrum(CHSV((mode.scale - 1U) * 2.6, (mode.scale > 98U) ? 0U : 255U, 127U), color);
    }

    if (random8(WIDTH) != 0U) // встречная спираль движется не всегда синхронно основной
        deltaHue2--;

    for (uint8_t y = 0; y < HEIGHT; y++) {
        leds[XY((deltaHue  + y + 1U)%WIDTH, HEIGHT - 1U - y)] += color;
        leds[XY((deltaHue  + y     )%WIDTH, HEIGHT - 1U - y)] += color; //color2
        leds[XY((deltaHue2 + y     )%WIDTH,               y)] += color;
        leds[XY((deltaHue2 + y + 1U)%WIDTH,               y)] += color; //color2
    }

    // скорость движения по массиву noise
    noise32_x[0] += 1500;//1000;
    noise32_y[0] += 1500;//1000;
    noise32_z[0] += 1500;//1000;

    // хрен знает что
    scale32_x[0] = 4000;
    scale32_y[0] = 4000;
    FillNoise(0);

    // допустимый отлёт зажжённого пикселя от изначально присвоенного местоположения (от 0 до указанного значения. дробное) 
    MoveFractionalNoiseX(3);//4
    MoveFractionalNoiseY(3);//4

    blurScreen(20); // без размытия как-то пиксельно, наверное...  
}

void picassoSelector(s_mode mode){
    if (loadingFlag)
    {
        if (mode.scale < 34U)           // если масштаб до 34
        enlargedObjectNUM = (mode.scale - 1U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
        else if (mode.scale >= 68U)      // если масштаб больше 67
        enlargedObjectNUM = (mode.scale - 68U) / 32.0 * (enlargedOBJECT_MAX_COUNT - 3U) + 3U;
        else                                          // для масштабов посередине
        enlargedObjectNUM = (mode.scale - 34U) / 33.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    }
    
    if (mode.scale < 34U)           // если масштаб до 34
        PicassoRoutine();
    else if (mode.scale > 67U)      // если масштаб больше 67
        PicassoRoutine3();
    else                                          // для масштабов посередине
        PicassoRoutine2();
    }

void WaveRoutine(s_mode mode) {
    if (loadingFlag)
    {
      loadingFlag = false;
      setCurrentPalette(mode);//а вот тут явно накосячено. палитры наложены на угол поворота несинхронно, но исправлять особого смысла нет
     
      waveRotation = (mode.scale % 11U) % 4U;
      waveCount = mode.speed & 0x01;//% 2;
    }
 
    dimAll(254);

    int n = 0;

    switch (waveRotation) {
        case 0:
            for (uint8_t x = 0; x < WIDTH; x++) {
                n = quadwave8(x * 2 + waveTheta) / waveScale;
                drawPixelXY(x, n, ColorFromPalette(*curPalette, hue + x));
                if (waveCount != 1)
                    drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(*curPalette, hue + x));
            }
            break;

        case 1:
            for (uint8_t y = 0; y < HEIGHT; y++) {
                n = quadwave8(y * 2 + waveTheta) / waveScale;
                drawPixelXY(n, y, ColorFromPalette(*curPalette, hue + y));
                if (waveCount != 1)
                    drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(*curPalette, hue + y));
            }
            break;

        case 2:
            for (uint8_t x = 0; x < WIDTH; x++) {
                n = quadwave8(x * 2 - waveTheta) / waveScale;
                drawPixelXY(x, n, ColorFromPalette(*curPalette, hue + x));
                if (waveCount != 1)
                    drawPixelXY(x, HEIGHT - 1 - n, ColorFromPalette(*curPalette, hue + x));
            }
            break;

        case 3:
            for (uint8_t y = 0; y < HEIGHT; y++) {
                n = quadwave8(y * 2 - waveTheta) / waveScale;
                drawPixelXY(n, y, ColorFromPalette(*curPalette, hue + y));
                if (waveCount != 1)
                    drawPixelXY(WIDTH - 1 - n, y, ColorFromPalette(*curPalette, hue + y));
            }
            break;
    }


    if (waveThetaUpdate >= waveThetaUpdateFrequency) {
        waveThetaUpdate = 0;
        waveTheta++;
    } else {
        waveThetaUpdate++;
    }

    if (hueUpdate >= hueUpdateFrequency) {
        hueUpdate = 0;
        hue++;
    } else {
        hueUpdate++;
    }
    
    blurScreen(20); // @Palpalych советует делать размытие. вот в этом эффекте его явно не хватает...
}

void lightBallsRoutine(s_mode mode)
{
  // Apply some blurring to whatever's already on the matrix
  // Note that we never actually clear the matrix, we just constantly
  // blur it repeatedly. Since the blurring is 'lossy', there's
  // an automatic trend toward black -- by design.
  //uint8_t blurAmount = dim8_raw(beatsin8(3, 64, 100));
  //blur2d(leds, WIDTH, HEIGHT, blurAmount);
  blurScreen(dim8_raw(beatsin8(3, 64, 100)));

  // Use two out-of-sync sine waves
  uint16_t i = beatsin16( 79, 0, 255); //91
  uint16_t j = beatsin16( 67, 0, 255); //109
  uint16_t k = beatsin16( 53, 0, 255); //73
  uint16_t m = beatsin16( 97, 0, 255); //123

  // The color of each point shifts over time, each at a different speed.
  uint32_t ms = millis() / (mode.scale / 4 + 1);
  leds[XY( highByte(i * paintWidth) + BORDERTHICKNESS, highByte(j * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 29, 200U, 255U);
  leds[XY( highByte(j * paintWidth) + BORDERTHICKNESS, highByte(k * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 41, 200U, 255U);
  leds[XY( highByte(k * paintWidth) + BORDERTHICKNESS, highByte(m * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 37, 200U, 255U);
  leds[XY( highByte(m * paintWidth) + BORDERTHICKNESS, highByte(i * paintHeight) + BORDERTHICKNESS)] += CHSV( ms / 53, 200U, 255U);
}

void lumenjerRoutine(s_mode mode) {
    if (loadingFlag)
    {
        loadingFlag = false;
        if (mode.scale > 100) mode.scale = 100; // чтобы не было проблем при прошивке без очистки памяти    
        if (mode.scale > 50) 
        curPalette = firePalettes[(uint8_t)((mode.scale - 50)/50.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
        else
        curPalette = palette_arr[(uint8_t)(mode.scale/50.0F * ((sizeof(palette_arr)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
        
        deltaHue = -1;
        deltaHue2 = -1;
        dimAll(245U);
    }
    dimAll(DIMSPEED);

    deltaHue = random8(3) ? deltaHue : -deltaHue;
    deltaHue2 = random8(3) ? deltaHue2 : -deltaHue2;
    #if (WIDTH % 2 == 0 && HEIGHT % 2 == 0)
    hue = (WIDTH + hue + (int8_t)deltaHue * (bool)random8(64)) % WIDTH;
    #else
    hue = (WIDTH + hue + (int8_t)deltaHue) % WIDTH;
    #endif
    hue2 = (HEIGHT + hue2 + (int8_t)deltaHue2) % HEIGHT;

    if (mode.scale == 100U)
        leds[XY(hue, hue2)] += CHSV(random8(), 255U, 255U);
    else
        leds[XY(hue, hue2)] += ColorFromPalette(*curPalette, step++);
}

void stormRoutine2(s_mode mode)
{
    bool isColored = mode.speed & 0x01; // сворачиваем 2 эффекта в 1
    // заполняем головами комет
    uint8_t Saturation = 0U;    // цвет хвостов
    uint8_t e_TAIL_STEP = 127U; // длина хвоста
    if (isColored)
        Saturation = mode.scale * 2.55;
    else
    {
        e_TAIL_STEP = 255U - mode.scale * 2.5;
    }
    for (uint8_t x = 0U; x < WIDTH - 1U; x++) // fix error i != 0U
    {
        if (!random8(e_sns_DENSE) &&
            !getPixColorXY(wrapX(x), HEIGHT - 1U) &&
            !getPixColorXY(wrapX(x + 1U), HEIGHT - 1U) &&
            !getPixColorXY(wrapX(x - 1U), HEIGHT - 1U))
        {
        drawPixelXY(x, HEIGHT - 1U, CHSV(random8(), Saturation, random8(64U, 255U)));
        }
    }

    // сдвигаем по диагонали
    for (uint8_t y = 0U; y < HEIGHT - 1U; y++)
    {
        for (uint8_t x = 0; x < WIDTH; x++)
        {
        drawPixelXY(wrapX(x + 1U), y, getPixColorXY(x, y + 1U));
        }
    }

    // уменьшаем яркость верхней линии, формируем "хвосты"
    for (uint8_t i = 0U; i < WIDTH; i++)
    {
        fadePixel(i, HEIGHT - 1U, e_TAIL_STEP);
    }
}

void stormyRain(s_mode mode)
{
  rain(60, 160, (mode.scale-1) * 2.58, 30, solidRainColor, true, true, true);
}

void ballsRoutine(s_mode mode)
{
  if (loadingFlag)
  {
    loadingFlag = false;

    for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
    {
      int8_t sign;
      // забиваем случайными данными
      coord[j][0U] = WIDTH / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][0U] = random(4, 15) * sign;
      coord[j][1U] = HEIGHT / 2 * 10;
      random(0, 2) ? sign = 1 : sign = -1;
      vector[j][1U] = random(4, 15) * sign;
      //ballColors[j] = CHSV(random(0, 9) * 28, 255U, 255U);
      // цвет зависит от масштаба
      ballColors[j] = CHSV((mode.scale * (j + 1)) % 256U, 255U, 255U);
    }
  }

  if (!BALL_TRACK)                                          // режим без следов шариков
  {
    FastLED.clear();
  }
  else                                                      // режим со следами
  {
    //fader(TRACK_STEP);
    dimAll(256U - TRACK_STEP);
  }

  // движение шариков
  for (uint8_t j = 0U; j < BALLS_AMOUNT; j++)
  {
    // движение шариков
    for (uint8_t i = 0U; i < 2U; i++)
    {
      coord[j][i] += vector[j][i];
      if (coord[j][i] < 0)
      {
        coord[j][i] = 0;
        vector[j][i] = -vector[j][i];
      }
    }

    if (coord[j][0U] > (int16_t)((WIDTH - 1) * 10))
    {
      coord[j][0U] = (WIDTH - 1) * 10;
      vector[j][0U] = -vector[j][0U];
    }
    if (coord[j][1U] > (int16_t)((HEIGHT - 1) * 10))
    {
      coord[j][1U] = (HEIGHT - 1) * 10;
      vector[j][1U] = -vector[j][1U];
    }
    drawPixelXYF(coord[j][0U] / 10., coord[j][1U] / 10., ballColors[j]);
  }
}

void ringsRoutine(s_mode mode){
    uint8_t h, x, y;
    if (loadingFlag)
    {
      loadingFlag = false;
      setCurrentPalette(mode);

      //deltaHue2 = (mode.scale - 1U) / 99.0 * (HEIGHT / 2 - 1U) + 1U; // толщина кольца в пикселях. если на весь бегунок масштаба (от 1 до HEIGHT / 2 + 1)
      deltaHue2 = (mode.scale - 1U) % 11U + 1U; // толщина кольца от 1 до 11 для каждой из палитр
      deltaHue = HEIGHT / deltaHue2 + ((HEIGHT % deltaHue2 == 0U) ? 0U : 1U); // количество колец
      hue2 = deltaHue2 - (deltaHue2 * deltaHue - HEIGHT) / 2U; // толщина верхнего кольца. может быть меньше нижнего
      hue = HEIGHT - hue2 - (deltaHue - 2U) * deltaHue2; // толщина нижнего кольца = всё оставшееся
      for (uint8_t i = 0; i < deltaHue; i++)
      {
        noise3d[0][0][i] = random8(257U - WIDTH / 2U); // начальный оттенок кольца из палитры 0-255 за минусом длины кольца, делённой пополам
        shiftHue[i] = random8();
        shiftValue[i] = 0U; //random8(WIDTH); само прокрутится постепенно
        step = 0U;
        //do { // песец конструкцию придумал бредовую
        //  step = WIDTH - 3U - random8((WIDTH - 3U) * 2U); само присвоится при первом цикле
        //} while (step < WIDTH / 5U || step > 255U - WIDTH / 5U);
        deltaValue = random8(deltaHue);
      }
      
    }
    for (uint8_t i = 0; i < deltaHue; i++)
    {
      if (i != deltaValue) // если это не активное кольцо
        {
          h = shiftHue[i] & 0x0F; // сдвигаем оттенок внутри кольца
          if (h > 8U)
            //noise3d[0][0][i] += (uint8_t)(7U - h); // с такой скоростью сдвиг оттенка от вращения кольца не отличается
            noise3d[0][0][i]--;
          else
            //noise3d[0][0][i] += h;
            noise3d[0][0][i]++;
        }
      else
        {
          if (step == 0) // если сдвиг активного кольца завершён, выбираем следующее
            {
              deltaValue = random8(deltaHue);
              do {
                step = WIDTH - 3U - random8((WIDTH - 3U) * 2U); // проворот кольца от хз до хз 
              } while (step < WIDTH / 5U || step > 255U - WIDTH / 5U);
            }
          else
            {
              if (step > 127U)
                {
                  step++;
                  shiftValue[i] = (shiftValue[i] + 1U) % WIDTH;
                }
              else
                {
                  step--;
                  shiftValue[i] = (shiftValue[i] - 1U + WIDTH) % WIDTH;
                }
            }
        }
        // отрисовываем кольца
        h = (shiftHue[i] >> 4) & 0x0F; // берём шаг для градиента вутри кольца
        if (h > 8U)
          h = 7U - h;
        for (uint8_t j = 0U; j < ((i == 0U) ? hue : ((i == deltaHue - 1U) ? hue2 : deltaHue2)); j++) // от 0 до (толщина кольца - 1)
        {
          y = i * deltaHue2 + j - ((i == 0U) ? 0U : deltaHue2 - hue);
          // mod для чётных скоростей by @kostyamat - получается какая-то другая фигня. не стоит того
          //for (uint8_t k = 0; k < WIDTH / ((mode.speed & 0x01) ? 2U : 4U); k++) // полукольцо для нечётных скоростей и четверть кольца для чётных
          for (uint8_t k = 0; k < WIDTH / 2U; k++) // полукольцо
            {
              x = (shiftValue[i] + k) % WIDTH; // первая половина кольца
              leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + k * h);
              x = (WIDTH - 1 + shiftValue[i] - k) % WIDTH; // вторая половина кольца (зеркальная первой)
              leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + k * h);
            }
          if (WIDTH & 0x01) //(WIDTH % 2U > 0U) // если число пикселей по ширине матрицы нечётное, тогда не забываем и про среднее значение
          {
            x = (shiftValue[i] + WIDTH / 2U) % WIDTH;
            leds[XY(x, y)] = ColorFromPalette(*curPalette, noise3d[0][0][i] + WIDTH / 2U * h);
          }
        }
    }
}

void cube2dRoutine(s_mode mode){
    uint8_t x, y;
    uint8_t anim0; // будем считать тут начальный пиксель для анимации сдвига строки/колонки
    int8_t shift, kudaVse; // какое-то расчётное направление сдвига (-1, 0, +1)
    CRGB color, color2;
    
    if (loadingFlag)
    {
      loadingFlag = false;
      setCurrentPalette(mode);
      FastLED.clear();

      razmerX = (mode.scale - 1U) % 11U + 1U; // размер ячейки от 1 до 11 пикселей для каждой из 9 палитр
      razmerY = razmerX;
      if (mode.speed & 0x01) // по идее, ячейки не обязательно должны быть квадратными, поэтому можно тут поизвращаться
        razmerY = (razmerY << 1U) + 1U;

      shtukY = HEIGHT / (razmerY + 1U);
      if (shtukY < 2U)
        shtukY = 2U;
      y = HEIGHT / shtukY - 1U;
      if (razmerY > y)
        razmerY = y;
      poleY = (razmerY + 1U) * shtukY;
      shtukX = WIDTH / (razmerX + 1U);
      if (shtukX < 2U)
        shtukX = 2U;
      x = WIDTH / shtukX - 1U;
      if (razmerX > x)
        razmerX = x;
      poleX = (razmerX + 1U) * shtukX;
      seamlessX = (poleX == WIDTH);
      deltaHue = 0U;
      deltaHue2 = 0U;
      globalShiftX = 0;
      globalShiftY = 0;

      for (uint8_t j = 0U; j < shtukY; j++)
      {
        y = j * (razmerY + 1U); // + deltaHue2 т.к. оно =0U
        for (uint8_t i = 0U; i < shtukX; i++)
        {
          x = i * (razmerX + 1U); // + deltaHue т.к. оно =0U
          if (mode.scale == 100U)
            color = CHSV(45U, 0U, 128U + random8(128U));
          else  
            color = ColorFromPalette(*curPalette, random8());
          for (uint8_t k = 0U; k < razmerY; k++)
            for (uint8_t m = 0U; m < razmerX; m++)
              leds[XY(x+m, y+k)] = color;
        }
      }
      step = 4U; // текущий шаг сдвига первоначально с перебором (от 0 до deltaValue-1)
      deltaValue = 4U; // всего шагов сдвига (от razmer? до (razmer?+1) * shtuk?)
      hue2 = 0U; // осталось шагов паузы
    }

  //двигаем, что получилось...
  if (hue2 == 0 && step < deltaValue) // если пауза закончилась, а цикл вращения ещё не завершён
  {
    step++;
    if (krutimVertikalno)
    {
      for (uint8_t i = 0U; i < shtukX; i++)
      {
        x = (deltaHue + i * (razmerX + 1U)) % WIDTH;
        if (noise3d[0][i][0] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          noise3d[0][i][0]--;
          shift = noise3d[0][i][1] - 1; // в первой ячейке храним направление прокрутки

          if (globalShiftY == 0)
            anim0 = (deltaHue2 == 0U) ? 0U : deltaHue2 - 1U;
          else if (globalShiftY > 0)
            anim0 = deltaHue2;
          else
            anim0 = deltaHue2 - 1U;
          
          if (shift < 0) // если крутим столбец вниз
          {
            color = leds[XY(x, anim0)];                                   // берём цвет от нижней строчки
            for (uint8_t k = anim0; k < anim0+poleY-1; k++)
            {
              color2 = leds[XY(x,k+1)];                                   // берём цвет от строчки над нашей
              for (uint8_t m = x; m < x + razmerX; m++)
                leds[XY(m % WIDTH,k)] = color2;                           // копируем его на всю нашу строку
            }
            for   (uint8_t m = x; m < x + razmerX; m++)
              leds[XY(m % WIDTH,anim0+poleY-1)] = color;                  // цвет нижней строчки копируем на всю верхнюю
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = leds[XY(x,anim0+poleY-1)];                            // берём цвет от верхней строчки
            for (uint8_t k = anim0+poleY-1; k > anim0 ; k--)
            {
              color2 = leds[XY(x,k-1)];                                   // берём цвет от строчки под нашей
              for (uint8_t m = x; m < x + razmerX; m++)
                leds[XY(m % WIDTH,k)] = color2;                           // копируем его на всю нашу строку
            }
            for   (uint8_t m = x; m < x + razmerX; m++)
              leds[XY(m % WIDTH, anim0)] = color;                         // цвет верхней строчки копируем на всю нижнюю
          }
        }
      }
    }
    else
    {
      for (uint8_t j = 0U; j < shtukY; j++)
      {
        y = deltaHue2 + j * (razmerY + 1U);
        if (noise3d[0][0][j] > 0) // в нулевой ячейке храним оставшееся количество ходов прокрутки
        {
          noise3d[0][0][j]--;
          shift = noise3d[0][1][j] - 1; // в первой ячейке храним направление прокрутки
      
          if (seamlessX)
            anim0 = 0U;
          else if (globalShiftX == 0)
            anim0 = (deltaHue == 0U) ? 0U : deltaHue - 1U;
          else if (globalShiftX > 0)
            anim0 = deltaHue;
          else
            anim0 = deltaHue - 1U;
          
          if (shift < 0) // если крутим строку влево
          {
            color = leds[XY(anim0, y)];                            // берём цвет от левой колонки (левого пикселя)
            for (uint8_t k = anim0; k < anim0+poleX-1; k++)
            {
              color2 = leds[XY(k+1, y)];                           // берём цвет от колонки (пикселя) правее
              for (uint8_t m = y; m < y + razmerY; m++)
                leds[XY(k, m)] = color2;                           // копируем его на всю нашу колонку
            }
            for   (uint8_t m = y; m < y + razmerY; m++)
              leds[XY(anim0+poleX-1, m)] = color;                  // цвет левой колонки копируем на всю правую
          }
          else if (shift > 0) // если крутим столбец вверх
          {
            color = leds[XY(anim0+poleX-1, y)];                    // берём цвет от правой колонки
            for (uint8_t k = anim0+poleX-1; k > anim0 ; k--)
            {
              color2 = leds[XY(k-1, y)];                           // берём цвет от колонки левее
              for (uint8_t m = y; m < y + razmerY; m++)
                leds[XY(k, m)] = color2;                           // копируем его на всю нашу колонку
            }
            for   (uint8_t m = y; m < y + razmerY; m++)
              leds[XY(anim0, m)] = color;                          // цвет правой колонки копируем на всю левую
          }
        }
      }
    }
   
  }
  else if (hue2 != 0U) // пропускаем кадры после прокрутки кубика (делаем паузу)
    hue2--;

  if (step >= deltaValue) // если цикл вращения завершён, меняем местами соответствующие ячейки (цвет в них) и точку первой ячейки
    {
      step = 0U; 
      hue2 = PAUSE_MAX;
      //если часть ячеек двигалась на 1 пиксель, пододвигаем глобальные координаты начала
      deltaHue2 = deltaHue2 + globalShiftY; //+= globalShiftY;
      globalShiftY = 0;
      //deltaHue += globalShiftX; для бесшовной не годится
      deltaHue = (WIDTH + deltaHue + globalShiftX) % WIDTH;
      globalShiftX = 0;

      //пришла пора выбрать следующие параметры вращения
      kudaVse = 0;
      krutimVertikalno = random8(2U);
      if (krutimVertikalno) // идём по горизонтали, крутим по вертикали (столбцы двигаются)
      {
        for (uint8_t i = 0U; i < shtukX; i++)
        {
          noise3d[0][i][1] = random8(3);
          shift = noise3d[0][i][1] - 1; // в первой ячейке храним направление прокрутки
          if (kudaVse == 0)
            kudaVse = shift;
          else if (shift != 0 && kudaVse != shift)
            kudaVse = 50;
        }
        deltaValue = razmerY + ((deltaHue2 - kudaVse >= 0 && deltaHue2 - kudaVse + poleY < (int)HEIGHT) ? random8(2U) : 1U);

        if (deltaValue == razmerY) // значит полюбому kudaVse было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
        {
          globalShiftY = 1 - kudaVse; //временно на единичку больше, чем надо
          for (uint8_t i = 0U; i < shtukX; i++)
            if (noise3d[0][i][1] == 1U) // если ячейка никуда не планировала двигаться
            {
              noise3d[0][i][1] = globalShiftY;
              noise3d[0][i][0] = 1U; // в нулевой ячейке храним количество ходов сдвига
            }
            else
              noise3d[0][i][0] = deltaValue; // в нулевой ячейке храним количество ходов сдвига
          globalShiftY--;
        }
        else
        {
          x = 0;
          for (uint8_t i = 0U; i < shtukX; i++)
            if (noise3d[0][i][1] != 1U)
              {
                y = random8(shtukY);
                if (y > x)
                  x = y;
                noise3d[0][i][0] = deltaValue * (x + 1U); // в нулевой ячейке храним количество ходов сдвига
              }  
          deltaValue = deltaValue * (x + 1U);
        }      
              
      }
      else // идём по вертикали, крутим по горизонтали (строки двигаются)
      {
        for (uint8_t j = 0U; j < shtukY; j++)
        {
          noise3d[0][1][j] = random8(3);
          shift = noise3d[0][1][j] - 1; // в первой ячейке храним направление прокрутки
          if (kudaVse == 0)
            kudaVse = shift;
          else if (shift != 0 && kudaVse != shift)
            kudaVse = 50;
        }
        if (seamlessX)
          deltaValue = razmerX + ((kudaVse < 50) ? random8(2U) : 1U);
        else  
          deltaValue = razmerX + ((deltaHue - kudaVse >= 0 && deltaHue - kudaVse + poleX < (int)WIDTH) ? random8(2U) : 1U);
        
        if (deltaValue == razmerX) // значит полюбому kudaVse было = (-1, 0, +1) - и для нуля в том числе мы двигаем весь куб на 1 пиксель
        {
          globalShiftX = 1 - kudaVse; //временно на единичку больше, чем надо
          for (uint8_t j = 0U; j < shtukY; j++)
            if (noise3d[0][1][j] == 1U) // если ячейка никуда не планировала двигаться
            {
              noise3d[0][1][j] = globalShiftX;
              noise3d[0][0][j] = 1U; // в нулевой ячейке храним количество ходов сдвига
            }
            else
              noise3d[0][0][j] = deltaValue; // в нулевой ячейке храним количество ходов сдвига
          globalShiftX--;
        }
        else
        {
          y = 0;
          for (uint8_t j = 0U; j < shtukY; j++)
            if (noise3d[0][1][j] != 1U)
              {
                x = random8(shtukX);
                if (x > y)
                  y = x;
                noise3d[0][0][j] = deltaValue * (x + 1U); // в нулевой ячейке храним количество ходов сдвига
              }  
          deltaValue = deltaValue * (y + 1U);
        }      
      }
   }
}

void LeapersRoutine(s_mode mode){
  //unsigned num = map(scale, 0U, 255U, 6U, sizeof(boids) / sizeof(*boids));
  if (loadingFlag)
  {
    loadingFlag = false;
    setCurrentPalette(mode);    
    enlargedObjectNUM = (mode.scale - 1U) % 11U / 10.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;

    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);

      trackingObjectHue[i] = random8();
    }
  }

  FastLED.clear();

  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    LeapersMove_leaper(i);
    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], ColorFromPalette(*curPalette, trackingObjectHue[i]));
  };

  blurScreen(20);
}

void newMatrixRoutine(s_mode mode)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    setCurrentPalette(mode);

    enlargedObjectNUM = map(mode.speed, 1, 255, 1, trackingOBJECT_MAX_COUNT);
    speedfactor = 0.136f; // фиксируем хорошую скорость

    for (uint8_t i = 0U; i < enlargedObjectNUM; i++)
    {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT);
      trackingObjectSpeedY[i] = random8(150, 250) / 100.; 
      trackingObjectState[i] = random8(127U, 255U);
    }
   hue = mode.scale * 2.55;
  } 
  dimAll(246); // для фиксированной скорости
  
  CHSV color;

  for (uint8_t i = 0U; i < enlargedObjectNUM; i++)
  {
    trackingObjectPosY[i] -= trackingObjectSpeedY[i]*speedfactor;

    if (mode.scale == 100U) {
      color = rgb2hsv_approximate(CRGB::Gray);
      color.val = trackingObjectState[i];
    } else if (mode.scale == 1U) {
      color = CHSV(++hue, 255, trackingObjectState[i]);
    } else {
      color = CHSV(hue, 255, trackingObjectState[i]);
    }

    drawPixelXYF(trackingObjectPosX[i], trackingObjectPosY[i], color);

    #define GLUK 20 // вероятность горизонтального сдвига капли
    if (random8() < GLUK) {
      //trackingObjectPosX[i] = trackingObjectPosX[i] + random(-1, 2);
      trackingObjectPosX[i] = (uint8_t)(trackingObjectPosX[i] + WIDTH - 1U + random8(3U)) % WIDTH ;
      trackingObjectState[i] = random8(196,255);
    }

    if(trackingObjectPosY[i] < -1) {
      trackingObjectPosX[i] = random8(WIDTH);
      trackingObjectPosY[i] = random8(HEIGHT - HEIGHT /2, HEIGHT);
      trackingObjectSpeedY[i] = random8(150, 250) / 100.; 
      trackingObjectState[i] = random8(127U, 255U);
      //trackingObjectHue[i] = hue; не похоже, что цвет используется
    }
  }
}

void polarRoutine(s_mode mode) {
    if (loadingFlag) {
        loadingFlag = false;
        emitterX = 400. / HEIGHT; // а это - максимум без яркой засветки крайних рядов матрицы (сверху и снизу)
        
        ff_y = map(WIDTH, 8, 64, 310, 63);
        ff_z = ff_y;
        speedfactor = map(mode.speed, 1, 255, 128, 16); // _speed = map(speed, 1, 255, 128, 16);

    }
  
    if (mode.scale == 100){
        if (hue2++ & 0x01 && deltaHue++ & 0x01 && deltaHue2++ & 0x01) hue++; // это ж бред, но я хз. как с 60ю кадрами в секунду можно эффективно скорость замедлять...
        fillMyPal16_2((uint8_t)((mode.scale - 1U) * 2.55) + hue, mode.scale & 0x01);
    } else
        fillMyPal16_2((uint8_t)((mode.scale - 1U) * 2.55) + AURORA_COLOR_RANGE - beatsin8(AURORA_COLOR_PERIOD, 0U, AURORA_COLOR_RANGE+AURORA_COLOR_RANGE), 
            mode.scale & 0x01);

    for (byte x = 0; x < WIDTH; x++) {
        for (byte y = 0; y < HEIGHT; y++) {
        polarTimer++;
        //uint16_t i = x*y;
        leds[XY(x, y)]= 
            ColorFromPalette(myPal,
                qsub8(
                inoise8(polarTimer % 2 + x * ff_z,
                    y * 16 + polarTimer % 16,
                    polarTimer / speedfactor
                ),
                fabs((float)HEIGHT/2 - (float)y) * emitterX
                )
            );
        }
    }
}

void DNARoutine(s_mode mode)
{
  if (loadingFlag)
  {
    loadingFlag = false;
    step = map8(mode.speed, 10U, 60U);
    hue = mode.scale;
    deltaHue = hue > 50U;
    if (deltaHue)
      hue = 101U - hue;
    hue = 255U - map( 51U - hue, 1U, 50U, 0, 255U);
  }
  double freq = 3000;
  float mn =255.0/13.8;
  
  fadeToBlackBy(leds, NUM_LEDS, step);
  uint16_t ms = millis();
  
    if (deltaHue)
        for (uint8_t i = 0; i < WIDTH; i++) {
            uint32_t x = beatsin16(step, 0, (HEIGHT - 1) * 256, 0, i * freq);
            uint32_t y = i * 256;
            uint32_t x1 = beatsin16(step, 0, (HEIGHT - 1) * 256, 0, i * freq + 32768);

            CRGB col = CHSV(ms / 29 + i * 255 / (WIDTH - 1), 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn)));
            CRGB col1 = CHSV(ms / 29 + i * 255 / (WIDTH - 1) + 128, 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn + 128)));
            wu_pixel(y , x, &col);
            wu_pixel(y , x1, &col1);
        } 
    else
        for (uint8_t i = 0; i < HEIGHT; i++) {
            uint32_t x = beatsin16(step, 0, (WIDTH - 1) * 256, 0, i * freq);
            uint32_t y = i * 256;
            uint32_t x1 = beatsin16(step, 0, (WIDTH - 1) * 256, 0, i * freq + 32768);

            CRGB col = CHSV(ms / 29 + i * 255 / (HEIGHT - 1), 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn)));
            CRGB col1 = CHSV(ms / 29 + i * 255 / (HEIGHT - 1) + 128, 255, qadd8(hue, beatsin8(step, 60, 255U, 0, i * mn + 128)));
            wu_pixel (x , y, &col);
            wu_pixel (x1 , y, &col1);
        }
    blurScreen(16);
}

void Fire2021Routine(s_mode mode){
  if (loadingFlag){ 
    loadingFlag = false;
    if (mode.scale > 100U) mode.scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
    deltaValue = mode.scale * 0.0899;// /100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F));
    if (deltaValue == 3U ||deltaValue == 4U)
      curPalette =  palette_arr[deltaValue]; // (uint8_t)(mode.scale/100.0F * ((sizeof(palette_arr) /sizeof(TProgmemRGBPalette16 *))-0.01F))];
    else
      curPalette = firePalettes[deltaValue]; // (uint8_t)(mode.scale/100.0F * ((sizeof(firePalettes)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
    deltaValue = (mode.scale - 1U) % 11U + 1U;
    if (mode.speed & 0x01){
      ff_x = mode.speed;
      deltaHue2 = FIXED_SCALE_FOR_Y;
    }
    else{
      if (deltaValue > FIXED_SCALE_FOR_Y)
        speedfactor = .4 * (deltaValue - FIXED_SCALE_FOR_Y) + FIXED_SCALE_FOR_Y;
      else
        speedfactor = deltaValue;
      ff_x = round(mode.speed*64./(0.1686*speedfactor*speedfactor*speedfactor - 1.162*speedfactor*speedfactor + 3.6694*speedfactor + 56.394)); // Ааааа! это тупо подбор коррекции. очень приблизитеьный
      deltaHue2 = deltaValue;
    }
    if (ff_x > 255U) 
      ff_x = 255U;
    if (ff_x == 0U) 
      ff_x = 1U;
    step = map(ff_x * ff_x, 1U, 65025U, (deltaHue2-1U)/2U+1U, deltaHue2 * 18U + 44);
    pcnt = map(step, 1U, 255U, 20U, 128U); // nblend 3th param
    deltaValue = 0.7 * deltaValue * deltaValue + 31.3; // ширина языков пламени (масштаб шума Перлина)
    deltaHue2 = 0.7 * deltaHue2 * deltaHue2 + 31.3; // высота языков пламени (масштаб шума Перлина)
  }
  
  ff_y += step; //static uint32_t t += speed;
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT; y++) {
      int16_t Bri = inoise8(x * deltaValue, (y * deltaHue2) - ff_y, ff_z) - (y * (255 / HEIGHT));
      byte Col = Bri;//inoise8(x * deltaValue, (y * deltaValue) - ff_y, ff_z) - (y * (255 / HEIGHT));
      if (Bri < 0) 
        Bri = 0; 
      if (Bri != 0) 
        Bri = 256 - (Bri * 0.2);
      //leds[XY(x, y)] = ColorFromPalette(*curPalette, Col, Bri);
      nblend(leds[XY(x, y)], ColorFromPalette(*curPalette, Col, Bri), pcnt);
    }
  }
  if (!random8())
    ff_z++;
}

void execStringsFlame(s_mode mode){ // внимание! эффект заточен на бегунок Масштаб с диапазоном от 0 до 255
  int16_t i,j;
  if (loadingFlag){ 
    loadingFlag = false;
    enlargedObjectNUM = (mode.speed - 1U) / 254.0 * (trackingOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
/*!!!  потом с этим разобраться  
    if (currentMode >= EFF_MATRIX) {
      ff_x = WIDTH * 2.4;
      enlargedObjectNUM = (ff_x > enlargedOBJECT_MAX_COUNT) ? enlargedOBJECT_MAX_COUNT : ff_x;
    }
*/
     hue = map8(myScale8(mode.scale+3U),3,10); // минимальная живучесть/высота языка пламени ...ttl
    hue2 = map8(myScale8(mode.scale+3U),6,31); // максимальная живучесть/высота языка пламени ...ttl
    for (i = 0; i < trackingOBJECT_MAX_COUNT; i++) // чистим массив объектов от того, что не похоже на языки пламени
      if (trackingObjectState[i] > 30U || trackingObjectPosY[i] >= HEIGHT || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= 0){
        trackingObjectHue[i] = 0U;
        trackingObjectState[i] = random8(20);
      }
    for (i=0; i < WIDTH; i++) // заполняем массив изображения из массива leds обратным преобразованием, которое нихрена не работает
      for (j=0; j < HEIGHT; j++ ) {
        CHSV tHSV = rgb2hsv_approximate(leds[XY(i,j)]);
        noise3d[0][i][j] = tHSV.hue;
        if (tHSV.val > 100U){ // такая защита от пересвета более-менее достаточна
          shiftValue[j] = tHSV.sat;
          if (tHSV.sat < 100U) // для перехода с очень тусклых эффектов, использующих заливку белым или почти белым светом
            noise3d[1][i][j] = tHSV.val / 3U;
          else
            noise3d[1][i][j] = tHSV.val - 32U;
        }
        else
          noise3d[1][i][j] = 0U;
          
        //CRGB tRGB = leds[XY(i,j)];
        //if (tRGB.r + tRGB.g + tRGB.b < 100U) // не пригодилось
        //  noise3d[1][i][j] = 0U;
      }
  }

  // угасание предыдущего кадра
  for (i=0; i < WIDTH; i++)
    for (j=0; j < HEIGHT; j++ )
      noise3d[1][i][j] = (uint16_t)noise3d[1][i][j] * 237U >> 8;

  // цикл перебора языков пламени
  for (i=0; i < enlargedObjectNUM; i++) {
    if (trackingObjectState[i]) { // если ещё не закончилась его жизнь
      wu_pixel_maxV(i);

      j = trackingObjectState[i];
      trackingObjectState[i]--;

      trackingObjectPosX[i] += trackingObjectSpeedX[i];
      trackingObjectPosY[i] += trackingObjectSpeedY[i];

      trackingObjectHue[i] = (trackingObjectState[i] * trackingObjectHue[i] + j / 2) / j;

      // если вышел за верхнюю границу или потух, то и жизнь закончилась
      if (trackingObjectPosY[i] >= HEIGHT || trackingObjectHue[i] < 2U)
        trackingObjectState[i] = 0;

      // если вылез за край матрицы по горизонтали, перекинем на другую сторону
      if (trackingObjectPosX[i] < 0)
        trackingObjectPosX[i] += WIDTH;
      else if (trackingObjectPosX[i] >= WIDTH)
        trackingObjectPosX[i] -= WIDTH;
    }
    else{ // если жизнь закончилась, перезапускаем
      trackingObjectState[i] = random8(hue, hue2);
      trackingObjectShift[i] = (uint8_t)(254U + mode.scale + random8(20U)); // 254 - это шаг в обратную сторону от выбранного пользователем оттенка (стартовый оттенок диапазона)
                                                                                          // 20 - это диапазон из градиента цвета от выбранного пользователем оттенка (диапазон от 254 до 254+20)
      trackingObjectPosX[i] = (float)random(WIDTH * 255U) / 255.;
      trackingObjectPosY[i] = -.9;
      trackingObjectSpeedX[i] = (float)(FLAME_MIN_DX + random8(FLAME_MAX_DX-FLAME_MIN_DX)) / 256.;
      trackingObjectSpeedY[i] = (float)(FLAME_MIN_DY + random8(FLAME_MAX_DY-FLAME_MIN_DY)) / 256.;
      trackingObjectHue[i] = FLAME_MIN_VALUE + random8(FLAME_MAX_VALUE - FLAME_MIN_VALUE + 1U);
      //saturation = 255U;
    }
  }

  //выводим кадр на матрицу
  for (i=0; i<WIDTH; i++)
    for (j=0; j<HEIGHT; j++)
      //hsv2rgb_spectrum(CHSV(noise3d[0][i][j], shiftValue[j], noise3d[1][i][j] * 1.033), leds[XY(i,j)]); // 1.033 - это коэффициент нормализации яркости (чтобы чутка увеличить яркость эффекта в целом)
      hsv2rgb_spectrum(CHSV(noise3d[0][i][j], shiftValue[j], noise3d[1][i][j]), leds[XY(i,j)]);
}

void smokeballsRoutine(s_mode mode){
  if (loadingFlag)
  {
    loadingFlag = false;
    setCurrentPalette(mode);
    
    enlargedObjectNUM = enlargedObjectNUM = (mode.scale - 1U) % 11U + 1U;
    speedfactor = fmap(mode.speed, 1., 255., .02, .1); // попробовал разные способы управления скоростью. Этот максимально приемлемый, хотя и сильно тупой.
    //randomSeed(millis());
    for (byte j = 0; j < enlargedObjectNUM; j++) {
      trackingObjectShift[j] =  random((WIDTH * 10) - ((WIDTH / 3) * 20)); // сумма trackingObjectState + trackingObjectShift не должна выскакивать за макс.Х
      //trackingObjectSpeedX[j] = EffectMath::randomf(5., (float)(16 * WIDTH)); //random(50, 16 * WIDTH) / random(1, 10);
      trackingObjectSpeedX[j] = (float)random(25, 80 * WIDTH) / 5.;
      trackingObjectState[j] = random((WIDTH / 2) * 10, (WIDTH / 3) * 20);
      trackingObjectHue[j] = random8();//(9) * 28;
      trackingObjectPosX[j] = trackingObjectShift[j];
    }
  }
  
  //shiftUp();
  for (byte x = 0; x < WIDTH; x++) {
    for (float y = (float)HEIGHT; y > 0.; y-= speedfactor) {
      drawPixelXY(x, y, getPixColorXY(x, y - 1));
    }
  }
  
  //dimAll(240); фиксированное число - очень плохо, когда матрицы разной высоты // fadeToBlackBy(leds, NUM_LEDS, 10);
  fadeToBlackBy(leds, NUM_LEDS, 128U / HEIGHT);
if (mode.speed & 0x01)
  blurScreen(20);
  for (byte j = 0; j < enlargedObjectNUM; j++) {
    trackingObjectPosX[j] = beatsin16((uint8_t)(trackingObjectSpeedX[j] * (speedfactor * 5.)), trackingObjectShift[j], trackingObjectState[j] + trackingObjectShift[j], trackingObjectHue[j]*256, trackingObjectHue[j]*8);
    drawPixelXYF(trackingObjectPosX[j] / 10., 0.05, ColorFromPalette(*curPalette, trackingObjectHue[j]));
  }

  EVERY_N_SECONDS(20){
    for (byte j = 0; j < enlargedObjectNUM; j++) {
      trackingObjectShift[j] += random(-20,20);
      trackingObjectHue[j] += 28;
    }
  }

  loadingFlag = random8() > 253U;
}

void PrismataRoutine(s_mode mode) {
  if (loadingFlag)
  {
    loadingFlag = false;
    setCurrentPalette(mode);
  } 
//  EVERY_N_MILLIS(33) { маловата задержочка
    hue++; // используем переменную сдвига оттенка из функций радуги, чтобы не занимать память
//  }
  blurScreen(20); // @Palpalych посоветовал делать размытие
  dimAll(255U - (mode.scale - 1U) % 11U * 3U);

  for (uint8_t x = 0; x < WIDTH; x++)
  {
    //uint8_t y = beatsin8(x + 1, 0, HEIGHT-1); // это я попытался распотрошить данную функцию до исходного кода и вставить в неё регулятор скорости
    // вместо 28 в оригинале было 280, умножения на .Speed не было, а вместо >>17 было (<<8)>>24. короче, оригинальная скорость достигается при бегунке .Speed=20
    uint8_t beat = (GET_MILLIS() * (accum88(x + 1)) * 28 * mode.speed) >> 17;
    uint8_t y = scale8(sin8(beat), HEIGHT-1);
    //и получилось!!!
    
    drawPixelXY(x, y, ColorFromPalette(*curPalette, x * 7 + hue));
  }
}

void spiroRoutine(s_mode mode) {
    if (loadingFlag)
    {
      loadingFlag = false;
      setCurrentPalette(mode);
    }
      
      blurScreen(20); // @Palpalych советует делать размытие
      dimAll(255U - mode.speed / 10);

      boolean change = false;
      
      for (uint8_t i = 0; i < spirocount; i++) {
        uint8_t x = mapsin8(spirotheta1 + i * spirooffset, spirominx, spiromaxx);
        uint8_t y = mapcos8(spirotheta1 + i * spirooffset, spirominy, spiromaxy);

        uint8_t x2 = mapsin8(spirotheta2 + i * spirooffset, x - spiroradiusx, x + spiroradiusx);
        uint8_t y2 = mapcos8(spirotheta2 + i * spirooffset, y - spiroradiusy, y + spiroradiusy);


       //CRGB color = ColorFromPalette( PartyColors_p, (hue + i * spirooffset), 128U); // вообще-то палитра должна постоянно меняться, но до адаптации этого руки уже не дошли
       //CRGB color = ColorFromPalette(*curPalette, hue + i * spirooffset, 128U); // вот так уже прикручена к бегунку Масштаба. за
       //leds[XY(x2, y2)] += color;
if (x2<WIDTH && y2<HEIGHT) // добавил проверки. не знаю, почему эффект подвисает без них
        leds[XY(x2, y2)] += (CRGB)ColorFromPalette(*curPalette, hue + i * spirooffset);
        
        if((x2 == spirocenterX && y2 == spirocenterY) ||
           (x2 == spirocenterX && y2 == spirocenterY)) change = true;
      }

      spirotheta2 += 2;

//      EVERY_N_MILLIS(12) { маловата задержочка
        spirotheta1 += 1;
//      }

      EVERY_N_MILLIS(75) {
        if (change && !spirohandledChange) {
          spirohandledChange = true;
          
          if (spirocount >= WIDTH || spirocount == 1) spiroincrement = !spiroincrement;

          if (spiroincrement) {
            if(spirocount >= 4)
              spirocount *= 2;
            else
              spirocount += 1;
          }
          else {
            if(spirocount > 4)
              spirocount /= 2;
            else
              spirocount -= 1;
          }

          spirooffset = 256 / spirocount;
        }
        
        if(!change) spirohandledChange = false;
      }

//      EVERY_N_MILLIS(33) { маловата задержочка
        hue += 1;
//      }
}

void BBallsRoutine(s_mode mode) {
  if (loadingFlag)
  {
    loadingFlag = false;
    //FastLED.clear();
    enlargedObjectNUM = (mode.scale - 1U) / 99.0 * (enlargedOBJECT_MAX_COUNT - 1U) + 1U;
    if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {             // Initialize variables
      trackingObjectHue[i] = random8();
      trackingObjectState[i] = random8(0U, WIDTH);
      enlargedObjectTime[i] = millis();
      trackingObjectPosY[i] = 0U;                                // Balls start on the ground
      trackingObjectSpeedY[i] = bballsVImpact0;                // And "pop" up at vImpact0
      trackingObjectShift[i] = 0.90 - float(i) / pow(enlargedObjectNUM, 2); // это, видимо, прыгучесть. для каждого мячика уникальная изначально
      trackingObjectIsShift[i] = false;
      hue2 = (mode.speed > 127U) ? 255U : 0U;                                           // цветные или белые мячики
      hue = (mode.speed == 128U) ? 255U : 254U - mode.speed % 128U * 2U;  // скорость угасания хвостов 0 = моментально
    }
  }
  
  float bballsHi;
  float bballsTCycle;
  if (deltaValue++ & 0x01) deltaHue++; // постепенное изменение оттенка мячиков (закомментировать строчку, если не нужно)
  dimAll(hue);
  for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
    //leds[XY(trackingObjectState[i], trackingObjectPosY[i])] = CRGB::Black; // off for the next loop around  // теперь пиксели гасятся в dimAll()

    bballsTCycle =  (millis() - enlargedObjectTime[i]) / 1000. ; // Calculate the time since the last time the ball was on the ground

    // A little kinematics equation calculates positon as a function of time, acceleration (gravity) and intial velocity
    //bballsHi = 0.5 * bballsGRAVITY * pow(bballsTCycle, 2) + trackingObjectSpeedY[i] * bballsTCycle;
    bballsHi = 0.5 * bballsGRAVITY * bballsTCycle * bballsTCycle + trackingObjectSpeedY[i] * bballsTCycle;

    if ( bballsHi < 0 ) {
      enlargedObjectTime[i] = millis();
      bballsHi = 0; // If the ball crossed the threshold of the "ground," put it back on the ground
      trackingObjectSpeedY[i] = trackingObjectShift[i] * trackingObjectSpeedY[i] ; // and recalculate its new upward velocity as it's old velocity * COR

      if ( trackingObjectSpeedY[i] < 0.01 ) // If the ball is barely moving, "pop" it back up at vImpact0
      {
        trackingObjectShift[i] = 0.90 - float(random8(9U)) / pow(random8(4U, 9U), 2); // сделал, чтобы мячики меняли свою прыгучесть каждый цикл
        trackingObjectIsShift[i] = trackingObjectShift[i] >= 0.89;                             // если мячик максимальной прыгучести, то разрешаем ему сдвинуться
        trackingObjectSpeedY[i] = bballsVImpact0;
      }
    }

    //trackingObjectPosY[i] = round( bballsHi * (HEIGHT - 1) / bballsH0); были жалобы, что эффект вылетает
    trackingObjectPosY[i] = constrain(round( bballsHi * (HEIGHT - 1) / bballsH0), 0, HEIGHT - 1);             // Map "h" to a "pos" integer index position on the LED strip
    if (trackingObjectIsShift[i] && (trackingObjectPosY[i] == HEIGHT - 1)) {                  // если мячик получил право, то пускай сдвинется на максимальной высоте 1 раз
      trackingObjectIsShift[i] = false;
      if (trackingObjectHue[i] & 0x01) {                                       // нечётные налево, чётные направо
        if (trackingObjectState[i] == 0U) trackingObjectState[i] = WIDTH - 1U;
        else --trackingObjectState[i];
      } else {
        if (trackingObjectState[i] == WIDTH - 1U) trackingObjectState[i] = 0U;
        else ++trackingObjectState[i];
      }
    }
    leds[XY(trackingObjectState[i], trackingObjectPosY[i])] = CHSV(trackingObjectHue[i] + deltaHue, hue2, 255U);
    //drawPixelXY(trackingObjectState[i], trackingObjectPosY[i], CHSV(trackingObjectHue[i] + deltaHue, hue2, 255U));  //на случай, если останутся жалобы, что эффект вылетает
  }
}

void colorsRoutine(s_mode mode) {
  hue += mode.scale;
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(hue, 255, 255);
  }
}

void colorRoutine(s_mode mode) {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CHSV(mode.scale * 2.5, 255, 255);
  }
}

void snowRoutine(s_mode mode) {
  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }

  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    // а также не даём двум блокам по вертикали вместе быть
    if (getPixColorXY(x, HEIGHT - 2) == 0 && (random(0, mode.scale) == 0))
      drawPixelXY(x, HEIGHT - 1, 0xE0FFFF - 0x101010 * random(0, 4));
    else
      drawPixelXY(x, HEIGHT - 1, 0x000000);
  }
}

void matrixRoutine(s_mode mode) {
  for (byte x = 0; x < WIDTH; x++) {
    // заполняем случайно верхнюю строку
    uint32_t thisColor = getPixColorXY(x, HEIGHT - 1);
    if (thisColor == 0)
      drawPixelXY(x, HEIGHT - 1, 0x00FF00 * (random(0, mode.scale) == 0));
    else if (thisColor < 0x002000)
      drawPixelXY(x, HEIGHT - 1, 0);
    else
      drawPixelXY(x, HEIGHT - 1, thisColor - 0x002000);
  }

  // сдвигаем всё вниз
  for (byte x = 0; x < WIDTH; x++) {
    for (byte y = 0; y < HEIGHT - 1; y++) {
      drawPixelXY(x, y, getPixColorXY(x, y + 1));
    }
  }
}

#define LIGHTERS_AM 100
int lightersPos[2][LIGHTERS_AM];
int8_t lightersSpeed[2][LIGHTERS_AM];
CHSV lightersColor[LIGHTERS_AM];
byte loopCounter;

void lightersRoutine(s_mode mode) {
  if (loadingFlag) {
    loadingFlag = false;
    randomSeed(millis());
    for (byte i = 0; i < LIGHTERS_AM; i++) {
      lightersPos[0][i] = random(0, WIDTH * 10);
      lightersPos[1][i] = random(0, HEIGHT * 10);
      lightersSpeed[0][i] = random(-10, 10);
      lightersSpeed[1][i] = random(-10, 10);
      lightersColor[i] = CHSV(random(0, 255), 255, 255);
    }
  }
  FastLED.clear();
  if (++loopCounter > 20) loopCounter = 0;
  for (byte i = 0; i < mode.scale; i++) {
    if (loopCounter == 0) {     // меняем скорость каждые 255 отрисовок
      lightersSpeed[0][i] += random(-3, 4);
      lightersSpeed[1][i] += random(-3, 4);
      lightersSpeed[0][i] = constrain(lightersSpeed[0][i], -20, 20);
      lightersSpeed[1][i] = constrain(lightersSpeed[1][i], -20, 20);
    }

    lightersPos[0][i] += lightersSpeed[0][i];
    lightersPos[1][i] += lightersSpeed[1][i];

    if (lightersPos[0][i] < 0) lightersPos[0][i] = (WIDTH - 1) * 10;
    if (lightersPos[0][i] >= WIDTH * 10) lightersPos[0][i] = 0;

    if (lightersPos[1][i] < 0) {
      lightersPos[1][i] = 0;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    if (lightersPos[1][i] >= (HEIGHT - 1) * 10) {
      lightersPos[1][i] = (HEIGHT - 1) * 10;
      lightersSpeed[1][i] = -lightersSpeed[1][i];
    }
    drawPixelXY(lightersPos[0][i] / 10, lightersPos[1][i] / 10, lightersColor[i]);
  }
}

void dimAll(uint8_t value, CRGB *LEDarray) {
  //for (uint16_t i = 0; i < NUM_LEDS; i++) {
  //  leds[i].nscale8(value); //fadeToBlackBy
  //}
  // теперь короткий вариант
  nscale8(LEDarray, NUM_LEDS, value);
  //fadeToBlackBy(LEDarray, NUM_LEDS, 255U - value); // эквивалент  
}

void setCurrentPalette(s_mode mode){
      if (mode.scale > 100U) mode.scale = 100U; // чтобы не было проблем при прошивке без очистки памяти
      curPalette = palette_arr[(uint8_t)(mode.scale/100.0F*((sizeof(palette_arr)/sizeof(TProgmemRGBPalette16 *))-0.01F))];
}

void blurScreen(fract8 blur_amount, CRGB *LEDarray)
{
  blur2d(LEDarray, WIDTH, HEIGHT, blur_amount);
}

uint8_t mapsin8(uint8_t theta, uint8_t lowest, uint8_t highest) {
  uint8_t beatsin = sin8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatsin, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t mapcos8(uint8_t theta, uint8_t lowest, uint8_t highest) {
  uint8_t beatcos = cos8(theta);
  uint8_t rangewidth = highest - lowest;
  uint8_t scaledbeat = scale8(beatcos, rangewidth);
  uint8_t result = lowest + scaledbeat;
  return result;
}

uint8_t myScale8(uint8_t x) { // даёт масштабировать каждые 8 градаций (от 0 до 7) бегунка Масштаб в значения от 0 до 255 по типа синусоиде
  uint8_t x8 = x % 8U;
  uint8_t x4 = x8 % 4U;
  if (x4 == 0U)
    if (x8 == 0U)       return 0U;
    else                return 255U;
  else if (x8 < 4U)     return (1U   + x4 * 72U); // всего 7шт по 36U + 3U лишних = 255U (чтобы восхождение по синусоиде не было зеркально спуску)
//else
                        return (253U - x4 * 72U); // 253U = 255U - 2U
}

void wu_pixel_maxV(int16_t item){
  //uint8_t xx = trackingObjectPosX[item] & 0xff, yy = trackingObjectPosY[item] & 0xff, ix = 255 - xx, iy = 255 - yy;
  uint8_t xx = (trackingObjectPosX[item] - (int)trackingObjectPosX[item]) * 255, yy = (trackingObjectPosY[item] - (int)trackingObjectPosY[item]) * 255, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {    
    uint8_t x1 = (int8_t)(trackingObjectPosX[item] + (i & 1)) % WIDTH; //делаем бесшовный по ИКСу
    uint8_t y1 = (int8_t)(trackingObjectPosY[item] + ((i >> 1) & 1));
    if (y1 < HEIGHT && trackingObjectHue[item] * wu[i] >> 8 >= noise3d[1][x1][y1]){
      noise3d[0][x1][y1] = trackingObjectShift[item];
      shiftValue[y1] = 255U;//saturation;
      noise3d[1][x1][y1] = trackingObjectHue[item] * wu[i] >> 8;
    }
  }  
}

void wu_pixel(uint32_t x, uint32_t y, CRGB * col) {      //awesome wu_pixel procedure by reddit u/sutaburosu
  // extract the fractional parts and derive their inverses
  uint8_t xx = x & 0xff, yy = y & 0xff, ix = 255 - xx, iy = 255 - yy;
  // calculate the intensities for each affected pixel
  #define WU_WEIGHT(a,b) ((uint8_t) (((a)*(b)+(a)+(b))>>8))
  uint8_t wu[4] = {WU_WEIGHT(ix, iy), WU_WEIGHT(xx, iy),
                   WU_WEIGHT(ix, yy), WU_WEIGHT(xx, yy)};
  // multiply the intensities by the colour, and saturating-add them to the pixels
  for (uint8_t i = 0; i < 4; i++) {
    uint16_t xy = XY((x >> 8) + (i & 1), (y >> 8) + ((i >> 1) & 1));
    if (xy < NUM_LEDS){
      leds[xy].r = qadd8(leds[xy].r, col->r * wu[i] >> 8);
      leds[xy].g = qadd8(leds[xy].g, col->g * wu[i] >> 8);
      leds[xy].b = qadd8(leds[xy].b, col->b * wu[i] >> 8);
    }
  }
}

void fillMyPal16_2(uint8_t hue, bool isInvert) { 
// я бы, конечно, вместо копии функции генерации палитры "_2"
// лучше бы сделал её параметром указатель на массив с базовой палитрой, 
// но я пониятия не имею, как это делается с грёбаным PROGMEM

  int8_t lastSlotUsed = -1;
  uint8_t istart8, iend8;
  CRGB rgbstart, rgbend;
  
  // начинаем с нуля
  if (isInvert)
    //с неявным преобразованием оттенков цвета получаются, как в фотошопе, но для данного эффекта не красиво выглядят
    //rgbstart = CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])); // начальная строчка палитры с инверсией
    hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])), rgbstart);
  else
    //rgbstart = CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])); // начальная строчка палитры
    hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[0][1]), pgm_read_byte(&MBAuroraColors_arr[0][2]), pgm_read_byte(&MBAuroraColors_arr[0][3])), rgbstart);
  int indexstart = 0; // начальный индекс палитры
  for (uint8_t i = 1U; i < 5U; i++) { // в палитре @obliterator всего 5 строчек
    int indexend = pgm_read_byte(&MBAuroraColors_arr[i][0]);
    if (isInvert)
      hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBAuroraColors_arr[i][1]), pgm_read_byte(&MBAuroraColors_arr[i][2]), pgm_read_byte(&MBAuroraColors_arr[i][3])), rgbend);
    else
      hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBAuroraColors_arr[i][1]), pgm_read_byte(&MBAuroraColors_arr[i][2]), pgm_read_byte(&MBAuroraColors_arr[i][3])), rgbend);
    istart8 = indexstart / 16;
    iend8   = indexend   / 16;
    if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
       istart8 = lastSlotUsed + 1;
       if (iend8 < istart8)
         iend8 = istart8;
    }
    lastSlotUsed = iend8;
    fill_gradient_RGB( myPal, istart8, rgbstart, iend8, rgbend);
    indexstart = indexend;
    rgbstart = rgbend;
  }
}

void LeapersMove_leaper(uint8_t l) {
    #define GRAVITY            0.06
    #define SETTLED_THRESHOLD  0.1
    #define WALL_FRICTION      0.95
    #define WIND               0.95    // wind resistance

    trackingObjectPosX[l] += trackingObjectSpeedX[l];
    trackingObjectPosY[l] += trackingObjectSpeedY[l];

    // bounce off the floor and ceiling?
    if (trackingObjectPosY[l] < 0 || trackingObjectPosY[l] > HEIGHT - 1) {
        trackingObjectSpeedY[l] = (-trackingObjectSpeedY[l] * WALL_FRICTION);
        trackingObjectSpeedX[l] = ( trackingObjectSpeedX[l] * WALL_FRICTION);
        trackingObjectPosY[l] += trackingObjectSpeedY[l];
        if (trackingObjectPosY[l] < 0) 
        trackingObjectPosY[l] = 0; // settled on the floor?
        if (trackingObjectPosY[l] <= SETTLED_THRESHOLD && fabs(trackingObjectSpeedY[l]) <= SETTLED_THRESHOLD) {
        LeapersRestart_leaper(l);
        }
    }

    // bounce off the sides of the screen?
    if (trackingObjectPosX[l] <= 0 || trackingObjectPosX[l] >= WIDTH - 1) {
        trackingObjectSpeedX[l] = (-trackingObjectSpeedX[l] * WALL_FRICTION);
        if (trackingObjectPosX[l] <= 0) {
        //trackingObjectPosX[l] = trackingObjectSpeedX[l]; // the bug?
        trackingObjectPosX[l] = -trackingObjectPosX[l];
        } else {
        //trackingObjectPosX[l] = WIDTH - 1 - trackingObjectSpeedX[l]; // the bug?
        trackingObjectPosX[l] = WIDTH + WIDTH - 2 - trackingObjectPosX[l];
        }
    }

    trackingObjectSpeedY[l] -= GRAVITY;
    trackingObjectSpeedX[l] *= WIND;
    trackingObjectSpeedY[l] *= WIND;
}

void LeapersRestart_leaper(uint8_t l) {
    // leap up and to the side with some random component
    trackingObjectSpeedX[l] = (1 * (float)random8(1, 100) / 100);
    trackingObjectSpeedY[l] = (2 * (float)random8(1, 100) / 100);

    // for variety, sometimes go 50% faster
    if (random8() < 12) {
        trackingObjectSpeedX[l] += trackingObjectSpeedX[l] * 0.5;
        trackingObjectSpeedY[l] += trackingObjectSpeedY[l] * 0.5;
    }

    // leap towards the centre of the screen
    if (trackingObjectPosX[l] > (WIDTH / 2)) {
        trackingObjectSpeedX[l] = -trackingObjectSpeedX[l];
    }
}

void rain(byte backgroundDepth, byte maxBrightness, byte spawnFreq, byte tailLength, CRGB rainColor, bool splashes, bool clouds, bool storm)
{
  ff_x = random16();
  ff_y = random16();
  ff_z = random16();

  CRGB lightningColor = CRGB(72,72,80);
  CRGBPalette16 rain_p( CRGB::Black, rainColor );
#ifdef SMARTMATRIX
  CRGBPalette16 rainClouds_p( CRGB::Black, CRGB(75,84,84), CRGB(49,75,75), CRGB::Black );
#else
  CRGBPalette16 rainClouds_p( CRGB::Black, CRGB(15,24,24), CRGB(9,15,15), CRGB::Black );
#endif

  //fadeToBlackBy( leds, NUM_LEDS, 255-tailLength);
  dimAll(tailLength);

  // Loop for each column individually
  for (uint8_t x = 0; x < WIDTH; x++) {
    // Step 1.  Move each dot down one cell
    for (uint8_t i = 0; i < HEIGHT; i++) {
      if (noise3d[0][x][i] >= backgroundDepth) {  // Don't move empty cells
        if (i > 0) noise3d[0][x][wrapY(i-1)] = noise3d[0][x][i];
        noise3d[0][x][i] = 0;
      }
    }

    // Step 2.  Randomly spawn new dots at top
    if (random8() < spawnFreq) {
      noise3d[0][x][HEIGHT-1] = random(backgroundDepth, maxBrightness);
    }

    // Step 3. Map from tempMatrix cells to LED colors
    for (uint8_t y = 0; y < HEIGHT; y++) {
      if (noise3d[0][x][y] >= backgroundDepth) {  // Don't write out empty cells
        leds[XY(x,y)] = ColorFromPalette(rain_p, noise3d[0][x][y]);
      }
    }

    // Step 4. Add splash if called for
    if (splashes) {
      // FIXME, this is broken
      byte j = line[x];
      byte v = noise3d[0][x][0];

      if (j >= backgroundDepth) {
        leds[XY(wrapX(x-2),0)] = ColorFromPalette(rain_p, j/3);
        leds[XY(wrapX(x+2),0)] = ColorFromPalette(rain_p, j/3);
        line[x] = 0;   // Reset splash
      }

      if (v >= backgroundDepth) {
        leds[XY(wrapX(x-1),1)] = ColorFromPalette(rain_p, v/2);
        leds[XY(wrapX(x+1),1)] = ColorFromPalette(rain_p, v/2);
        line[x] = v; // Prep splash for next frame
      }
    }

    // Step 5. Add lightning if called for
    if (storm) {
      //uint8_t lightning[WIDTH][HEIGHT];
      // ESP32 does not like static arrays  https://github.com/espressif/arduino-esp32/issues/2567
      uint8_t *lightning = (uint8_t *) malloc(WIDTH * HEIGHT);
      while (lightning == NULL) { Serial.println("lightning malloc failed"); }


      if (random16() < 72) {    // Odds of a lightning bolt
        lightning[scale8(random8(), WIDTH-1) + (HEIGHT-1) * WIDTH] = 255;  // Random starting location
        for(uint8_t ly = HEIGHT-1; ly > 1; ly--) {
          for (uint8_t lx = 1; lx < WIDTH-1; lx++) {
            if (lightning[lx + ly * WIDTH] == 255) {
              lightning[lx + ly * WIDTH] = 0;
              uint8_t dir = random8(4);
              switch (dir) {
                case 0:
                  leds[XY(lx+1,ly-1)] = lightningColor;
                  lightning[(lx+1) + (ly-1) * WIDTH] = 255; // move down and right
                break;
                case 1:
                  leds[XY(lx,ly-1)] = CRGB(128,128,128); // я без понятия, почему у верхней молнии один оттенок, а у остальных - другой
                  lightning[lx + (ly-1) * WIDTH] = 255;    // move down
                break;
                case 2:
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx-1) + (ly-1) * WIDTH] = 255; // move down and left
                break;
                case 3:
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx-1) + (ly-1) * WIDTH] = 255; // fork down and left
                  leds[XY(lx-1,ly-1)] = CRGB(128,128,128);
                  lightning[(lx+1) + (ly-1) * WIDTH] = 255; // fork down and right
                break;
              }
            }
          }
        }
      }
      free(lightning);
    }

    // Step 6. Add clouds if called for
    if (clouds) {
      uint16_t noiseScale = 250;  // A value of 1 will be so zoomed in, you'll mostly see solid colors. A value of 4011 will be very zoomed out and shimmery
      //const uint16_t cloudHeight = (HEIGHT*0.2)+1;
      const uint8_t cloudHeight = HEIGHT * 0.4 + 1; // это уже 40% c лишеним, но на высоких матрицах будет чуть меньше

      // This is the array that we keep our computed noise values in
      //static uint8_t noise[WIDTH][cloudHeight];
      static uint8_t *noise = (uint8_t *) malloc(WIDTH * cloudHeight);
      
      while (noise == NULL) { Serial.println("noise malloc failed"); }
      int xoffset = noiseScale * x + hue;

      for(uint8_t z = 0; z < cloudHeight; z++) {
        int yoffset = noiseScale * z - hue;
        uint8_t dataSmoothing = 192;
        uint8_t noiseData = qsub8(inoise8(ff_x + xoffset,ff_y + yoffset,ff_z),16);
        noiseData = qadd8(noiseData,scale8(noiseData,39));
        noise[x * cloudHeight + z] = scale8( noise[x * cloudHeight + z], dataSmoothing) + scale8( noiseData, 256 - dataSmoothing);
        nblend(leds[XY(x,HEIGHT-z-1)], ColorFromPalette(rainClouds_p, noise[x * cloudHeight + z]), (cloudHeight-z)*(250/cloudHeight));
      }
      ff_z ++;
    }
  }
}

uint8_t wrapX(int8_t x){
  return (x + WIDTH)%WIDTH;
}
uint8_t wrapY(int8_t y){
  return (y + HEIGHT)%HEIGHT;
}

void PicassoRoutine(){
  PicassoGenerate(false);
  PicassoPosition();

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i+=2) 
    DrawLine(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i+1U], trackingObjectPosY[i+1U], CHSV(trackingObjectHue[i], 255U, 255U));

  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void PicassoGenerate(bool reset){
    if (loadingFlag)
    {
        loadingFlag = false;
        if (enlargedObjectNUM > enlargedOBJECT_MAX_COUNT) enlargedObjectNUM = enlargedOBJECT_MAX_COUNT;
        if (enlargedObjectNUM < 2U) enlargedObjectNUM = 2U;

        double minSpeed = 0.2, maxSpeed = 0.8;
        
        for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) { 
        trackingObjectPosX[i] = random8(WIDTH);
        trackingObjectPosY[i] = random8(HEIGHT);

        trackingObjectHue[i] = random8();

        trackingObjectSpeedY[i] = +((-maxSpeed / 3) + (maxSpeed * (float)random8(1, 100) / 100));
        trackingObjectSpeedY[i] += trackingObjectSpeedY[i] > 0 ? minSpeed : -minSpeed;

        trackingObjectShift[i] = +((-maxSpeed / 2) + (maxSpeed * (float)random8(1, 100) / 100));
        trackingObjectShift[i] += trackingObjectShift[i] > 0 ? minSpeed : -minSpeed;

        trackingObjectState[i] = trackingObjectHue[i];
        }
    }
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) {
        if (reset) {
            trackingObjectState[i] = random8();
            trackingObjectSpeedX[i] = (trackingObjectState[i] - trackingObjectHue[i]) / 25;
        }
        if (trackingObjectState[i] != trackingObjectHue[i] && trackingObjectSpeedX[i]) {
            trackingObjectHue[i] += trackingObjectSpeedX[i];
        }
    }
}

void PicassoPosition(){
    for (uint8_t i = 0 ; i < enlargedObjectNUM ; i++) { 
        if (trackingObjectPosX[i] + trackingObjectSpeedY[i] > WIDTH || trackingObjectPosX[i] + trackingObjectSpeedY[i] < 0) {
        trackingObjectSpeedY[i] = -trackingObjectSpeedY[i];
        }

        if (trackingObjectPosY[i] + trackingObjectShift[i] > HEIGHT || trackingObjectPosY[i] + trackingObjectShift[i] < 0) {
        trackingObjectShift[i] = -trackingObjectShift[i];
        }

        trackingObjectPosX[i] += trackingObjectSpeedY[i];
        trackingObjectPosY[i] += trackingObjectShift[i];
    };
}

void PicassoRoutine3(){
  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 2U ; i+=2) 
    drawCircleF(fabs(trackingObjectPosX[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosY[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosX[i] - trackingObjectPosY[i]), CHSV(trackingObjectHue[i], 255U, 255U));
    //drawCircleF(fabs(trackingObjectPosX[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosY[i] - trackingObjectPosX[i+1U]), fabs(trackingObjectPosX[i] - trackingObjectPosY[i]), ColorFromPalette(*curPalette, trackingObjectHue[i]));
    
  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void PicassoRoutine2(){
  PicassoGenerate(false);
  PicassoPosition();
  dimAll(180);

  for (uint8_t i = 0 ; i < enlargedObjectNUM - 1U ; i++) 
    DrawLineF(trackingObjectPosX[i], trackingObjectPosY[i], trackingObjectPosX[i+1U], trackingObjectPosY[i+1U], CHSV(trackingObjectHue[i], 255U, 255U));

  EVERY_N_MILLIS(20000){
    PicassoGenerate(true);
  }

  blurScreen(80);
}

void FillNoise(int8_t layer) {
  for (uint8_t i = 0; i < WIDTH; i++) {
    int32_t ioffset = scale32_x[layer] * (i - CENTER_X_MINOR);
    for (uint8_t j = 0; j < HEIGHT; j++) {
      int32_t joffset = scale32_y[layer] * (j - CENTER_Y_MINOR);
      int8_t data = inoise16(noise32_x[layer] + ioffset, noise32_y[layer] + joffset, noise32_z[layer]) >> 8;
      int8_t olddata = noise3d[layer][i][j];
      int8_t newdata = scale8( olddata, noisesmooth ) + scale8( data, 255 - noisesmooth );
      data = newdata;
      noise3d[layer][i][j] = data;
    }
  }
}

void MoveFractionalNoiseX(int8_t amplitude, float shift) {
  for (uint8_t y = 0; y < HEIGHT; y++) {
    int16_t amount = ((int16_t)noise3d[0][0][y] - 128) * 2 * amplitude + shift * 256  ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (uint8_t x = 0 ; x < WIDTH; x++) {
      if (amount < 0) {
        zD = x - delta; zF = zD - 1;
      } else {
        zD = x + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black  ;
      if ((zD >= 0) && (zD < WIDTH)) PixelA = leds[XY(zD, y)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < WIDTH)) PixelB = leds[XY(zF, y)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));   // lerp8by8(PixelA, PixelB, fraction );
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}

void MoveFractionalNoiseY(int8_t amplitude, float shift) {
  for (uint8_t x = 0; x < WIDTH; x++) {
    int16_t amount = ((int16_t)noise3d[0][x][0] - 128) * 2 * amplitude + shift * 256 ;
    int8_t delta = abs(amount) >> 8 ;
    int8_t fraction = abs(amount) & 255;
    for (uint8_t y = 0 ; y < HEIGHT; y++) {
      if (amount < 0) {
        zD = y - delta; zF = zD - 1;
      } else {
        zD = y + delta; zF = zD + 1;
      }
      CRGB PixelA = CRGB::Black ;
      if ((zD >= 0) && (zD < HEIGHT)) PixelA = leds[XY(x, zD)];
      CRGB PixelB = CRGB::Black ;
      if ((zF >= 0) && (zF < HEIGHT)) PixelB = leds[XY(x, zF)];
      ledsbuff[XY(x, y)] = (PixelA.nscale8(ease8InOutApprox(255 - fraction))) + (PixelB.nscale8(ease8InOutApprox(fraction)));
    }
  }
  memcpy(leds, ledsbuff, sizeof(CRGB)* NUM_LEDS);
}

void fairyEmit(uint8_t i) //particlesEmit(Particle_Abstract *particle, ParticleSysConfig *g)
{
    if (deltaHue++ & 0x01)
      if (hue++ & 0x01)
        hue2++;//counter++;
    trackingObjectPosX[i] = boids[0].location.x;
    trackingObjectPosY[i] = boids[0].location.y;

    //хотите навставлять speedfactor? - тут не забудьте
    //trackingObjectSpeedX[i] = ((float)random8()-127.)/512./0.25*speedfactor; // random(_hVar)-_constVel; // particle->vx
    trackingObjectSpeedX[i] = ((float)random8()-127.)/512.; // random(_hVar)-_constVel; // particle->vx
    //trackingObjectSpeedY[i] = SQRT_VARIANT((speedfactor*speedfactor+0.0001)-trackingObjectSpeedX[i]*trackingObjectSpeedX[i]); // SQRT_VARIANT(pow(_constVel,2)-pow(trackingObjectSpeedX[i],2)); // particle->vy зависит от particle->vx - не ошибка
    trackingObjectSpeedY[i] = SQRT_VARIANT(0.0626-trackingObjectSpeedX[i]*trackingObjectSpeedX[i]); // SQRT_VARIANT(pow(_constVel,2)-pow(trackingObjectSpeedX[i],2)); // particle->vy зависит от particle->vx - не ошибка
    if(random8(2U)) { trackingObjectSpeedY[i]=-trackingObjectSpeedY[i]; }

    trackingObjectState[i] = random8(20, 80); // random8(minLife, maxLife);// particle->ttl
    trackingObjectHue[i] = hue2;// (counter/2)%255; // particle->hue
    trackingObjectIsShift[i] = true; // particle->isAlive
}

void particlesUpdate2(uint8_t i){
  //age
  trackingObjectState[i]--; //ttl // ещё и сюда надо speedfactor вкорячить. удачи там!

  //apply acceleration
  //trackingObjectSpeedX[i] = min((int)trackingObjectSpeedX[i]+ax, WIDTH);
  //trackingObjectSpeedY[i] = min((int)trackingObjectSpeedY[i]+ay, HEIGHT);

  //apply velocity
  trackingObjectPosX[i] += trackingObjectSpeedX[i];
  trackingObjectPosY[i] += trackingObjectSpeedY[i];
  if(trackingObjectState[i] == 0 || trackingObjectPosX[i] <= -1 || trackingObjectPosX[i] >= WIDTH || trackingObjectPosY[i] <= -1 || trackingObjectPosY[i] >= HEIGHT) 
    trackingObjectIsShift[i] = false;
}

void nexusReset(uint8_t i){
    trackingObjectHue[i] = random8();
    trackingObjectState[i] = random8(4);
    //trackingObjectSpeedX[i] = (255. + random8()) / 255.;
    trackingObjectSpeedX[i] = (float)random8(5,11) / 70 + speedfactor; // делаем частицам немного разное ускорение и сразу пересчитываем под общую скорость
    switch (trackingObjectState[i]) {
        case B01:
            trackingObjectPosY[i] = HEIGHT;
            trackingObjectPosX[i] = random8(WIDTH);
        break;
        case B00:
            trackingObjectPosY[i] = -1;
            trackingObjectPosX[i] = random8(WIDTH);
        break;
        case B10:
            trackingObjectPosX[i] = WIDTH;
            trackingObjectPosY[i] = random8(HEIGHT);
        break;
        case B11:
            trackingObjectPosX[i] = -1;
            trackingObjectPosY[i] = random8(HEIGHT);
        break;
    }
}

void LiquidLampPosition(){
  //bool physic_on = modes[currentMode].Speed & 0x01;
  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    liquidLampHot[i] += mapcurve(trackingObjectPosY[i], 0, HEIGHT-1, 5, -5, InOutQuad) * speedfactor;

    float heat = (liquidLampHot[i] / trackingObjectState[i]) - 1;
    if (heat > 0 && trackingObjectPosY[i] < HEIGHT-1) {
      trackingObjectSpeedY[i] += heat * liquidLampSpf[i];
    }
    if (trackingObjectPosY[i] > 0) {
      trackingObjectSpeedY[i] -= 0.07;
    }

    if (trackingObjectSpeedY[i]) trackingObjectSpeedY[i] *= 0.85;
    trackingObjectPosY[i] += trackingObjectSpeedY[i] * speedfactor;

    //if (physic_on) {
      if (trackingObjectSpeedX[i]) trackingObjectSpeedX[i] *= 0.7;
      trackingObjectPosX[i] += trackingObjectSpeedX[i] * speedfactor;
    //}

    if (trackingObjectPosX[i] > WIDTH-1) trackingObjectPosX[i] -= WIDTH-1;
    if (trackingObjectPosX[i] < 0) trackingObjectPosX[i] += WIDTH-1;
    if (trackingObjectPosY[i] > HEIGHT-1) trackingObjectPosY[i] = HEIGHT-1;
    if (trackingObjectPosY[i] < 0) trackingObjectPosY[i] = 0;
  };
}

void LiquidLampPhysic(){
  for (uint8_t i = 0; i < enlargedObjectNUM; i++) {
    //Particle *p1 = (Particle *)&particles[i];
    // отключаем физику на границах, чтобы не слипались шары
    if (trackingObjectPosY[i] < 3 || trackingObjectPosY[i] > HEIGHT - 1) continue;
    for (uint8_t j = 0; j < enlargedObjectNUM; j++) {
      //Particle *p2 = (Particle *)&particles[j];
      if (trackingObjectPosY[j] < 3 || trackingObjectPosY[j] > HEIGHT - 1) continue;
      float radius = 3;//(trackingObjectShift[i] + trackingObjectShift[j]);
      if (trackingObjectPosX[i] + radius > trackingObjectPosX[j]
       && trackingObjectPosX[i] < radius + trackingObjectPosX[j]
       && trackingObjectPosY[i] + radius > trackingObjectPosY[j]
       && trackingObjectPosY[i] < radius + trackingObjectPosY[j]
      ){
          //float dist = EffectMath::distance(p1->position_x, p1->position_y, p2->position_x, p2->position_y);
          float dx =  min((float)fabs(trackingObjectPosX[i] - trackingObjectPosX[j]), (float)WIDTH + trackingObjectPosX[i] - trackingObjectPosX[j]); //по идее бесшовный икс
          float dy =  fabs(trackingObjectPosY[i] - trackingObjectPosY[j]);
          float dist = SQRT_VARIANT((dx * dx) + (dy * dy));
          
          if (dist <= radius) {
            float nx = (trackingObjectPosX[j] - trackingObjectPosX[i]) / dist;
            float ny = (trackingObjectPosY[j] - trackingObjectPosY[i]) / dist;
            float p = 2 * (trackingObjectSpeedX[i] * nx + trackingObjectSpeedY[i] * ny - trackingObjectSpeedX[j] * nx - trackingObjectSpeedY[j] * ny) / (trackingObjectState[i] + trackingObjectState[j]);
            float pnx = p * nx, pny = p * ny;
            trackingObjectSpeedX[i] = trackingObjectSpeedX[i] - pnx * trackingObjectState[i];
            trackingObjectSpeedY[i] = trackingObjectSpeedY[i] - pny * trackingObjectState[i];
            trackingObjectSpeedX[j] = trackingObjectSpeedX[j] + pnx * trackingObjectState[j];
            trackingObjectSpeedY[j] = trackingObjectSpeedY[j] + pny * trackingObjectState[j];
          }
        }
    }
  }
}
void fillMyPal16(uint8_t hue, bool isInvert){
  int8_t lastSlotUsed = -1;
  uint8_t istart8, iend8;
  CRGB rgbstart, rgbend;
  
  // начинаем с нуля
  if (isInvert)
    //с неявным преобразованием оттенков цвета получаются, как в фотошопе, но для данного эффекта не красиво выглядят
    //rgbstart = CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])); // начальная строчка палитры с инверсией
    hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])), rgbstart);
  else
    //rgbstart = CHSV(hue + pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])); // начальная строчка палитры
    hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBVioletColors_arr[0][1]), pgm_read_byte(&MBVioletColors_arr[0][2]), pgm_read_byte(&MBVioletColors_arr[0][3])), rgbstart);
  int indexstart = 0; // начальный индекс палитры
  for (uint8_t i = 1U; i < 5U; i++) { // в палитре @obliterator всего 5 строчек
    int indexend = pgm_read_byte(&MBVioletColors_arr[i][0]);
    if (isInvert)
      //rgbend = CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])); // следующая строчка палитры с инверсией
      hsv2rgb_spectrum(CHSV(256 + hue - pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])), rgbend);
    else
      //rgbend = CHSV(hue + pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])); // следующая строчка палитры
      hsv2rgb_spectrum(CHSV(hue + pgm_read_byte(&MBVioletColors_arr[i][1]), pgm_read_byte(&MBVioletColors_arr[i][2]), pgm_read_byte(&MBVioletColors_arr[i][3])), rgbend);
    istart8 = indexstart / 16;
    iend8   = indexend   / 16;
    if ((istart8 <= lastSlotUsed) && (lastSlotUsed < 15)) {
       istart8 = lastSlotUsed + 1;
       if (iend8 < istart8)
         iend8 = istart8;
    }
    lastSlotUsed = iend8;
    fill_gradient_RGB( myPal, istart8, rgbstart, iend8, rgbend);
    indexstart = indexend;
    rgbstart = rgbend;
  }
}