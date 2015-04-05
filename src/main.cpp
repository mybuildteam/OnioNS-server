

#include "Records/Registration.hpp"
#include "main.hpp"
#include <botan/pem.h>
#include <botan/ber_dec.h>
#include <botan/sha2_32.h>
#include <iostream>

Botan::LibraryInitializer init("thread_safe");

int main(int argc, char** argv)
{
    try
    {
        Botan::AutoSeeded_RNG rng;
        Botan::RSA_PrivateKey* rsaKey = loadKey("assets/example.key", rng);
        if (rsaKey != NULL)
            std::cout << "RSA private key loaded successfully!" << std::endl;

        Botan::SHA_256 sha;
        auto hash = sha.process("hello world");

        uint8_t cHash[32];
        memcpy(cHash, hash, 32);

        Registration d(rsaKey, cHash, "example.tor", "AD97364FC20BEC80");

        std::cout << std::endl;
        std::cout << "Initial JSON: " << d.asJSON() << std::endl;

        d.makeValid(4);

        std::cout << std::endl;
        std::cout << "Result:" << std::endl;
        std::cout << d << std::endl;

        std::cout << std::endl;
        std::cout << "Final JSON: " << d.asJSON() << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << e.what() << "\n";
    }

    return EXIT_SUCCESS;
}



Botan::RSA_PrivateKey* loadKey(const char* filename, Botan::RandomNumberGenerator& rng)
{
    try
    {
        //attempt reading key as standardized PKCS8 format
        std::cout << "Opening " << filename << " as PKCS8..." << std::endl;
        auto pvtKey = Botan::PKCS8::load_key(filename, rng);
        auto rsaKey = dynamic_cast<Botan::RSA_PrivateKey*>(pvtKey);
        if (!rsaKey)
            throw std::invalid_argument("The loaded key is not a RSA key!");
        return rsaKey;
    }
    catch (Botan::Decoding_Error& de)
    {
        std::cerr << de.what() << std::endl;
        std::cerr << "  " << filename << " may not be PKCS8-formatted, and likely has been generated by OpenSSL." << std::endl;
        //std::cerr << "  \"openssl pkcs8 -topk8 -nocrypt -in " << filename << "\" will convert the key into PKCS8." << std::endl;

        //if PKCS8 decoding fails, try manual decoding of OpenSSL's format
        std::cout << "Attempting manual decoding..." << std::endl;
        return loadOpenSSLRSA(filename, rng);
    }

    return NULL;
}



//http://botan.randombit.net/faq.html#how-do-i-load-this-key-generated-by-openssl-into-botan
//http://lists.randombit.net/pipermail/botan-devel/2010-June/001157.html
//http://lists.randombit.net/pipermail/botan-devel/attachments/20100611/1d8d870a/attachment.cpp
Botan::RSA_PrivateKey* loadOpenSSLRSA(const char* filename, Botan::RandomNumberGenerator& rng)
{
    Botan::DataSource_Stream in(filename);

    Botan::DataSource_Memory key_bits(
        Botan::PEM_Code::decode_check_label(in, "RSA PRIVATE KEY"));

    //Botan::u32bit version;
    size_t version;
    Botan::BigInt n, e, d, p, q;

    Botan::BER_Decoder(key_bits)
        .start_cons(Botan::SEQUENCE)
        .decode(version)
        .decode(n).decode(e).decode(d).decode(p).decode(q);

    if(version != 0)
      return NULL;

    return new Botan::RSA_PrivateKey(rng, p, q, e, d, n);
}