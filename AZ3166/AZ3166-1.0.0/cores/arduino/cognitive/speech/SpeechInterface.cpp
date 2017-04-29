#include "SpeechInterface.h"
#include "http_client.h"
#include <json.h>

#define SPEECH_RECOGNITION_API_REQUEST_URL  ""                                                                  \
                                            "https://speech.platform.bing.com/recognize?"                       \
                                            "scenarios=smd&appid=D4D52672-91D7-4C74-8AD8-42B1D98141A5"          \
                                            "&locale=en-us&device.os=bot"                                       \
                                            "&form=BCSSTT&version=3.0&format=json&instanceid=%s&requestid=%s"

#define GUID_GENERATOR_HTTP_REQUEST_URL  "http://www.fileformat.info/tool/guid.htm?count=1&format=text&hyphen=true"
#define TOKEN_REQUEST_URL "https://api.cognitive.microsoft.com/sts/v1.0/issueToken"

SpeechInterface::SpeechInterface(const char * subscriptionKey, const char * deviceId, bool debug)
{
    _requestUri = (char *)malloc(260);
    _cognitiveSubKey = (char *)malloc(33);
    _deviceId = (char *)malloc(37);

    memcpy(_cognitiveSubKey, subscriptionKey, 33);
    memcpy(_deviceId, deviceId, 37);
    
    _debug = debug;

    if (_debug) printf("subscriptionKey: %s, deviceId: %s \r\n", subscriptionKey, deviceId);
}

SpeechInterface::~SpeechInterface(void)
{  
    delete _cognitiveSubKey;
    delete _deviceId;
    delete _requestUri;
}

char* SpeechInterface::generateGuidStr()
{
    HTTPClient guidRequest = HTTPClient(HTTP_GET, GUID_GENERATOR_HTTP_REQUEST_URL);
    const Http_Response* _response = guidRequest.send();
    if (_response == NULL)
    {
        if (_debug) printf("Guid generator HTTP request failed.\r\n");
        return NULL;
    }

    char* guidStr = (char *)malloc(37);
    strcpy(guidStr, _response->body);
    if (_debug) printf("Got new guid: <%s> message <%s>\r\n", guidStr, _response -> status_message);
    return guidStr;
}

char* SpeechInterface::getJwtToken()
{
    HTTPClient tokenRequest = HTTPClient(HTTP_POST, TOKEN_REQUEST_URL);
    tokenRequest.set_header("Ocp-Apim-Subscription-Key", _cognitiveSubKey);
    const Http_Response* _response = tokenRequest.send();
    if (!_response)
    {
        if (_debug) printf("HttpRequest failed (error code %d)\n", tokenRequest.get_error());
        return NULL;
    }

    char* token = (char *)malloc(strlen(_response->body) + 8);
    sprintf(token, "Bearer %s", _response->body);
    if (_debug) printf("Got JwtToken: <%s> message <%s>\r\n", token, _response -> status_message);
    return token;
}

SpeechResponse* SpeechInterface::recognizeSpeech(char * audioFileBinary, int length)
{
    if (_debug) printf("file length : %d\r\n", length);

    // Generate a new guid for cognitive service API request
    char* guid = generateGuidStr();

    // Generate a JWT token for cognitove service authentication
    char* jwtToken = getJwtToken();
    
    // Preapre Speech Recognition API request URL
    sprintf(_requestUri, SPEECH_RECOGNITION_API_REQUEST_URL, _deviceId, guid);
    if (_debug) printf("recognizeSpeech request URL: %s\r\n", _requestUri);

    HTTPClient speechRequest = HTTPClient(HTTP_POST, (const char*)_requestUri);
    speechRequest.set_header("Content-Type", "plain/text");
    speechRequest.set_header("Authorization", jwtToken);
    
    const Http_Response* _response = speechRequest.send(audioFileBinary, length);
    if (!_response)
    {
        if (_debug) printf("Speech API request failed (error code %d).\r\n", speechRequest.get_error());
        return NULL;
    }
    char* bodyStr = (char*)malloc(strlen(_response->body) + 1);
    strcpy(bodyStr, _response->body);
    if (_debug) printf("congnitive result: %s\r\n", bodyStr);
    
    SpeechResponse *speechResponse = new SpeechResponse();
    if (speechResponse == NULL) {
        if (_debug) printf("SpeechResponse is null \r\n");
        return NULL;
    }
    // Parse Jso n result to SpeechResponse object
    struct json_object *responseObj, *subObj, *valueObj, *bestResult;

    responseObj = json_tokener_parse(bodyStr);
    if (_debug) printf("JSON object from response:\n%s\n", json_object_to_json_string_ext(responseObj, JSON_C_TO_STRING_SPACED | JSON_C_TO_STRING_PRETTY));

     // parse status value from header->status
    json_object_object_get_ex(responseObj, "header", &subObj);
    json_object_object_get_ex(subObj, "status", &valueObj);
    speechResponse->status =  (char *)json_object_get_string(valueObj);

    if (strcmp(speechResponse->status, "error") == 0)
    {
        if (_debug) printf("Audio recognize failed.");
        
    }
    else
    {
        // parse status value from header->lexical
        json_object_object_get_ex(subObj, "lexical", &valueObj);
        speechResponse->text =  (char *)json_object_get_string(valueObj);

        // parse confidence value from results[0]->confidence
        json_object_object_get_ex(responseObj, "results", &subObj);
        array_list * array = json_object_get_array(valueObj);
        bestResult = json_object_array_get_idx(subObj, 0);
        json_object_object_get_ex(bestResult, "confidence", &valueObj);

        speechResponse->confidence = (double)json_object_get_double(valueObj);
    }

    free(guid);
    free(jwtToken);
    free(bodyStr);
    return speechResponse;
}

int SpeechInterface::convertTextToSpeech(char * text, int length, char * audioFileBinary, int audioLen)
{
    return 0;
}