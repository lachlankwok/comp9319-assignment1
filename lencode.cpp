#include <iostream>
#include <unordered_map>
#include <fstream>
#include <iostream>

using namespace std;

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// DEFINES ///////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

#define ASCII_END 256
#define EIGHT_ONE_BITS 0b11111111

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////// FUNCTION HEADERS //////////////////////////////
// /////////////////////////////////////////////////////////////////////////////

uint32_t encode(string input_path, string output_path, uint32_t reset_freq);
void header(u_int32_t reset_freq, ofstream &out_file);
void toBits(u_int32_t code, ofstream &out_file);
void initDictionary(unordered_map<string, u_int32_t>& dictionary);

// /////////////////////////////////////////////////////////////////////////////
// ///////////////////////////////// MAIN //////////////////////////////////////
// /////////////////////////////////////////////////////////////////////////////
int main(int argc, char **argv) {
    // Error checking - requires 3 arguments
    if (argc != 4) {
        cerr << "./lencode [input_path] [output_path] [N]" << endl;
        return 1;
    }

    string input_path = argv[1];
    string output_path = argv[2];
    uint32_t reset_freq = static_cast<uint32_t>(stoul(argv[3]));

    // Encode given file into output file
    encode(input_path, output_path, reset_freq);

    return 0;
}

/**
 * Encode given input file into output file with given N reset frequency
 *
 * input_path:  string      - input file path
 * output_path: string      - output file path
 * reset_freq:  uint32_t    - every reset_freq bytes, the dictionary resets
 */
uint32_t encode(string input_path, string output_path, uint32_t reset_freq) {

    // Initialise the dictionary
    unordered_map<string, uint32_t> dictionary = {};
    initDictionary(dictionary);

    // Initialise variables for dictionary (how many codes, prev, read_bytes)
    uint32_t code_count = ASCII_END;
    string prev = "";
    uint32_t read_bytes = 0;

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

    // Establish the header bytes for the output file
    header(reset_freq, out_file);

    // Read all bytes from the input file and encode them
    char c;
    while (in_file.get(c)) {
        // If "pc" is in the dictionary, p = pc
        if (dictionary.find(prev + c) != dictionary.end()) {
            prev = prev + c;

        // Add "pc" to dictionary and output code(p); p = c
        } else {
            // Add "pc" to dictionary only if not full (4,194,304 entries)
            if (code_count < (1u << 22)) {
                dictionary[prev + c] = code_count;
                code_count++;
            }

            // Output the binary bytes into output file
            if (!prev.empty()) {
                toBits(dictionary[prev], out_file);
            }

            // p = c
            prev = string(1, c);
        }

        // Reset the dictionary if N has been reached
        // Increment read_bytes if reset is enabled
        if (reset_freq != 0) {
            read_bytes++;

            // Check if reset is needed
            if (read_bytes >= reset_freq) {
                // Flush the current prev
                if (!prev.empty()) {
                    toBits(dictionary[prev], out_file);
                }

                // Reset dictionary
                initDictionary(dictionary);
                code_count = ASCII_END;
                prev = "";
                read_bytes = 0;
            }
        }
    }

    // Need to output remaining prev
    if (!prev.empty()) {
        toBits(dictionary[prev], out_file);
    }

    // Close the output and input files
    out_file.close();
    in_file.close();

    return 1;
}

/**
 * Given the output file and the reset frequency, write the frequency in bytes
 *
 * reset_freq: u_int32_t    - 4 bytes worth (a number)
 * outfile: ofstream &      - the output file
 */
void header(u_int32_t reset_freq, ofstream &out_file) {
    // Get the 4 bytes to write into output file in big endian
    uint8_t b1 = ((reset_freq) >> 24) & EIGHT_ONE_BITS;
    uint8_t b2 = ((reset_freq) >> 16) & EIGHT_ONE_BITS;
    uint8_t b3 = ((reset_freq) >> 8) & EIGHT_ONE_BITS;
    uint8_t b4 = reset_freq & EIGHT_ONE_BITS;

    out_file.put(b1);
    out_file.put(b2);
    out_file.put(b3);
    out_file.put(b4);
}

/**
 * Given a code (max 4 bytes), output the binary into the output file in
 * big endian
 *
 * code         u_int32_t   - code of <= 22 bits (but in 32 bits space)
 * out_file     ofstream &  - output file reference
 */
void toBits(u_int32_t code, ofstream &out_file) {
    if (code < (1u << 8)) {
        uint8_t byte = code & 0b111'1111;
        out_file.put(static_cast<char>(byte));
    } else if (code < (1u << 14)) {
        // Append 10 - it is 2 bytes long
        uint16_t bits = code & 0b11'1111'1111'1111;
        bits |= (1u << 15);

        // Write high byte then low byte (big-endian)
        uint8_t high = (bits >> 8) & EIGHT_ONE_BITS;
        uint8_t low  = bits & EIGHT_ONE_BITS;

        out_file.put(static_cast<char>(high));
        out_file.put(static_cast<char>(low));
    } else {
        // Append '11' - it is 3 bytes long - 22 bits of code
        uint32_t bits = code & 0b111111'11111111'11111111;
        bits |= (3u << 22);

        // Write 3 bytes big-endian
        uint8_t b1 = (bits >> 16) & EIGHT_ONE_BITS;
        uint8_t b2 = (bits >> 8)  & EIGHT_ONE_BITS;
        uint8_t b3 = bits & EIGHT_ONE_BITS;

        out_file.put(static_cast<char>(b1));
        out_file.put(static_cast<char>(b2));
        out_file.put(static_cast<char>(b3));
    }
}

/**
 * Initialise dictionary with the 256 ASCII code
 *
 * dictionary   unordered_map<string, u_int32_t>&   - dictionary being used
 */
void initDictionary(unordered_map<string, u_int32_t>& dictionary) {
    dictionary.clear();
    for (int i = 0; i < ASCII_END; i++) {
        string character(1, static_cast<char>(i));
        dictionary[character] = i;
    }
}
