#include <fstream>
#include <string>
#include "cryptlib.h"
#include "hex.h"
#include "files.h"
#include "osrng.h"
#include "rijndael.h"
#include "modes.h"
#include "eax.h"

int main() {

    std::ifstream credentialsFile("credentials.txt");
    if (!credentialsFile.is_open()) {
        std::cerr << "Failed to open credentials file." << std::endl;
        return 1;
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
        std::cerr << "Failed to open cipher file." << std::endl;
        return 1;
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
        exit(1);
    }

    std::cout << "Recovered Message: " << recovered << "\n";

    return 0;
}
