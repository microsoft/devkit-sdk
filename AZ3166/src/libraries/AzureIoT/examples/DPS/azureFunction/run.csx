using System;
using System.Net;
using Microsoft.Azure.Devices;
using System.Text;
using System.Threading;
using Newtonsoft.Json;

static string tokenSuffix = "dps-ai-demo";
static RegistryManager registryManager;
static string connectionString = Environment.GetEnvironmentVariable("iotHubConnectionString");
static string aiUrl = Environment.GetEnvironmentVariable("luisUrl");

public class DeviceObject
{
    public string deviceId;
}

public static void Run(string myEventHubMessage, TraceWriter log)
{
    log.Info($"C# Event Hub trigger function processed a message: {myEventHubMessage}");

    registryManager = RegistryManager.CreateFromConnectionString(connectionString);

    string deviceAccessKey = string.Empty;
    string deviceToken = string.Empty;

    DeviceObject deviceObject = Newtonsoft.Json.JsonConvert.DeserializeObject<DeviceObject>(myEventHubMessage);
    if (String.IsNullOrEmpty(deviceObject.deviceId))
    {
        return;
    }
    deviceAccessKey = deviceObject.deviceId;
    
    var plainTextBytes = System.Text.Encoding.UTF8.GetBytes(deviceAccessKey + tokenSuffix);
    deviceToken = System.Convert.ToBase64String(plainTextBytes);

    string endpoint = string.Format("Endpoint={0};clientName={1};accessToken={2}", aiUrl, deviceAccessKey, deviceToken);
    SetDeviceTwin(deviceObject.deviceId, endpoint).Wait();
}

public static async Task SetDeviceTwin(string deviceId, string endpoint)
{
    var twin = await registryManager.GetTwinAsync(deviceId);
    var patch = new
    {
        properties = new
        {
            desired = new
            {
                LUISEndpoint = endpoint
            }
        }
    };

    await registryManager.UpdateTwinAsync(twin.DeviceId, JsonConvert.SerializeObject(patch), twin.ETag);
}
