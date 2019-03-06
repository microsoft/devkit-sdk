Param

(

[String]$PortName = "COM5",

[String]$Workspace = $env:WORKSPACE,

[String]$TestCaseRelativePath = "devkit-sdk\AZ3166\tests\UnitTest\UnitTest.ino"

)



$AppConfig = Join-Path -Path (Get-Location).Path -ChildPath "scripts\DevKitTestTool.exe.config"



$FileExist = Test-Path $AppConfig

if($FileExist -eq $false) 

{

    Write-Host $AppConfig + "does not exist!"

    return

}



$xml =  [xml](Get-Content -Path $AppConfig)

$xml.SelectSingleNode("//appSettings/add[@key='PortName']").Attributes["value"].Value = $PortName

$xml.SelectSingleNode("//appSettings/add[@key='Workspace']").Attributes["value"].Value = $Workspace

$xml.SelectSingleNode("//appSettings/add[@key='TestCaseRelativePath']").Attributes["value"].Value = $TestCaseRelativePath

$xml.Save("$AppConfig");