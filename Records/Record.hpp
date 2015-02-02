
#ifndef RECORD
#define RECORD

#include <cstdint>
#include <botan-1.10/botan/botan.h>
#include <botan-1.10/botan/rsa.h>

typedef std::pair<uint8_t*, size_t> UInt32Data;

class Record
{
    public:
        static const int DIFFICULTY = 4; //1/2^x chance of success, so order of magnitude
        static const uint32_t THRESHOLD = UINT32_MAX / (1 << DIFFICULTY);

        static const uint64_t SCR_N = 1 << 20;
        static const uint32_t SCR_P = 1 << 0;
            //RAM load = O(SCR_N)
            //CPU time = O(SCR_N * SCR_P)

        static const uint8_t NONCE_LEN = 4;
        static const uint8_t SCRYPTED_LEN = 16;

        static const uint32_t RSA_LEN = 1024;
        static const uint32_t SIGNATURE_LEN = RSA_LEN / 8;
        static const uint32_t SHA256_LEN = 256 / 8;
        static const uint32_t SHA1_LEN = 160 / 8;

        bool setKey(Botan::RSA_PrivateKey*);
        UInt32Data getPublicKey() const;
        std::string getOnion() const;

        bool refresh();
        virtual bool makeValid(uint8_t) = 0;
        bool isValid() const;

        virtual std::string asJSON() const = 0;

        enum WorkStatus
        {
            Success, NotFound, Aborted
        };

    protected:
        Record(Botan::RSA_PrivateKey* key, uint8_t*);
        ~Record();

        virtual UInt32Data getCentral(uint8_t* nonce) const = 0;
        WorkStatus mineParallel(uint8_t);
        WorkStatus makeValid(uint8_t, uint8_t, uint8_t*, uint8_t*, uint8_t*);

        size_t signMessageDigest(const uint8_t*, size_t,
            const Botan::Private_Key*, uint8_t*) const;
        int scrypt(const uint8_t*, size_t, uint8_t*) const;

        Botan::RSA_PrivateKey* key_;
        uint8_t consensusHash_[SHA256_LEN];
        uint8_t nonce_[NONCE_LEN];
        uint8_t scrypted_[SCRYPTED_LEN];
        uint8_t signature_[SIGNATURE_LEN];
        long timestamp_;
        bool valid_;
};

#endif





