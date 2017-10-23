param(
    [string]
    $Environment = "staging",
    $MD5FileName = "devkit_package_md5.json",
    $ArduinoConfigFileName = "package_azureboard_index.json"
)

$ErrorActionPreference = "Stop"
$CurrentDir = (Get-Location).Path

Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\ResourceManager\AzureResourceManager\AzureRM.Profile\AzureRM.Profile.psd1"
Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\Storage\Azure.Storage\Azure.Storage.psd1"
Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\ServiceManagement\Azure\Azure.psd1"

Write-Host("Start $Environment Deployment.");

################################################################################
#              Step 1: upload packages to Azure blob storage                   #
################################################################################

Write-Host("Step 1: upload packages to Azure blob storage");

$SubscriptionName = "VSChina IoT DevINT"
$SubscriptionId = "faab228d-df7a-4086-991e-e81c4659d41a"
$StorageAccountName = "azureboard2"
$ArduinoPackageContainer = "arduinopackage"

$PackageContainerForWin = "windows"
$PackageContainerForMac = "mac"
$PackageInfoContainer = "packageinfo"

# We can move this credential to Azure Key Vault once we have deploy with production subscription
$Key = $env:DevKitStorageKey
$StorageContext = New-AzureStorageContext -StorageAccountName $StorageAccountName -StorageAccountKey $Key

# Get current package version
$CurrentVersion =  Get-Content '.\system_version.txt' | Out-String
$CurrentVersion = $CurrentVersion.ToString().Trim()
$CurrentVersionWithBuildNumber = $CurrentVersion + "." + $env:BUILD_NUMBER

# Upload installation package
$WindowsInstallPackageName = "devkit_install_win_" + $CurrentVersionWithBuildNumber + ".zip"
$MacInstallPackageName = "devkit_install_mac_" + $CurrentVersionWithBuildNumber + ".zip"

$InstallPackageFilePathForWin = Join-Path -Path (Get-Location).Path -ChildPath "TestResult\$WindowsInstallPackageName"
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $InstallPackageFilePathForWin -Blob "$PackageContainerForWin\$WindowsInstallPackageName" -Force

$InstallPackageFilePathForMac = Join-Path -Path (Get-Location).Path -ChildPath "TestResult\$MacInstallPackageName"
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $InstallPackageFilePathForMac -Blob "$PackageContainerForMac\$MacInstallPackageName" -Force

# Upload Arduino package
$ArduinoPackageFilePath = Join-Path -Path (Get-Location).Path -ChildPath "\TestResult\AZ3166-$CurrentVersionWithBuildNumber.zip"
$ArduinoPackageBlobName = "AZ3166-" + $CurrentVersion + ".zip"
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $ArduinoPackageFilePath -Blob "$ArduinoPackageContainer\$ArduinoPackageBlobName" -Force

# Upload Firmware bin file
$FirmwareFileName = "devkit-firmware-" + $CurrentVersionWithBuildNumber + ".bin"
$FirmwareFilePath = Join-Path -Path (Get-Location).Path -ChildPath "TestResult\$FirmwareFileName"
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $FirmwareFilePath -Blob $FirmwareFileName -Force



#################################################################################
# Step 2: Calculate package MD5 checksum and upload this MD5 info to Azure Blob #
#################################################################################

Write-Host("Step 2: Calculate package MD5 checksum and upload this MD5 info to Azure Blob");

$MD5 = New-Object -TypeName System.Security.Cryptography.MD5CryptoServiceProvider
$InstallPackageHash = [System.BitConverter]::ToString($MD5.ComputeHash([System.IO.File]::ReadAllBytes($InstallPackageFilePathForWin)))
$InstallPackageHash = $InstallPackageHash.ToLower() -replace '-', ''
Write-Host($InstallPackageHash);

$ArduinoPackageHash = [System.BitConverter]::ToString($MD5.ComputeHash([System.IO.File]::ReadAllBytes($ArduinoPackageFilePath)))
$ArduinoPackageHash = $ArduinoPackageHash.ToLower() -replace '-', ''
Write-Host($ArduinoPackageHash);

# Update MD5 checksum in json file
Get-AzureStorageBlobContent -Context $StorageContext -Container $Environment -Blob "$PackageInfoContainer\$MD5FileName" -Destination $MD5FileName -Force

$MD5Json = Get-Content $MD5FileName | Out-String | ConvertFrom-Json
$NewItem = $MD5Json.installPackageMD5.Where({$_.version -eq $CurrentVersion})

if (!$NewItem) {
    Write-Host("Add new Version: " + $CurrentVersion)
    $NewItem = '{"version":"", "checksum":""}' | ConvertFrom-Json
    $NewItem.version = $CurrentVersion
    $NewItem.checksum = $InstallPackageHash
    $MD5Json.installPackageMD5 += $NewItem

} else {   
    Write-Host("Update old version: " + $NewItem[0].version)

    $NewItem[0].checksum = $InstallPackageHash
}


$MD5Json | ConvertTo-Json -Depth 10 | Out-File $MD5FileName

# Update the MD5 JSON file in Azure blob storage
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $MD5FileName -Blob "$PackageInfoContainer\$MD5FileName" -Force

# TODO: Check release note is updated!

################################################################################
#             Step 3: Update Arduino configuration JSON file                   #
################################################################################

Write-Host("Step 3: Update Arduino configuration JSON file")

Get-AzureStorageBlobContent -Context $StorageContext -Container $Environment -Blob "$PackageInfoContainer\$ArduinoConfigFileName" -Destination $ArduinoConfigFileName -Force
$ArduinoConfigJson = Get-Content $ArduinoConfigFileName | Out-String | ConvertFrom-Json

$totalVersions = $ArduinoConfigJson.packages[0].platforms.Count
$LastPlatform = ([PSCustomObject]($ArduinoConfigJson.packages[0].platforms[$totalVersions - 1]))


if ($LastPlatform.version -eq $CurrentVersion)
{
    # Update the latest version
    $LastPlatform.url = "https://azureboard2.blob.core.windows.net/$Environment/$ArduinoPackageContainer/$ArduinoPackageBlobName"
    $LastPlatform.archiveFileName = $ArduinoPackageBlobName
    $LastPlatform.checksum = "MD5:" + $ArduinoPackageHash
    $LastPlatform.size = (Get-Item $ArduinoPackageFilePath).Length.ToString()
}
else
{
    # Add new version
    $NewPlatform = New-Object PSCustomObject

    $LastPlatform.psobject.properties | % {
        $newPlatform | Add-Member -MemberType $_.MemberType -Name $_.Name -Value $_.Value   
    }

    $NewPlatform.version = $CurrentVersion
    $NewPlatform.url = "https://azureboard2.azureedge.net/$Environment/$ArduinoPackageContainer/$ArduinoPackageBlobName"
    $NewPlatform.archiveFileName = $ArduinoPackageBlobName
    $NewPlatform.checksum = "MD5:" + $ArduinoPackageHash
    $NewPlatform.size = (Get-Item $ArduinoPackageFilePath).Length.ToString()

    $ArduinoConfigJson.packages[0].platforms += $newPlatform
    $totalVersions += 1
}


Write-Host("Total packages versions: $totalVersions")

# We only maintain the latest 5 versions
if ($totalVersions -gt 5)
{
    # Remove the oldest one
    $ArduinoConfigJson.packages[0].platforms = $ArduinoConfigJson.packages[0].platforms[1..($totalVersions - 1)]
}

$ArduinoConfigJson | ConvertTo-Json -Depth 10 | Out-File $ArduinoConfigFileName -Encoding ascii

# Upload Arduino configuration file to Azure blob storage
$ArduinoConfigJsonBlobName = "$PackageInfoContainer/$ArduinoConfigFileName"
Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $ArduinoConfigFileName -Blob $ArduinoConfigJsonBlobName -Force

$ArduinoConfigJsonBlobURL = "https://azureboard2.blob.core.windows.net/$Environment/$ArduinoConfigJsonBlobName"
Write-Host("Arduino board manager JSON file URI: $ArduinoConfigJsonBlobURL")

Write-Host("$Environment deployment completed.");
