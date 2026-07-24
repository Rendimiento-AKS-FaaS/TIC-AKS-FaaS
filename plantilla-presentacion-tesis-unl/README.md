# 📊 Plantilla de Presentación de Tesis — Universidad Nacional de Loja

Plantilla en **Quarto Revealjs** con la identidad corporativa de la UNL para presentaciones de tesis de la **Carrera de Computación** de la **Facultad de la Energía, Las Industrias y los Recursos Naturales No Renovables**.

---

## 🚀 Requisitos

1. **Quarto** (v1.3 o superior): [Descargar aquí](https://quarto.org/docs/get-started/)
2. Un editor de texto (VS Code recomendado con la [extensión de Quarto](https://marketplace.visualstudio.com/items?itemName=quarto.quarto))
3. Conexión a internet la primera vez (las fuentes se cargan desde Google Fonts)

> **Nota:** No necesitas instalar LaTeX ni ninguna otra dependencia.

---

## 📁 Estructura del Proyecto

```
plantillaPresentaciones/
├── _quarto.yml              ← Configuración del proyecto (no tocar)
├── presentacion-tesis.qmd   ← 📝 ARCHIVO PRINCIPAL (edita este)
├── assets/
│   ├── logoUNL.png          ← Logo UNL blanco (portada)
│   └── logoUnlOscuro.png    ← Logo UNL a color (cabecera de slides)
├── css/
│   └── unl-theme.css        ← Tema visual
└── README.md                ← Este archivo
```

---

## ✏️ Cómo Usar

### 1. Personalizar la Presentación

Abre `presentacion-tesis.qmd` y reemplaza todos los textos entre **«»** con tu información:

- **«Título de tu Trabajo de Tesis aquí»** → Tu título real
- **«Nombre del Estudiante»** → Tu nombre completo
- **«Nombre del Director de Tesis»** → Nombre de tu director
- **«Año»** → Año de presentación
- Cada sección tiene comentarios `<!-- EDITAR: ... -->` que te guían

Para cambiar el texto del pie de página ("Carrera de Computación — UNL"), edita la regla `.reveal .slide-number::before` en `css/unl-theme.css` (está marcada con `EDITAR`).

### 2. Vista Previa (Live Preview)

```bash
quarto preview presentacion-tesis.qmd
```

Abre la presentación en tu navegador con recarga automática. Si un cambio de estilos no se refleja, recarga con `Ctrl+Shift+R`.

### 3. Renderizar

```bash
quarto render presentacion-tesis.qmd
```

### 4. Exportar a PDF

1. Abre la presentación en el navegador
2. Agrega `?print-pdf` al final de la URL
3. `Ctrl+P` → Guardar como PDF (activa "Gráficos de fondo")

---

## 🧩 Anatomía de un Slide

Cada slide de contenido lleva automáticamente: la **línea roja superior** con el **logo montado** que la corta, la **banda roja del título** (el `## Título` del slide) y el **pie de página** con numeración.

Los slides sin cabecera (portada, secciones, cierre) llevan `data-state="portada"` en su encabezado — eso oculta línea, logo y pie. **Si creas un slide de este tipo, no olvides ese atributo.**

### Slide de contenido normal

```markdown
## Título del Slide

- Punto 1
- Punto 2
```

### Slide de sección (fondo rojo, separador de bloques)

```markdown
## Nombre de la Sección {.seccion data-state="portada"}

<span class="numero-seccion">01</span>
```

### Imágenes

```markdown
![Descripción](ruta/a/imagen.png){width="70%"}
```

### Cajas de resaltado

```markdown
::: {.caja-importante}
**Texto importante** que quieres resaltar
:::

::: {.caja-info}
**Información** complementaria
:::
```

### Columnas lado a lado

```markdown
:::: {.columns}
::: {.column width="50%"}
Contenido izquierdo
:::
::: {.column width="50%"}
Contenido derecho
:::
::::
```

---

## 🎨 Colores Corporativos UNL

| Color | Código | Uso |
|-------|--------|-----|
| 🔴 Rojo UNL | `#E30613` | Color principal, banda de títulos, línea superior |
| ⚪ Blanco | `#FFFFFF` | Fondos, texto sobre rojo |
| ⚫ Gris Oscuro | `#333333` | Texto principal |
| 🔵 Azul Oscuro | `#1B2A4A` | Subtítulos (h3), acentos |

Todos están definidos como variables CSS (`--unl-*`) al inicio de `css/unl-theme.css`.

---

## 📋 Slides Incluidos

1. **Portada** — Logo, facultad y carrera, título, autor, director
2. **Índice** — Tabla de contenidos
3. **Tema** — Enunciado del tema de investigación
4. **Introducción** — Contexto breve
5. **Planteamiento del Problema** y **Objetivos**
6. **Metodología** — Enfoque, herramientas, población y muestra
7. **Resultados** — Un slide por objetivo específico
8. **Discusión** — Análisis de resultados
9. **Conclusiones** y **Recomendaciones**
10. **Cierre** — "Gracias por su atención." sobre fondo rojo

---

## ❓ Solución de Problemas

| Problema | Solución |
|----------|----------|
| Quarto no encontrado | Verifica la instalación: `quarto --version` |
| Las fuentes no cargan | Necesitas conexión a internet (Google Fonts) |
| El logo no aparece | Verifica que existan los archivos en `assets/` |
| La línea/logo aparece en la portada | Falta `data-state="portada"` en ese slide |
| Cambios de CSS no se ven | Recarga sin caché: `Ctrl+Shift+R` |

---

## 📝 Licencia

Plantilla creada para uso académico de la Universidad Nacional de Loja.
Libre para uso y modificación por estudiantes de la UNL.
