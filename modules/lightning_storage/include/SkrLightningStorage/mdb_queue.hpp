#pragma once
#include "SkrRT/misc/types.h"
#include "SkrRT/platform/thread.h"
#include "SkrRT/containers_new/sptr.hpp"
#include "SkrRT/containers_new/umap.hpp"
#include "SkrRT/containers_new/string.hpp"
#include "SkrLightningStorage/mdb.h"

namespace skr {
namespace mdbq {

struct Topic;

struct EnvironmentDesc {
    uint64_t max_topic_num SKR_IF_CPP(= 0);
    uint64_t map_size SKR_IF_CPP(= 0);
};

struct TopicDesc {
    uint64_t chunk_size SKR_IF_CPP(= 0);
    uint64_t chunks_to_keep SKR_IF_CPP(= 0);
};

struct TopicStatus{
    uint64_t producer_head;
    skr::UMap<skr::string, uint64_t> consumer_heads;
};

struct Environment {
public:
    ~Environment();

    const skr::string& get_root() { return _root; }
    skr::mdb::EnvironmentId get_mdb_env() { return _env; }

    Topic* get_topic(const skr::string& name);

private:
    friend struct EnvironmentManager;
    friend struct Transaction;

    Environment(const skr::string& root, EnvironmentDesc* desc);
    Environment(const Environment&);
    Environment& operator=(const Environment&);

    using TopicPtr = skr::SPtr<Topic>;
    using TopicMap = skr::UMap<skr::string, TopicPtr>;

    skr::string _root;
    skr::mdb::EnvironmentId _env;
    SMutex _mtx;
    TopicMap _topics;
};

struct EnvironmentManager {
public:
    static Environment* GetEnv(const skr::string& root, EnvironmentDesc* desc = nullptr);

private:
    using EnvPtr = skr::SPtr<Environment>;
    using EnvMap = skr::UMap<skr::string, EnvPtr>;
    SMutex _mtx;
    EnvMap _envMap;
};

struct Transaction {
public:
    Transaction(Environment* env, skr::mdb::EnvironmentId consumerOrProducerEnv, bool readOnly = false) SKR_NOEXCEPT
        : _abort(false), _envTxn(nullptr), _cpTxn(nullptr) 
    {
        _envTxn = env->_env->open_transaction(nullptr, 0);
        if (consumerOrProducerEnv)
        {
            _cpTxn = consumerOrProducerEnv->open_transaction(nullptr, 0);
        } 
    }

    ~Transaction() SKR_NOEXCEPT
    {
        if (_cpTxn) 
            skr_lightning_transaction_abort(_cpTxn);
        if (_envTxn) 
            skr_lightning_transaction_abort(_envTxn);
    }

public:
    inline skr::mdb::TransactionId getEnvTxn() SKR_NOEXCEPT { return _envTxn; }
    inline skr::mdb::TransactionId getTxn() SKR_NOEXCEPT { return _cpTxn; }

    void abort() SKR_NOEXCEPT
    {
        if (_cpTxn)
            skr_lightning_transaction_abort(_cpTxn);
        skr_lightning_transaction_abort(_envTxn);

        _cpTxn = _envTxn = nullptr;
    }

    bool commit() SKR_NOEXCEPT
    {
        if (_cpTxn) 
        {
            if (!skr_lightning_transaction_commit(_cpTxn))
            {
                skr_lightning_transaction_abort(_envTxn);
                _cpTxn = _envTxn = nullptr;
                return false;
            }
        }
        if (!skr_lightning_transaction_commit(_envTxn))
        {
            _cpTxn = _envTxn = nullptr;
            return false;
        }
        return true;
    }

private:
    Transaction(const Transaction&);
    Transaction& operator=(const Transaction&);

private:
    bool _abort;
    skr::mdb::TransactionId _envTxn, _cpTxn;
};

template<typename INT_TYPE> 
int mdbIntCmp(const SLightningStorageValue* a, const SLightningStorageValue* b) 
{
    INT_TYPE ia = *(INT_TYPE*)a->data;
    INT_TYPE ib = *(INT_TYPE*)b->data;
    return ia < ib ? -1 : ia > ib ? 1 : 0;
}

} // namespace mdbq
} // namespace skr