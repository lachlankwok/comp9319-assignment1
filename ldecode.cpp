#include <iostream>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace std;

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// DEFINES ///////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

#define ASCII_END 256

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////// FUNCTION HEADERS //////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

uint32_t decode(string input_path, string output_path);
uint32_t getHeader(ifstream &in_file);
uint32_t readCode(ifstream &in_file);
void initDictionary(unordered_map<uint32_t, string>& dictionary);

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// MAIN //////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    // Error checking - requires 2 arguments
    if (argc != 3) {
        cerr << "./lencode [input_path] [output_path]" << endl;
        return 1;
    }

    string input_path = argv[1];
    string output_path = argv[2];

    // Decode given input file into output file
    decode(input_path, output_path);

    return 0;
}

uint32_t decode(string input_path, string output_path) {

    // Initialise the dictionary
    unordered_map<uint32_t, string> dictionary = {};
    initDictionary(dictionary);

    // Open the input file to read and error check
    ifstream in_file(input_path, ios::binary);
    if (!in_file) {
        cerr << "Failed to open input file!" << endl;
        return 1;
    }

    // Open the output file to write and error check
    ofstream out_file(output_path, ios::binary);
    if (!out_file) {
        cerr << "Failed to open output file!" << endl;
        return 1;
    }

    // Determine what the reset number is
    uint32_t reset_freq = getHeader(in_file);

    // Initialise variables for dictionary (how many codes, prev, read_bytes)
    uint32_t code_count = ASCII_END;
    string prev = "";
    uint32_t read_bytes = 0;
    uint32_t c;
    string entry;

    // Read all bytes from the input file and encode them
    while (true) {
        if (read_bytes == 0) {
            // First item and output
            c = readCode(in_file);
            entry = dictionary[c];
            out_file.write(entry.data(), entry.size());

            prev = entry;

            // Reset the dictionary if N has been reached
            read_bytes += entry.size();
            if (read_bytes >= reset_freq && reset_freq != 0) {
                initDictionary(dictionary);
                code_count = ASCII_END;
                prev = "";
                read_bytes = 0;
            }
        }

        c = readCode(in_file);
        if (c == UINT32_MAX) break;

        if (dictionary.find(c) != dictionary.end()) {
            entry = dictionary[c];
        } else if (c == code_count) {
            // Special case - KwKwK case
            entry = prev + prev[0];
        }
        out_file.write(entry.data(), entry.size());

        dictionary[code_count] = prev + entry[0];
        code_count++;
        prev = entry;

        // Reset the dictionary if N has been reached
        read_bytes += entry.size();
        if (read_bytes >= reset_freq && reset_freq != 0) {
            initDictionary(dictionary);
            code_count = ASCII_END;
            prev = "";
            read_bytes = 0;
        }
    }

    // Close the output and input files
    out_file.close();
    in_file.close();

    return 1;
}

/**
 * Given the input file, get the reset frequency
 *
 * in_file: ifstream &      - the input file to read from
 */
uint32_t getHeader(ifstream &in_file) {
    unsigned char buffer[4];
    in_file.read(reinterpret_cast<char*>(buffer), 4);

    uint32_t reset_freq = (static_cast<uint32_t>(buffer[0]) << 24) |
                          (static_cast<uint32_t>(buffer[1]) << 16) |
                          (static_cast<uint32_t>(buffer[2]) << 8) |
                          (static_cast<uint32_t>(buffer[3]));

    return reset_freq;
}

/**
 * Decoding the code from the input file. Note that it depends on what the code
 * is:
 * 1 byte: 0xxxxxxx
 * 2 byte: 10xxxxxx yyyyyyyy
 * 3 byte: 11xxxxxx yyyyyyyy zzzzzzzz
 * The code needs to remove the leading one/two bits
 * 
 * in_file: ifstream &      - the input file to read from
 */
uint32_t readCode(ifstream &in_file) {
    char first = in_file.get();
    if (first == EOF) return UINT32_MAX;

    uint8_t b1 = static_cast<uint8_t>(first);

    if ((b1 & 0b10000000) == 0) {
        // 0xxxxxxx
        return b1 & 0b1111111;
    } else if ((b1 & 0b11000000) == 0b10000000) {
        // 10xxxxxx yyyyyyyy
        int second = in_file.get();
        if (second == EOF) return UINT32_MAX;

        uint8_t b2 = static_cast<uint8_t>(second);
        uint16_t code = ((b1 & 0b111111) << 8) | b2;
        return code;
    } else {
        // 11xxxxxx yyyyyyyy zzzzzzzz
        int second = in_file.get();
        int third = in_file.get();
        if (second == EOF || third == EOF) return UINT32_MAX;

        uint8_t b2 = static_cast<uint8_t>(second);
        uint8_t b3 = static_cast<uint8_t>(third);
        uint16_t code = ((b1 & 0b111111) << 16) | (b2 << 8) | b3;
        return code;
    }
}

/**
 * Initialise dictionary with the 256 ASCII code
 *
 * dictionary   unordered_map<uint32_t, string>&   - dictionary being used
 */
void initDictionary(unordered_map<uint32_t, string>& dictionary) {
    dictionary.clear();
    for (uint32_t i = 0; i < ASCII_END; i++) {
        string character(1, static_cast<char>(i));
        dictionary[i] = character;
    }
}

