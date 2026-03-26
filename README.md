# FICA Motor

Aplicativo Android para rastreamento de pedaladas com fusão de sensores (EKF), suporte a 20 Hz de GPS e integração com Google Maps.

## Repositório

- **GitHub:** https://github.com/nikecodedev/C-para-Fica.app

## Requisitos

- Android Studio Hedgehog (2023.1.1) ou superior
- JDK 17
- Android NDK (instalado via SDK Manager)
- CMake 3.22.1
- Gradle 8.x

## Configuração

### 1. Chave da API do Google Maps

Para o mapa ser exibido, configure a chave do Google Maps:

1. Crie ou use o arquivo `local.properties` na raiz do projeto.
2. Adicione a linha:
   ```
   MAPS_API_KEY=sua_chave_google_maps_aqui
   ```
3. No Google Cloud Console, habilite **Maps SDK for Android** no projeto da API.

Sem a chave, o app mostra um placeholder e continua funcionando (velocidade, ritmo, distância).

### 2. Compilação

#### Debug (para testes)

```bash
# Na raiz do projeto (fica-motor)
./gradlew :app:assembleDebug
```

O APK será gerado em:
```
app/build/outputs/apk/debug/app-debug.apk
```

#### Release

```bash
./gradlew :app:assembleRelease
```

APK em:
```
app/build/outputs/apk/release/app-release.apk
```

### 3. Instalação via ADB

```bash
adb install -r app/build/outputs/apk/debug/app-debug.apk
```

## Versão testada (auditoria)

- **Código:** Commit atual do repositório
- **Configurações:**
  - GPS: 20 Hz (50 ms)
  - Fusão EKF: 20 Hz
  - Controles: Start, Pause, New Ride
  - Correção de drift em repouso: velocidade mínima 0,5 km/h para acumular distância

## Funcionalidades

- **20 Hz de entrada GPS** (conforme Bíblia)
- **Controles Start/Pause** para iniciar e pausar o rastreamento
- **New Ride** para zerar a sessão
- **Velocidade, ritmo e distância** em tempo real
- **Trajetória no mapa** (quando `MAPS_API_KEY` está configurada)
- **Proteção contra drift em repouso:** a distância não é acumulada quando o dispositivo está parado

## Estrutura do projeto

- `app/` – Módulo Android (UI, lógica de negócio)
- `engine/` – Motor C++ (EKF, fusão de sensores)
- `platform/android/` – Código Kotlin/Android (SensorBridge, EngineBridge, etc.)

## Licença

Proprietário. Projeto FICA.
