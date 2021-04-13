#include <string>
#include <exception>
#include <unistd.h>

#include <cpprest/http_client.h>
#include <cpprest/filestream.h>
#include <cpprest/json.h>

#include <fstream>
#include <streambuf>

using namespace std;
using namespace utility;                    // Common utilities like string conversions
using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features


string SubmitJob( string host,string accessToken,string media_url) 
{
    try{
        // Create the JSON body to be sent in the request
        json::value body;
        body["media_url"] = json::value::string(U(media_url));

        http_request req(methods::POST);
        req.set_request_uri(U("/speechtotext/v1/jobs"));
        req.headers().add(U("Authorization"), U("Bearer " + accessToken));
        req.headers().add(U("Content-Type"), "application/json");
        req.set_body(body);
        
        http_client client(U("https://api.rev.ai"));
        http_response response = client.request(req).get();

        /* 
            If we recieve a 200 response then we know the job was 
            successfully created and we can extract the Id 
        */
        if(response.status_code() == status_codes::OK){
            json::value data = response.extract_json().get();
            string id =  data["id"].as_string();
            cout << "Job ID: " << id << "\n";
            return id;
        } else {
            cout << "Error Creating the Job: " << response.status_code() << "\n";
        }
        return NULL;
        
    }
    catch (exception& e)
    {
        cout << e.what() << '\n';
        return NULL;
    }
}

json::value GetJSONTranscript(string jobID, string token) {
    http_request req(methods::GET);
    req.set_request_uri(U("/speechtotext/v1/jobs/" + jobID + "/transcript"));
    req.headers().add(U("Authorization"), U("Bearer " + token));
    req.headers().add(U("Accept"), U("application/vnd.rev.transcript.v1.0+json"));

    /*
        Although the response will be JSON, the SDK does't know how to handle the 
        vendor specific response content-type. This handler simply changes the 
        content-type to application/json so that it can be extracted and read.
    */
    auto response_content_type_handler =
        [](http_request request, std::shared_ptr<http_pipeline_stage> next_stage) -> pplx::task<http_response>
        {
            return next_stage->propagate(request).then([](http_response resp) -> http_response
            {
                resp.headers().set_content_type("application/json");
                return resp;
            });
        };
    
    http_client client(U("https://api.rev.ai"));
    client.add_handler(response_content_type_handler);

    http_response response = client.request(req).get();
    if(response.status_code() == status_codes::OK){
        json::value data = response.extract_json().get();
        return data;
    } else {
        cout << "Error getting the Transcript: " << response.status_code() << "\n";
        return NULL;
    }
}

void PollForJobCompletion(string jobID, string host, string token) {
    try {
        http_client client(U("https://api.rev.ai"));
        http_request req(methods::GET);
        req.set_request_uri(U("speechtotext/v1/jobs/" + jobID));
        req.headers().add(U("Authorization"), U("Bearer " + token));
        
        while(true) {
            http_response response = client.request(req).get();
            json::value status = response.extract_json().get()["status"];
            if(status.as_string() == "transcribed") {
                std::cout << "Video Transcribed \n";
                return;
            }
            std::cout << "Job Status: " << status << "\n";
            std::cout << "Check again in 5 seconds \n";
            sleep(5);
        }

    }
    catch (exception& e)
    {
        cout << e.what() << '\n';
    }
}

void ShowTranscript(json::value transcript) {
    try {
        cout << "--- Begin Transcript ---\n";
        
        json::array words = transcript["monologues"].as_array()[0]
            .as_object()["elements"].as_array();

        for(int index = 0; index < words.size(); index++) {
            json::object word = words[index].as_object();

            if(word["type"].as_string() == "text") {
                cout << word["value"].as_string() << "\n";
                double startTime = word["ts"].as_number().to_double();
                double endTime = word["end_ts"].as_number().to_double();
                double elapsedTime = (endTime - startTime);
                
                // use usleep for Unix and Sleep for windows
                // usleep uses microseconds 
                usleep((int) (elapsedTime * 1000000));
            }
        }
        cout << "--- Transcript Complete ---\n";
    }
    catch (exception& e)
    {
        cout << e.what() << '\n';
    }
}

int main(int argc, char* argv[]) {
    string const media_url = "https://audio-samples.github.io/samples/mp3/blizzard_unconditional/sample-1.mp3";
    string const host = "https://api.rev.ai";
    string const accessToken = "";
    
    string jobID = SubmitJob(host, accessToken, media_url);
    PollForJobCompletion(jobID, host, accessToken);
    json::value transcript = GetJSONTranscript(jobID, accessToken);
    ShowTranscript(transcript);
    return 0;
}