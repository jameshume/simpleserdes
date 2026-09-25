#include <cstdint> 
#include <tuple> 
#include <type_traits> 


template <typename T> 
concept Serialisable = requires(T& o1, const T& o2) { 
    o1.serialisable_members(); 
    o2.serialisable_members(); 
}; 

template <typename Serdes, typename T>
bool deserialise(Serdes& reader, T& object) {
    return std::apply(
        [&](auto&... fields) {
            return (Serdes.read(fields) && ...);
        },
        object.serialisable_members());
}

template <typename Reader, typename T>
bool serialise(Serdes& writer, T& object) {
    return std::apply(
        [&](const auto&... fields) {
            return (Serdes.write(fields) && ...);
        },
        object.serialisable_members());
}


struct SerdesBufferReader {
    SerdesReader(std::span<const uint8_t> _buffer_view) : buffer_view(_buffer_view) {}

    template <typename T>
    bool read(T &value) {
        if constexpr(std::is_trivially_copyable_v<T>) {
            if (buffer_view.size() < sizeof(T)) {
                return false;
            }

            memcpy(&value, buffer.data(), sizeof(T));
            buffer = buffer.offset(sizeof(T));
            return true;
        }
        else if constexpr (std::is_class_v<T>) {
            return deserialise(*this, value);
        }
        else {
            return false;
        }
    }


private:
    std::span<const std::uint8_t> buffer_view;    
};

struct SerdesBufferWriter {
    SerdesReader(std::span<uint8_t> _buffer_view) : buffer_view(_buffer_view) {}

    template <typename T>
    bool write(T &value) {
        if constexpr(std::is_trivially_copyable_v<T>) {
            if (buffer_view.size() < sizeof(T)) {
                return false;
            }

            memcpy(buffer.data(), &value, sizeof(T));
            buffer = buffer.offset(sizeof(T));
            return true;
        }
        else if constexpr (std::is_class_v<T>) {
            return deserialise(*this, value);
        }
        else {
            return false;
        }
    }


private:
    std::span<std::uint8_t> buffer_view;    
};


struct Arse { 
    std::uint8_t a; 
    bool b; 
    auto serialisable_members() { 
        return std::tie(a, b); 
    } 
    auto serialisable_members() const { 
        return std::tie(a, b); 
    } 
}; 
    
struct Message { 
    std::uint32_t node_id; 
    std::int32_t temperature; 
    Arse flags; 
    auto serialisable_members() { 
        return std::tie(node_id, temperature, flags); 
    } 
    auto serialisable_members() const { 
        return std::tie(node_id, temperature, flags); 
    } 
}; 


struct SerdesStream  { 
    bool read(std::uint32_t &val) { 
        val = 1234; 
        return true; 
    } 
    
    bool read(std::uint8_t &val) { 
        val = 34; return true; 
    } 
    
    bool read(std::int32_t &val) { 
        val = 4321; 
        return true; 
    } 
    
    template<Serialisable T> bool read(T &t) {
        return deserialise(*this, t); 
    } 
}; 



int main() { 
    Message test; 
    MyReader reader; 
    deserialise(reader, test); 
    return 0; 
}

