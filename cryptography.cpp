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

bool encrypt(std::string plain);
std::string decrypt();

int main() {
    std::string plain;
    std::cout << "Please enter your plain text: ";
    std::getline(std::cin, plain);  // Use getline to read the whole line

    if (!encrypt(plain)) {
        std::cerr << "[Error] Could not perform encryption.\n";
    }

    std::string confirmation;
    std::cout << "Press enter to reveal recovered text: ";
    std::getline(std::cin, confirmation); // Also use getline here if expecting to press enter only

    std::string recovered = decrypt();

    std::cout << recovered << "\n";

    return 0;
}

bool encrypt(std::string plain) {
    // Create key and initialisation vector
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);

    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());

    std::ofstream credentialsFile("credentials.txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "[Error] Failed to open file for credentials.\n";
        return false;
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
        std::cerr << "[Error] " << e.what() << "\n";
        return false;
    }

    std::ofstream cipherFile("cipher.txt");
    if (!cipherFile.is_open()) {
        std::cerr << "[Error ]Failed to open file for cipher.\n";
        return false;
    }

    // Write cipher to file
    CryptoPP::HexEncoder cipherEncoder(new CryptoPP::FileSink(cipherFile));
    cipherEncoder.Put((const CryptoPP::byte*)&cipher[0], cipher.size());
    cipherEncoder.MessageEnd();

    cipherFile.close();

    std::cout << "[Info] Encryption complete.\n";

    return true;
}

std::string decrypt() {
    std::ifstream credentialsFile("credentials.txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "[Error] Failed to open credentials file.\n";
        return "";
    }

    std::string encodedKey, encodedIV;
    std::getline(credentialsFile, encodedKey);
    std::getline(credentialsFile, encodedIV);

    credentialsFile.close();

    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);

    // Decode hexadecimal key and IV
    CryptoPP::HexDecoder keyDecoder;
    keyDecoder.Put((const CryptoPP::byte*) encodedKey.data(), encodedKey.size());
    keyDecoder.MessageEnd();
    keyDecoder.Get(key, key.size());

    CryptoPP::HexDecoder ivDecoder;
    ivDecoder.Put((const CryptoPP::byte*)encodedIV.data(), encodedIV.size());
    ivDecoder.MessageEnd();
    ivDecoder.Get(iv, iv.size());

    // Decrypt using key and IV...
    // Rest of decryption code


    std::ifstream cipherFile("cipher.txt");
    if (!cipherFile.is_open()) {
        std::cerr << "[Error] Failed to open cipher file.\n";
        return "";
    }

    std::string encodedCipher;
    std::getline(cipherFile, encodedCipher);

    cipherFile.close();

    // Decode hexadecimal cipher text
    std::string cipher;
    CryptoPP::HexDecoder cipherDecoder;
    cipherDecoder.Attach(new CryptoPP::StringSink(cipher));  // Attach the StringSink before putting data
    cipherDecoder.Put((const CryptoPP::byte*)encodedCipher.data(), encodedCipher.size());
    cipherDecoder.MessageEnd();

    std::string recovered;

    try
    {
        CryptoPP::EAX< CryptoPP::AES >::Decryption d;
        d.SetKeyWithIV(key, key.size(), iv);

        CryptoPP::StringSource s(cipher, true,
                                 new CryptoPP::AuthenticatedDecryptionFilter(d,
                                                                             new CryptoPP::StringSink(recovered)
                                 ) // AuthenticatedDecryptionFilter
        ); // StringSource
    }
    catch(const CryptoPP::Exception& e)
    {
        std::cerr << "[Error] " << e.what() << "\n";
        return "";
    }

    return recovered;
}
