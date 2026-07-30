# TIC-AKS-FaaS

> Evaluación del rendimiento entre Azure Kubernetes Service (AKS) y Azure Functions utilizando un microservicio del benchmark DeathStarBench.

![GitHub](https://img.shields.io/badge/Status-Active-success)
![Azure](https://img.shields.io/badge/Azure-AKS%20%7C%20Functions-0078D4)

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
│       ├── mediaFunctions/
│       └── ...
│
├── data/
│   ├── BDs de MediaMicroservices/
│   ├── Resultados de Costos/
│   └── Script de las pruebas/
│
├── figures/
│   
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

# 🔬 Caso de estudio

El proyecto utiliza el microservicio **Media** perteneciente al benchmark **DeathStarBench**, adaptándolo para su ejecución tanto en:

- Azure Kubernetes Service
- Azure Functions

Esto permite realizar una comparación objetiva entre un modelo basado en contenedores (CaaS) y uno Serverless (FaaS).

