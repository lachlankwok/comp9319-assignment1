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

    encode("TOBEORNOTTOBEORTOBEORNOT", 10);
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
    uint32_t read_bytes = 0;

    for (char c : input) {
        if (dictionary.find(prev + c) != dictionary.end()) {
            prev = prev + c;
        } else {
            dictionary[prev + c] = code_count;
            code_count++;

            // Output the binary
            if (!prev.empty()) {
                u_int32_t output = dictionary[prev];
                toBits(output);
            }

            prev = string(1, c);
        }

        // Reset the dictionary
        read_bytes++;
        if (read_bytes == reset_freq && reset_freq != 0) {
            // Need to output remaining prev
            if (!prev.empty()) {
                toBits(dictionary[prev]);
            }

            initDictionary(dictionary);
            code_count = ASCII_END;
            prev = "";
            read_bytes = 0;
        }
    }

    // Need to output remaining prev
    if (!prev.empty()) {
        toBits(dictionary[prev]);
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

    if (code < (1u << 8)) {
        bitset<8> binary(code);
        cout << binary << " ";
    } else if (code < (1u << 14)) {
        // Append 10 - it is 2 bytes long
        uint16_t bits = code & 0x3FFF;
        bits |= (1u << 15);

        // Write high byte then low byte (big-endian)
        uint8_t high = (bits >> 8) & 0xFF;
        uint8_t low  = bits & 0xFF;
        cout << bitset<8>(high) << " " << bitset<8>(low) << " ";
    } else {
        // Append 11 - it is 3 bytes long
        uint32_t bits = code & 0x3FFFFF; // 22 bits
        bits |= (3u << 22);              // leading '11' in top bits
        // Write 3 bytes big-endian
        uint8_t b1 = (bits >> 16) & 0xFF;
        uint8_t b2 = (bits >> 8)  & 0xFF;
        uint8_t b3 = bits & 0xFF;
        cout << bitset<8>(b1) << " " << bitset<8>(b2) << " " << bitset<8>(b3) << " ";
    }

    return 0;
}

// Initialise dictionary
void initDictionary(unordered_map<string, u_int32_t>& dictionary) {
    dictionary.clear();
    for (int i = 0; i < ASCII_END; i++) {
        string character(1, static_cast<char>(i));
        dictionary[character] = i;
    }
}
