#include <iostream>
#include <unordered_map>
#include <bitset>
#include <fstream>
#include <iostream>

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

uint32_t encode(string input_path, string output_path, uint32_t reset_freq);
void header(u_int32_t reset_req, ofstream &out_file);
void toBits(u_int32_t code, ofstream &out_file);
void initDictionary(unordered_map<string, u_int32_t>& dictionary);

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// MAIN //////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {

    cout << argc << endl;

    // Use Case: ./lencode [input_path] [output_path] [N]
    string input_path = argv[1];
    string output_path = argv[2];
    uint32_t reset_freq = static_cast<uint32_t>(stoul(argv[3]));

    encode(input_path, output_path, reset_freq);
    return 0;
}

// Encode

uint32_t encode(string input_path, string output_path, uint32_t reset_freq) {

    cout << input_path << endl;

    unordered_map<string, uint32_t> dictionary = {};
    initDictionary(dictionary);

    uint32_t code_count = ASCII_END;
    string prev = "";
    uint32_t read_bytes = 0;

    ifstream in_file(input_path, ios::binary);
    if (!in_file) {
        cerr << "Failed to open file!" << endl;
        return 1;
    }

    ofstream out_file(output_path, ios::binary);
    if (!out_file) {
        cerr << "Failed to open file!" << endl;
        return 1;
    }

    header(reset_freq, out_file);

    char c;
    while (in_file.get(c)) {
        if (dictionary.find(prev + c) != dictionary.end()) {
            prev = prev + c;
        } else {
            dictionary[prev + c] = code_count;
            code_count++;

            // Output the binary
            if (!prev.empty()) {
                u_int32_t output = dictionary[prev];
                toBits(output, out_file);
            }

            prev = string(1, c);
        }

        // Reset the dictionary
        read_bytes++;
        if (read_bytes == reset_freq && reset_freq != 0) {
            // Need to output remaining prev
            if (!prev.empty()) {
                toBits(dictionary[prev], out_file);
            }

            initDictionary(dictionary);
            code_count = ASCII_END;
            prev = "";
            read_bytes = 0;
        }
    }

    // Need to output remaining prev
    if (!prev.empty()) {
        toBits(dictionary[prev], out_file);
    }

    out_file.close();

    return 1;
}

void header(u_int32_t reset_req, ofstream &out_file) {
    
    uint8_t b1 = ((reset_req) >> 24) & 0xFF;
    uint8_t b2 = ((reset_req) >> 16) & 0xFF;
    uint8_t b3 = ((reset_req) >> 8) & 0xFF;
    uint8_t b4 = reset_req & 0xFF;

    out_file.put(b1);
    out_file.put(b2);
    out_file.put(b3);
    out_file.put(b4);
}

// Bits
void toBits(u_int32_t code, ofstream &out_file) {

    if (code < (1u << 8)) {
        uint8_t byte = code & 0x7F;
        byte &= 0x7F;
        out_file.put(static_cast<char>(byte));

    } else if (code < (1u << 14)) {
        // Append 10 - it is 2 bytes long
        uint16_t bits = code & 0x3FFF;
        bits |= (1u << 15);

        // Write high byte then low byte (big-endian)
        uint8_t high = (bits >> 8) & 0xFF;
        uint8_t low  = bits & 0xFF;

        out_file.put(static_cast<char>(high));
        out_file.put(static_cast<char>(low));
    } else {
        // Append 11 - it is 3 bytes long
        uint32_t bits = code & 0x3FFFFF; // 22 bits
        bits |= (3u << 22);              // leading '11' in top bits
        // Write 3 bytes big-endian
        uint8_t b1 = (bits >> 16) & 0xFF;
        uint8_t b2 = (bits >> 8)  & 0xFF;
        uint8_t b3 = bits & 0xFF;

        out_file.put(static_cast<char>(b1));
        out_file.put(static_cast<char>(b2));
        out_file.put(static_cast<char>(b3));
    }
}

// Initialise dictionary
void initDictionary(unordered_map<string, u_int32_t>& dictionary) {
    dictionary.clear();
    for (int i = 0; i < ASCII_END; i++) {
        string character(1, static_cast<char>(i));
        dictionary[character] = i;
    }
}
