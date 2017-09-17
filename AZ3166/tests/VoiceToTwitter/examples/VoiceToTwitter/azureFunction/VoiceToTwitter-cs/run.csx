using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Net.Http;
using System.Net;
using System.Threading;
using System.IO;
using Newtonsoft.Json;
using Microsoft.Azure.Devices;

private const int TokenExpiryInSeconds = 600;

static ServiceClient serviceClient;
static string connectionString = GetEnvironmentVariable("speechRecognitionIothub");


    public class Authentication
    {
        public static readonly string FetchTokenUri = "https://api.cognitive.microsoft.com/sts/v1.0";
        private string subscriptionKey;
        private string token;
        private Timer accessTokenRenewer;

        //Access token expires every 10 minutes. Renew it every 9 minutes.
        private const int RefreshTokenDuration = 9;

        public Authentication(string subscriptionKey)
        {
            this.subscriptionKey = subscriptionKey;
            this.token = FetchToken(FetchTokenUri, subscriptionKey).Result;

            // renew the token on set duration.
            accessTokenRenewer = new Timer(new TimerCallback(OnTokenExpiredCallback),
                                           this,
                                           TimeSpan.FromMinutes(RefreshTokenDuration),
                                           TimeSpan.FromMilliseconds(-1));
        }

        public string GetAccessToken()
        {
            return this.token;
        }

        private void RenewAccessToken()
        {
            this.token = FetchToken(FetchTokenUri, this.subscriptionKey).Result;
            Console.WriteLine("Renewed token.");
        }

        private void OnTokenExpiredCallback(object stateInfo)
        {
            try
            {
                RenewAccessToken();
            }
            catch (Exception ex)
            {
                Console.WriteLine(string.Format("Failed renewing access token. Details: {0}", ex.Message));
            }
            finally
            {
                try
                {
                    accessTokenRenewer.Change(TimeSpan.FromMinutes(RefreshTokenDuration), TimeSpan.FromMilliseconds(-1));
                }
                catch (Exception ex)
                {
                    Console.WriteLine(string.Format("Failed to reschedule the timer to renew access token. Details: {0}", ex.Message));
                }
            }
        }

        private async Task<string> FetchToken(string fetchUri, string subscriptionKey)
        {
            using (var client = new HttpClient())
            {
                client.DefaultRequestHeaders.Add("Ocp-Apim-Subscription-Key", subscriptionKey);
                UriBuilder uriBuilder = new UriBuilder(fetchUri);
                uriBuilder.Path += "/issueToken";

                var result = await client.PostAsync(uriBuilder.Uri.AbsoluteUri, null);
                Console.WriteLine("Token Uri: {0}", uriBuilder.Uri.AbsoluteUri);
                return await result.Content.ReadAsStringAsync();
            }
        }
    }





public static void Run(Stream myBlob, string name, TraceWriter log)
{
    serviceClient = ServiceClient.CreateFromConnectionString(connectionString);
	GetTextFromAudioAsync(StreamToBytes(myBlob), name.Substring(0, name.IndexOf('/')), log);
}

public static void GetTextFromAudioAsync(byte[] bytes, string deviceName, TraceWriter log)
{
	string requestUri = @"https://speech.platform.bing.com/speech/recognition/interactive/cognitiveservices/v1?language=en-US";
    string serviceKey = GetEnvironmentVariable("speechServiceKey");

	HttpWebRequest request =(HttpWebRequest)HttpWebRequest.Create(requestUri);
	request.SendChunked = true;
    request.Accept = @"application/json;text/xml";
    request.Method = "POST";
    request.ProtocolVersion = HttpVersion.Version11;
    request.Host = @"speech.platform.bing.com";
    request.ContentType = @"audio/wav; codec=""audio/pcm""; samplerate=16000";
    string token = new Authentication(serviceKey).GetAccessToken();
    request.Headers["Authorization"] = "Bearer " + token;

	using (var memoryStream = new MemoryStream(bytes))
	{
        byte[] buffer = null;
        int bytesRead = 0;
        using (Stream requestStream = request.GetRequestStream())
        {
            /*
            * Read 1024 raw bytes from the input audio file.
            */
            buffer = new Byte[checked((uint)Math.Min(1024, (int)bytes.Length))];
            while ((bytesRead = memoryStream.Read(buffer, 0, buffer.Length)) != 0)
            {
                requestStream.Write(buffer, 0, bytesRead);
            }

            // Flush
            requestStream.Flush();
        }            
    }

    string responseString = string.Empty;
    using (WebResponse response = request.GetResponse())
    {
        if(((HttpWebResponse)response).StatusCode == HttpStatusCode.OK)
        {
            using (StreamReader sr = new StreamReader(response.GetResponseStream()))
            {
                responseString = sr.ReadToEnd();
            }
            
            try
            {
                dynamic data = JsonConvert.DeserializeObject(responseString);
                try {
                    string text = data.DisplayText;
                    log.Info(text + "\r\n");
                    tweet(text, log);
                    SendCloudToDeviceMessageAsync(deviceName, "{\"text\":\"" + text + "\"}");
                }
                catch 
                {
                    log.Info("get text from audio failed");
                    SendCloudToDeviceMessageAsync(deviceName, "{\"error\":\"get text from audio failed\"}");
                }
            }
            catch (Exception ex)
            {
                log.Info("response sting is not json :" + responseString);
                SendCloudToDeviceMessageAsync(deviceName, "{\"error\":\"get text from audio request failed\"}");
            }
        }
	}
}


public async static void tweet(string text, TraceWriter log)
{
    var requestUri = GetEnvironmentVariable("logicAppTrigger");
    using (var client = new HttpClient()) {
        var jsonString = JsonConvert.SerializeObject(text + "[" + string.Format("{0:HH:mm:ss tt}", DateTime.Now) + "]");
        var content = new StringContent(jsonString, Encoding.UTF8, "application/json");
        var response = await client.PostAsync(requestUri, content);
        if (!response.IsSuccessStatusCode) {
            log.Info("trigger tweet failed");
        }
    }
}

private async static void SendCloudToDeviceMessageAsync(string deviceName, string message)
{
    var commandMessage = new Message(Encoding.ASCII.GetBytes(message));
    await serviceClient.SendAsync(deviceName, commandMessage);
}

public static string GetEnvironmentVariable(string name)
{
    return System.Environment.GetEnvironmentVariable(name, EnvironmentVariableTarget.Process);
}

private static byte[] StreamToBytes(Stream input)
{
	using (MemoryStream ms = new MemoryStream())
	{
		input.CopyTo(ms);
		byte[] bytes = ms.ToArray();
		return bytes;
	}
}