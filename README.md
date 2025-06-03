# QuizzTeam

**QuizzTeam** est une application de quiz en temps réel, construite en C++ avec Qt6. Le principe : une instance peut devenir « hôte » et envoyer une question à tous les autres clients connectés en LAN. Les participants répondent via l’interface, et les réponses sont recueillies et affichées sous forme de statistiques en direct. Il n’y a pas de serveur central ; la communication est purement TCP.

---

## 🔍 Description

- Élection d’un hôte (host) qui envoie les questions  
- Envoi et réception de questions/ réponses sérialisées en JSON  
- Timer par question, collecte des réponses en temps réel  
- Affichage des statistiques (histogramme) et mise à jour des scores/leaderboard  
- Pas de configuration manuelle d’adresses IP : tout se fait en LAN  
- Architecture modulaire : séparation réseau / logique métier / UI

---

## 📦 Technologies & Prérequis

1. **Langage & Framework**  
   - C++17 ou supérieur  
   - Qt 6 (Widgets et QtNetwork)  
   - CMake ≥ 3.24

2. **Bibliothèques**  
   - QtCore, QtWidgets, QtNetwork, QtCharts (optionnel pour l’histogramme)  
   - (Prévu) GoogleTest / Qt Test pour tests unitaires

3. **Outils**  
   - Git  
   - CMake  
   - Un compilateur compatible C++17 (gcc ≥ 9, Clang ≥ 10, MSVC 2019+)  
   - Qt6 installé sur votre machine (via l’installateur officiel ou votre distribution)

---

## 🚀 Installation & Compilation (état initial)

### 1. Cloner le dépôt
```bash
git clone https://github.com/cammonp799/QuizzTeam.git
cd QuizzTeam
