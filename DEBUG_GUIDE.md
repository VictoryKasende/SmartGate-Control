# 🔍 SmartGate Debug System

## 📊 Informations de Debug Disponibles

Le système de debug vous donnera les informations suivantes au démarrage et pendant le fonctionnement :

### 🔄 Au Démarrage (Boot)
```
==================================================
🔄 BOOT #1 - DEBUG MODE ENABLED
==================================================
🔍 Reset Reason: POWERON
   → Normal power-on reset
📊 Chip Model: ESP32
📊 Chip Revision: 3
📊 CPU Frequency: 240 MHz
📊 Flash Size: 4194304 bytes
📊 Free Heap: 298765 bytes
📊 Min Free Heap: 298765 bytes
📊 Stack High Water Mark: 1234 bytes
✅ Debug Helper initialized
```

### ⚠️ Causes de Redémarrage
Le système identifie automatiquement la cause du redémarrage :

- **POWERON** : Démarrage normal (alimentation)
- **EXTERNAL** : Reset button pressé
- **SOFTWARE** : Redémarrage par code (restart())
- **PANIC** : ⚠️ **Crash du code** (erreur fatale)
- **INT_WATCHDOG** : ⚠️ **Tâche principale bloquée**
- **TASK_WATCHDOG** : ⚠️ **Tâche spécifique bloquée**
- **HW_WATCHDOG** : ⚠️ **Système complètement figé**
- **BROWNOUT** : ⚠️ **Problème d'alimentation**

### 💾 Surveillance Mémoire
```
💾 Memory: Free=245678, Min=234567, MinSeen=234567
⚠️  LOW MEMORY WARNING: 45000 bytes free (min seen: 40000)
```

### 🔧 Opérations Critiques Tracées
```
🔧 Critical Operation: Initializing Distance Sensor (Heap: 298765)
🔧 Critical Operation: ESP32CAM Photo Request START (Heap: 245678)
🔧 Critical Operation: ESP32CAM Photo Request SUCCESS (Heap: 245123)
```

## 🎯 Comment Utiliser

### 1. **Upload le nouveau firmware**
```bash
pio run --target upload
```

### 2. **Ouvrir le moniteur série**
```bash
pio device monitor
```

### 3. **Analyser les logs de redémarrage**

**Si ça redémarre encore**, regardez le message au démarrage :

#### ✅ **Redémarrage Normal**
```
🔍 Reset Reason: POWERON
   → Normal power-on reset
```
→ Tout va bien, c'est un démarrage normal

#### ❌ **Problème de Watchdog** 
```
🔍 Reset Reason: TASK_WATCHDOG
   ⚠️  TASK WATCHDOG - Specific task blocked!
```
→ Une tâche est bloquée (HTTP request, lecture capteur, etc.)

#### ❌ **Crash du Code**
```
🔍 Reset Reason: PANIC
   ⚠️  PANIC RESET - Code crashed!
```
→ Erreur fatale dans le code

#### ❌ **Problème d'Alimentation**
```
🔍 Reset Reason: BROWNOUT
   ⚠️  BROWNOUT - Power supply issue!
```
→ Alimentation insuffisante ou instable

### 4. **Surveillance en Temps Réel**

Pendant le fonctionnement, surveillez :

- **Mémoire** : Si `Free Heap` descend sous 50KB → problème
- **Opérations critiques** : Si ça se bloque sur une opération spécifique
- **Stack** : Si Stack High Water Mark < 1024 bytes → problème

## 🚨 Actions selon les Erreurs

### TASK_WATCHDOG ou INT_WATCHDOG
- **Cause** : HTTP requests trop longs, capteur bloqué
- **Solution** : Réduire encore les timeouts, vérifier ESP32-CAM

### PANIC
- **Cause** : Division par zéro, accès mémoire invalide
- **Solution** : Bug dans le code, vérifier les pointeurs

### BROWNOUT
- **Cause** : Alimentation faible (< 3.0V)
- **Solution** : Meilleure alimentation, câbles plus courts

### LOW MEMORY WARNING
- **Cause** : Fuite mémoire, trop de variables
- **Solution** : Optimiser le code, libérer la mémoire

## 📈 Surveillance Continue

Le système affiche toutes les 30 secondes :
```
📊 Status - Distance: 25.3 cm | Gate: CLOSED | Heap: 245678
💾 Memory: Free=245678, Min=234567, MinSeen=234567
```

**Surveillez** :
- Si `Free Heap` diminue constamment → fuite mémoire
- Si `Distance` reste à 999.0 → problème capteur
- Si pas de logs → système figé

---

## 🎯 Résumé

Avec ce système, vous saurez **exactement** pourquoi l'ESP32 redémarre et où ça bloque !

La prochaine fois que ça redémarre, copiez-moi les **premiers messages** du moniteur série pour qu'on identifie la cause exacte.
