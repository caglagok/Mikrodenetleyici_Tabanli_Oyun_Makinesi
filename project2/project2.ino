#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Menü durumlarını tanımlayan enum yapısı
enum MenuState {
  MENU_START,
  MENU_EXIT
};
// Oyun durumlarını tanımlayan enum yapısı
enum GameState {
  GAME_MENU,
  GAME_RUNNING,
  GAME_EXIT
};
// Mevcut menü seçeneğini ve oyun durumunu takip eden değişkenler
MenuState currentMenuSelection = MENU_START;
GameState currentGameState = GAME_MENU;

// Buton pinleri
const int buttonPinDown = 34;  // Aşağı hareket için buton
const int buttonPinUp = 35;    // Yukarı hareket için buton
const int buttonPinStart = 36; // Başlatma/onaylama için buton

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define PLATE_WIDTH 30
#define PLATE_HEIGHT 4
#define PLATE_SPEED 5 // PLATE_SPEED değeri artırıldı
#define BALL_SIZE 2
#define BALL_SPEED_X 2
#define BALL_SPEED_Y 2

#define WINDOW_SIZE 1 // Kayan pencere boyutu
#define BRICK_ROWS 3
#define BRICK_COLUMNS 7
#define BRICK_WIDTH 16
#define BRICK_HEIGHT 6
#define BRICK_GAP 2

bool bricks[BRICK_ROWS][BRICK_COLUMNS]; // Tuğla matrisi
int potValues[WINDOW_SIZE]; // Potansiyometre değerlerinin saklanacağı dizi
int potIndex = 0;           // Kayan pencere dizisindeki mevcut indeks

int platePosition = 0; // SCREEN_WIDTH / 2 - PLATE_WIDTH/2;
int ballX = SCREEN_WIDTH / 2;
int ballY = SCREEN_HEIGHT - 10;
int ballSpeedX = BALL_SPEED_X;
int ballSpeedY = BALL_SPEED_Y;
int currentLevel = 1; // Oyunun başlangıç seviyesi
// Oyuncunun canlarını takip eden değişkenler
int lives = 3;
// LED pinleri (can göstergesi için)
const int ledPins[3] = {31, 32, 33}; // Analog pinlere güncellendi
bool isLifeUpFalling = false;
int lifeUpX, lifeUpY;
// Işık sensörü pini
const int lightSensorPin = 31; // Örnek olarak A1 analog pinini kullanıyoruz
const int someThreshold = 512; // Işık sensörü değeri için örnek bir eşik değer

unsigned long previousMillis = 0;
const long interval = 20; // Hareket zamanlayıcısı
int score = 0;
int digit[10] = {
                  0b0111111, //0
                  0b0000110, //1
                  0b1011011, //2
                  0b1001111, //3
                  0b1100110, //4
                  0b1101101, //5
                  0b1111101, //6
                  0b0000111, //7
                  0b1111111, //8
                  0b1101111  //9
                };
// Seviye 1 tuğla dizilimi
bool level1[BRICK_ROWS][BRICK_COLUMNS] = {
  {false, false, false, true, false, false, false},
  {false, false, true, true, true, false, false},
  {false, true, true, true, true, true, false}
};
// Seviye 2 tuğla dizilimi
bool level2[BRICK_ROWS][BRICK_COLUMNS] = {
  {true, false, true, false, true, false, true},
  {false, true, false, true, false, true, false},
  {true, false, true, false, true, false, true}
};
// Seviye 3 tuğla dizilimi
bool level3[BRICK_ROWS][BRICK_COLUMNS] = {
  {true, true, false, false, false, true, true},
  {true, false, true, true, true, false, true},
  {true, true, false, false, false, true, true}
};
// Seviye 4 tuğla dizilimi
bool level4[BRICK_ROWS][BRICK_COLUMNS] = {
  {true, true, true, true, true, true,true},  
  {false, false, false, false, false, false,false},
  {true, true, true, true, true, true,true}
};
// Seviye 5 tuğla dizilimi
bool level5[BRICK_ROWS][BRICK_COLUMNS] = {
  {true, false, true, false, true, false},
  {false, true, false, true, false, true},
  {true, false, true, false, true, false}
};

void setup()
{
  Serial.begin(9600);
  pinMode(buttonPinDown, INPUT_PULLUP);
  pinMode(buttonPinUp, INPUT_PULLUP);
  pinMode(buttonPinStart, INPUT_PULLUP);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Kütüphane kullanarak ekranın başlatılması
  display.display();
  delay(500); //  beklet
  display.clearDisplay();

  // Giriş ekranının gösterilmesi
  displayMenu();

  for (int i = 22; i < 29; i++)
  {
    pinMode(i, OUTPUT); // 0-9 inci pini çıkış olarak ayarlama(kod kalabağı olmaması için for ile ayarlandı)
  }
  pinMode(29, OUTPUT); //1. hane output
  pinMode(30, OUTPUT);//2.hane output

  currentLevel = 1; // Seviyeyi 1 olarak başlat

  // LED pinlerini çıkış olarak başlat ve yanacak şekilde ayarla
  for (int i = 0; i < 3; i++) {
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH); // Başlangıçta bütün LED'ler yanık
  }

  display.clearDisplay();
  
  displayScore();
}
void loop() {
  if (currentGameState == GAME_MENU) {
    // Menüde yukarı hareket
    if (isButtonPressed(buttonPinUp)) {
      currentMenuSelection = MENU_START; // Her zaman ilk seçeneğe git
      displayMenu();
    }
    // Menüde aşağı hareket
    else if (isButtonPressed(buttonPinDown)) {
      currentMenuSelection = MENU_EXIT; // Her zaman ikinci seçeneğe git
      displayMenu();
    }
    // Seçimi onayla ve ilgili duruma git
    else if (isButtonPressed(buttonPinStart)) {
      if (currentMenuSelection == MENU_START) {
        currentGameState = GAME_RUNNING;
        startGame();
      } else {
        currentGameState = GAME_EXIT;
      }
    }
  } 
  else if (currentGameState == GAME_RUNNING) {
    
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      moveBall();
      previousMillis = currentMillis;
    }
    // Can kazanma özelliğini düşür
    if (isLifeUpFalling) {
      moveLifeUp();
    }
    // Potansiyometre değerini oku
    int potValue = analogRead(A0);

    // Plakayı hareket ettir
    movePlate(potValue);

    if (isLifeUpFalling) {
      moveLifeUp();
    }
    // Oyunu kontrol et
    if (ballY >= SCREEN_HEIGHT) {
      // Top düştüğünde can kaybını işle ve LED'yi söndür
      checkBallLoss();
      
      if (lives <= 0) {
        // Canlar bittiğinde oyunu sonlandır
        gameOver();
        return; // Oyun bitince döngüden çık
      }
    }
    
    // Tuğlaların tümü yoksa yeni bölüme geç
    bool allBricksDestroyed = true;
    for (int i = 0; i < BRICK_ROWS; i++) {
      for (int j = 0; j < BRICK_COLUMNS; j++) {
        if (bricks[i][j]) {
          allBricksDestroyed = false;
          break;
        }
      }
      if (!allBricksDestroyed) {
        break;
      }
    }
    if (allBricksDestroyed) {
      increaseBallSpeed(); // Topun hızını artır
      checkBricks();
    }
    display.clearDisplay();
    drawPlate();
    drawBall();
    drawBricks();
    display.display();
  } 
  else if (currentGameState == GAME_EXIT) {
    // Ekranı temizle
    display.clearDisplay();

    // Çıkış mesajını ayarla
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0,0);
    display.println(F("Oyunumuza\ngosterdiginiz\nilgi icin\ntesekkur ederiz."));
    display.display();
    // Bir süre mesajı göster
    delay(3000); // 3 saniye beklet
  }
}
void loadLevel(int level) {
  switch(level) {
    case 1:
      loadBricks(level1);
      break;
    case 2:
      loadBricks(level2);
      break;
    case 3:
      loadBricks(level3);
      break;
    case 4:
      loadBricks(level4);
      break;
    case 5:
      loadBricks(level5);
      break;
    default:
      // Hata durumu veya varsayılan bir seviye yükleme
      break;
  }
}
void loadBricks(bool level[BRICK_ROWS][BRICK_COLUMNS]) {
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      bricks[i][j] = level[i][j];
    }
  }
}
void startGame() {
  // Oyunu başlatırken ilk seviyeyi yükle
  lives = 3; // Oyuncunun can sayısını 3'e ayarla
  updateLivesDisplay(); // Can göstergesini güncelle
  
  currentLevel = 1; // Seviyeyi 1 olarak başlat
  loadLevel(currentLevel); // İlk seviyeyi yükle

  // Oyun başladığında ilk ayarlar burada yapılacak
  display.clearDisplay();
  // Oyunun başlangıç ekranı vs. ayarlamalar
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.print(F("Level "));
  display.println(currentLevel);
  display.println(F("Oyun Basliyor..."));
  display.display();
  delay(1000); // Ekranı bir süre göster

  // Oyun başlangıç değerlerinin ayarlanması
  // Örneğin, topun başlangıç konumu, puanın sıfırlanması vs.
  resetBall(); // Topu paletin üstünden başlat
  currentLevel++;
  ballSpeedY =-BALL_SPEED_Y;
}
void displayMenu() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  if (currentMenuSelection == MENU_START) {
    display.println(F(">* BASLAT"));
    display.println(F("  CIKIS"));
  } 
  else {
    display.println(F("  BASLAT"));
    display.println(F(">* CIKIS"));
  }

  display.display();
}
void drawPlate(){
  display.fillRect(platePosition, SCREEN_HEIGHT - PLATE_HEIGHT, PLATE_WIDTH, PLATE_HEIGHT, WHITE);
}
void erasePlate(){
  display.fillRect(platePosition, SCREEN_HEIGHT - PLATE_HEIGHT, PLATE_WIDTH, PLATE_HEIGHT, BLACK);
}
void movePlate(int potValue){
  erasePlate();
  // Potansiyometre değerini doğrudan plate'in X pozisyonuna dönüştür
  // Eğer plate ekranın dışına çıkıyorsa, bunu önlemek için sınırları kontrol et
  platePosition = map(potValue, 0, 1023, 0, SCREEN_WIDTH - PLATE_WIDTH);

  drawPlate();
}
void drawBall(){
  display.fillCircle(ballX, ballY, BALL_SIZE, WHITE);
}
void eraseBall(){
  display.fillCircle(ballX, ballY, BALL_SIZE, BLACK);
}
void moveBall() {
  eraseBall();
  
  ballX += ballSpeedX;
  ballY += ballSpeedY;

  // Ekran kenarlarına çarpma kontrolü
  if (ballX <= 0 || ballX >= SCREEN_WIDTH - BALL_SIZE) {
    ballSpeedX = -ballSpeedX;
  }
  if (ballY <= 0) {
    ballSpeedY = -ballSpeedY;
  }
  else if (ballY >= SCREEN_HEIGHT - BALL_SIZE) {
    // Top ekranın altına düştüğünde can kaybını işle ve topu yeniden konumlandır
    checkBallLoss();
  }
  // Plakaya çarptığında yönünü değiştir
  if (ballY + BALL_SIZE >= SCREEN_HEIGHT - PLATE_HEIGHT &&
      ballX + BALL_SIZE >= platePosition && ballX <= platePosition + PLATE_WIDTH) {
    ballSpeedY = -ballSpeedY;
  }

  // Tuğlaları kontrol et ve kır
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      if (bricks[i][j]) {
        int brickX = j * (BRICK_WIDTH + BRICK_GAP) + BRICK_WIDTH / 2; // Tuğlanın merkezi
        int brickY = i * (BRICK_HEIGHT + BRICK_GAP) + BRICK_HEIGHT / 2; // Tuğlanın merkezi
        // Topun tuğla ile çarpışmasını kontrol et
        if (sqrt(pow(ballX + BALL_SIZE / 2 - brickX, 2) + pow(ballY + BALL_SIZE / 2 - brickY, 2)) < BALL_SIZE / 2 + BRICK_WIDTH / 2) {
          bricks[i][j] = false; // Tuğlayı yok et
          score++; // Skoru artır
          displayScore(); // Skoru ekrana yazdır
          ballSpeedY = -ballSpeedY; // Yönü değiştir
          brickHit(i, j); // Tuğla vurulduğunda özel etkileri işle
          break; // Döngüden çık, çünkü bir tuğla zaten kırıldı
        }
      }
    }
  }

  drawBall();
}
void drawBricks()
{
  for (int i = 0; i < BRICK_ROWS; i++)
  {
    for (int j = 0; j < BRICK_COLUMNS; j++)
    {
      if (bricks[i][j])
      {
        int brickX = j * (BRICK_WIDTH + BRICK_GAP);
        int brickY = i * (BRICK_HEIGHT + BRICK_GAP);
        display.fillRect(brickX, brickY, BRICK_WIDTH, BRICK_HEIGHT, WHITE);
      }
    }
  }
}
void displayScore() {
  int digit1 = score / 10; // Onlar basamağı
  int digit2 = score % 10; // Birler basamağı
  
  digitalWrite(30, HIGH);
  digitalWrite(29, LOW);
  dis(digit1); // Onlar basamağını yazdır
  delay(1);

  // Eğer skor 10 veya daha fazlaysa, onlar basamağını göster
  if (digit2 > 0) {
    digitalWrite(29, HIGH); // Önceki hane kapalı
    digitalWrite(30, LOW);  // Bu hane açık
    dis(digit2);
    delay(1); // Gözlemleyebilmek için yeterli süre
  }
}
void dis(int num){
  // 7 segment display'i güncelle
  for (int i = 22; i < 29; i++)
  {
    digitalWrite(i, bitRead(digit[num], i - 22));
  }
}
// Oyunu sonlandır ve sonuçları göster
void gameOver() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(F("Oyunu\nKaybettiniz!"));
  display.setCursor(0, 30);
  display.print(F("Skor: "));
  display.println(score);
  display.display();
  delay(3000); // Skoru 3 saniye göster
  // Oyunu sıfırla
  resetGame();
  while (!((isButtonPressed(buttonPinStart) || isButtonPressed(buttonPinUp) || isButtonPressed(buttonPinDown)))) {
      // Tuşa basılmayı bekleyelim
    }
}
// Oyunu sıfırlama işlemleri
void resetGame() {
  // Skoru ve canları sıfırla
  score = 0;
  lives = 3;
  // Oyun durumunu ana menüye ayarla
  currentGameState = GAME_MENU;
  // Ana menüyü göster
  displayMenu();
}
void increaseBallSpeed() {
  // Topun hızını %20 artır
  ballSpeedX *= 1.2;
  ballSpeedY *= 1.2;
}
bool isButtonPressed(int buttonPin) {
  static unsigned long lastDebounceTime = 0;
  const long debounceDelay = 50;
  
  bool pressed = false;
  if (digitalRead(buttonPin) == LOW) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      pressed = true;
      lastDebounceTime = millis();
    }
  }
  return pressed;
}
// Can göstergesini güncelle
void updateLivesDisplay() {
  for (int i = 0; i < 3; i++) {
    if (i < lives) {
      digitalWrite(ledPins[i], HIGH); // Can varsa LED'i yak
    } else {
      digitalWrite(ledPins[i], LOW); // Can yoksa veya azaldıysa LED'i söndür
    }
  }
}
/*
// Top aşağı düştüğünde can kaybını işle
void checkBallLoss() {
  lives--; // Canı azalt
  updateLivesDisplay(); // Can göstergesini güncelle
  if (lives > 0) {
    resetBall(); // Topu sıfırla ve oyunu devam ettir
  }
   else {
    gameOver(); // Canlar bittiğinde oyunu sonlandır
  }
  
  if (ballY >= SCREEN_HEIGHT) {
    lives--; // Canı azalt
    if (lives > 0) {
      resetBall(); // Topu sıfırla ve oyunu devam ettir
    } else {
      currentGameState = GAME_EXIT; // Canlar bitti, oyunu sonlandır
    }
    resetBall(); // Topu yeniden başlat
    if (lives <= 0) {
      displayScoreAndReset(); // Skoru göster ve sıfırla
    }
    updateLivesDisplay();
  }
  
}
*/
// Top aşağı düştüğünde can kaybını işle
void checkBallLoss() {
  lives--; // Canı azalt
  if (lives <= 0) {
    // Canlar bitti, oyunu sonlandır
    gameOver();
  } else {
    // Can kaldıysa topu sıfırla
    resetBall();
  }
  updateLivesDisplay(); // Can göstergesini güncelle
}
// Işık sensörüne göre renkleri güncelle
void updateDisplayColors() {
  int lightValue = analogRead(lightSensorPin);
  if (lightValue < someThreshold) {
    // Düşük ışıkta: Arka planı siyah, tuğlalar beyaz
    display.fillScreen(BLACK);
    for (int i = 0; i < BRICK_ROWS; i++) {
      for (int j = 0; j < BRICK_COLUMNS; j++) {
        if (bricks[i][j]) {
          display.fillRect(j * (BRICK_WIDTH + BRICK_GAP), i * (BRICK_HEIGHT + BRICK_GAP), BRICK_WIDTH, BRICK_HEIGHT, WHITE);
        }
      }
    }
  } else {
    // Yüksek ışıkta: Arka planı beyaz, tuğlalar siyah
    display.fillScreen(WHITE);
    for (int i = 0; i < BRICK_ROWS; i++) {
      for (int j = 0; j < BRICK_COLUMNS; j++) {
        if (bricks[i][j]) {
          display.fillRect(j * (BRICK_WIDTH + BRICK_GAP), i * (BRICK_HEIGHT + BRICK_GAP), BRICK_WIDTH, BRICK_HEIGHT, BLACK);
        }
      }
    }
  }
}
// Oyunda bekleme ekranı ve yeni bölüm başlatma
void checkBricks() {
  bool allBricksDestroyed = true;
  for (int i = 0; i < BRICK_ROWS; i++) {
    for (int j = 0; j < BRICK_COLUMNS; j++) {
      if (bricks[i][j]) {
        allBricksDestroyed = false;
        break;
      }
    }
    if (!allBricksDestroyed) {
      break;
    }
  }

  if (allBricksDestroyed) {
    displayLevelComplete();
    delay(1500); // 2 saniye bekle
    currentLevel++; // Seviyeyi artır
    if (currentLevel > 5) {
      currentLevel = 1; // 5. seviyeden sonra tekrar 1. seviyeye dön
    }
   
    increaseBallSpeed();
    loadLevel(currentLevel);
    currentGameState = GAME_RUNNING; // Yeni bölümü başlat
  }
}
// Can artırma objesinin hareketini yönetme
void moveLifeUp() {
  // Eğer can artırma objesi aktifse
  if (isLifeUpFalling) {
    // Objeyi aşağı doğru hareket ettir
    lifeUpY += 2;
    // Eğer obje paletin konumunda ve ekranın altına ulaşırsa
    if (lifeUpY > SCREEN_HEIGHT - PLATE_HEIGHT && lifeUpX >= platePosition && lifeUpX <= platePosition + PLATE_WIDTH) {
      // Palet tarafından yakalandıysa canı artır ve objeyi devre dışı bırak
      lives++;
      isLifeUpFalling = false;
      updateLivesDisplay(); // Can göstergesini güncelle
    } else if (lifeUpY >= SCREEN_HEIGHT) {
      // Ekranın altına ulaştıysa objeyi devre dışı bırak
      isLifeUpFalling = false;
    }
    // Ekranda çiz
    display.clearDisplay(); // Ekranı temizle
    drawPlate(); // Plakayı çiz
    drawBall(); // Topu çiz
    drawBricks(); // Tuğlaları çiz
    display.fillCircle(lifeUpX, lifeUpY, 3, WHITE); // Can artırma objesini çiz
    display.display(); // Ekranı güncelle
  }
}

// Her tuğla kırıldığında %10 şans ile can artırma objesi oluşturulur
void brickHit(int row, int col) {
  if (random(10) < 1) { // %10 şans
    isLifeUpFalling = true;
    // Can artırma objesinin başlangıç pozisyonunu ayarla
    lifeUpX = col * (BRICK_WIDTH + BRICK_GAP) + BRICK_WIDTH / 2;
    lifeUpY = row * (BRICK_HEIGHT + BRICK_GAP);
  }
}
// Topu sıfırla ve oyunu devam ettir
void resetBall() {
  int potValue = analogRead(A0); // Potansiyometre değerini oku
  platePosition = map(potValue, 0, 1023, 0, SCREEN_WIDTH - PLATE_WIDTH);
   ballX = platePosition + (PLATE_WIDTH / 2) - (BALL_SIZE / 2);
  ballY = SCREEN_HEIGHT - PLATE_HEIGHT - BALL_SIZE; // Topu paletin üstünde başlat
  // Reset ball Y position

 
  display.clearDisplay();
  drawPlate();
  drawBricks();
  drawBall();
  display.display();
  delay(1000); // Oyuncuya hazırlanma süresi ver
}
void displayLevelComplete() {
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor((SCREEN_WIDTH - (12 * 6)) / 2, SCREEN_HEIGHT / 2 - 8); // 12px harf genişliği, 6 harf "Level " kelimesi
  display.print(F("Level "));
  display.println(currentLevel);
  display.display();
}