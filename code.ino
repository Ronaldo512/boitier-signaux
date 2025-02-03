#include <DS1302.h>

// Broches de connexion du module DS1302
#define RST_PIN 2
#define DAT_PIN 3
#define CLK_PIN 4

// Broche du relais
#define relayPin 6 // Pin utilisée pour contrôler le relais

// Création d'une instance de la classe DS1302
DS1302 rtc(RST_PIN, DAT_PIN, CLK_PIN);

#define pwm 5 // Pin PWM

// Durée et paramètres du cycle
unsigned long impulseDuration = 1000; 
unsigned long pauseDuration = 5000; 
const unsigned long cycleDuration = 5000; 
const unsigned long totalDuration = 120000; 

// Variables de gestion des impulsions et de la pause
int pulseCount = 0;
float dutyCycle[5] = {0.05, 0.10, 0.20, 0.40, 0.80};
unsigned long pulseStartTime = 0;
float currentDutyCycle = 0;
int seriesCount = 0; 
bool isPause = false; 
unsigned long pauseStartMillis = 0; 
unsigned long cycleStartMillis = 0; 

void setup() {
  pinMode(pwm, OUTPUT);
  pinMode(relayPin, OUTPUT);

  Serial.begin(9600);

  // Vérifier si l'horloge fonctionne correctement
  rtc.writeProtect(false);
  rtc.halt(false);
}

void loop() {
  Time now = rtc.time();

  if ((now.hr == 17 && now.min == 42) || (now.hr == 17 && now.min == 47)) {
    Serial.print("Heure de démarrage atteinte : ");
    printTime(now);
    Serial.println();

    cycleStartMillis = millis();
    isPause = false;
    pulseCount = 0;
    seriesCount = 0;

    // Exécuter le cycle complet
    while (millis() - cycleStartMillis < totalDuration) {
      unsigned long currentMillis = millis();

      if (isPause) {
        // Gérer la pause
        if (currentMillis - pauseStartMillis >= pauseDuration) {
          // Terminer la pause et recommencer le cycle
          isPause = false;
          pulseCount = 0;
          pulseStartTime = currentMillis; 
          seriesCount++;
          if (seriesCount >= 5) {
            seriesCount = 0;
          }
          Serial.println("Reprise après pause...");
        }
      } else {
        // Gérer les impulsions
        unsigned long elapsed = currentMillis - pulseStartTime;
        if (elapsed % impulseDuration == 0 && (elapsed / impulseDuration) < 5) {
         // Mise à jour du rapport cyclique en fonction de la série en cours
          currentDutyCycle = dutyCycle[seriesCount];
          Serial.print("Impulsion ");
          Serial.print((elapsed / impulseDuration) + 1);
          Serial.print(" - Rapport cyclique : ");
          Serial.print(currentDutyCycle * 100);
          Serial.println(" %");
        }

        // Contrôler le signal PWM en fonction du rapport cyclique
        unsigned long timeInImpulse = elapsed % impulseDuration;
        if (timeInImpulse < (impulseDuration * currentDutyCycle)) {
          analogWrite(pwm, 255); 
        } else {
          analogWrite(pwm, 0);
        }

        digitalWrite(relayPin, HIGH);

        // Si le cycle des impulsions est terminé, passer en mode pause
        if (elapsed >= cycleDuration) {
          isPause = true;
          pauseStartMillis = currentMillis; 
          digitalWrite(pwm, LOW); 
          digitalWrite(relayPin, LOW); 
          Serial.println("Début de la pause...");
        }
      }
    }

    Serial.println("Cycle complet terminé.");
    digitalWrite(pwm, LOW);
    digitalWrite(relayPin, LOW); 

    // Attendre jusqu'à ce que la minute actuelle soit passée pour éviter de redémarrer immédiatement
    while (rtc.time().min == now.min) {
      delay(1000);
    }
  }
}

// Fonction pour afficher l'heure au format HH:MM:SS
void printTime(const Time &t) {
  if (t.hr < 10) {
    Serial.print("0");
  }
  Serial.print(t.hr);
  Serial.print(":");
  if (t.min < 10) {
    Serial.print("0");
  }
  Serial.print(t.min);
  Serial.print(":");
  if (t.sec < 10) {
    Serial.print("0");
  }
  Serial.print(t.sec);
}
