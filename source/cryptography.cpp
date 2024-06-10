//
// Created by Zhiyang Lu on 26/05/2024.
//

#include "../headers/cryptography.h"

bool encrypt(std::string username, std::string filename, std::string plain) {
    // Create key and initialisation vector
    CryptoPP::AutoSeededRandomPool prng;
    CryptoPP::SecByteBlock key(CryptoPP::AES::DEFAULT_KEYLENGTH);
    CryptoPP::SecByteBlock iv(CryptoPP::AES::BLOCKSIZE);

    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());

    std::ofstream credentialsFile("data/credentials/" + username + "/" + filename + ".txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "Failed to open file for credentials." << std::endl;
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
        std::cerr << e.what() << std::endl;
        return false;
    }

    std::ofstream cipherFile("data/ciphers/" + username + "/" + filename + ".txt");
    if (!cipherFile.is_open()) {
        std::cerr << "Failed to open file for cipher." << std::endl;
        return false;
    }

    // Write cipher to file
    CryptoPP::HexEncoder cipherEncoder(new CryptoPP::FileSink(cipherFile));
    cipherEncoder.Put((const CryptoPP::byte*)&cipher[0], cipher.size());
    cipherEncoder.MessageEnd();

    cipherFile.close();

    std::cout << "Encryption complete\n";

    return true;
}

std::string decrypt(std::string username, std::string filename) {
    std::ifstream credentialsFile("data/credentials/" + username + "/" + filename + ".txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "Failed to open credentials file." << std::endl;
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

    std::ifstream cipherFile("data/ciphers/" + username + "/" + filename + ".txt");
    if (!cipherFile.is_open()) {
        std::cerr << "Failed to open cipher file." << std::endl;
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
        std::cerr << e.what() << std::endl;
        return "";
    }

    return recovered;
}