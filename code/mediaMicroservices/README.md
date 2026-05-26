# Media Microservices — Solo Flujo de Lectura

Esta bifurcación conserva únicamente el flujo de lectura (`ReadPage`) utilizado para comparar despliegues en **AKS** y **Azure Functions**.

Elementos eliminados del árbol original de DeathStarBench:

- Todas las rutas HTTP de escritura e inicialización
- Memcached y Redis
- Contenedores MongoDB embebidos (se utiliza **MongoDB Atlas** en su lugar)

---

## Flujo de lectura

```
GET /wrk2-api/page/read?movie_id=<id>&review_start=0&review_stop=10
  → nginx-web-server
  → page-service
      ├─ movie-info-service   → MongoDB Atlas (movie-info)
      ├─ movie-review-service → MongoDB Atlas (movie-review) → review-storage-service
      ├─ cast-info-service    → MongoDB Atlas (cast-info)
      └─ plot-service         → MongoDB Atlas (plot)
```

---

## Servicios

| Binario                  | Rol                     |
|--------------------------|-------------------------|
| `PageService`            | Orquesta `ReadPage`     |
| `MovieInfoService`       | `ReadMovieInfo`         |
| `MovieReviewService`     | `ReadMovieReviews`      |
| `ReviewStorageService`   | `ReadReviews`           |
| `CastInfoService`        | `ReadCastInfo`          |
| `PlotService`            | `ReadPlot`              |

Cada servicio tiene su propio directorio en `src/` y se compila como un binario independiente instalado en `/usr/local/bin`.

---

## Configuración de MongoDB Atlas

Todos los servicios leen la cadena de conexión desde una sola fuente, en este orden de prioridad:

1. Variable de entorno `MONGODB_URI`
2. Campo `mongodb_uri` en `config/service-config.json`

Copia el archivo de ejemplo y edítalo con tu cadena real:

```bash
cp config/service-config.example.json config/service-config.json
```

Las bases de datos en Atlas deben llamarse exactamente: `movie-info`, `plot`, `cast-info`, `movie-review`, `review` (nombres originales del benchmark).

---

## Ejecución con Docker Compose

> **Nota:** El `nginx.conf` incluido usa el resolver DNS de Kubernetes (`kube-dns.kube-system.svc.cluster.local`). En Docker Compose esto no tendrá efecto ya que Kubernetes no está presente; nginx usará su resolver por defecto. Para pruebas locales esto es suficiente, pero ten en cuenta esta diferencia.

```bash
export MONGODB_URI="mongodb+srv://USER:PASSWORD@cluster.mongodb.net/?retryWrites=true&w=majority"
docker compose up -d --build
```

Endpoint disponible:

```
http://localhost:8080/wrk2-api/page/read?movie_id=1&review_start=0&review_stop=10
```

---

## Carga con wrk2 (opcional)

Primero compila `wrk2` (requiere `make` y dependencias de compilación en Linux/Mac):

```bash
cd wrk2 && make
cd ..
```

Luego ejecuta la prueba de carga:

```bash
wrk2/wrk -D exp -t 4 -c 32 -d 30s -L \
  -s wrk2/scripts/media-microservices/read-page.lua \
  http://localhost:8080/wrk2-api/page/read -R 100
```

El script `wrk2/scripts/media-microservices/read-page.lua` genera peticiones al endpoint de lectura con `movie_id=1`.

---

## Notas sobre AKS / Azure Functions

- **AKS**: desplegar los seis binarios (o una imagen única con distintos entrypoints) junto con nginx; inyectar `MONGODB_URI` mediante un secreto de Kubernetes. El `nginx.conf` ya está configurado para usar el resolver DNS de Kubernetes.
- **Azure Functions**: una función por llamada downstream o una única función orquestadora que replique el comportamiento de `page-service`; todas las funciones se conectan a Atlas mediante `MONGODB_URI`.

---

## Regenerar bindings Thrift

`media_service.thrift` define las RPCs de solo lectura. Para regenerar los archivos generados en `gen-cpp/` y `gen-lua/`:

```bash
bash scripts/regenerate-thrift.sh
```

El script detecta automáticamente si `thrift` está instalado localmente o si debe usarlo vía Docker. La imagen Docker hace este mismo paso durante el build.

## Licencia

- El proyecto está bajo **Apache License 2.0** (ver archivo `LICENSE`).
- La licencia original GPL v2.0 se conserva en `LICENSE-GPL.txt` para referencia.
- El archivo `NOTICE` contiene la atribución a terceros y notas de licencia.
