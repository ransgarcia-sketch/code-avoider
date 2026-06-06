#include <AFMotor.h>
#include <NewPing.h>

// ================= MOTOR =================
AF_DCMotor motorKiri(1);  // M1
AF_DCMotor motorKanan(4); // M2

// ================= ULTRASONIC =================
#define TRIG_PIN A0
#define ECHO_PIN A1
#define MAX_DISTANCE 200

NewPing sonar(TRIG_PIN, ECHO_PIN, MAX_DISTANCE);

// ================= 1. BAGIAN SPEED & TIMING (UBAH DI SINI) =================
// FIX PARABOLA: Karena robot melengkung ke kanan, speed kanan sengaja dinaikkan
int speedMajuKiri   = 67; 
int speedMajuKanan  = 100; // <--- Atur angka ini jika masih kurang lurus

int speedBelok      = 85;

// FIX OVER CEK KANAN-KIRI: Waktu dipisah agar akurat
int waktuBelokKanan = 269; 
int waktuBelokKiri  = 261; // <--- Hasil kalibrasi agar kembali ke tengah pas 90 derajat

void setup() {
  motorKiri.setSpeed(speedMajuKiri);
  motorKanan.setSpeed(speedMajuKanan);

  Serial.begin(9600);
  delay(1000); // Waktu jeda sebelum robot mulai jalan pertama kali
}

// ================= 2. BAGIAN GERAK RODA (SUDAH DISESUAIKAN) =================
// ================= 2. BAGIAN GERAK RODA (SUDAH DIBALIK) =================
void maju() {
  // DIBALIK: Sekarang roda berputar ke arah belakang (searah sensor)
  motorKiri.setSpeed(speedMajuKiri);
  motorKanan.setSpeed(speedMajuKanan);
  motorKiri.run(BACKWARD);  // <--- Dibalik jadi BACKWARD
  motorKanan.run(BACKWARD); // <--- Dibalik jadi BACKWARD
}

void mundur() {
  // DIBALIK: Sekarang roda berputar menjauhi sensor
  motorKiri.run(FORWARD);  // <--- Dibalik jadi FORWARD
  motorKanan.run(FORWARD); // <--- Dibalik jadi FORWARD
}

void belokKiri() {
  // TETAP: Karena rotasi putar porosnya tidak berubah
  motorKiri.run(BACKWARD);
  motorKanan.run(FORWARD);
}

void belokKanan() {
  // TETAP: Karena rotasi putar porosnya tidak berubah
  motorKiri.run(FORWARD);
  motorKanan.run(BACKWARD);
}

void berhenti() {
  motorKiri.run(RELEASE);
  motorKanan.run(RELEASE);
}

// ================= FUNGSI BACA SENSOR =================
int bacaJarak() {
  delay(50); // Stabilkan bacaan
  int jarak = sonar.ping_cm();
  if (jarak == 0) jarak = 250; // Kalau gagal baca, anggap jauh
  return jarak;
}

// ================= LOGIKA UTAMA =================
void loop() {
  int jarakDepan = bacaJarak();
  Serial.println(jarakDepan);

  if (jarakDepan > 25) {
    // Jalur depan aman -> Maju normal
    maju();
  } 
  else if (jarakDepan > 15 && jarakDepan <= 25) {
    // Mulai dekat -> Pelankan kecepatan (dikurangi proporsional)
    motorKiri.setSpeed(speedMajuKiri - 20);
    motorKanan.setSpeed(speedMajuKanan - 20);
    maju();
  } 
  else {
    // ===== HALANGAN TERDETEKSI (<= 15 CM) =====
    berhenti();
    delay(300);

    // Siapkan kecepatan untuk memutar badan
    motorKiri.setSpeed(speedBelok);
    motorKanan.setSpeed(speedBelok);

    // --- CEK KANAN ---
    belokKanan(); 
    delay(waktuBelokKanan); 
    berhenti(); 
    delay(300);
    int jarakKanan = bacaJarak(); 
    
    // Kembali ke tengah
    belokKiri(); 
    delay(waktuBelokKiri);
    berhenti(); 
    delay(300);

    // --- CEK KIRI ---
    belokKiri(); 
    delay(waktuBelokKiri); 
    berhenti(); 
    delay(300);
    int jarakKiri = bacaJarak(); 

    // Kembali ke tengah
    belokKanan(); 
    delay(waktuBelokKanan);
    berhenti(); 
    delay(340);

    // --- 3. BAGIAN KEPUTUSAN (FIX PUTAR BALIK 180 KEBABLASAN) ---
    if (jarakKanan >= jarakKiri && jarakKanan > 15) { // <--- Ditambah tanda '='
      belokKanan();
      delay(waktuBelokKanan);
      berhenti(); // <--- Ditambah rem agar tidak kebablasan karena momentum
      delay(200);
    } 
    else if (jarakKiri > jarakKanan && jarakKiri > 15) {
      belokKiri();
      delay(waktuBelokKiri);
      berhenti(); // <--- Ditambah rem agar tidak kebablasan karena momentum
      delay(200);
    } 
    else {
      // Dua-duanya buntu -> Mundur lalu Putar Balik
      mundur();
      delay(400);
      belokKanan();
      delay(waktuBelokKanan * 2); 
      berhenti();
      delay(200);
    }
  }
}
