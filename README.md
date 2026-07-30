# TIC-AKS-FaaS

> Evaluación del rendimiento entre Azure Kubernetes Service (AKS) y Azure Functions utilizando un microservicio del benchmark DeathStarBench.

![GitHub](https://img.shields.io/badge/Status-Active-success)
![Azure](https://img.shields.io/badge/Azure-AKS%20%7C%20Functions-0078D4)
![License](https://img.shields.io/badge/License-MIT-green)

---

## 📖 Descripción

**TIC-AKS-FaaS** es un proyecto desarrollado para comparar el desempeño de dos modelos de computación en la nube:

- Azure Kubernetes Service (AKS)
- Azure Functions (FaaS)

La evaluación se realiza mediante la implementación del microservicio **Media** del benchmark **DeathStarBench**, ejecutando pruebas de carga controladas para analizar métricas como latencia, throughput, utilización de recursos y costo operativo.

Este repositorio también incluye los conjuntos de datos utilizados durante la experimentación y las figuras empleadas en la documentación del proyecto.

---

# 📂 Estructura del repositorio

```text
TIC-AKS-FaaS/
│
├── code/
│   └── mediaMicroservices/
│       ├── AKS/
│       ├── AzureFunctions/
│       └── ...
│
├── data/
│   ├── resultados/
│   ├── mediciones/
│   └── scripts/
│
├── figures/
│   ├── arquitectura/
│   ├── resultados/
│   └── diagramas/
│
└── README.md
```

---

# 🚀 Objetivos

- Implementar el microservicio Media en AKS.
- Implementar el mismo microservicio mediante Azure Functions.
- Ejecutar pruebas de carga bajo diferentes niveles de concurrencia.
- Comparar el rendimiento entre ambos modelos de despliegue.
- Analizar el consumo de recursos y el costo operativo.

---

# 🛠 Tecnologías utilizadas

## Backend

- .NET
- C#
- ASP.NET Core

## Cloud

- Microsoft Azure
- Azure Kubernetes Service (AKS)
- Azure Functions
- Azure Monitor

## Contenedores

- Docker
- Kubernetes

## Base de datos

- MongoDB Atlas

## Herramientas de pruebas

- Locust
- Prometheus
- Grafana

---

# 📊 Contenido del repositorio

## `code/`

Contiene la implementación del microservicio Media para los diferentes entornos evaluados durante la investigación.

## `data/`

Incluye:

- Resultados experimentales
- Datos utilizados durante las pruebas
- Scripts auxiliares
- Mediciones de rendimiento

## `figures/`

Contiene todas las imágenes utilizadas en la tesis y documentación, incluyendo:

- Arquitectura del sistema
- Diagramas
- Resultados experimentales
- Gráficas comparativas

---

# 🏗 Arquitectura general

```text
                 Cliente
                    │
                    ▼
            Servicio Frontend
                    │
        ┌───────────┴───────────┐
        │                       │
        ▼                       ▼
 Azure Functions            AKS Cluster
        │                       │
        └───────────┬───────────┘
                    │
                    ▼
             MongoDB Atlas
```

---

# 📈 Evaluación realizada

Durante el estudio se analizaron los siguientes indicadores:

- Latencia promedio
- Latencia P95
- Latencia P99
- Throughput
- Uso de CPU
- Uso de memoria
- Tiempo de respuesta
- Costo operativo

Las pruebas fueron ejecutadas utilizando diferentes niveles de carga para comparar el comportamiento de ambas arquitecturas.

---

# 📁 Organización de datos

Los resultados experimentales se encuentran organizados para facilitar su análisis y reproducibilidad.

Entre ellos se incluyen:

- Archivos CSV
- Resultados de Locust
- Métricas de Azure
- Datos utilizados para generar las gráficas del estudio

---

# 🔬 Caso de estudio

El proyecto utiliza el microservicio **Media** perteneciente al benchmark **DeathStarBench**, adaptándolo para su ejecución tanto en:

- Azure Kubernetes Service
- Azure Functions

Esto permite realizar una comparación objetiva entre un modelo basado en contenedores (CaaS) y uno Serverless (FaaS).

---

# 👥 Autores

Proyecto desarrollado como parte del trabajo de investigación sobre evaluación de arquitecturas Cloud Native utilizando servicios de Microsoft Azure.

---

# 📄 Licencia

Este proyecto se distribuye bajo la licencia **MIT**.
