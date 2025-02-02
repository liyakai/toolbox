#pragma once

namespace CoroRpc {

template<typename T>
struct GetTypeTraits{
    using type = T;
};

template<typename T>
struct GetTypeTraits<std::shared_ptr<T>>{
    using type = T;
};




class Context{
public:
    Context(uint64_t client_id, uint64_t server_id, uint64_t request_id)
    : client_id_(client_id), server_id_(server_id), request_id_(request_id) {}
    
};

} // namespace CoroRpc