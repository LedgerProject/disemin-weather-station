#ifndef IPFS_CLIENT_H
#define IPFS_CLIENT_H

#include <WiFiClient.h>

/**
 * Return codes
 */
enum IPFSClientResult
{
    IPFS_CLIENT_OK,
    IPFS_CLIENT_CANNOT_CONNECT,
    IPFS_CLIENT_INVALID_RESPONSE
};

struct IPFSClientFile
{
    char name[255];
    char hash[50];
    uint32_t size;
};

class IPFSClient
{
public:
    IPFSClient(WiFiClient client);

    void set_node_address(String addr, uint16_t port);

    IPFSClientResult add_json(String json, String filename, IPFSClientFile* file_out = NULL);
private:
    // Default constructor private
    IPFSClient(){};

    String _node_addr = "";
    uint16_t _node_port = 0;

    WiFiClient _client;

    static const char API_PATH[];
};



#endif