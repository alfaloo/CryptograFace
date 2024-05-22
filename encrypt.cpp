#include "cryptlib.h"
#include "rijndael.h"
#include "modes.h"
#include "files.h"
#include "osrng.h"
#include "hex.h"
#include "eax.h"

#include <iostream>
#include <fstream>
#include <string>

int main() {

    // Create key and initialisation vector
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);

    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());

    std::ofstream credentialsFile("credentials.txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "Failed to open file for credentials." << std::endl;
        return 1;
    }

    // Encode key
    CryptoPP::HexEncoder keyEncoder(new CryptoPP::FileSink(credentialsFile));
    keyEncoder.Put(key, key.size());
    keyEncoder.MessageEnd();
    credentialsFile << "\n";

    // Encode IV
    CryptoPP::HexEncoder ivEncoder(new CryptoPP::FileSink(credentialsFile));
    ivEncoder.Put(iv, iv.size());
    ivEncoder.MessageEnd();
    credentialsFile << "\n";

    credentialsFile.close();

    std::string plain = "This is a testing message";
    std::string cipher;

    try {
        CryptoPP::EAX<CryptoPP::AES>::Encryption e;
        e.SetKeyWithIV(key, key.size(), iv);

        CryptoPP::StringSource(plain, true,
                               new CryptoPP::AuthenticatedEncryptionFilter(e,
                                                                           new CryptoPP::StringSink(cipher)
                               ) // AuthenticatedEncryptionFilter
        ); // StringSource
    } catch (const CryptoPP::Exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }

    std::ofstream cipherFile("cipher.txt");
    if (!cipherFile.is_open()) {
        std::cerr << "Failed to open file for cipher." << std::endl;
        return 1;
    }

    // Write cipher to file
    CryptoPP::HexEncoder cipherEncoder(new CryptoPP::FileSink(cipherFile));
    cipherEncoder.Put((const CryptoPP::byte*)&cipher[0], cipher.size());
    cipherEncoder.MessageEnd();

    cipherFile.close();

    std::cout << "Encryption complete\n";

    return 0;
}
