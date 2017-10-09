$downloadURLs = "https://aka.ms/devkit/prod/firmware/latest", 
    "https://aka.ms/devkit/prod/installpackage/latest", 
    "https://aka.ms/devkit/prod/installpackage/mac/latest"

foreach ($url in $downloadURLs)
{
    Write-Host("Starting to check: " + $url)

    $statusCode = wget $url | % {$_.StatusCode}

    If ($statusCode -eq 200) 
	{
        Write-Host("It is OK!")
    }
    Else 
	{
        Write-Host("It may be down, please check!")
    }
}