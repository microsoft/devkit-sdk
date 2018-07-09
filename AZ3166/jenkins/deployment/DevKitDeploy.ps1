param(
    [string]
    $Environment = "staging",
    $ArduinoConfigFileName = "package_azureboard_index.json"
)

$ErrorActionPreference = "Stop"
$CurrentDir = (Get-Location).Path

Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\ResourceManager\AzureResourceManager\AzureRM.Profile\AzureRM.Profile.psd1"
Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\Storage\Azure.Storage\Azure.Storage.psd1"
Import-Module "C:\Program Files (x86)\Microsoft SDKs\Azure\PowerShell\ServiceManagement\Azure\Azure.psd1"

$StorageHashTable = @{"azureboard2" = $env:Azureboard2StorageKey;
					"azureboard" = $env:AzureboardStorageKey}; 

foreach($StorageAccountName in $StorageHashTable.Keys) 
{
	Write-Host("[$Environment][$StorageAccountName]: Start $Environment Deployment.");

	################################################################################
	#              Step 1: upload AZ3166 package to Azure blob storage             #
	################################################################################

	Write-Host("Step 1: upload AZ3166 package to Azure blob storage");
	$ArduinoPackageContainer = "arduinopackage"
	$PackageInfoContainer = "packageinfo"

	# We can move this credential to Azure Key Vault once we have deploy with production subscription
	$Key = $StorageHashTable[$StorageAccountName]; 
	$StorageContext = New-AzureStorageContext -StorageAccountName $StorageAccountName -StorageAccountKey $Key

	# Get current package version
	$CurrentVersion =  Get-Content '.\system_version.txt' | Out-String
	$CurrentVersion = $CurrentVersion.ToString().Trim()

	# Upload Arduino package
	$ArduinoPackageFilePath = Join-Path -Path (Get-Location).Path -ChildPath "\TestResult\AZ3166-$CurrentVersion.zip"
	$ArduinoPackageBlobName = "AZ3166-" + $CurrentVersion + ".zip"
	Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $ArduinoPackageFilePath -Blob "$ArduinoPackageContainer\$ArduinoPackageBlobName" -Force

	# Upload Firmware bin file
	$FirmwareFileName = "devkit-firmware-" + $CurrentVersion + "." + $env:BUILD_NUMBER + ".bin"
	$FirmwareFilePath = Join-Path -Path (Get-Location).Path -ChildPath "TestResult\$FirmwareFileName"
	Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $FirmwareFilePath -Blob $FirmwareFileName -Force

	# Upload Firmware bin file for OTA
	$OTAFirmwareFileName = "devkit-firmware-latest.ota.bin"
	$OTAFirmwareFilePath = Join-Path -Path (Get-Location).Path -ChildPath "TestResult\$OTAFirmwareFileName"
	Set-AzureStorageBlobContent -Context $StorageContext -Container $Environment -File $OTAFirmwareFilePath -Blob $OTAFirmwareFileName -Force

	################################################################################
	# Step 2: Calculate package MD5 checksum and Update to  configuration JSON file#
	################################################################################

	Write-Host("Step 2: Calculate package MD5 checksum and Update to  configuration JSON file")

	$MD5 = New-Object -TypeName System.Security.Cryptography.MD5CryptoServiceProvider
	$ArduinoPackageHash = [System.BitConverter]::ToString($MD5.ComputeHash([System.IO.File]::ReadAllBytes($ArduinoPackageFilePath)))
	$ArduinoPackageHash = $ArduinoPackageHash.ToLower() -replace '-', ''
	Write-Host($ArduinoPackageHash);

	Get-AzureStorageBlobContent -Context $StorageContext -Container $Environment -Blob "$PackageInfoContainer\$ArduinoConfigFileName" -Destination $ArduinoConfigFileName -Force
	$ArduinoConfigJson = Get-Content $ArduinoConfigFileName | Out-String | ConvertFrom-Json

	$totalVersions = $ArduinoConfigJson.packages[0].platforms.Count
	$LastPlatform = ([PSCustomObject]($ArduinoConfigJson.packages[0].platforms[$totalVersions - 1]))

	if ($LastPlatform.version -eq $CurrentVersion)
	{
		# Update the latest version
		$LastPlatform.url = "https://azureboard2.azureedge.net/$Environment/$ArduinoPackageContainer/$ArduinoPackageBlobName"
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

	$ArduinoConfigJsonBlobURL = "https://azureboard2.azureedge.net/$Environment/$ArduinoConfigJsonBlobName"
	Write-Host("Arduino board manager JSON file URI: $ArduinoConfigJsonBlobURL")

	Write-Host("[$Environment][$StorageAccountName]: Deployment completed.");
}