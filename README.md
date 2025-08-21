# SmartGate Control API

Un système de barrière intelligente modulaire utilisant une **architecture séparée** avec ESP32 principal et ESP32-CAM dédié pour plus de stabilité et de performance.

## 🏗️ Architecture Modulaire

### 🔧 **ESP32 Principal** (Contrôleur API - ACTUEL)
- **Rôle** : API REST, capteur ultrasonique, servo moteur, interface web
- **Fichier** : `main.cpp` (version actuelle)
- **IP** : Attribution dynamique WiFi

### 📷 **ESP32-CAM** (Serveur Photo Dédié)
- **Rôle** : Capture et stockage des photos uniquement
- **IP Fixe** : `10.244.250.144`
- **Communication** : HTTP Client depuis ESP32 principal (JSON seulement)

## Fonctionnalités

- 📏 **Mesure de distance** via capteur ultrasonique (seuil: 20cm)
- 🚪 **Contrôle de barrière** via servo moteur (0° fermé, 90° ouvert)
- 📸 **Capture de photos** via ESP32-CAM dédié (**manuelle uniquement, anti-crash**)
- 🌐 **API REST complète** avec interface web ultra-minimaliste
- ⚙️ **Monitoring système** (heap, uptime, connectivité)
- 🚫 **Auto-photo désactivée** (prévention redémarrages)
- 🔍 **Système debug avancé** (identification causes redémarrage)

## Architecture Technique

### Composants principaux:
- `ESP32APIServer`: Serveur web avec endpoints REST et interface ultra-légère
- `DistanceSensor`: Gestion capteur ultrasonique HC-SR04 optimisé
- `ServoController`: Contrôle servo moteur avec positions définies  
- `ESP32CAMClient`: Client HTTP ESP32-CAM (JSON uniquement, anti-crash)
- `DebugHelper`: Système debug avancé pour identifier redémarrages

### Configuration matérielle (ESP32):
- **Capteur ultrasonique**: TRIG_PIN 5, ECHO_PIN 18
- **Servo moteur**: SERVO_PIN 13
- **LED Status**: LED_PIN 12
- **WiFi**: WINS / WINNER20

## Fonctionnalités anti-crash et debug

### Protection mémoire
- **Images**: Aucune image chargée en mémoire ESP32 (anti-crash)
- **API**: JSON uniquement avec StaticJsonDocument pour économiser RAM
- **Interface**: Web UI ultra-minimaliste (HTML pur, sans CSS/design)

### Système debug avancé
- **DebugHelper**: Logs détaillés des redémarrages et état système
- **Watchdog**: Surveillance et alimentation automatique pour éviter resets
- **Mémoire**: Monitoring heap en temps réel
- **Opérations critiques**: Logs des actions importantes pour diagnostic

### Optimisations performance
- Réduction drastique utilisation mémoire
- Suppression détection automatique photos (économie CPU)
- Timeouts HTTP courts pour éviter blocages
- Interface web minimale (pas de design, fonctionnel uniquement)

## Endpoints API

### GET /api/status
Retourne le statut complet du système
```json
{
  "distance": 15.2,
  "gate": false,
  "auto_photo": false,
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
Demande de capture photo via ESP32-CAM (**manuelle uniquement**)
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
Toggle auto-photo on/off (**fonctionnalité désactivée**)
```json
{
  "status": "success",
  "auto_photo": false,
  "message": "Auto photo feature is disabled"
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
- **Intervalles**: UPDATE_INTERVAL_MS=1000
- **Auto-photo**: Désactivée dans le code (AUTO_PHOTO_INTERVAL_MS commenté)

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

# Prendre une photo via ESP32-CAM (manuelle)
curl -X POST "http://[IP_ESP32]/api/photo"

# Statut système complet
curl "http://[IP_ESP32]/api/status"

# Distance actuelle
curl "http://[IP_ESP32]/api/distance"

# Statut ESP32-CAM
curl "http://[IP_ESP32]/api/esp32cam"
```

## Fonctionnement du Système

- **Monitoring continu** : Lecture distance toutes les 1000ms
- **Capture photo manuelle** : Via API `/api/photo` uniquement
- **Auto-photo désactivée** : Plus de déclenchement automatique pour éviter les erreurs
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
        """Capture manuelle uniquement"""
        response = requests.post(f"{self.base_url}/api/photo")
        return response.json()
    
    def get_distance(self):
        response = requests.get(f"{self.base_url}/api/distance")
        return response.json()["distance"]

# Utilisation
gate = SmartGateAPI("192.168.1.100")
status = gate.get_status()
print(f"Distance: {status['distance']} cm")
print(f"Auto-photo: {status['auto_photo']} (désactivée)")
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
        // Capture manuelle uniquement
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
- **Auto-photo désactivée** : Pour éviter les erreurs "Auto photo request failed"
- **Capture manuelle uniquement** : Via API `/api/photo` ou interface web
- **Monitoring système** : Heap memory, uptime, connectivité ESP32-CAM

## 🚫 Auto-Photo Désactivée

L'auto-photo a été **désactivée par défaut** pour éviter les erreurs de communication. Les photos restent disponibles en **mode manuel uniquement** :

- ✅ **Capture manuelle** : `POST /api/photo`
- ✅ **Interface web** : Bouton "Capture Photo"
- ❌ **Déclenchement automatique** : Désactivé
- ❌ **Toggle auto** : Fonctionnalité inactive

### Réactivation future (si nécessaire)
Pour réactiver l'auto-photo plus tard :
1. Décommentez le code dans `ESP32APIServer.cpp::handleAutoPhoto()`
2. Décommentez `apiServer.handleAutoPhoto()` dans `main.cpp`
3. Changez `autoPhotoEnabled(false)` en `autoPhotoEnabled(true)`

---

## 🚀 Déploiement Rapide

1. **Configurer ESP32-CAM** (IP: 10.253.254.144)
2. **Compiler ESP32 principal** : `pio run --target upload`
3. **Accéder interface web** : IP affichée dans moniteur série
4. **Tester API** : Utiliser curl ou interface web intégrée
5. **Captures photos** : Mode manuel uniquement via `/api/photo`