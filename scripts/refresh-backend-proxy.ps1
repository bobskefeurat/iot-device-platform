param(
    [int]$ListenPort = 8000,
    [int]$ConnectPort = 8000,
    [string]$ListenAddress = "0.0.0.0"
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

function Get-WslIPv4Address {
    $addresses = (& wsl.exe hostname -I).Trim() -split "\s+"
    return $addresses |
        Where-Object { $_ -match '^\d{1,3}(\.\d{1,3}){3}$' -and $_ -ne '127.0.0.1' } |
        Select-Object -First 1
}

function Get-WindowsLanIPv4Address {
    $defaultRoute = Get-NetRoute -DestinationPrefix "0.0.0.0/0" -AddressFamily IPv4 |
        Sort-Object RouteMetric, ifMetric |
        Select-Object -First 1

    if ($null -eq $defaultRoute) {
        return $null
    }

    return Get-NetIPAddress -InterfaceIndex $defaultRoute.InterfaceIndex -AddressFamily IPv4 |
        Where-Object { $_.IPAddress -ne '127.0.0.1' -and $_.IPAddress -notlike '169.254*' } |
        Select-Object -ExpandProperty IPAddress -First 1
}

$wslAddress = Get-WslIPv4Address
if ([string]::IsNullOrWhiteSpace($wslAddress)) {
    throw "Could not determine the current WSL IPv4 address."
}

$lanAddress = Get-WindowsLanIPv4Address

Write-Host
Write-Host "========================================"
Write-Host "ESP32 backend proxy refresh"
Write-Host "========================================"
Write-Host "WSL backend IP : ${wslAddress}"
if ($lanAddress) {
    Write-Host "Windows LAN IP : ${lanAddress}"
}
Write-Host "Listen address : ${ListenAddress}:$ListenPort"
Write-Host "Connect target : ${wslAddress}:$ConnectPort"
Write-Host

& netsh interface portproxy delete v4tov4 listenaddress=$ListenAddress listenport=$ListenPort | Out-Null

if ($lanAddress -and $lanAddress -ne $ListenAddress) {
    & netsh interface portproxy delete v4tov4 listenaddress=$lanAddress listenport=$ListenPort | Out-Null
}

& netsh interface portproxy add v4tov4 `
    listenaddress=$ListenAddress `
    listenport=$ListenPort `
    connectaddress=$wslAddress `
    connectport=$ConnectPort | Out-Null

Write-Host "Current portproxy rules:"
& netsh interface portproxy show all
Write-Host

Write-Host "Port listeners on ${ListenPort}:"
netstat -ano | Select-String ":$ListenPort"
Write-Host

Write-Host "WSL health check:"
try {
    $wslHealth = Invoke-RestMethod -TimeoutSec 5 -Uri "http://${wslAddress}:$ConnectPort/health"
    $wslHealth | ConvertTo-Json -Compress
} catch {
    Write-Warning "Could not reach WSL backend on ${wslAddress}:$ConnectPort"
}

if ($lanAddress) {
    Write-Host
    Write-Host "LAN health check:"
    try {
        $lanHealth = Invoke-RestMethod -TimeoutSec 5 -Uri "http://${lanAddress}:$ListenPort/health"
        $lanHealth | ConvertTo-Json -Compress
    } catch {
        Write-Warning "Could not reach backend on ${lanAddress}:$ListenPort from Windows."
    }

    Write-Host
    Write-Host "Use this host IP in BACKEND_URL if needed:"
    Write-Host "  ${lanAddress}"
}
