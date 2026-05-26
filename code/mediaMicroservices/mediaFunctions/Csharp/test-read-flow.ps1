# Script para probar el flujo de lectura (ReadPage)

$baseUrl = "http://localhost:7071/api"
$movieId = "movie_1"

Write-Host "--- Iniciando prueba del flujo de lectura ---" -ForegroundColor Cyan

# 1. Sembrar datos si es necesario
Write-Host "Paso 1: Sembrando datos de prueba..." -ForegroundColor Yellow
$seedResult = Invoke-RestMethod -Uri "$baseUrl/SeedData" -Method Get
Write-Host "Resultado del seed: $seedResult"

# 2. Llamar a ReadPage
Write-Host "`nPaso 2: Consultando ReadPage para movie_id=$movieId..." -ForegroundColor Yellow
try {
    $response = Invoke-RestMethod -Uri "$baseUrl/ReadPage?movie_id=$movieId" -Method Get
    
    # Mostrar resultados
    Write-Host "`nRespuesta exitosa!" -ForegroundColor Green
    Write-Host "Película: $($response.movie_info.title)"
    Write-Host "Reseñas encontradas: $($response.reviews.Count)"
    Write-Host "Trama (Plot): $($response.plot)"
    Write-Host "Información de Elenco (Cast): $($response.cast_infos.Count) registros"
    
    Write-Host "`nJSON Completo de la Página:" -ForegroundColor Gray
    $response | ConvertTo-Json -Depth 10 | Write-Host
}
catch {
    Write-Host "Error al llamar a ReadPage: $($_.Exception.Message)" -ForegroundColor Red
}

Write-Host "`n--- Prueba Finalizada ---" -ForegroundColor Cyan
