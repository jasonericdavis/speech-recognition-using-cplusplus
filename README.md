# Speech Recognition Using C++

Speech recognition in C++ using [Rev.ai's](https://www.rev.ai) speech to text API and Microsofts [C++Rest SDK](https://github.com/microsoft/cpprestsdk).

### Setup

1. Get an access token from [Rev.ai's](https://www.rev.ai).

2. Install the C++RestSDK using the [Getting Started](https://github.com/microsoft/cpprestsdk) guide.

3. Update the source files with your access token

4. If using VSCode select the file you would like to execute and launch the _g++ - Build and debug active file_ debug configuration.

    or run the following commmands:

        g++ -std=c++11 -g <cpp_filename> -lboost_system -lcrypto -lssl  -lcpprest -pthread -o output/<output_filename>

        output/<output_filename>


### Credits
- [Harvard list 01.wav](https://figshare.com/articles/media/Speech_corpus_-_example_of_raw_audio_HARVARD_list_01_wav/7857770/1)
