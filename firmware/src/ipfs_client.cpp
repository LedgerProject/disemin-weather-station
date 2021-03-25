#include "ipfs_client.h"
#include <ArduinoJson.h>

const char IPFSClient::API_PATH[] = "/api/v0";

/******************************************************************************
* Constructor, sets wifi client. Default constructor is private.
* @param client Client to use for requests
******************************************************************************/
IPFSClient::IPFSClient(WiFiClient client) : _client(client)
{}

/******************************************************************************
* Set node connection data
******************************************************************************/
void IPFSClient::set_node_address(String addr, uint16_t port)
{
    _node_addr = addr;
    _node_port = port;
}

/******************************************************************************
* Start the HTTP req by sumbitting all required data up until the file part
******************************************************************************/
IPFSClientResult IPFSClient::add_json(String json, String filename, IPFSClientFile *file_out)
{
    IPFSClientResult ret = IPFS_CLIENT_OK;

    // String json = "{\"a\": 3, \"b\": 4}";

    // Attempt to connect if not connected already
    if(!_client.connected())
    {
        if(!_client.connect(_node_addr.c_str(), _node_port))
        {
            return IPFS_CLIENT_CANNOT_CONNECT;
        }
    }

    //
    // Main request headers
    //
    String boundary = "EXM-IPFSClient";// + String(millis());
    String boundary_start = "--" + boundary + "\r\n";
    String boundary_end = "\r\n--" + boundary + "--\r\n\r\n";

    String headers_main = "POST " + String(API_PATH) + "/add HTTP/1.1\r\n"
                          "Host: " + String(_node_addr) + ":" + _node_port + "\r\n"
                          "User-Agent: EXM-IPFSClient/1.0\r\n"
                          "Content-Type: multipart/form-data; boundary=" + boundary + "\r\n";
    //
    // Data part headers
    //
    String headers_file =
        "Content-Disposition: form-data; name=\"file\"; filename=\"" + filename + "\"\r\n"
	    "Content-Type: application/json\r\n\r\n";

    //
    // Boundaries
    //                    
    int content_length = headers_file.length() + boundary_start.length() + boundary_end.length() + json.length();
    headers_main += "Content-Length: " + String(content_length) + "\n\n";

    // Serial.println(F("Request: "));
    // Serial.print(headers_main);
    // Serial.print(boundary_start);
    // Serial.print(headers_file);
    // Serial.print(json);
    // Serial.print(boundary_end);

    Serial.println(F("Submitting request."));

    _client.write(headers_main.c_str());
    _client.write(boundary_start.c_str());
    _client.write(headers_file.c_str());
    _client.write(json.c_str());
    _client.write(boundary_end.c_str());
    _client.write("\r\n\r\n");

    Serial.println(F("Request submitted"));

    _client.setTimeout(5);


    //
    // Read headers
    //
    String line = "";

    bool headers = true;
    StaticJsonDocument<255> json_doc;

    bool failed = true;
    while(line = _client.readStringUntil('\n'))
    {
        if(headers)
        {
            // Done with headers
            if(line.length() <= 1)
            {
                headers = false;
            }
        }
        else
        {
            if(deserializeJson(json_doc, line) == DeserializationError::Ok)
            {
                JsonObject obj = json_doc.as<JsonObject>();

                if(obj["Name"].isNull() || obj["Hash"].isNull() || obj["Size"].isNull())
                {
                    Serial.print(F("Invalid JSON object received."));
                }
                else
                {
                    failed = false;

                    // Serial.println(F("Parsed:"));
                    // serializeJsonPretty(json_doc, Serial);

                    if(file_out != NULL)
                    {
                        strncpy(file_out->name, obj["Name"].as<char*>(), sizeof(file_out->name));
                        strncpy(file_out->hash, obj["Hash"].as<char*>(), sizeof(file_out->hash));
                        file_out->size = obj["Size"].as<uint32_t>();
                    }
                    break;
                }
            }            
            else
            {
                // Not JSON
            }
        }

        // Serial.println(line);
    }

    _client.stop();

    if(failed)
    {
        return IPFS_CLIENT_INVALID_RESPONSE;
    }
    
    return IPFS_CLIENT_OK;
}