#include <iostream>
#include <unordered_map>
#include <bitset>

using namespace std;


// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// DEFINES ///////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

#define ASCII_END 256
#define ONE_BYTE 8
#define TWO_BYTE 16
#define CODE_BITS 24
#define FOUR_BYTE 32
// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////// FUNCTION HEADERS //////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

uint32_t encode(string input, uint32_t reset_freq);
u_int32_t header(u_int32_t reset_req);
u_int32_t toBits(u_int32_t code);
void initDictionary(unordered_map<string, u_int32_t>& dictionary);

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// MAIN //////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {

    cout << argc << endl;

    encode("TOBEORNOTTOBE", 40);
    return 0;
}

// Encode

uint32_t encode(string input, uint32_t reset_freq) {

    header(reset_freq);

    cout << input << endl;

    unordered_map<string, uint32_t> dictionary = {};
    initDictionary(dictionary);

    uint32_t code_count = ASCII_END;
    string prev = "";
    for (char c : input) {
        if (dictionary.find(prev + c) != dictionary.end()) {
            prev = prev + c;
        } else {
            dictionary[prev + c] = code_count;
            code_count++;

            // Output the binary
            u_int32_t output = dictionary.find(prev)->second;
            toBits(output);
            
            prev = string(1, c);
        }

    }

    return 1;
}

u_int32_t header(u_int32_t reset_req) {
    bitset<FOUR_BYTE> binary(reset_req);
    cout << binary << endl;

    return 0;
}

// Bits
u_int32_t toBits(u_int32_t code) {


    if (code < pow(2, 7)) {
        bitset<8> binary(code);
        cout << binary << " ";
    } else if (code < pow(2, 14)) {

    } else {

    }

    return 0;
}

// Initialise dictionary
void initDictionary(unordered_map<string, u_int32_t>& dictionary) {
    for (int i = 0; i < ASCII_END; i++) {
        string character(1, static_cast<char>(i));
        dictionary[character] = i;
    }
}
