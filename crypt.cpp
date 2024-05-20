#include "cryptlib.h"
#include "rijndael.h"
#include "modes.h"
#include "files.h"
#include "osrng.h"
#include "hex.h"
#include "eax.h"

#include <iostream>
#include <string>

int main(int argc, char* argv[])
{
    using namespace CryptoPP;

    AutoSeededRandomPool prng;
    HexEncoder encoder(new FileSink(std::cout));

    SecByteBlock key(AES::DEFAULT_KEYLENGTH);
    SecByteBlock iv(AES::BLOCKSIZE);

    prng.GenerateBlock(key, key.size());
    prng.GenerateBlock(iv, iv.size());

    std::string plain = "This is a testing message";
    std::string cipher, recovered;

    std::cout << "plain text: " << plain << std::endl;

    /*********************************\
    \*********************************/

    try
    {
        EAX< AES >::Encryption e;
        e.SetKeyWithIV(key, key.size(), iv);

        StringSource(plain, true,
                     new AuthenticatedEncryptionFilter(e,
                                                       new StringSink(cipher)
                     ) // AuthenticatedEncryptionFilter
        ); // StringSource
    }
    catch(const Exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    /*********************************\
    \*********************************/

    std::cout << "key: ";
    encoder.Put(key, key.size());
    encoder.MessageEnd();
    std::cout << std::endl;

    std::cout << "iv: ";
    encoder.Put(iv, iv.size());
    encoder.MessageEnd();
    std::cout << std::endl;

    std::cout << "cipher text: ";
    encoder.Put((const byte*)&cipher[0], cipher.size());
    encoder.MessageEnd();
    std::cout << std::endl;

    /*********************************\
    \*********************************/

    try
    {
        EAX< AES >::Decryption d;
        d.SetKeyWithIV(key, key.size(), iv);

        StringSource s(cipher, true,
                       new AuthenticatedDecryptionFilter(d,
                                                         new StringSink(recovered)
                       ) // AuthenticatedDecryptionFilter
        ); // StringSource

        std::cout << "recovered text: " << recovered << std::endl;
    }
    catch(const Exception& e)
    {
        std::cerr << e.what() << std::endl;
        exit(1);
    }

    return 0;
}