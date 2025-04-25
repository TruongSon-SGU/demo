#include <Wire.h>
#include "SH1106Wire.h"
#include "images.h"

#define MAIN_MENU_SCREEN 0
#define GAMEPLAY_SCREEN 1
#define GAMEOVER_SCREEN 2
#define BIRD_WIDTH 14
#define BIRD_HEIGHT 9

SH1106Wire display(0x3c, 11, 12); // OLED SH1106

float wall[4];
int empty[4];
int space = 32;
int widthOfPassage = 30;
int gameState = MAIN_MENU_SCREEN;

const int buttonPin = D3;
const int buzzerPin = 2;

int buttonState = 0;
int score = 0;
int isJumping = 0;
int currentTime = 0;
int direction = 1;
int play = 0;
bool gameOverDisplayed = false; // Thêm biến để kiểm tra đã hiển thị màn hình game over chưa
unsigned long gameOverStartTime = 0; // Thời điểm bắt đầu màn hình game over
const unsigned long gameOverDelay = 5000; // Thời gian delay 5 giây
bool canRestart = false; // Biến cho phép restart sau delay

float birdX = 5.00;
float birdY = 22.00;

unsigned long ton = 0;

void setup() {
  display.init();
  display.flipScreenVertically();
  generateWalls();
  display.drawRect(0, 0, 128, 63);
  delay(500);
  pinMode(buttonPin, INPUT);
  pinMode(A0, INPUT);
  pinMode(buzzerPin, OUTPUT);
  digitalWrite(buzzerPin, LOW);
  Serial.begin(9600);
}

void loop() {
  display.clear();
  buttonState = digitalRead(buttonPin);

  if (gameState == MAIN_MENU_SCREEN) {
    showMainMenuScreen();
  } else if (gameState == GAMEPLAY_SCREEN) {
    showGamePlayScreen();
  } else if (gameState == GAMEOVER_SCREEN) {
    showGameOverScreen();
  }
  display.display();
}

void showMainMenuScreen() {
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 4, "Flappy ");
  display.drawXbm(0, 0, 128, 64, background);
  drawBird(20, 32);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 44, "Press to start");
  if (buttonState == HIGH) {
    gameState = GAMEPLAY_SCREEN;
    resetGame(); // Thêm hàm resetGame để đặt lại các biến khi bắt đầu chơi mới
  }
}

void showGamePlayScreen() {
  if (gameState == GAMEPLAY_SCREEN) { // Chỉ thực hiện logic trò chơi khi ở trạng thái GAMEPLAY_SCREEN
    int val = analogRead(A0) * 12.5 / 100;
    display.setFont(ArialMT_Plain_10);
    display.drawString(3, 0, String(score));

    if (buttonState == HIGH) {
      if (isJumping == 0) {
        currentTime = millis();
        ton = millis();
        direction = 1;
        play = 1;
        isJumping = 1;
      }
    } else {
      isJumping = 0;
    }

    for (int i = 0; i < 4; i++) {
      display.setColor(WHITE);
      display.fillRect(wall[i], 0, 6, 64);
      display.setColor(BLACK);
      display.fillRect(wall[i], empty[i], 6, widthOfPassage);
      wall[i] -= 0.005; // Tốc độ tường chậm hơn
      if (wall[i] < 0) {
        score = score + 1;
        empty[i] = random(8, 32);
        wall[i] = 128 + val;
      }

      if (wall[i] <= birdX + 7 && birdX + 7 <= wall[i] + 6) {
        if (birdY < empty[i] || birdY + BIRD_HEIGHT > empty[i] + widthOfPassage) {
          gmaeOver();
        }
      }
    }

    drawBird(birdX, birdY);

    if ((currentTime + 185) < millis()) {
      direction = 0;
    }
    if ((ton + 40) < millis()) {
      play = 0;
    }

    if (direction == 0) {
      birdY += 0.002; // Rơi rất chậm
    } else {
      birdY -= 0.01; // Giảm tốc độ nhảy lên (chậm hơn)
    }

    if (birdY >= 64 || birdY < -3) {
      gmaeOver();
    }

    display.drawRect(0, 0, 128, 63);
  } else if (gameState == GAMEOVER_SCREEN) {
    // Khi ở màn hình GAMEOVER, không thực hiện bất kỳ logic di chuyển nào
    drawBird(birdX, birdY); // Vẽ lại con chim ở vị trí cuối cùng
    for (int i = 0; i < 4; i++) {
      display.setColor(WHITE);
      display.fillRect(wall[i], 0, 6, 64);
      display.setColor(BLACK);
      display.fillRect(wall[i], empty[i], 6, widthOfPassage);
    }
    display.drawRect(0, 0, 128, 63);
  }
}

void showGameOverScreen() {
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 4, "Son tuoi tre :)) !"); // Thay "Score:" bằng "Quá ngu!"
  display.drawXbm(0, 0, 128, 64, background);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 25, "DIEM NE: " + String(score)); // Vẫn hiển thị điểm ở dòng dưới

  if (millis() - gameOverStartTime >= gameOverDelay) {
    display.drawString(0, 44, "Press to restart"); // Hiển thị nút nhấn sau delay
    canRestart = true; // Cho phép restart sau delay
  } else {
    display.drawString(0, 44, "TOP 1 GLOBAL   " + String((gameOverDelay - (millis() - gameOverStartTime)) / 1000) + "s..."); // Hiển thị thời gian đếm ngược
    canRestart = false; // Ngăn không cho restart trước delay
  }

  if (canRestart && buttonState == HIGH) {
    gameState = GAMEPLAY_SCREEN;
    resetGame(); // Gọi hàm resetGame khi chơi lại
  }
}

void drawBird(int x, int y) {
  display.setColor(WHITE);
  display.drawXbm(x, y, BIRD_WIDTH, BIRD_HEIGHT, bird);
}

void gmaeOver() {
  tone(buzzerPin, 800, 600); // Buzzer kêu khi thua
  gameState = GAMEOVER_SCREEN;
  gameOverDisplayed = true; // Đánh dấu đã hiển thị màn hình game over
  gameOverStartTime = millis(); // Lưu lại thời điểm chuyển sang màn hình game over
  canRestart = false; // Đặt lại trạng thái restart
}

void generateWalls() {
  for (int i = 0; i < 4; i++) {
    wall[i] = 128 + ((i + 1) * space);
    empty[i] = random(8, 32);
  }
}

void resetGame() {
  score = 0;
  birdY = 22.00;
  generateWalls();
  gameOverDisplayed = false; // Đặt lại biến khi bắt đầu trò chơi mới
  canRestart = false; // Đảm bảo restart không được kích hoạt ngay sau khi reset
  //test thoy nha dung de y 
}