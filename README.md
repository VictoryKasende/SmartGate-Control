# SmartGate Control API

Un système de barrière intelligente modulaire utilisant une **architecture séparée** avec ESP32 principal et ESP32-CAM dédié pour plus de stabilité et de performance.

## 🏗️ Architecture Modulaire

### 🔧 **ESP32 Principal** (Contrôleur API - ACTUEL)
- **Rôle** : API REST, capteur ultrasonique, servo moteur, interface web
- **Fichier** : `main.cpp` (version actuelle)
- **IP** : Attribution dynamique WiFi

### 📷 **ESP32-CAM** (Serveur Photo Dédié)
- **Rôle** : Capture et stockage des photos uniquement
- **IP Fixe** : `10.253.254.144`
- **Communication** : HTTP Client depuis ESP32 principal

## Fonctionnalités

- 📏 **Mesure de distance** via capteur ultrasonique (seuil: 20cm)
- 🚪 **Contrôle de barrière** via servo moteur (0° fermé, 90° ouvert)
- � **Capture de photos** via ESP32-CAM dédié (manuelle et automatique)
- 🌐 **API REST complète** avec interface web responsive
- � **Auto-photo activable/désactivable** avec intervalle configurable
- � **Monitoring système** (heap, uptime, connectivité)

## Architecture Technique

### Composants principaux:
- `ESP32APIServer`: Serveur web avec endpoints REST et interface graphique
- `DistanceSensor`: Gestion capteur ultrasonique HC-SR04
- `ServoController`: Contrôle servo moteur avec positions définies  
- `ESP32CAMClient`: Client HTTP pour communication avec ESP32-CAM

### Configuration matérielle (ESP32):
- **Capteur ultrasonique**: TRIG_PIN 2, ECHO_PIN 4
- **Servo moteur**: SERVO_PIN 18
- **LED Status**: LED_PIN 2
- **WiFi**: WINS / WINNER20

## Endpoints API

### GET /api/status
Retourne le statut complet du système
```json
{
  "distance": 15.2,
  "gate": false,
  "auto_photo": true,
  "esp32cam_ip": "10.253.254.144",
  "esp32cam_reachable": true,
  "free_heap": 234567,
  "uptime": 123456
}
```

### GET /api/distance
Retourne les données du capteur de distance
```json
{
  "distance": 15.2,
  "detected": false,
  "threshold": 20
}
```

### GET /api/gate
Retourne le statut de la barrière
```json
{
  "gate": false,
  "position": 0
}
```

### POST /api/gate?action=[open|close]
Contrôle la barrière
```json
{
  "status": "success",
  "gate": true,
  "position": 90,
  "action": "opened"
}
```

### POST /api/photo
Demande de capture photo via ESP32-CAM
```json
{
  "status": "success",
  "message": "Photo request sent to ESP32-CAM",
  "esp32cam_ip": "10.253.254.144"
}
```

### POST /api/photo/capture
Alias pour capture photo directe
```json
{
  "status": "success", 
  "message": "Photo capture requested",
  "esp32cam_ip": "10.253.254.144"
}
```

### POST /api/auto
Toggle auto-photo on/off
```json
{
  "status": "success",
  "auto_photo": true,
  "message": "Auto photo enabled"
}
```

### GET /api/esp32cam
Retourne le statut de l'ESP32-CAM
```json
{
  "status": "response from ESP32-CAM /status endpoint"
}
```

## Configuration

Modifiez le fichier `include/ESP32Config.h` pour:
- **WiFi**: WIFI_SSID = "WINS", WIFI_PASSWORD = "WINNER20"
- **ESP32-CAM IP**: ESP32CAM_IP = "10.253.254.144"
- **Seuil de détection**: DETECTION_DISTANCE_CM = 20cm
- **Pins**: SERVO_PIN=18, TRIG_PIN=2, ECHO_PIN=4, LED_PIN=2
- **Intervalles**: UPDATE_INTERVAL_MS=1000, AUTO_PHOTO_INTERVAL_MS=5000

## Compilation et Déploiement

### ESP32 Principal (API Server) - Configuration Actuelle
```bash
# Compiler et uploader sur ESP32 principal
pio run --target upload

# Monitoring série
pio device monitor
```

### Interface Web
- Accédez à l'IP affichée dans le moniteur série
- Interface responsive avec contrôles temps réel
- Monitoring automatique du statut système

## Tests API et Utilisation

### Tests avec curl
```bash
# Ouvrir la barrière
curl -X POST "http://[IP_ESP32]/api/gate?action=open"

# Fermer la barrière  
curl -X POST "http://[IP_ESP32]/api/gate?action=close"

# Prendre une photo via ESP32-CAM
curl -X POST "http://[IP_ESP32]/api/photo"

# Statut système complet
curl "http://[IP_ESP32]/api/status"

# Distance actuelle
curl "http://[IP_ESP32]/api/distance"

# Toggle auto-photo
curl -X POST "http://[IP_ESP32]/api/auto"

# Statut ESP32-CAM
curl "http://[IP_ESP32]/api/esp32cam"
```

## Fonctionnement Automatique

- **Monitoring continu** : Lecture distance toutes les 1000ms
- **Auto-photo** : Déclenchable quand objet détecté < 20cm (intervalle 5000ms)
- **Architecture séparée** : ESP32 principal gère l'API, ESP32-CAM gère uniquement les photos
- **Communication HTTP** : Requêtes vers ESP32-CAM via client HTTP intégré
- **Interface Web** : Mise à jour temps réel via JavaScript

## Exemple d'intégration

### Python
```python
import requests

class SmartGateAPI:
    def __init__(self, esp32_ip):
        self.base_url = f"http://{esp32_ip}"
    
    def get_status(self):
        response = requests.get(f"{self.base_url}/api/status")
        return response.json()
    
    def open_gate(self):
        response = requests.post(f"{self.base_url}/api/gate?action=open")
        return response.json()
    
    def close_gate(self):
        response = requests.post(f"{self.base_url}/api/gate?action=close")
        return response.json()
    
    def capture_photo(self):
        response = requests.post(f"{self.base_url}/api/photo")
        return response.json()
    
    def get_distance(self):
        response = requests.get(f"{self.base_url}/api/distance")
        return response.json()["distance"]
    
    def toggle_auto_photo(self):
        response = requests.post(f"{self.base_url}/api/auto")
        return response.json()

# Utilisation
gate = SmartGateAPI("192.168.1.100")
status = gate.get_status()
print(f"Distance: {status['distance']} cm")
```

### JavaScript (Web)
```javascript
class SmartGateAPI {
    constructor(baseUrl) {
        this.baseUrl = baseUrl;
    }
    
    async getStatus() {
        const response = await fetch(`${this.baseUrl}/api/status`);
        return await response.json();
    }
    
    async controlGate(action) {
        const response = await fetch(`${this.baseUrl}/api/gate?action=${action}`, {
            method: 'POST'
        });
        return await response.json();
    }
    
    async capturePhoto() {
        const response = await fetch(`${this.baseUrl}/api/photo`, {
            method: 'POST'
        });
        return await response.json();
    }
}
```

## Dépendances (platformio.ini)

```ini
[env:esp32_normal]
platform = espressif32
board = esp32dev
framework = arduino

lib_deps = 
    esphome/ESPAsyncWebServer-esphome@^3.2.2
    madhephaestus/ESP32Servo@^3.0.5
    bblanchon/ArduinoJson@^7.0.4
```

## Structure du Projet

```
SmartGate Control/
├── include/
│   ├── ESP32Config.h          # Configuration centralisée
│   ├── DistanceSensor.h       # Capteur ultrasonique
│   ├── ServoController.h      # Contrôle servo moteur
│   ├── ESP32CAMClient.h       # Client HTTP ESP32-CAM
│   └── ESP32APIServer.h       # Serveur web/API
├── src/
│   ├── DistanceSensor.cpp     # Implémentation capteur
│   ├── ServoController.cpp    # Implémentation servo
│   ├── ESP32CAMClient.cpp     # Implémentation client HTTP
│   ├── ESP32APIServer.cpp     # Implémentation serveur web
│   └── main.cpp               # Programme principal ESP32
├── platformio.ini             # Configuration ESP32 principal
└── README.md                  # Documentation
```

## Notes Importantes

- **Architecture modulaire** : ESP32 principal (API) + ESP32-CAM dédié (photos)
- **IP Fixe ESP32-CAM** : Configurée à `10.253.254.144` pour communication stable
- **Interface Web Intégrée** : Interface responsive avec monitoring temps réel
- **Communication HTTP** : ESP32 principal communique avec ESP32-CAM via HTTP
- **Auto-photo configurable** : Activable/désactivable via API ou interface web
- **Monitoring système** : Heap memory, uptime, connectivité ESP32-CAM

---

## 🚀 Déploiement Rapide

1. **Configurer ESP32-CAM** (IP: 10.253.254.144)
2. **Compiler ESP32 principal** : `pio run --target upload`
3. **Accéder interface web** : IP affichée dans moniteur série
4. **Tester API** : Utiliser curl ou interface web intégrée
