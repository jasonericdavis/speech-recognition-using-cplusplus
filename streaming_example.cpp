#include <cpprest/filestream.h>
#include <cstdio>
#include <cpprest/ws_client.h>
#include <cpprest/streams.h>
#include <cpprest/json.h>

using namespace std;
using namespace web;
using namespace web::websockets::client;

pplx::task<void> SendEOSMessage(websocket_callback_client  client) {
    websocket_outgoing_message closeMsg;
    closeMsg.set_utf8_message("EOS");
    return client.send(closeMsg);
}

void SendAudio(websocket_callback_client client, string mediaFilename) {
    try {
        websocket_outgoing_message msg;
        concurrency::streams::file_stream<uint8_t> buffer;
        auto dd = buffer.open_istream(mediaFilename, std::ios::binary);
    
        msg.set_binary_message(dd.get());
        client.send(msg).then([=] () {
            SendEOSMessage(client);
        });
    }
    catch (exception& e)
    {
        cout << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    string const mediaFilename = "Harvard list 01.wav";
    string const websocketEndpoint = "wss://api.rev.ai/speechtotext/v1/stream";
    string const accessToken = "";
    string const contentType = "&content_type=audio/x-wav";
    string url = websocketEndpoint + "?access_token=" + accessToken + contentType;
    
    try {
        bool keepListening = true;
        websocket_callback_client client = websocket_callback_client();
        client.connect(U(url)).wait();        

        client.set_message_handler([=] (websocket_incoming_message msg)
        {
            string response = msg.extract_string().get();
            json::value data = json::value::parse(response); 

            if(data["type"].as_string() == "connected") {
                std::cout << "WebSocket Connected\n";

                // Now that the socket is connected send the data
                SendAudio(client, mediaFilename);
            }

            if(data["type"].as_string() == "final") {
                // go thru the response and output the values
                json::array elements = data["elements"].as_array();
                for(int index = 0; index < elements.size(); index++) {
                    json::object element = elements[index].as_object();
                    string value = element["value"].as_string();
                    std::cout << value;
                }    
                std::cout << endl;    
            }
        }); 

        client.set_close_handler(
            [&] (
                websocket_close_status close_status, 
                const utility::string_t &reason, 
                const std::error_code &error
            ) {
                std::cout << "Closing Connection " << endl;
                client.close();
                keepListening = false;
            }
        );

        while(keepListening);
        std::cout << "Streaming Complete\n";
    }
    catch (exception& e)
    {
        std::cout << e.what() << '\n';
    }
    return 0;
}