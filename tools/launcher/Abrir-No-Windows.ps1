$ErrorActionPreference = 'Stop'
Add-Type -AssemblyName System.Windows.Forms
$title = 'Megaman X4 Ds'
$null = [System.Windows.Forms.Application]::EnableVisualStyles()
$wsl = Get-Command wsl.exe -ErrorAction SilentlyContinue
if (-not $wsl) {
    [System.Windows.Forms.MessageBox]::Show('Instale o WSL com uma distribuicao Linux x64 para executar este launcher.', $title) | Out-Null
    exit 1
}

$display = New-Object System.Windows.Forms.Form
$display.TopMost = $true
$display.StartPosition = 'CenterScreen'
try {
    $dump = New-Object System.Windows.Forms.OpenFileDialog
    $dump.Title = 'Selecione seu BIN original USA (SLUS-00561)'
    $dump.Filter = 'Dump de PlayStation (*.bin)|*.bin|Todos os arquivos (*.*)|*.*'
    if ($dump.ShowDialog($display) -ne 'OK') { exit 0 }

    $output = New-Object System.Windows.Forms.FolderBrowserDialog
    $output.Description = 'Escolha onde salvar mmx4_ds_beta.nds'
    $output.SelectedPath = Split-Path -Parent $dump.FileName
    if ($output.ShowDialog($display) -ne 'OK') { exit 0 }
    $target = Join-Path $output.SelectedPath 'mmx4_ds_beta.nds'
    $replace = Test-Path -LiteralPath $target
    if ($replace -and [System.Windows.Forms.MessageBox]::Show($display, "Substituir o arquivo existente?`n$target", $title, 'YesNo') -ne 'Yes') {
        exit 0
    }

    $arguments = @()
    if ($env:X4_WSL_DISTRO) { $arguments += @('--distribution', $env:X4_WSL_DISTRO) }
    $arguments += @('--cd', $PSScriptRoot, '--exec', 'python3', 'launcher.py',
                    '--windows-paths', '--dump', $dump.FileName, '--output-dir', $output.SelectedPath)
    if ($replace) { $arguments += '--replace' }
    Write-Host 'Preparando o jogo. Acompanhe as etapas nesta janela.'
    & $wsl.Source @arguments
    if ($LASTEXITCODE -ne 0) {
        [System.Windows.Forms.MessageBox]::Show($display, 'A compilacao falhou. Confira os detalhes na janela de terminal. O WSL precisa ter Python 3.11 ou mais recente.', $title) | Out-Null
        exit 1
    }
    [System.Windows.Forms.MessageBox]::Show($display, "Arquivo salvo em `n$target", $title) | Out-Null
} finally {
    if ($dump) { $dump.Dispose() }
    if ($output) { $output.Dispose() }
    $display.Dispose()
}
