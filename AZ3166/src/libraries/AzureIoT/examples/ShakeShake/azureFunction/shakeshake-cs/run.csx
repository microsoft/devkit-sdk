#r "Newtonsoft.Json"
#r "System.Web"

using System;
using System.Text;
using System.Configuration;
using System.Net;
using System.IO;
using System.Web;

using Microsoft.Azure.Devices;
using Newtonsoft.Json;
using Newtonsoft.Json.Linq;

public class DeviceObject
{
    public string topic;
}

public class ShakeShakeException : Exception
{
    public ShakeShakeException(string message)
        : base($"ShakeShakeError:{message}")
    {
    }
}

public static void Run(string myEventHubMessage, TraceWriter log)
{
    log.Info($"C# Event Hub trigger function processed a message: {myEventHubMessage}");
    
    // Get the hash tag
    string hashTag = string.Empty;
    try
    {
        DeviceObject deviceObject = Newtonsoft.Json.JsonConvert.DeserializeObject<DeviceObject>(myEventHubMessage);
        if (String.IsNullOrEmpty(deviceObject.topic))
        {
            // No hash tag or this is a heartbeat package
            return;
        }
        hashTag = deviceObject.topic;
    }
    catch (Exception ex)
    {
        throw new ShakeShakeException($"Failed to deserialize message:{myEventHubMessage}. Error detail: {ex.Message}");
    }

    // Retrieve the tweet according to the given hash tag
    string message = string.Empty;
    try
    {
        string tweet = string.Empty;
        string url = "https://api.twitter.com/1.1/search/tweets.json" + "?count=3&q=%23" + HttpUtility.UrlEncode(hashTag);
        string authHeader = "Bearer " + "AAAAAAAAAAAAAAAAAAAAAGVU0AAAAAAAucpxA9aXc2TO6rNMnTcVit1P3YM%3DrQpyFeQ6LOwyvy7cqW5djhLPnFfjEK8H3hA1qfGDh93JRbI1le";

        HttpWebRequest request = (HttpWebRequest)WebRequest.Create(url);
        request.Headers.Add("Authorization", authHeader);
        request.Method = "GET";
        request.ContentType = "application/x-www-form-urlencoded;charset=UTF-8";

        string objText = string.Empty;
        using(HttpWebResponse response = (HttpWebResponse)request.GetResponse())
        {
            using(StreamReader reader = new StreamReader(response.GetResponseStream()))
            {
                objText = reader.ReadToEnd();
            }
        }
        
        try
        {
            JObject o = JObject.Parse(objText);
            string name = o["statuses"][0]["user"]["name"].ToString();
            string txt = o["statuses"][0]["text"].ToString();

            message = $"@{name}:\n {txt}";
        }
        catch
        {
            message = "No new tweet.";
        }
    }
    catch(Exception ex)
    {
        throw new ShakeShakeException($"Failed to call twitter API: {ex.Message}");
    }
    log.Info($"Twitter: {message}");
    
    // Send back to device
    string deviceId = "AZ3166";
    try
    {
        string connectionString = ConfigurationManager.AppSettings["iotHubConnectionString"];
        using(ServiceClient serviceClient = ServiceClient.CreateFromConnectionString(connectionString))
        {
            Message commandMessage = new Message(Encoding.UTF8.GetBytes(message));
            serviceClient.SendAsync(deviceId, commandMessage).Wait();
        }
    }
    catch(Exception ex)
    {
        throw new ShakeShakeException($"Failed to send C2D message: {ex.Message}");
    }
}